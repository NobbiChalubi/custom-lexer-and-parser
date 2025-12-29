#pragma once
#ifndef PARSER_H
#define PARSER_H
#include <fstream>
#include <variant>
#include <vector>
#include "token.h"

struct ASTNode;

enum class statements {
	DECLARATION,// int x = 5;
	CONDITIONAL,// if(){}
	EXPRESSION,//x = 10;
	BLOCK,//block inside a function
	FUNCTION,//bool isFun(){return true;}
	ROOT//special statement declaring root node
};

struct TreeNode {
	Token obj;
	std::unique_ptr<ASTNode>  functionObj;
	std::unique_ptr <TreeNode> left;
	std::unique_ptr<TreeNode> right;
	TreeNode() : obj(Token{}), left(nullptr), right(nullptr) {}
	TreeNode(Token input) : obj(std::move(input)), left(nullptr), right(nullptr) {}
	TreeNode(Token input, std::unique_ptr<TreeNode>&& left, std::unique_ptr<TreeNode>&& right) : obj(std::move(input)), left(std::move(left)), right(std::move(right)) {}
	TreeNode(Token input, std::unique_ptr <TreeNode>&& left, char c) : obj(std::move(input)), left(std::move(left)), right(nullptr) {}
	TreeNode(Token input, char c, std::unique_ptr<TreeNode>&& right) : obj(std::move(input)), left(nullptr), right(std::move(right)) {}
};

struct ASTNode {
	statements type;//Expression, declaration, block, function etc.
	std::unique_ptr<TreeNode> expression;
	std::vector<std::unique_ptr<ASTNode>> arguments;//For function arguments.
	std::vector<std::unique_ptr<ASTNode>> children;//For body statements.
	std::vector<std::unique_ptr<ASTNode>> returnNodes;//For body statements.
	Token dataType;//Function data type.
	Token functionIdentifier;
	ASTNode() = default;
	ASTNode(statements typeInput) : type(std::move(typeInput)) {}
	ASTNode(statements typeInput, std::unique_ptr<TreeNode>&& inputNode) : type(std::move(typeInput)), expression(std::move(inputNode)) {}

};





ASTNode ParseFile(const std::vector<Token>& tokens);

#endif
