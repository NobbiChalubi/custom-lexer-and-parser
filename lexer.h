#pragma once
#ifndef LEXER_H
#define LEXER_H
#include <string>
#include <vector>
#include "token.h"

std::vector<Token> Tokenize(const std::string& fileName);


#endif // !"LEXER_H"
