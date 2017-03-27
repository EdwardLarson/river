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

#define META_BEGIN		0x00
#define META_END		0x01
#define META_VERSION	0x02
#define META_STRING		0x03
#define META_NSTRING	0x04
#define META_CHECKSUM	0x05

#define UNION_CAST(x, destType) \
	(((union {__typeof__(x) a; destType b;})x).b)
	
typedef unsigned char Byte;
typedef signed long IntegerType;
typedef double RationalType;

typedef unsigned long PCType;

/*
OPCODES
Opcodes for the atomic operations this virtual machine supports
*/
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

/*
Data_Type
Different atomic types of data. By default: INTEGERs are
64-bit signed longs; RATIONALs are 64-bit double precision
floats; NIL is a generic null-value type and have no data 
representation; BOOLs are single bytes; STRINGs are c
strings; POINTERs are void pointers.
*/
typedef enum { // 4 bytes maybe, according to C++ standard but not C
	INTEGER,
	RATIONAL,
	NIL,
	BOOL,
	STRING,
	POINTER
	//,FUNCTION
} Data_Type;

/*
Data 
Representation for atomic data types:
RATIONAL is represented by d
INTEGER is represented by n
STRING is represented by s
BOOL is represented by b
POINTER is represented by p
*/
typedef union {
	RationalType d;
	IntegerType n;
	char* s; // string length stored separately by string object
	Byte b; // represents boolean value
	void* p; // garbage collected value
	// how to represent function type?
} Data;

/* Data_Object 
An atomic data representation which has a type (Data_Type)
representing what kind of data is holds and a Data union 
which holds its representation
*/
typedef struct {
	Data data; // 8 bytes
	Data_Type type; // 4 bytes
	// 4 extra bytes
	Byte extra[4];
} Data_Object;

/* Register_File
Holds the registers used by a VM as a 2D array, where
each register has an associated depth (maximum determined 
by FRAME_STACK_SIZE) representing the current position in
the runtime register stack. Also contains 
*/
typedef struct {
	Data_Object registers[NUM_REGISTERS][FRAME_STACK_SIZE]; // depth in persistent registers is wasted, but could be reused maybe as temp?
	Byte defaultOutput;
	Byte depth;
	
	PCType pcStack[PC_STACK_SIZE];
	unsigned int pcTop;
} Register_File;

typedef struct {
	char** stringHeads;
	unsigned int nstrings;
	unsigned int lastStored;
	
	Byte versionMain;
	Byte versionSub;
} Meta_Data;

//==========================================================
// VM FUNCTIONS
//==========================================================

void execute(Byte* byteStream, PCType* length);
PCType read_metadata(const Byte* byteStream, PCType* length, Meta_Data* metaData);

// Bytestream manipulation
inline Data_Object*	fetch_data(Byte* rawBytes);
Data_Object 		read_bytes(Byte* rawBytes);
Byte 				read_bool_direct(Byte* rawBegin);
IntegerType 		read_integer_direct(Byte* rawBegin);
RationalType 		read_double_direct(Byte* rawBegin);
PCType 				read_address_literal(Byte* rawBytes);

// Data_Object manipulation
Data_Object create_object_INTEGER(IntegerType n);
Data_Object create_object_RATIONAL(RationalType d);
Data_Object create_object_BOOL(Byte b);
inline void clear_data(Data_Object* object);
char* get_object_cstring(Data_Object* stringObject);

// Register file functions
void initialize_register_file(Register_File* rFile);
Data_Object* access_register(Byte reg, Register_File* rf);
void write_default_output(Data_Object* object, Register_File* rf);
void push_pc(PCType* pc_entry, Register_File* rf);
PCType pop_pc(Register_File* rf);



#endif