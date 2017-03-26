#include "Lexer.h"

Lexer::Lexer(): 
		state(S_NONE), 
		currentToken(), 
		previousCharType(C_WHITESPACE), 
		lineNo(0), 
		colNo(0), 
		currentLine(), 
		previousIndent(0),
		multiLineComment(false),
		multiLineCommentExit(false),
		lineNotBlank(false),
		m_indentChar('\t'){
	
}

Lexer::Lexer(char indentChar): 
		state(S_NONE), 
		currentToken(), 
		previousCharType(C_WHITESPACE), 
		lineNo(0), 
		colNo(0), 
		currentLine(), 
		previousIndent(0),
		multiLineComment(false),
		multiLineCommentExit(false),
		lineNotBlank(false),
		m_indentChar(indentChar){
	
}

void Lexer::interpret_line(const std::string& line){
	// keep track of line number and start the count at 1
	++lineNo;
	
	currentLine = line;
	CharType charType;
	bool noErrors = true;
	lineNotBlank = false;
	
	colNo = 0;
	
	if (state != S_ELLIPSE){
		unsigned int lineIndent = count_whitespace(currentLine);
		if (lineIndent > previousIndent){
			// if (lineIndent == previousIndent + 1){
				// TokenPair newToken = {TKN_ENTERBLOCK, ""};
				// tokenStream.push_back( newToken );
			// }
			for (unsigned int i = 0; i < lineIndent - previousIndent; i++){
				TokenPair newToken = {TKN_EXITBLOCK, ""};
				tokenStream.push_back( newToken );
			}
		}else if (lineIndent < previousIndent){
			for (unsigned int i = 0; i < previousIndent - lineIndent; i++){
				TokenPair newToken = {TKN_EXITBLOCK, ""};
				tokenStream.push_back( newToken );
			}
		}
		
		previousIndent = lineIndent;
	}
	
	for (; colNo < line.length() && noErrors; colNo++){
		
		//std::cout << "CHARACTER: " << (unsigned int) line[colNo] << "\tCHARTYPE: " << get_char_type(line[colNo]) <<  "\tSTATE" << state << std::endl;
		
		// next actions are determined by lexer state, previousCharType, and charType
		switch (state){
			case S_OPERATOR:
				process_char_S_OPERATOR(line[colNo]);
				break;
			case S_TEXT:
				process_char_S_TEXT(line[colNo]);
				break;
			case S_STRING:
				process_char_S_STRING(line[colNo]);
				break;
			case S_NUMBER:
				process_char_S_NUMBER(line[colNo]);
				break;
			case S_DELIM:
				process_char_S_DELIM(line[colNo]);
				break;
			case S_NONE:
				start_new_token(line[colNo]);
				break;
			case S_COMMENT:
				process_char_S_COMMENT(line[colNo]);
				break;
			case S_ENDSTMNT:
				process_char_S_ENDSTMNT(line[colNo]);
				break;
			case S_ELLIPSE:
				if (!is_whitespace(line[colNo])){
					start_new_token(line[colNo]);
				}
				break;
			case S_DONE:
				// do nothing when done
				break;
		}
		
		previousCharType = get_char_type(line[colNo]);
	}
	
	if (!noErrors){
		std::cout << "Unable to continue reading through last error!" << std::endl;
	}
	
	//at the end of a line:
	
	// push current token
	switch (state){
		case S_OPERATOR:
			label_and_push_token(TKN_OPERATOR);
			break;
		case S_TEXT:
			label_and_push_token(is_reserved_word(currentToken)? 
				TKN_RWORD : TKN_IDENTIFIER);
			break;
		case S_STRING:
			label_and_push_token(TKN_STRLITERAL);
			break;
		case S_NUMBER:
			label_and_push_token(TKN_NUMLITERAL);
			break;
		case S_DELIM:
			label_and_push_token(TKN_DELIMINATOR);
			break;
		case S_NONE:
			//either no more characters to build tokens from or syntax issues
			std::cout << "DISCARDED TOKEN " << currentToken << " in line " << lineNo << std::endl;
			currentToken = "";
			break;
		case S_ENDSTMNT:
			currentToken = "";
			label_and_push_token(TKN_ENDSTATEMENT);
		case S_ELLIPSE:
			// do nothing when ending a line in an ellipse
			break;
		default:
			break;
	}
	
	// end statement logic
	if (!multiLineComment && state != S_ELLIPSE){
		
		//when a line is not blank
		if (lineNotBlank && state != S_COMMENT){
			TokenPair newToken = {TKN_ENDSTATEMENT, ""};
			tokenStream.push_back( newToken );
		}
		
		//reset state
		state = S_NONE;
	}
}

void Lexer::finish(){
	// add leave block tokens for every indent left at end
	
	for (unsigned int i = 0; i < previousIndent; i++){
		TokenPair newToken = {TKN_EXITBLOCK, ""};
		tokenStream.push_back( newToken );
	}
	
	previousIndent = 0;
	state = S_NONE;
}

void Lexer::process_char_S_OPERATOR(char c){
	
	CharType charType = get_char_type(c);
	
	switch (charType){
		case C_OPERATOR:
			// still reading in more of an operator
			currentToken += c;
			
			//check for comments
			if (currentToken == "//"){
				state = S_COMMENT;
				currentToken = "";
				if (lineNotBlank){
					label_and_push_token(TKN_ENDSTATEMENT);
				}
				
			}else if (currentToken == "/*"){
				state = S_COMMENT;
				currentToken = "";
			}
			break;
		default:
			// operator is done; save the operator token to tokenStream
			label_and_push_token(TKN_OPERATOR);

			// set new state based on character type
			//state = next_state_from_char(charType);
			//if (state != S_NONE){
			//	currentToken += c;
			//}
			//break;
			start_new_token(c);
			break;
		
	}
}

void Lexer::process_char_S_STRING(char c){
	
	CharType charType = get_char_type(c);
	
	if (previousCharType == C_ESCAPE){
		// no matter what this character is, add it to the string literal
		currentToken += c;
		return;
		
		//TO-DO: process other escapables: \t, \n, \r
	}
	
	switch (charType){
		case C_QUOTE:
			// string literal has ended; save token to tokenStream
			label_and_push_token(TKN_STRLITERAL);
			
			// set a new state
			state = S_NONE;
			break;
		case C_ESCAPE:
			// escaping next character, ignore this '\'
			break;
		default:
			// next character in string literal
			currentToken += c;
			break;
	}
}

void Lexer::process_char_S_TEXT(char c){
	
	CharType charType = get_char_type(c);
	
	switch (charType){
		case C_GENCHAR:
			// next character in identifier or reserved word
			currentToken += c;
			break;
		case C_NUMBER:
			// identifiers are allowed to have numbers in them
			currentToken += c;
			
		/* NOT BOTHERING WITH ERROR CHECKING IN LEXER
		case C_QUOTE:
			///print_syntax_error("\tMisplaced '\"' character; try whitespace or operator to separate string literal");
			
			// push the identifer or reserved word we have to tokenStream
			label_and_push_token(is_reserved_word(currentToken)? 
				TKN_RWORD : TKN_IDENTIFIER);
				
			// state is undefined
			//state = S_NONE;
			
			start_new_token(c);
				
			break;
		case C_ESCAPE:
			// syntax error; line is something like: foo\ 
			///print_syntax_error("\tMisplaced '\\' character; try adding whitespace or removing character");
			
			// push the identifer or reserved word we have to tokenStream
			label_and_push_token(is_reserved_word(currentToken)? 
				TKN_RWORD : TKN_IDENTIFIER);
			
			// state is undefined
			//state = S_NONE;
			
			start_new_token(c);
			
			break;*/
		default:
			// identifer or reserved word is done
			label_and_push_token(is_reserved_word(currentToken)? 
				TKN_RWORD : TKN_IDENTIFIER);
				
			// state = next_state_from_char(charType);
			// if (state != S_NONE){
				// currentToken += c;
			// }
			start_new_token(c);
			
			break;
	}
}

void Lexer::process_char_S_NUMBER(char c){
	
	CharType charType = get_char_type(c);
	
	switch (charType){
		case C_NUMBER:
			// next digit in number
			currentToken += c;
			break;
		case C_PERIOD:
			// period in a number; only valid if followed by more numbers JK NOT GONNA BOTHER WITH ERROR CHECKING IN LEXER
			//if (previousCharType != C_PERIOD){
				
				currentToken += c;
				
			//}else{
				///print_syntax_error("\tMisplaced '.' character. A number cannot have more than two dots.");
				
				//label_and_push_token(TKN_NUMLITERAL);
				//state = S_NONE;
			//}
			break;
			
		/* NOT BOTHERING WITH ERROR CHECKING IN LEXER
		case C_QUOTE:
			// syntax error: line is something like: 53 + 45"blah"
			///print_syntax_error("\tMisplaced '\"' character; try adding whitespace or removing character");
			// assume the quote marks beginning of string literal
			state = S_STRING;
			
			// push the identifer or reserved word we have to tokenStream
			label_and_push_token(TKN_NUMLITERAL);
			
			break;
		case C_ESCAPE:
			// syntax error: line is something like: 53 + 45\ 
			print_syntax_error("\tMisplaced '\\' character; try removing character");
			///std::cout << "\tMisplaced '\\' character; try removing character" << std::endl;
			
			// push the identifer or reserved word we have to tokenStream
			label_and_push_token(TKN_NUMLITERAL);
			
			// state is undefined
			state = S_NONE;
			
			break;
			
			*/
		default:
			// number literal has ended in some way
			label_and_push_token(TKN_NUMLITERAL);
			//
			//state = next_state_from_char(charType);
			//if (state != S_NONE){
			//	currentToken += c;
			//}
			start_new_token(c);
			
			break;
	}
}

void Lexer::process_char_S_DELIM(char c){
	
	CharType charType = get_char_type(c);
	
	switch(charType){
		case C_PERIOD:
			currentToken += c;
			if (currentToken == "..."){
				//in an ellipse
				currentToken = "";
				state = S_ELLIPSE;
			}
			break;
		default:
			//push the current token (which should be a deliminator) and update state for new character
			label_and_push_token(TKN_DELIMINATOR);
			start_new_token(c);
	}
	
}

void Lexer::process_char_S_COMMENT(char c){
	//handle multi-line comments
	if (multiLineComment){
		//look for end of multi-line comment
		if (multiLineCommentExit){
			if (c == '/'){
				state = S_NONE;
				multiLineComment = false;
			}
			
			multiLineCommentExit = false;
		}else if (c == '*'){
			multiLineCommentExit = true;
		}
	}
}
void Lexer::process_char_S_ENDSTMNT(char c){
	CharType charType = get_char_type(c);
	
	if (charType != C_WHITESPACE){
		TokenPair newToken = {TKN_ENDSTATEMENT, ""};
		tokenStream.push_back(newToken);
		
		currentToken = "";
		start_new_token(c);
	}
}

// labels the currentToken with a TokenType and pushes it to the tokenStream. Resets currentToken.
void Lexer::label_and_push_token(TokenType type){
	TokenPair newToken = {type, currentToken};
	tokenStream.push_back( newToken );
	currentToken = "";
	
	if (type != TKN_ENDSTATEMENT && type != TKN_ENTERBLOCK && type != TKN_EXITBLOCK){
		lineNotBlank = true;
	}
}

// determine the next state from the first CharType of the next token
Lexer::LexState Lexer::next_state_from_char(CharType type){
	switch (type){
		case C_OPERATOR:
			return S_OPERATOR;
			break;
		case C_QUOTE:
			return S_STRING;
			break;
		case C_NUMBER:
			return S_NUMBER;
			break;
		case C_GENCHAR:
			return S_TEXT;
			break;
		case C_WHITESPACE:
			return S_NONE;
			break;
		case C_DELIMITER:
			return S_DELIM;
			break;
		case C_ESCAPE:
			return S_NONE;
			break;
		case C_PERIOD:
			return S_DELIM;
			break;
		case C_COLON:
			return S_ENDSTMNT;
			break;
	}
}

void Lexer::start_new_token(char c){
	CharType charType = get_char_type(c);
	
	state = next_state_from_char(charType);
	if (state != S_NONE 
			&& previousCharType != C_ESCAPE 
			&& state != S_STRING
			&& state != S_ENDSTMNT){
		currentToken += c;
	}
	
	// treat anything after a ':' as a new line
	if (state == S_ENDSTMNT){
		lineNotBlank = false;
	}
}

unsigned int Lexer::count_whitespace(const std::string& line){
	unsigned int result = 0;
	for (unsigned int i = 0; i < line.length(); i++){
		if (line[i] == m_indentChar){
			result++;
		}else{
			break;
		}
	}
	
	return result;
}

Lexer::CharType Lexer::get_char_type(char c){
	if (c >= 48 && c <= 57){
		return C_NUMBER;
	}else if(c == '.'){
		return C_PERIOD;
	}else if(c == '"'){
		return C_QUOTE;
	}else if(c == '\\'){
		return C_ESCAPE;
	}else if(c == ':'){
		return C_COLON;
	}else if(is_delimiter(c)){
		return C_DELIMITER;
	}else if(is_operator(c)){
		return C_OPERATOR;
	}else if(is_whitespace(c)){
		return C_WHITESPACE;
	}else{
		// if not matching any other profile, call it a generic character (part of an identifier, reserved word, or string literal)
		return C_GENCHAR;
	}
}

bool Lexer::is_delimiter(char c){
	for (unsigned int i = 0; i < nDELIMITERS; i++){
		if (c == delimiterChars[i])
			return true;
	}
	return false;
}

bool Lexer::is_operator(char c){
	for (unsigned int i = 0; i < nOPERATORCHARS; i++){
		if (c == operatorChars[i])
			return true;
	}
	return false;
}

bool Lexer::is_whitespace(char c){
	for (unsigned int i = 0; i < nWSPACECHARS; i++){
		//std::cout << "Checking defined whitespace char " << (unsigned int) whitespaceChars[i] << ", c = " << (unsigned int) c << std::endl;
		if (c == whitespaceChars[i]){
			//std::cout << "Determined ASCII " << (unsigned int) c << " to be whitespace" << std::endl;
			return true;
		}
	}
	return false;
}

bool Lexer::is_reserved_word(const std::string& token){
	for (unsigned int i = 0; i < nRWORDS; i++){
		if (token == reservedWords[i])
			return true;
	}
	return false;
}