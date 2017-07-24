#ifndef VM_H
#define VM_H

#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#define WWHEEL_VERSION_MAIN	0
#define WWHEEL_VERSION_SUB	4

#define REGISTER_NODE_SIZE 512
#define DATA_OBJECT_SIZE 16
#define PC_STACK_SIZE 256
#define STRING_LENGTH_BYTES 8
#define FRAME_STACK_SIZE 64
#define NUM_REGISTERS 256
#define NUM_PERSISTENT_REGISTERS 32

#define META_BEGIN		0xf0
#define META_END		0xf1
#define META_VERSION	0xf2
#define META_STRING		0xf3
#define META_NSTRING	0xf4
#define META_CHECKSUM	0xf5

#define P_ARRAY			0x00
#define P_OBJECT		0X01

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
/* 0*/	ABS,
/* 1*/	ADD,
/* 2*/	AND,
/* 3*/	BRANCH,
/* 4*/	CALL,
/* 5*/	DIV,
/* 6*/	EQ,
/* 7*/	GT,
/* 8*/	HALT,
/* 9*/	INPUT,
/*10*/	JUMP,
/*11*/	LSH,
/*12*/	LT,
/*13*/	MALLOC,
/*14*/	MFREE,
/*15*/	MLOAD,
/*16*/	MOD,
/*17*/	MOVE,
/*18*/	MSTORE,
/*19*/	MUL,
/*20*/	NOT, // a bitwise inversion (~), the logical operator (!) can be found by BRANCH
/*21*/	OR,
/*22*/	POPFRAME,
/*23*/	PUSHFRAME,
/*24*/	POW,
/*25*/	PRINT,
/*26*/	RETURN,
/*27*/	RSH,
/*28*/	SETDO,
/*29*/	SUB,
/*30*/	XOR
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
	Byte aux[4]; // reserve last two, allow first two to be used by data types
} Data_Object;

/* Register_File
Holds the registers used by a VM as a 2D array, where
each register has an associated depth (maximum determined 
by FRAME_STACK_SIZE) representing the current position in
the runtime register stack. Also contains 
*/
/*
typedef struct {
	Data_Object registers[NUM_REGISTERS][FRAME_STACK_SIZE]; // depth in persistent registers is wasted, but could be reused maybe as temp?
	Byte defaultOutput;
	Byte depth;
	
	PCType pcStack[PC_STACK_SIZE];
	unsigned int pcTop;
} Register_File;
*/
typedef struct Register_File_{
	Data_Object localRegisters[128][FRAME_STACK_SIZE];
	unsigned int depth;
	
	Data_Object globalRegisters[64];
	
	PCType pcStack[PC_STACK_SIZE];
	unsigned int pcTop;
	
} Register_File;

typedef struct {
	char** stringHeads;
	unsigned int* stringLens;
	unsigned int nstrings;
	unsigned int lastStored;
	
	Byte versionMain;
	Byte versionSub;
} Meta_Data;

//==========================================================
// VM FUNCTIONS
//==========================================================

void execute(const Byte* byteStream, const PCType* length, Byte log);
PCType read_metadata(const Byte* byteStream, const PCType* length, Meta_Data* metaData);

// Bytestream manipulation
inline const Data_Object*	fetch_data(const Byte* rawBytes);
Data_Object 				read_bytes(const Byte* rawBytes);
Byte 						read_bool_direct(const Byte* rawBegin);
IntegerType 				read_integer_direct(const Byte* rawBegin);
RationalType 				read_double_direct(const Byte* rawBegin);
PCType 						read_address_literal(const Byte* rawBytes);

// Data_Object manipulation
Data_Object create_object_INTEGER(IntegerType n);
Data_Object create_object_RATIONAL(RationalType d);
Data_Object create_object_BOOL(Byte b);
inline void clear_data(Data_Object* object);
char* get_object_cstring(Data_Object* stringObject);

// Register file functions
void initialize_register_file(Register_File* rFile);
///Data_Object* access_register(Byte reg, Register_File* rf);
const Data_Object* read_register(Byte reg, Register_File* rf);
void write_register(Byte reg, Register_File* rf, const Data_Object* data);
void write_default_output(const Data_Object* object, Register_File* rf);
void push_pc(PCType* pc_entry, Register_File* rf);
PCType pop_pc(Register_File* rf);



#endif