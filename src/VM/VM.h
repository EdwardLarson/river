#ifndef VM_H
#define VM_H

#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#define REGISTER_NODE_SIZE 512
#define DATA_OBJECT_SIZE sizeof(Data_Object)
#define PC_STACK_SIZE 256
#define STRING_LENGTH_BYTES 8
#define FRAME_STACK_SIZE 64
#define NUM_REGISTERS 256
#define NUM_PERSISTENT_REGISTERS 32

#define UNION_CAST(x, destType) \
	(((union {__typeof__(x) a; destType b;})x).b)
	
typedef unsigned char Byte;
typedef signed long IntegerType;
typedef double RationalType;

typedef unsigned long PCType;

/* OPCODES */
typedef enum _OPCODE{
	ABS,
	ADD,
	AND,
	BRANCH,
	CALL,
	DIV,
	EQ,
	GT,
	HALT,
	INPUT,
	JUMP,
	LOAD,
	LSH,
	LT,
	MALLOC,
	MFREE,
	MLOAD,
	MOD,
	MOVE,
	MSTORE,
	MUL,
	NOT, // a bitwise inversion (~), the logical operator (!) can be found by BRANCH
	OR,
	POPFRAME,
	PUSHFRAME,
	POW,
	PRINT,
	RETURN,
	RSH,
	SETDO,
	SUB,
	XOR
} OPCODE;

/* Data_Type */
typedef enum { // 4 bytes maybe, according to C++ standard but not C
	INTEGER,
	RATIONAL,
	NIL,
	BOOL,
	STRING,
	POINTER
	//,FUNCTION
} Data_Type;

/* Data */
typedef union {
	RationalType d;
	IntegerType n;
	char* s; // string length stored separately by string object
	Byte b; // represents boolean value
	void* p; // garbage collected value
	// how to represent function type?
} Data;

/* Data_Object */
typedef struct {
	Data data; // 8 bytes
	Data_Type type; // 4 bytes
	// 4 extra bytes
	Byte extra[4];
} Data_Object;

char* get_object_cstring(Data_Object* stringObject){
	return (stringObject->data.s) + STRING_LENGTH_BYTES;
}

typedef struct {
	Data_Object registers[NUM_REGISTERS][FRAME_STACK_SIZE];
	Byte defaultOutput;
	Byte depth;
	
	PCType pcStack[PC_STACK_SIZE];
	unsigned int pcTop;
} Register_File;

/* DESIGNED FOR A COMPRESSED, MEMORY-SPACE OPTIMIZED SYSTEM

struct RegisterNode{
	unsigned long registers[REGISTER_NODE_SIZE];
	unsigned int first; // index of register[0]
	
	RegisterNode* nxt;
	RegisterNode* prv;
}

struct Registers{
	RegisterNode* head;
	
	RegisterNode* currentNode; // the node containing the index stored in 'currentOffset'
	unsigned int currentOffset;
	unsigned int lastIndexInCurrent;
}

unsigned long access_register(unsigned char reg, Registers* registers){
	unsigned int index = (unsigned int) reg + registers->currentOffset;
	
	if (index > registers->lastIndexInCurrent){
		// find register in next registerNode
		return registers->currentNode->nxt->registers[index - registers->currentNode->nxt->first];
	}else {
		return registers->currentNode->registers[index - registers->currentNode->first];
	}
}

void push_frame(unsigned char& highestRegister, Registers* registers){
	// find the global index of the highestRegister accessed so far
	unsigned int highestIndex = (unsigned int) highestRegister + registers->currentOffset;
	
	// check the highest register accessed
	if (highestIndex < registers->lastIndexInCurrent){
		// start new frame in the same array
	}else{
		// create a new frame
		newRegisterNode = new RegisterNode;
		newRegisterNode->prv = registers->currentNode;
		newRegisterNode->nxt = 0x00; //NULL
		newRegisterNode->first = registers->lastIndexInCurrent + 1;
		
		// add frame to register file
		registers->currentOffset = highestIndex + 1;
		registers->lastIndexInCurrent += 256;
		registers->currentNode = newRegisterNode;
		
	}
}

void pop_frame(Register* registers){
	
}
*/

#endif