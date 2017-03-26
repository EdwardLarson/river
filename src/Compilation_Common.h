#ifndef COMPILATION_COMMON_H
#define COMPILATION_COMMON_H

#define nRWORDS 10
#define nOPERATORCHARS 12
#define nWSPACECHARS 3
#define nDELIMITERS 7

#include <string>
#include <unordered_map>

// Types of tokens: reserved words, identifier, literal, operator

// TO-DO: Implement COMMENTS

typedef enum e_TokenType {TKN_RWORD, TKN_IDENTIFIER, TKN_NUMLITERAL, TKN_STRLITERAL, TKN_OPERATOR, TKN_DELIMINATOR, TKN_ENTERBLOCK, TKN_EXITBLOCK, TKN_ENDSTATEMENT} TokenType;
// Reserved words: function, for, while, true, false, returns, return, if, else, elif
const std::string reservedWords[nRWORDS] = {"function", "for", "while", "true", "false", "returns", "return", "if", "else", "elif"};
// Identifiers: could be a typename, variable name, or function name

// Numeric Literals: Literals can be unsigned integers or floats

// String Literals: Strings
// Operators: See AST_Node.h for full list of operators
const char operatorChars[nOPERATORCHARS] = {'+', '-', '/', '*', '^', '%', '>', '<', '|', '&', '!', '='};
// Delimiters: (, ), ?, [, ]
const char delimiterChars[nDELIMITERS] = {'(', ')', '?', '[', ']', ','};

const char whitespaceChars[nWSPACECHARS] = {' ', '\t', '\r'};

struct TokenPair{
	TokenType type;
	std::string token;
};

struct ClassIdent{
	
};

class Data_Object;
class Data_ObjectFunction;
	
#endif