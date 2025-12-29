#include "token.h"
#include <fstream>
#include <vector>
#include <unordered_map>
#include <stdexcept>
#include <unordered_set>

/*Get the tokens this is called lexical analysis or tokenization, then the parser part will identify if the tokens makes sense*/

bool isWhitespace(const char character) {
	return strchr("\n \v\r\f\r\t", character) != nullptr;
}

bool isOperator(const char character) {//Don't pass by reference for a char, it's one byte and pass by reference uses a pointer which on 64 bit architecture is 8 bytes.
	return strchr("+=-*/&^%<>|!", character) != nullptr;
}
bool isGrouping(const char character) {
	return strchr("(){}[]'\"", character) != nullptr;
}
bool isDelimiter(const char character) {
	return strchr(";:,./", character) != nullptr;
}

Token makeToken(const std::string& input, tokenTypes inputType) {
	Token currentToken =
	{
		{inputType},
		{input}//Don't do currentToken.value = input because you are trying to read garbage(before the equal sign it has to read) before its initialized, you must initialize first.
	};
	return currentToken;
}

std::vector<Token> Tokenize(const std::string& fileName) {
	std::ifstream file(fileName, std::ios::binary);//Also opens after you declare it like (file.open("path")).
	if (!file.is_open()) { throw std::exception("File could not open!"); }
	const static std::unordered_set<std::string> doubleOperators = {
		{">="},
		{"<="},
		{"||"},
		{"&&"},
		{"=="},
		{"+="},
		{"-="},
		{"/="},
		{"*="},
		{"^="},
		{"|="},
		{"&="},
		{"++"},
		{"--"},
		{"%="},
		{">>"},
		{"<<"}
	};
	static const std::unordered_map<std::string, Token> tokenMap = {
		{"float", makeToken("float", tokenTypes::DATATYPE)},
		{"int", makeToken("int", tokenTypes::DATATYPE)},
		{"double", makeToken("double", tokenTypes::DATATYPE)},
		{"char", makeToken("char", tokenTypes::DATATYPE)},
		{"bool", makeToken("bool", tokenTypes::DATATYPE)},
		{"long", makeToken("long", tokenTypes::DATATYPE)},
		{"auto", makeToken("auto", tokenTypes::DATATYPE)},
		{"void", makeToken("void", tokenTypes::DATATYPE)},

		{"while", makeToken("while", tokenTypes::IFSTATEMENT)},
		{"for", makeToken("for", tokenTypes::IFSTATEMENT)},
		{"if", makeToken("if", tokenTypes::IFSTATEMENT)},
		{"else", makeToken("else", tokenTypes::IFSTATEMENT)},

		{"return", makeToken("return", tokenTypes::RETURN)},

		{"break", makeToken("break", tokenTypes::KEYWORD)},
		{"continue", makeToken("continue", tokenTypes::KEYWORD)},

		{"(", makeToken("(", tokenTypes::GROUPING_)},
		{")", makeToken(")", tokenTypes::GROUPING_)},
		{"{", makeToken("{", tokenTypes::GROUPING_)},
		{"}", makeToken("}", tokenTypes::GROUPING_)},
		{"]", makeToken("]", tokenTypes::GROUPING_)},
		{"[", makeToken("[", tokenTypes::GROUPING_)},
		{"'", makeToken("'", tokenTypes::GROUPING_)},
		{"\"", makeToken("\"", tokenTypes::GROUPING_)},


		{"//", makeToken("//", tokenTypes::_NULL)},
		{"/*", makeToken("/*", tokenTypes::_NULL)},
		{"*/", makeToken("*/", tokenTypes::_NULL)},

		{",", makeToken(",", tokenTypes::DELIMITER)},
		{".", makeToken(".", tokenTypes::DELIMITER)},
		{":", makeToken(":", tokenTypes::DELIMITER)},

		{"+", makeToken("+", tokenTypes::OPERATOR)},
		{"-", makeToken("-", tokenTypes::OPERATOR)},
		{"=", makeToken("=", tokenTypes::OPERATOR)},
		{"/", makeToken("/", tokenTypes::OPERATOR)},
		{"*", makeToken("*", tokenTypes::OPERATOR)},
		{"%", makeToken("%", tokenTypes::OPERATOR)},
		{"^", makeToken("^", tokenTypes::OPERATOR)},
		{">", makeToken(">", tokenTypes::OPERATOR)},
		{"<", makeToken("<", tokenTypes::OPERATOR)},
		{"&", makeToken("&", tokenTypes::OPERATOR)},
		{"|", makeToken("|", tokenTypes::OPERATOR)},
		{"!", makeToken("!", tokenTypes::OPERATOR)},

		{";", makeToken(";", tokenTypes::SEMICOLON)}

	};
	char reader = '\0';
	std::vector<Token> lineContents;
	size_t line = 1;//1 indexed.
	int column = -1;//0 indexed.
	std::string tempString = "";  
	while (file.get(reader)) {
		if (reader == '\n' || reader == '\v' || reader == '\f') { ++line; column = 0; continue; }
		if (isWhitespace(reader)) { continue; }
		if (reader == '/' && file.peek() == '/') {
			while (file.get(reader)) {
				if (reader == '\n' || reader == '\v' || reader == '\f') { ++line; column = 0; break; }
			}
			continue;
		}
		if (reader == '/' && file.peek() == '*') {
			while (file.get(reader)) {
				if (reader == '\n' || reader == '\v' || reader == '\f') { ++line; continue; }
				if (reader == '*' && file.peek() == '/') { 
					file.get(reader);
					break;
				}
			}
			column = 0;
			continue;
		}
		tempString += reader;
		column++;
		if (tempString.size()==1) {
			if (isOperator(tempString[0])) {
				if (isOperator(file.peek())) {
					char prevChar = '\0';
					file.get(prevChar);
					tempString += prevChar;
					if (doubleOperators.find(tempString) != doubleOperators.end()) {
						lineContents.push_back(makeToken(tempString, tokenTypes::OPERATOR));
						lineContents[lineContents.size() - 1].column = column;
						lineContents[lineContents.size() - 1].line = line;
						tempString = "";
						continue;
					}
					else {
						file.unget();
						--column;
						tempString.pop_back();
					}
				}
			}
			if (tokenMap.find(tempString) != tokenMap.end()) {
				lineContents.push_back(tokenMap.at(tempString));
				lineContents[lineContents.size() - 1].column = column;
				lineContents[lineContents.size() - 1].line = line;
				tempString = "";
				continue;
			}
		}
		if (isOperator(file.peek()) || isWhitespace(file.peek()) || isDelimiter(file.peek()) || isGrouping(file.peek())) {
			if (tokenMap.find(tempString) != tokenMap.end()) {
				lineContents.push_back(tokenMap.at(tempString));
				lineContents[lineContents.size() - 1].column = column;
				lineContents[lineContents.size() - 1].line = line;
				tempString = "";
				continue;
			}
			else {
				lineContents.push_back(makeToken(tempString, tokenTypes::IDENTIFIER));
				lineContents[lineContents.size() - 1].column = column;
				lineContents[lineContents.size() - 1].line = line;
				tempString = "";
				continue;
			}
		}



	}
	file.close();//not neccessary since file destuructor handles it but is good practice for writing instantly(safer for crashes) or if you have to open another file with same object
	//tabs are one byte \t not a bunch of spaces, for example 4 spaces are 4 bytes.
	return lineContents;
}
