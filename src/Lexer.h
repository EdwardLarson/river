#ifndef LEXER_H
#define LEXER_H

#include <list>
#include <iostream>

#include "Compilation_Common.h"

/*
	LEXER:
	Performs line-by-line conversion of a text into a stream of recognizable tokens.
	The token stream can then be analyzed for correctness and then compiled into an AST.

*/

class Lexer{
public:
	Lexer();
	Lexer(char indentChar);
	
	void interpret_line(const std::string& line);
	void finish();
	
	std::list<TokenPair>& get_token_stream(){ return tokenStream; };
private:
	typedef enum e_LexState {S_STRING, S_OPERATOR, S_TEXT, S_NUMBER, S_NONE, S_DELIM, S_COMMENT, S_DONE, S_ENDSTMNT, S_ELLIPSE} LexState;
	typedef enum e_CharType {C_OPERATOR, C_WHITESPACE, C_DELIMITER, C_QUOTE, C_PERIOD, C_GENCHAR, C_NUMBER, C_ESCAPE, C_COLON} CharType;
	
	///inline void print_syntax_error(const std::string& message) const;
	
	// Utility functions
	CharType get_char_type(char c);
	bool is_delimiter(char c);
	bool is_operator(char c);
	bool is_whitespace(char c);
	bool is_reserved_word(const std::string& token);
	
	// Token Stream and State Manipulation functions
	void label_and_push_token(TokenType type);
	LexState next_state_from_char(CharType type);
	void start_new_token(char c);
	
	// State Functions
	void process_char_S_OPERATOR(char c);
	void process_char_S_STRING(char c);
	void process_char_S_TEXT(char c);
	void process_char_S_NUMBER(char c);
	void process_char_S_DELIM(char c);
	void process_char_S_COMMENT(char c);
	void process_char_S_ENDSTMNT(char c);
	
	unsigned int count_whitespace(const std::string& line);

	// list storing a stream of tokens
	std::list<TokenPair> tokenStream;
	
	// various state data
	LexState state;
	std::string currentToken;
	CharType previousCharType;
	bool multiLineComment;
	bool multiLineCommentExit;
	bool lineNotBlank;
	
	std::string currentLine;
	unsigned int lineNo;
	unsigned int colNo;
	unsigned int previousIndent;
	
	// lexer options / configurations
	const char m_indentChar;
};

#endif