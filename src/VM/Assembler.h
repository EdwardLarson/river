#ifndef ASSEMBLER_H
#define ASSEMBLER_H

#include <iostream>
#include <fstream>
#include <vector>
#include <map>
#include <stack>

#include "VM.h"

#define LONGEST_INSTRUCTION_LENGTH 9

/* ASSEMBLER
* Assembling object which reads a file containing
* some assembly code and converts it into bytecode,
* saved to an output stream. Contains all necessary
* state for assembling.
* 
* argument delimiters
* '$' : register
* '#' : persistent register
* future implementation: ':' for labels
* 
* sample assembly
* sum digits 1-10 and print the results
* 
* 0:	SETDO $a
* 1:	MOVE $zero 			;-> $a
* 2:	LOAD 1 > $b
* 3:	ADD $a, $b			;-> $a
* 4:	ADD 1 > $b
* 5:	LT $b, 11 > $c
* 6:	BRANCH $c :3
* 7:	PRINT $a
*/

class Assembler{
	
private: typedef std::map<std::string, Byte> RegisterMap;
private: typedef enum {
	VALUE,
	REGISTER,
	REGISTER_P,
	ADDRESS,
	LABEL
} ArgType;
private: typedef struct {
	ArgType type;
	std::string arg;
} Argument;

private: typedef union {
	Data_Object data;
	Byte bytes[DATA_OBJECT_SIZE];
} Data_Object_Cast_Union;
	
public:
	Assembler(std::istream& instrm, std::vector<Byte>* byteVector);
	Assembler(std::istream& instrm, std::ostream& outstrm);
	
	bool assemble();
	
private:
	typedef enum {AERROR_NONE, AERROR_WRONGARGS, AERROR_ARGTYPES, AERROR_NOPCODE, AERROR_REGLIMIT, AERROR_BADRET} AssemblerError;
	typedef enum {AOUT_OSTREAM, AOUT_VECTOR} OutputType;
	
	AssemblerError error;
	const OutputType outType;

	std::istream& inStream;
	std::ostream& outStream;
	std::vector<Byte>* byteVec;
	
	RegisterMap mapping;
	RegisterMap persistent;
	std::stack<RegisterMap> mappingStack;
	
	Byte nextFree;
	Byte nextFreePersistent;
	std::stack<Byte> freeStack;
	
	Byte get_register(const std::string& identifier, bool persistent=false);
	void push_frame();
	void pop_frame();
	
	void assemble_instruction(const std::string& instruction);
	Byte read_opcode(const std::string& instruction, unsigned int& index, char& subfunction);
	void read_args(const std::string& instruction, unsigned int& index, std::vector<Argument>& args);
	Argument read_argument(const std::string& instruction, unsigned int& index);
	Byte read_returns(const std::string& instruction, unsigned int& index, Byte& returnReg);
	Data_Object read_literal(const std::string& literal);
	void assemble_argument(const Argument& arg);
	
	void write_byte(Byte b);
	
	Byte get_opcode_hex(std::string& opcode) const;
	Byte get_funct(Byte opcodeShifted, std::vector<Argument> args, char subfunction);
	unsigned int char_to_int(char c);
};

#endif