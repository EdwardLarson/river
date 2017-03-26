#include "AST_node.h"

AST_Node::AST_Node(){
	
}

Data_Object* AST_Node::evaluate(Scope& scope){
	return NULL;
}

AST_Node_Constant::AST_Node_Constant(){
	
}

Data_Object* AST_Node_Constant::evaluate(Scope& scope){
	return NULL;
}

Data_Object* AST_Node_Variable::evaluate(Scope& scope){
	// look up identifier in vtable
	return scope.lookup_variable(identifier);
	return NULL;
}

Data_Object* AST_Node_Assignment::evaluate(Scope& scope){
	return NULL;
}

Data_Object* AST_Node_Expression::evaluate(Scope& scope){
	/*
	//evaluate each side of the expression, left first
	Data_Object* leftValue = left->evaluate(scope);
	Data_Object* rightValue = right->evaluate(scope);
	
	if (leftValue->dtype == Data_Object::OBJECT || rightValue->dtype == Data_Object::OBJECT){
		//look up user-defined operand for these types
		//Function* custom_operator = findOperator(operation, left_value->dtype, left_value->dtype);
		//if (custom_operator != NULL){
		//	return custom_operator->evaluate();
		//}else{
		//	throw some kind of exception
		//}
		return NULL;
	}
	
	//typedef enum{ADD, SUB, MUL, DIV, PWR, MOD, RSH, LSH, BND, BOR, AND, OR} OP_TYPE;
	switch(operation){
		case ADD:
			//return new Data_Object
			return NULL;
			break;
		case SUB:
			return NULL;
			break;
		case MUL:
			return NULL;
			break;
		case DIV:
			return NULL;
			break;
		case PWR:
			return NULL;
			break;
		case MOD:
			return NULL;
			break;
		case RSH:
			return NULL;
			break;
		case LSH:
			return NULL;
			break;
		case BND:
			return NULL;
			break;
		case BOR:
			return NULL;
			break;
		case AND:
			return NULL;
			break;
		case OR:
			return NULL;
			break;
		default:
			//no valid operation
			return NULL;
	}
	*/
	return NULL;
}

Data_Object* AST_Node_Expression_Block::evaluate(Scope& scope){
	Data_Object* returnValue = NULL;
	for (unsigned int i = 0; i < expressionCount; i++){
		returnValue = expressions[i]->evaluate(scope);
		if (expressions[i]->is_return()){
			// break and return when a node that returns a value is found
			return returnValue;
		}
	}
	return NULL;
}

Data_Object* AST_Node_Branch::evaluate(Scope& scope){
	Data_Object* conditionResult = condition->evaluate(scope);
	if (condition != NULL){
		bool conditionSatisfied = false;
		//TO-DO: function converting arbitrary data types to boolean value
		
		//note: these should return NULL except in possible case of trinary operator (var = condition? true_branch : false_branch)
		if (conditionSatisfied){
			return trueBranch->evaluate(scope);
		}else{
			return falseBranch->evaluate(scope);
		}
	}else{
		//TO-DO: define behavior when condition does not return anything
		return NULL;
	}
}

Data_Object* AST_Node_Throwable::evaluate(Scope& scope){
	
	Scope tryScope = Scope(scope);
	
	tryBranch->evaluate(tryScope);
	
	// TO-DO: some way to have multiple catch blocks that accept different exception types
	
	//if try_scope.exception != NULL
	//	Scope catchScope(scope)
	//	catchScope.add_variable(exceptionIdentifier, tryScope.exception)
	//	catchBranch->evalue(catchScope)
	if(false){
		catchBranch->evaluate(scope);
	}
	
	finallyBranch->evaluate(scope);
	return NULL;
}

Data_Object* AST_Node_Return::evaluate(Scope& scope){
	// set some sort of "return" flag as true
	
	return returnValue->evaluate(scope);
}

Data_Object* AST_Node_Declaration::evaluate(Scope& scope){
	 Data_Object* newObject = scope.create_new_variable(type, identifier);
	return newObject;
}

Data_Object* AST_Node_Function_Defined::evaluate(Scope& scope){
	
	// create new scope for function
	Scope functionCallScope;
	// for each argument:
	for (unsigned int i = 0; i < argumentCount; i++){
		// evaluate argument and add to function scope
		functionCallScope.add_variable(argumentIdentifiers[i], arguments[i]->evaluate(scope));
	}
	// pass function scope to AST head
	return functionTree->evaluate(functionCallScope);
}

template <class F>
Data_Object* AST_Node_Function_Hardcode<F>::evaluate(Scope& scope){
	
	// pass arguments to C++ function object
	return F(argumentIdentifiers, argumentCount);
}