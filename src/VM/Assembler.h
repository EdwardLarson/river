#ifndef ASSEMBLER_H
#define ASSEMBLER_H

#include <iostream>
#include <fstream>
#include <vector>
#include <map>
#include <stack>
#include <list>

#include "VM.h"

#define LONGEST_INSTRUCTION_LENGTH 9

#define ASSEMBLER_VERSION_MAIN	0
#define ASSEMBLER_VERSION_SUB	4

/* ASSEMBLER
* Assembling object which reads a file containing
* some assembly code and converts it into bytecode,
* saved to an output stream. Contains all necessary
* state for assembling.
* 
* argument delimiters
* '$' : local register
* '#' : pass register (args or returns)
* '@' : global register
* ':' for labels
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
	A_VALUE,
	A_REGISTER,
	A_REGISTER_P,
	A_ADDRESS,
	A_STRING,
	A_LABEL,
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
	Assembler(std::istream& instrm);
	
	bool assemble();
	const std::vector<Byte>& get_bytecode(){return byteVec;};
	
	void set_log(bool enableLogging){log = enableLogging;};
	
private:
	typedef enum {AERROR_NONE, AERROR_WRONGARGS, AERROR_ARGTYPES, AERROR_NOPCODE, AERROR_REGLIMIT, AERROR_BADRET, AERROR_REGBOUNDS} AssemblerError;
	// for each label, stores the actual pointer for that label and a list of every instruction which references this label
	typedef std::map<std::string, std::pair<PCType, std::list<PCType> > > LabelMap; 
	
	
	AssemblerError error;
	
	bool log;

	std::istream& inStream;
	std::vector<Byte> byteVec;
	
	RegisterMap mapping;
	RegisterMap persistent;
	std::stack<RegisterMap> mappingStack;
	LabelMap labelMap;
	
	Byte nextFree;
	Byte nextFreePersistent;
	std::stack<Byte> freeStack;
	
	PCType lastOpcodePosition;
	
	//FUNCTIONS
	
	// assembler state functions
	Byte get_register(const std::string& identifier, bool persistent=false);
	void define_label(const std::string& label); ///
	void add_label_ref(const std::string& label);
	void push_frame();
	void pop_frame();
	
	// output/writing functions
	void write_byte(Byte b); ///
	void assemble_instruction(const std::string& instruction);
	void assemble_argument(const Argument& arg);
	void assemble_labels();
	void recordOpcodePosition(); ///
	
	// input/reading functions
	bool assemble_line(const std::string& line); ///
	Byte convert_register(const std::string& registerName); ///
	
	Byte 					read_opcode(const std::string& instruction, unsigned int& index, char& subfunction);
	std::vector<Argument>	read_args(const std::string& instruction, unsigned int& index);
	Argument 				read_argument(const std::string& instruction, unsigned int& index);
	Byte 					read_returns(const std::string& instruction, unsigned int& index, Byte& returnReg);
	Data_Object 			read_literal(const std::string& literal); ///
	
	// utility functions
	Byte get_opcode_hex(const std::string& opcode) const; ///
	Byte get_funct(Byte opcodeShifted, std::vector<Argument> args, char subfunction);
	unsigned int char_to_int(char c);
	unsigned int string_to_int(const std::string& s);
};

#endif