#include "VM.h"
#include <iostream>
#include <fstream>
#include <map>
#include <stack>

#define LONGEST_INSTRUCTION_LENGTH 9

/* SAMPLE ASSEMBLY
sum digits 1-10 and print the results

0:	SETDO $a
1:	MOVE $zero 			;-> $a
2:	LOAD 1 > $b
	:LOOP
3:	ADD $a, $b			;-> $a
4:	ADD 1 > $b
5:	LT $b, 11 > $c
6:	BRANCH $c :LOOP
7:	PRINT $a
*/

/*
	'$' : register
	'#' : persistent register
	'@' : address
*/

class RegisterAllocater{
private:
	typedef std::map<std::string, Byte> RegisterMap;
	RegisterMap* mappings;
	std::stack<RegisterMap*> mappingStack;
	Byte lastAllocated;
	
public:
	RegisterAllocater(): lastAllocated(0){
	}
	
	Byte get_register(const std::string& identifier){
		RegisterMap::iterator registerLocation = this->mappings->find(identifier);
		if (registerLocation == this->mappings->end()){
			// add new mapping
			++(this->lastAllocated);
			if (this->lastAllocated >= NUM_REGISTERS - NUM_PERSISTENT_REGISTERS){
				// out of non-persistent registers to allocate!
				std::cerr << "ERROR: Not enough registers to allocate" << std::endl;
				return 0;
			}
			
			this->mappings->insert(std::pair<std::string, Byte>(identifier, lastAllocated));
			return this->lastAllocated;
		}else{
			return registerLocation->second;
		}
	}
	
	void push_frame(){
		this->mappingStack.push(this->mappings);
		this->mappings = new RegisterMap;
	}
	
	void pop_frame(){
		delete this->mappings;
		this->mappings = this->mappingStack.top();
		mappingStack.pop();
	}
};

typedef enum {
	VALUE,
	REGISTER,
	REGISTER_P,
	ADDRESS,
	LABEL
} ArgType;

typedef struct {
	ArgType type;
	std::string arg;
} Argument;

void assemble(std::ifstream& inStream, std::ofstream& outStream);
PCType assemble_instruction(std::string& instruction, Byte*& byteOutput, RegisterAllocater& ra, RegisterAllocater& raPersistent);
Byte get_opcode_hex(std::string& opcode);
void read_identifier(const std::string& instruction, std::string& identifier, unsigned int& index);
Byte get_funct(Byte opcodeShifted, Argument* arguments, unsigned char argCounter, const std::string& subfunction);

Argument read_argument(const std::string& instruction, unsigned int& index);

#ifndef INTEGRATE_ASSEMBLER

int main(int argc, char** argv){
	if (argc < 3){
		return 1;
	}
	
	std::ifstream inStream(argv[1]);
	std::ofstream outStream(argv[2], std::ios::binary);
			
	if (!inStream.is_open()){
		std::cerr << "Unable to open file " << argv[1] << " for reading!" << std::endl;
	}else if (!outStream.is_open()){
		std::cerr << "Unable to open file " << argv[2] << " for writing!" << std::endl;
	}else{
		assemble(inStream, outStream);
		
		outStream.close();
		inStream.close();
	}
}

#endif

void assemble(std::ifstream& inStream, std::ofstream& outStream){
	std::string line;
	Byte* instructionByteStream;
	PCType instructionByteLength;
	
	RegisterAllocater ra;
	RegisterAllocater raPersistent;
	
	while (std::getline(inStream, line)){
		std::cout << line << std::endl;
		
		//instructionByteLength = assemble_instruction(line, instructionByteStream, ra, raPersistent);
		
		//outStream.write((char*) instructionByteStream, instructionByteLength);
	}
}

PCType assemble_instruction(std::string& instruction, Byte*& byteOutput, RegisterAllocater& ra, RegisterAllocater& raPersistent){
	Byte opcodeHex = 0x00;
	Byte functHex = 0x00;
	Byte returnBitHex = 0x00;
	
	PCType byteLength;
	
	// need to know if arguments go:
	//  G, G -> funct = 0
	//  G, V -> funct = 1
	//  V, G -> funct = 2
	
	std::string identifierTmp;
	Argument arguments[4]; // 4 arguments max
	unsigned char argCounter = 0;
	
	Argument returnArg;
	
	
	std::string opcode;
	std::string subfunction;
	
	enum {INSTRUCTION, SUBFUNCTION, ARGUMENTS, RETURNS} state = INSTRUCTION;
	
	for (unsigned int i = 0; i < instruction.length(); i++){
		if (instruction[i] == ';'){
			// comment
			break;
		}else if(instruction[i] == ':'){
			// label
		}
		
		switch(state){
		case INSTRUCTION:
			if (instruction[i] == ' '){
				state = ARGUMENTS;
			}else if(instruction[i] == '_'){
				state = SUBFUNCTION;
			}else{
				opcode += instruction[i];
				break;
			}
			
			opcodeHex = get_opcode_hex(opcode);
			if (opcodeHex == (0x12 << 2)){ // POPFRAME
				ra.pop_frame();
			}else if (opcodeHex == (0x13 << 2)){ // PUSHFRAME
				ra.push_frame();
			}
			
			break;
			
		case SUBFUNCTION:
			if (instruction[i] == ' '){
				state = ARGUMENTS;
			}else{
				subfunction += instruction[i];
			}
			break;
			
		case ARGUMENTS:
			if (instruction[i] == '>'){
				state = RETURNS;
			}else if (instruction[i] != ' '){
				arguments[argCounter] = read_argument(instruction, i);
				++argCounter;
			}
			/*
			}else if (instruction[i] == '$'){
				//register variable
				read_identifier(instruction, identifierTmp, i);
				Byte reg = ra.get_register(identifierTmp);
				
			}else if (instruction[i] == '#'){
				//persistent register
				read_identifier(instruction, identifierTmp, i);
				Byte reg = raPersistent.get_register(identifierTmp);
				
			}else if (instruction[i] == ' '){
				//end of an argument
				continue;
			}else{
				//part of an argument
			}*/
			break;
		case RETURNS:
			if(instruction[i] != ' '){
				// reach register variable
				returnArg = read_argument(instruction, i);
				//read_identifier(instruction, identifierTmp, i);
			}
			break;
		}
	}
	
	
	if (returnArg.type == REGISTER || returnArg.type == REGISTER_P){
		returnBitHex = 0x80;
	}else{
		returnBitHex = 0x00;
	}
	
	Byte instruction_hex = opcodeHex & functHex & returnBitHex;
}

Byte get_opcode_hex(std::string& opcode){
		
	if(opcode == "ABS"){
		return 0x00 << 2;
	}
	else if(opcode == "ADD"){
		return 0x01 << 2;
	}
	else if(opcode == "AND"){
		return 0x02 << 2;
	}
	else if(opcode == "BRANCH"){
		return 0x03 << 2;
	}
	else if(opcode == "CALL"){
		return 0x04 << 2;
	}
	else if(opcode == "DIV"){
		return 0x05 << 2;
	}
	else if(opcode == "EQ"){
		return 0x06 << 2;
	}
	else if(opcode == "GT"){
		return 0x07 << 2;
	}
	else if(opcode == "HALT"){
		return 0x08 << 2;
	}
	else if(opcode == "JUMP"){
		return 0x09 << 2;
	}
	else if(opcode == "LOAD"){
		return 0x0A << 2;
	}
	else if(opcode == "LSH"){
		return 0x0B << 2;
	}
	else if(opcode == "LT"){
		return 0x0C << 2;
	}
	else if(opcode == "MOD"){
		return 0x0D << 2;
	}
	else if(opcode == "MOVE"){
		return 0x0E << 2;
	}
	else if(opcode == "MUL"){
		return 0x0F << 2;
	}
	else if(opcode == "NOT"){
		return 0x10 << 2;
	}
	else if(opcode == "OR"){
		return 0x11 << 2;
	}
	else if(opcode == "POPFRAME"){
		return 0x12 << 2;
	}
	else if(opcode == "PUSHFRAME"){
		return 0x13 << 2;
	}
	else if(opcode == "POW"){
		return 0x14 << 2;
	}
	else if(opcode == "PRINT"){
		return 0x15 << 2;
	}
	else if(opcode == "RETURN"){
		return 0x16 << 2;
	}
	else if(opcode == "RSH"){
		return 0x17 << 2;
	}
	else if(opcode == "SETDO"){
		return 0x18 << 2;
	}
	else if(opcode == "XOR"){
		return 0x19 << 2;
	}else{
		return 0x08 << 2; //HALT instruction
	}
}

void read_identifier(const std::string& instruction, std::string& identifier, unsigned int& index){
	while(instruction[index] != ' '){
		identifier += instruction[index];
		++index;
	}
	--index; // pull index away from the space after the identifier
}

Argument read_argument(const std::string& instruction, unsigned int& index){
	Argument argument;
	
	switch(instruction[0]){
	case '$':
		argument.type = REGISTER;
		break;
	case '#':
		argument.type = REGISTER_P;
		break;
	case '@':
		argument.type = ADDRESS;
		break;
	case ':':
		argument.type = LABEL;
		break;
	default:
		argument.type = VALUE;
	}
	
	while(instruction[index] != ' '){
		argument.arg += instruction[index];
		++index;
	}
	--index; // pull index away from the space after the identifier
	
	return argument;
}

Byte get_funct(Byte opcodeShifted, Argument* arguments, unsigned char argCounter, const std::string& subfunction){
	
	switch(opcodeShifted >> 2){
		case POPFRAME:
		case PUSHFRAME:
		case HALT:
		case RETURN:
		
			if (argCounter != 0)
				std::cerr << "Incorrect number of arguments!" << std::endl;
			
			return 0x00;
			break;
			
		case SETDO:
		case ABS:
		case MOVE:
		case NOT:
		case INPUT:
			if (argCounter != 1)
				std::cerr << "Incorrect number of arguments!" << std::endl;
			
			return 0x00;
			break;
			
		case ADD:
		case AND:
		case DIV:
		case EQ:
		case GT:
		case LSH:
		case LT:
		case MOD:
		case MUL:
		case OR:
		case POW:
		case RSH:
		case SUB:
		case XOR:
			if ((arguments[0].type == REGISTER || arguments[0].type == REGISTER_P) 
					&& (arguments[1].type == REGISTER || arguments[1].type == REGISTER_P)){ // GG
				return 0x00;
			}else if ((arguments[0].type == REGISTER || arguments[0].type == REGISTER_P) 
					&& (arguments[1].type == VALUE)){ // GV
				return 0x01;
			}else if ((arguments[0].type == VALUE) 
					&& (arguments[1].type == REGISTER || arguments[1].type == REGISTER_P)){ // VG
				return 0x02;
			}else{
				std::cerr << "Incorrect argument types!" << std::endl;
				return 0x00;
			}
			break;
		
		case MLOAD:
		case MSTORE:
			if (subfunction == "D"){
				if (argCounter == 2)
					return 0x02;
				else if (argCounter == 3)
					return 0x03;
				else{
					std::cerr << "Incorrect number of arguments!" << std::endl;
					return 0x00;
				}
			}else{
				if (argCounter == 2)
					return 0x00;
				else if (argCounter == 3)
					return 0x01;
				else{
					std::cerr << "Incorrect number of arguments!" << std::endl;
					return 0x00;
				}
			}
			break;
		
		case BRANCH:
			if (argCounter != 2)
				std::cerr << "Incorrect number of arguments!" << std::endl;
			
			if (subfunction == "T"){
				return 0x00;
			}else if (subfunction == "F"){
				return 0x01;
			}else{
				return 0x00;
			}
		case CALL:
			return 0x00;
			break;
		case JUMP:
			if (argCounter != 1)
				std::cerr << "Incorrect number of arguments!" << std::endl;
			
			if (subfunction == "L"){
				return 0x01;
			}else{
				return 0x00;
			}
		case LOAD:
			if (argCounter != 2)
				std::cerr << "Incorrect number of arguments!" << std::endl;
			
			return 0x00;
			break;
		case MALLOC:
			return 0x00;
			break;
		case MFREE:
			return 0x00;
			break;
		case PRINT:
			if (subfunction == "D"){
				return 0x02;
			}else{
				if (argCounter != 1)
					std::cerr << "Incorrect number of arguments!" << std::endl;
				
				if (arguments[0].type == REGISTER || arguments[0].type == REGISTER_P)
					return 0x00;
				else if (arguments[0].type == VALUE)
					return 0x01;
			}
			break;
		
	}
}