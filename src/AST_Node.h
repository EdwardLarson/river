#ifndef AST_NODE_H
#define AST_NODE_H

#include <string>
#include "Data_Object.h"
#include "Compilation_Common.h"

/*
Need several enums:
-op type: +, -, /, *, **, %, >>, <<, |, &, ||, &&, ==, !=, :=
-unary operators: ++, --, 
-data type: string, uint, int, ufloat, float, bool

:= or CEQ is an assignment operator which creates a copy of the assigned value (Data_Value* vs new Data_Value)
*/

class AST_Node{
public:
	AST_Node();
	
	virtual Data_Object* evaluate(Scope& scope);
	
	typedef enum e_OP_TYPE {ADD, SUB, MUL, DIV, PWR, MOD, RSH, LSH, BND, BOR, AND, OR, EQ, NEQ, NOT, LSS, GRT, AEQ, INC, DEC, CEQ, NOP} OP_TYPE;
	// single-argument operators: NOT, INC, DEC, NOP(?)
	//
private:
	AST_Node* parent;
};

class AST_Node_Expression_Block: public AST_Node{
public:
	Data_Object* evaluate(Scope& scope);
private:
	unsigned int expressionCount;
	AST_Node** expressions;
};

class AST_Node_Constant: public AST_Node{
public:
	Data_Object* evaluate(Scope& scope);
private:
//no children
	Data_Object* mdata;
};

class AST_Node_Variable: public AST_Node{
public:
	virtual Data_Object* evaluate(Scope& scope);
private:
	std::string identifier;
};

class AST_Node_Assignment: public AST_Node{
public:
	Data_Object* evaluate(Scope& Scope);
private:
	AST_Node_Variable* variable;
	AST_Node* value;
};

class AST_Node_Expression: public AST_Node{
public:
	Data_Object* evaluate(Scope& scope);
private:
	OP_TYPE operation;
	//left and right nodes
	AST_Node* left;
	AST_Node* right;
};

class AST_Node_Branch: public AST_Node{
public:
	Data_Object* evaluate(Scope& scope);
private:
	AST_Node* condition;
	AST_Node* trueBranch;
	AST_Node* falseBranch;
};

class AST_Node_Throwable: public AST_Node{
public:
	Data_Object* evaluate(Scope& scope);
private:
	AST_Node* tryBranch;
	AST_Node* catchBranch;
	AST_Node* finallyBranch;
	
	std::string exceptionIdentifier;
};

class AST_Node_Return: public AST_Node{
public:
	Data_Object* evaluate(Scope& scope);
private:
	AST_Node_Expression* returnValue;
};

class AST_Node_Declaration: public AST_Node_Variable{
public:
	Data_Object* evaluate(Scope& scope);
private:
	std::string type;
}

class AST_Node_Function: public AST_Node{
public:
	virtual Data_Object* evaluate(Scope& scope) = 0;
protected:
	AST_Node** arguments;
	unsigned int argumentCount;
	std::string* argumentIdentifiers;
};

class AST_Node_Function_Defined: public AST_Node_Function{
public:
	Data_Object* evaluate(Scope& scope);
private:
	AST_Node* functionTree;
};

template <class F>
class AST_Node_Function_Hardcode: public AST_Node_Function{
public:
	Data_Object* evaluate(Scope& scope);
private:
	F functionObject;
};

#endif