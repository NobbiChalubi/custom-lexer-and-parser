#include "token.h"
#include "lexer.h"
#include "Parser.h"
#include <fstream>
#include <vector>
#include <stdexcept>
#include <string>
#include <unordered_map>
#include <stack>
#include <utility>

void iterate(int& index, const std::vector<Token>& inputVector) {
	if (index + 1 >= inputVector.size()) { throw std::runtime_error("Unexpected end on line: "
		+ std::to_string(inputVector[index].line) + " column: " + std::to_string(inputVector[index].column)); }
	++index;
}

const std::unordered_map<std::string, std::string>& complimentMap() {
	const static std::unordered_map<std::string, std::string> compliment = {
		{"[","]"},
		{"(",")"},
		{"{","}"}
	};
	return compliment;
}

std::unique_ptr<ASTNode> parseIdentifier(int& index, const std::vector<Token>& tokens, bool dataTypeFlag);
std::unique_ptr<ASTNode> parseIfStatement(const std::vector<Token>& tokens, int& index);
std::unique_ptr<ASTNode> parseOperator(std::unique_ptr <TreeNode>& Node, const std::vector<Token>& tokens, int& index);
std::pair<std::vector<std::unique_ptr<ASTNode>>, std::vector<std::unique_ptr<ASTNode>>> parseBody(int& index, const std::vector<Token>& tokens, bool conditionalFlag);
std::unique_ptr<ASTNode> parseIdentifier(int& index, const std::vector<Token>& tokens, bool dataTypeFlag);


std::unique_ptr <ASTNode> parseFunction(const std::vector<Token>& tokens, int& index, bool declarationFlag) {
	std::string opening = tokens[index].value;
	const auto& compliment = complimentMap();
	std::vector<std::unique_ptr<ASTNode>> arguments;
	std::vector<std::unique_ptr<ASTNode>> children;
	std::unique_ptr <TreeNode> tempNode = nullptr;

	std::unique_ptr <ASTNode> parsedFunction = std::make_unique<ASTNode>( statements::EXPRESSION );
	parsedFunction->functionIdentifier = tokens[index - 1];//Should be on the first grouping so one before the grouping should be the identifier.
	if (declarationFlag) { parsedFunction->dataType = tokens[index - 2]; }
	iterate(index, tokens);
	while (tokens[index].value != compliment.at(opening)) {//Make sure it ends.
		if (declarationFlag && tokens[index].type == tokenTypes::DATATYPE) {
			iterate(index, tokens);
			if (tokens[index].type != tokenTypes::IDENTIFIER) { throw std::runtime_error("Expected identifier on line: " + std::to_string(tokens[index].line) + " column: " + std::to_string(tokens[index].column)); }
			arguments.push_back(parseIdentifier(index, tokens, true));
			continue;
		}
		
		else if (!declarationFlag && tokens[index].type == tokenTypes::IDENTIFIER) {
			arguments.push_back(parseIdentifier(index, tokens, false));
			continue;
		}
		else if (tokens[index].value == ",") {
			iterate(index, tokens);
			continue;
		}
		else {
			throw std::runtime_error("Unexpected syntax on line: " + std::to_string(tokens[index].line) + " column: " + std::to_string(tokens[index].column));
			continue;
		}
	}
	iterate(index, tokens);
	if (declarationFlag && tokens[index].value == "{") {
		auto parsedBody = parseBody(index, tokens, false );
		parsedFunction->children = std::move(parsedBody.first);
		parsedFunction->returnNodes = std::move(parsedBody.second);
		if(index<tokens.size()-1) iterate(index, tokens);
	}
	else if (declarationFlag && tokens[index].value != "{" ) {
		throw std::runtime_error("Expected '{' on line: " + std::to_string(tokens[index].line) + " column: " + std::to_string(tokens[index].column));
	}
	parsedFunction->arguments = std::move(arguments);
	return parsedFunction;
}

std::unique_ptr<ASTNode> parseIfStatement(const std::vector<Token>& tokens, int& index) {
	iterate(index, tokens);
	std::string openingGrouping = tokens[index].value;
	const auto& compliment = complimentMap();
	if (compliment.find(openingGrouping) == compliment.end()) {
		throw std::runtime_error("Wrong grouping on line: " + std::to_string(tokens[index].line) + " column: " + std::to_string(tokens[index].column));
	}

	std::vector<std::unique_ptr<ASTNode>> arguments;
	std::vector<std::unique_ptr<ASTNode>> children;
	std::unique_ptr <TreeNode> tempNode = nullptr;
	std::unique_ptr <ASTNode> parsedConditional = std::make_unique<ASTNode>( statements::CONDITIONAL, std::make_unique<TreeNode>(tokens[index-1]));


	int semiColonCount = 0;

	if (tokens[index - 1].value != "else") {
		iterate(index, tokens);
		while (tokens[index].value != compliment.at(openingGrouping)) {
			if (parsedConditional->expression->obj.value == "for") {
				if (tokens[index].type == tokenTypes::DATATYPE) {
					iterate(index, tokens);
					if (tokens[index].type != tokenTypes::IDENTIFIER) { throw std::runtime_error("Expected identifier on line: " + std::to_string(tokens[index].line) + " column: " + std::to_string(tokens[index].column)); }
					arguments.push_back(parseIdentifier(index, tokens, true));
					continue;
				}
				else if (tokens[index].type == tokenTypes::SEMICOLON) {
					semiColonCount++;
					if (semiColonCount <= 2) {
						iterate(index, tokens);
						continue;
					}
					else {
						throw std::runtime_error("Expected identifier on line: " + std::to_string(tokens[index].line) + " column: " + std::to_string(tokens[index].column));
					}
				}
			}
			if (tokens[index].type == tokenTypes::GROUPING_) {
				throw std::runtime_error("Expected grouping on line: " + std::to_string(tokens[index].line) + " column: " + std::to_string(tokens[index].column));
				continue;
			}
			else if (tokens[index].type == tokenTypes::IDENTIFIER) {
				arguments.push_back(parseIdentifier(index, tokens, false));
				continue;

			}
			else if (tokens[index].type == tokenTypes::DELIMITER) {
				iterate(index, tokens);
				continue;
			}
			else {
				throw std::runtime_error("Unexpected syntax on line: " + std::to_string(tokens[index].line) + " column: " + std::to_string(tokens[index].column));
				continue;
			}
		}
		iterate(index, tokens);
	}
	if (tokens[index].value == "{") {
		auto parsedBody = parseBody(index, tokens, false);
		parsedConditional->children = std::move(parsedBody.first);
		parsedConditional->returnNodes = std::move(parsedBody.second);
		if (index < tokens.size()-1) iterate(index, tokens);
	}
	else {
		throw std::runtime_error("Expected '{' on line: " + std::to_string(tokens[index].line) + " column: " + std::to_string(tokens[index].column));
	}
	parsedConditional->arguments = std::move(arguments);
	return parsedConditional;
}

std::unique_ptr<ASTNode> parseOperator(std::unique_ptr <TreeNode>& Node, const std::vector<Token>& tokens, int& index) {
	TreeNode* tempNode = Node.get();
	std::stack<std::string> stack;
	const auto& compliment = complimentMap();
	std::unique_ptr<ASTNode> parsedOperator = std::make_unique<ASTNode>( statements::EXPRESSION,std::move( Node ));

	if (tokens[index].type == tokenTypes::DATATYPE) {
		iterate(index, tokens);
	}

	while (true) {
		if (tokens[index].type == tokenTypes::_NULL) {
			iterate(index, tokens);
			continue;
		}
		if (tokens[index].type == tokenTypes::DELIMITER || tokens[index].type == tokenTypes::SEMICOLON) {
			if (stack.empty()) {
				iterate(index, tokens);
				return parsedOperator;
			}
			else if (tokens[index].value == ".") {
				iterate(index, tokens);
				continue;
			}
			else {
				throw std::runtime_error("Unexpected ending on line: " + std::to_string(tokens[index - 1].line) + " column: " + std::to_string(tokens[index - 1].column));
			}
		}
		if (tempNode->obj.type == tokenTypes::DATATYPE) {
			if (tempNode->right) {
				tempNode = tempNode->right.get();
				continue;
			}
			else {
				throw std::runtime_error("Unexpected operator on line: " + std::to_string(tokens[index - 1].line) + " column: " + std::to_string(tokens[index - 1].column));
			}
		}
		else if (tempNode->obj.type == tokenTypes::OPERATOR) {
			if (tempNode->left && tempNode->right) {
				throw std::runtime_error("Unexpected error on line: " + std::to_string(tokens[index - 1].line) + " column: " + std::to_string(tokens[index - 1].column));
			}
			else if (tokens[index].type == tokenTypes::IDENTIFIER) {
				iterate(index, tokens);
				if (tokens[index].value == ".") {
					iterate(index, tokens);
					iterate(index, tokens);
					if (tokens[index].type == tokenTypes::OPERATOR) {
						tempNode->right = std::make_unique<TreeNode>(tokens[index]);
						tempNode->right->left = std::make_unique<TreeNode>(tokens[index - 3]);
						tempNode->right->left->obj.value += "." + tokens[index - 1].value;
						tempNode = tempNode->right.get();
						iterate(index, tokens);
					}
					else {
						--index;
						if (tempNode->left) {
							tempNode->right = std::make_unique<TreeNode>(tokens[index - 2]);
							tempNode->right->obj.value += "." + tokens[index].value;
							tempNode = tempNode->right.get();
						}
						else {
							tempNode->left = std::make_unique<TreeNode>(tokens[index - 2]);
							tempNode->left->obj.value += "." + tokens[index].value;
						}
						iterate(index, tokens);
					}
				}
				else if (tokens[index].type == tokenTypes::OPERATOR) {
					tempNode->right = std::make_unique<TreeNode>(tokens[index]);
					tempNode->right->left = std::make_unique<TreeNode>(tokens[index - 1]);
					tempNode = tempNode->right.get();
					iterate(index, tokens);
				}
				else if (tokens[index].type == tokenTypes::DELIMITER || tokens[index].type == tokenTypes::SEMICOLON) {
					tempNode->right = std::make_unique<TreeNode>(tokens[index - 1]);
					tempNode = tempNode->right.get();
					continue;
				}
				else if (tokens[index].type == tokenTypes::GROUPING_) {
					if (compliment.find(tokens[index].value) != compliment.end()) {
						tempNode->right = std::make_unique<TreeNode>();
						tempNode->right->functionObj = parseFunction(tokens, index, false);
						return parsedOperator;
					}
					else {
						if (stack.empty()) {
							return parsedOperator;
						}
						tempNode->right = std::make_unique<TreeNode>(tokens[index - 1]);
						tempNode = tempNode->right.get();
					}
				}
				else {
					tempNode->left = std::make_unique<TreeNode>(tokens[index]);
					iterate(index, tokens);
				}
				continue;
			}
			else if (tokens[index].type == tokenTypes::GROUPING_) {
				if (compliment.find(tokens[index].value) == compliment.end()) {
					if (stack.empty()) {
						return parsedOperator;
					}else {
						stack.pop();
						tempNode->right = std::make_unique<TreeNode>(tokens[index]);
						tempNode = tempNode->right.get();
						iterate(index, tokens);
						continue;
					}
				}
				else {
					if (tempNode->left) {
						tempNode->right = std::make_unique<TreeNode>(tokens[index]);
						tempNode->right = std::move(parseOperator(tempNode->right, tokens, index)->expression);
						while(tempNode->right) tempNode = tempNode->right.get();
					}
					else {
						tempNode->left = std::make_unique<TreeNode>(tokens[index]);
						tempNode->left = std::move(parseOperator(tempNode->left, tokens, index)->expression);
					}
				}
				iterate(index, tokens);
				continue;
			}
			else {
				throw std::runtime_error("Unexpected on line: " + std::to_string(tokens[index].line) + " column: " + std::to_string(tokens[index].column));
			}
		}

		else if (tempNode->obj.type == tokenTypes::GROUPING_) {
			if (compliment.find(tokens[index].value) != compliment.end()) {
				stack.push(tempNode->obj.value);
			}
			else {
				if (stack.empty()) {
					return parsedOperator;
				}
				else {
					stack.pop();
					tempNode->right = std::make_unique<TreeNode>(tokens[index]);
					tempNode = tempNode->right.get();
					iterate(index, tokens);
					continue;
				}
			}
			iterate(index, tokens);
			if (tokens[index].type == tokenTypes::IDENTIFIER) {
				iterate(index, tokens);
				if (tokens[index].type == tokenTypes::OPERATOR) {
					tempNode->right = std::make_unique<TreeNode>(tokens[index]);
					tempNode->right->left = std::make_unique<TreeNode>(tokens[index - 1]);
					tempNode = tempNode->right.get();
					iterate(index, tokens);
				}
				else if (tokens[index].value == ".") {
					iterate(index, tokens);
					iterate(index, tokens);
					if (tokens[index].type == tokenTypes::OPERATOR) {
						tempNode->right = std::make_unique<TreeNode>(tokens[index]);
						tempNode->right->left = std::make_unique<TreeNode>(tokens[index - 3]);
						tempNode->right->left->obj.value += "." + tokens[index - 1].value;
						tempNode = tempNode->right.get();
						iterate(index, tokens);
					}
					else {
						--index;
						tempNode->right = std::make_unique<TreeNode>(tokens[index-2]);
						tempNode->right->obj.value += "." + tokens[index].value;
						tempNode = tempNode->right.get();
						iterate(index, tokens);
					}
				}
				else {
					tempNode->right = std::make_unique<TreeNode>(tokens[index-1]);
					tempNode = tempNode->right.get();
				}
				continue;
			}
			else {
				throw std::runtime_error("Expected identifier on line: " + std::to_string(tokens[index].line) + " column: " + std::to_string(tokens[index].column));
			}
		}

		else if (tokens[index].type == tokenTypes::GROUPING_) {
			if (compliment.find(tokens[index].value) == compliment.end()) {
				stack.pop();
				tempNode->right = std::make_unique<TreeNode>(tokens[index]);
				if (stack.empty()) {
					return parsedOperator;
				}
				tempNode = tempNode->right.get();
				iterate(index, tokens);
				continue;
			}
			else {
				if (tempNode->left) {
					tempNode->right = std::make_unique<TreeNode>(tokens[index]);
					parseOperator(tempNode->right, tokens, index);
				}
				else {
					tempNode->left = std::make_unique<TreeNode>(tokens[index]);
					parseOperator(tempNode->left, tokens, index);
				}
			}
		}
		else {
			throw std::runtime_error("Unexpected on line: " + std::to_string(tokens[index].line) + " column: " + std::to_string(tokens[index].column));
		}
	}
}


std::pair<std::vector<std::unique_ptr<ASTNode>>, std::vector<std::unique_ptr<ASTNode>>> parseBody(int& index, const std::vector<Token>& tokens, bool conditionalFlag){
	const auto& compliment = complimentMap();
	std::vector<std::unique_ptr<ASTNode>> returnNodes;
	std::vector<std::unique_ptr<ASTNode>> children;


	iterate(index, tokens);
	while (tokens[index].value != "}") {
		if (tokens[index].type == tokenTypes::DATATYPE) {
			iterate(index, tokens);
			if (tokens[index].type != tokenTypes::IDENTIFIER) { throw std::runtime_error("Expected identifier on line: " + std::to_string(tokens[index].line) + " column: " + std::to_string(tokens[index].column)); }
			children.push_back(parseIdentifier(index, tokens, true));
			continue;
		}
		else if (tokens[index].type == tokenTypes::IDENTIFIER) {
			children.push_back(parseIdentifier(index, tokens, false));
			continue;
		}
		else if (conditionalFlag && tokens[index].type == tokenTypes::KEYWORD) {
			std::unique_ptr<ASTNode> keyword = std::make_unique<ASTNode>( statements::EXPRESSION, std::make_unique<TreeNode>(tokens[index]));
			children.push_back(std::move(keyword));
			iterate(index, tokens);
			continue;
		}
		else if (tokens[index].type == tokenTypes::IFSTATEMENT) {
			children.push_back(parseIfStatement(tokens, index));
			continue;
		}
		else if (tokens[index].type == tokenTypes::DELIMITER) {
			iterate(index, tokens);
			continue;
		}
		else if (tokens[index].type == tokenTypes::SEMICOLON) {
			iterate(index, tokens);
			continue;
		}
		else if (tokens[index].type == tokenTypes::RETURN) {
			iterate(index, tokens);
			if (tokens[index].type == tokenTypes::SEMICOLON) {
				std::unique_ptr<ASTNode> voidReturn = std::make_unique<ASTNode>( statements::EXPRESSION, std::make_unique<TreeNode>(tokens[index - 1]) );
				returnNodes.push_back(std::move(voidReturn));
				iterate(index, tokens);
				continue;
			}
			else if (tokens[index].type == tokenTypes::IDENTIFIER) {
				returnNodes.push_back(parseIdentifier(index, tokens, false));
				continue;
			}
			else {
				throw std::runtime_error("Unexpected syntax on line: " + std::to_string(tokens[index].line) + " column: " + std::to_string(tokens[index].column));
			}
		}
		else {
			throw std::runtime_error("Unexpected syntax on line: " + std::to_string(tokens[index].line) + " column: " + std::to_string(tokens[index].column));
		}
	}
	return { std::move(children), std::move(returnNodes)};
}

std::unique_ptr<ASTNode> parseIdentifier(int& index, const std::vector<Token>& tokens, bool dataTypeFlag) {
	std::unique_ptr<TreeNode> currentNode = nullptr;
	const auto& compliment = complimentMap();

	//Inside identifier so iterate.
	iterate(index, tokens);
	if (tokens[index].type == tokenTypes::OPERATOR) {
		if (dataTypeFlag) {
			currentNode = std::make_unique<TreeNode>(tokens[index - 2]);//Data Type
			currentNode->right = std::make_unique<TreeNode>(tokens[index]);//Operator
			currentNode->right->left = std::make_unique<TreeNode>(tokens[index - 1]);//Identifier
			dataTypeFlag = false;
			iterate(index, tokens);
			return parseOperator(currentNode, tokens, index);
		}
		else {
			currentNode = std::make_unique<TreeNode>(tokens[index]);//Operator
			currentNode->left = std::make_unique<TreeNode>(tokens[index - 1]);//Identifier
			iterate(index, tokens);
			return parseOperator(currentNode, tokens, index);
		}
	}
	else if (tokens[index].type == tokenTypes::GROUPING_) {
		if (compliment.find(tokens[index].value) != compliment.end()) {
			return parseFunction(tokens, index, dataTypeFlag);
		}
		else {
			if (dataTypeFlag) {
				currentNode = std::make_unique<TreeNode>(tokens[index - 2]);//Data Type
				currentNode->right = std::make_unique<TreeNode>(tokens[index - 1]);//Identifier
				std::unique_ptr<ASTNode> parsedExpression = std::make_unique<ASTNode>(statements::EXPRESSION, std::move(currentNode));
				return parsedExpression;
			}
			else {
				currentNode = std::make_unique<TreeNode>(tokens[index - 1]);//Identifier
				std::unique_ptr<ASTNode> parsedExpression = std::make_unique<ASTNode>(statements::EXPRESSION, std::move(currentNode));
				return parsedExpression;
			}
		}
	}
	else if(!dataTypeFlag && (tokens[index].type == tokenTypes::DELIMITER || tokens[index].type == tokenTypes::SEMICOLON)) {
		std::unique_ptr <ASTNode> parsedIdentifier = std::make_unique<ASTNode>( statements::EXPRESSION, std::make_unique<TreeNode>(tokens[index-1]) );
		return parsedIdentifier;
	}
	else {
		throw std::runtime_error("Expected operator on line : " + std::to_string(tokens[index].line) + " column : " + std::to_string(tokens[index].column));
	}
}


ASTNode ParseFile(const std::vector<Token>& tokens) {
	ASTNode root = { statements::ROOT };
	if (tokens.empty()) return root;

	const auto& compliment = complimentMap();
	std::vector<std::unique_ptr<ASTNode>> childrenNodes;	
	int index = 0;

	while (index < tokens.size()-1) {
		if (tokens[index].type == tokenTypes::DATATYPE) {
			iterate(index, tokens);
			if (tokens[index].type == tokenTypes::IDENTIFIER) {
				childrenNodes.push_back(parseIdentifier(index, tokens, true));
				continue;
			}
			else {
				throw std::runtime_error("Expected identifier on line: " + std::to_string(tokens[index].line) + " column: " + std::to_string(tokens[index].column));
			}
		}
		else if (tokens[index].type == tokenTypes::IDENTIFIER) {
			childrenNodes.push_back(parseIdentifier(index, tokens, false));
			continue;
		}
		else if (tokens[index].type == tokenTypes::IFSTATEMENT) {
			childrenNodes.push_back(parseIfStatement(tokens, index));
			continue;
		}
		else if (tokens[index].type == tokenTypes::SEMICOLON) {
			iterate(index, tokens);
			continue;
		}
		else if (tokens[index].type == tokenTypes::_NULL) {
			if (tokens[index].value != "/*") {
				iterate(index, tokens);
			}
			else {
				while (tokens[index].value != "*/") {
					iterate(index, tokens);
				}
				iterate(index, tokens);
			}
			continue;
			}
		else {
			throw std::runtime_error("Unexpected syntax on line: " + std::to_string(tokens[index].line) + " column: " + std::to_string(tokens[index].column));
		}
	}
	root.children = std::move(childrenNodes);
	return root;
}
