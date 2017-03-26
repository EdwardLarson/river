#ifndef ASSEMBLER_H
#define ASSEMBLER_H

#include <fstream>
#include <vector>

#include "VM.h"

/* ASSEMBLER
* Assembling object which reads a file containing
* some assembly code and converts it into bytecode,
* saved to an output stream. Contains all necessary
* state for assembling.
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
	
public:
	Assembler(std::ifstream& instrm, std::ofstream& outstrm);
	
	void assemble();
	void assemble_instruction(const std::string& instruction);
private:
	typedef enum {AERROR_NONE, AERROR_WRONGARGS, AERROR_ARGTYPES, AERROR_NOPCODE, AERROR_REGLIMIT, AERROR_BADRET} AssemblerError;
	AssemblerError error;

	std::ifstream& inStream;
	std::ofstream& outStream;
	
	RegisterMap mapping;
	RegisterMap persisent;
	std::stack<RegisterMap> mappingStack;
	
	Byte nextFree;
	std::stack<Byte> freeStack;
	
	Byte get_register(const std::string& identifier);
	void push_frame();
	void pop_frame();
	
	Byte read_opcode(const std::string& instruction, unsigned int index, char& subfunction);
	void read_args(const std::string& instruction, unsigned int index, std::vector<Argument>& args);
	Argument read_argument(const std::string& instruction, unsigned int& index);
	Byte read_returns(const std::string& instruction, unsigned int index, Byte& returnReg);
	Data_Object read_literal(const std::string& literal);
	void assemble_argument(const Argument& arg);
	
	Byte get_opcode_hex(std::string& opcode) const;
	Byte get_funct(Byte opcodeShifted, std::vector<Argument> args, char subfunction);
	unsigned int char_to_int(char c);
};

#endif