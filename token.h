#pragma once
#ifndef TOKEN_H
#define TOKEN_H
#include <string>


enum class tokenTypes {
	GROUPING_,
	KEYWORD,
	IFSTATEMENT,
	DATATYPE,
	IDENTIFIER,
	OPERATOR,
	DELIMITER,
	SEMICOLON,
	CONDITIONAL,
	RETURN,
	_NULL
};

struct Token {
	tokenTypes type = tokenTypes::_NULL;
	std::string value;
	size_t line;
	size_t column;
};


#endif
