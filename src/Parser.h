#ifndef PARSER_H
#define PARSER_H

#include <list>

#include "AST_Node.h"
#include "CompilationCommon.h"

class Parser{
public:
	Parser();
	
	AST_Node* parse(const std::list<TokenPair>& tokenStream);
private:
	
};

#endif