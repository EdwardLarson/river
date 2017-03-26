#include <iostream>
#include <fstream>
#include <map>
#include <stack>

#include "VM.h"
#include "Assembler.h"

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
		Assembler a(inStream, outStream);
		a.assemble();
		
		std::cout << "Wrote " << (outStream.tellp() * sizeof(Byte)) << " bytes to " << argv[2] << std::endl;
		outStream.close();
		inStream.close();
	}
}

#endif

Assembler::Assembler(std::istream& instrm, std::ostream& outstrm): inStream(instrm), outStream(outstrm){
	nextFree = 0;
	nextFreePersistent = 0;
	
	// add mapping to persistent register 0 for "$zero"
	std::cout << "\tzero reg: " << (int) get_register("zero") << std::endl;
}

// Assemble the given input into bytecode. Outputs to and closes given output file.
void Assembler::assemble(){
	
	error = AERROR_NONE;
	unsigned int currLine = 1;
	
	std::string line;
	while (std::getline(inStream, line)){
		assemble_instruction(line);
		
		switch(error){
		case AERROR_NONE:
			std::cout << "line assemby success" << std::endl;
			break;
		case AERROR_WRONGARGS:
			std::cout << "error [" << currLine << "]: instruction has wrong args" << std::endl;
			break;
		case AERROR_NOPCODE:
			std::cout << "error [" << currLine << "]: opcode not found" << std::endl;
			break;
		case AERROR_REGLIMIT:
			std::cout << "error [" << currLine << "]: not enough registers to allocate" << std::endl;
			break;
		case AERROR_BADRET:
			std::cout << "error [" << currLine << "]: bad return type" << std::endl;
			break;
		case AERROR_ARGTYPES:
			std::cout << "error [" << currLine << "]: incorrect argument types" << std::endl;
			break;
		}
		
		error = AERROR_NONE;
		
		++currLine;
	}
}

void Assembler::assemble_instruction(const std::string& instruction){
	unsigned int index = 0;
	
	std::cout << std::hex;
	
	std::cout << "instruction: " << instruction << std::endl; ///DEBUG 
	
	char subfunction = ' ';
	Byte opcode = read_opcode(instruction, index, subfunction);
	
	std::cout << "opcode: " << (int) opcode << std::endl; ///DEBUG 
	
	std::vector<Argument> args;
	read_args(instruction, index, args);
	
	std::cout << "checkpoint: args read" << std::endl; ///DEBUG 
	
	Byte funct = get_funct(opcode, args, subfunction);
	
	std::cout << "funct: " << (int) funct << std::endl; ///DEBUG 
	
	Byte returnReg = 0;
	Byte returnBit = read_returns(instruction, index, returnReg);
	
	std::cout << "return bit: " << (int) returnBit << std::endl; ///DEBUG 
	
	// compile full opcode and output
	// opcode format: <rooo-ooff>
	Byte fullOpcode = returnBit | opcode | funct;
	
	std::cout << "full opcode: " << (int) fullOpcode << std::endl; ///DEBUG 
	
	outStream << std::hex << fullOpcode;
	
	// compile arguments into bytes and output
	for (unsigned int i = 0; i < args.size(); i++){
		assemble_argument(args[i]);
	}
	
	// add return argument to tail of instruction
	if (returnBit){
		outStream << std::hex << returnReg;
	}
}

Byte Assembler::get_register(const std::string& identifier, bool isPersistent){
	
	if (isPersistent){
		if (nextFreePersistent == NUM_REGISTERS - 1){
			//std::cerr << "error: no more registers to allocate" << std::endl;
			error = AERROR_REGLIMIT;
			return 0;
		}
	}else{
		// return 0 if no more registers to allocate
		if (nextFree == NUM_REGISTERS - NUM_PERSISTENT_REGISTERS){
			//std::cerr << "error: no more registers to allocate" << std::endl;
			error = AERROR_REGLIMIT;
			return 0;
		}
	}
	
	std::pair<std::string, Byte> entry(identifier, nextFree);
	std::pair<RegisterMap::iterator, bool> search;
	
	if (isPersistent){
		search = persistent.insert(entry);
	}else{
		search = mapping.insert(entry);
	}
	
	if (search.second){
		// entry inserted
		
		// increment nextFree and return the allocated register
		
		std::cout << "\tallocated new register " << identifier << std::endl;
		
		if (isPersistent){
			++nextFreePersistent;
			return nextFree - 1;
		}else{
			++nextFree;
			return nextFree - 1;
		}
	}else{
		// entry not inserted
		
		// return the previously allocated register
		std::cout << "\tidentified old register " << identifier << std::endl;
		return search.first->second;
	}
}

void Assembler::push_frame(){
	mappingStack.push(mapping);
	mapping.clear();
	freeStack.push(nextFree);
	nextFree = 0;
}

void Assembler::pop_frame(){
	mapping = mappingStack.top();
	mappingStack.pop();
	
	nextFree = freeStack.top();
	freeStack.pop();
}

Byte Assembler::read_opcode(const std::string& instruction, unsigned int& index, char& subfunction){
	std::string opcodeRaw = "";
	
	for ( ; instruction[index] != ' '; ++index){
		if (instruction[index] == '_'){
			subfunction = instruction[index + 1];
		}
		opcodeRaw += instruction[index];
	}
	
	Byte opcode = get_opcode_hex(opcodeRaw);
	
	if (opcode == 0xFF){
		error = AERROR_NOPCODE;
	}
	return opcode;
}

void Assembler::read_args(const std::string& instruction, unsigned int& index, std::vector<Argument>& args){
	while (index < instruction.length() && instruction[index] != '>'){
		
		if (instruction[index] != ' '){
			args.push_back(read_argument(instruction, index));
		}
		
		++index;
	}
}

Assembler::Argument Assembler::read_argument(const std::string& instruction, unsigned int& index){
	Argument argument;
	
	switch(instruction[index]){
	case '$':
		argument.type = REGISTER;
		++index;
		break;
	case '#':
		argument.type = REGISTER_P;
		++index;
		break;
	case '@':
		argument.type = ADDRESS;
		++index;
		break;
	case ':':
		argument.type = LABEL;
		++index;
		break;
	default:
		argument.type = VALUE;
	}
	
	while(index < instruction.length() && instruction[index] != ' '){
		argument.arg += instruction[index];
		++index;
	}
	//--index; // pull index away from the space after the identifier
	
	std::cout << "argument " << argument.arg << " of type " << argument.type << std::endl; /// DEBUG
	
	return argument;
}

Byte Assembler::read_returns(const std::string& instruction, unsigned int& index, Byte& returnReg){
	// iterate index to the beginning of the argument
	for (; index < instruction.length() && (instruction[index] == ' ' || instruction[index] == '>'); ++index){
		// return on finding a comment
		
		if (instruction[index] == ';'){
			return 0x00;
		}
	}
	
	// check for a no-argument case
	if (index >= instruction.length()){
		return 0x00;
	}else{
	}
	
	Argument returnArg = read_argument(instruction, index);
	if (!(returnArg.type == REGISTER || returnArg.type == REGISTER_P)){
		// bad return type error
		error = AERROR_BADRET;
	}else{
		returnReg = get_register(returnArg.arg);
	}
	
	return 0x80;
}

Data_Object Assembler::read_literal(const std::string& literal){
	Data_Object object;
	object.type = INTEGER;
	
	if (literal == "NIL"){
		object.type = NIL;
		return object;
	}
	
	IntegerType tmpInt = 0;
	RationalType tmpRat = 0;
	unsigned int decimal = 0;
	
	for (unsigned int i = 0; i < literal.length(); i++){
		if (literal[i] == '.'){
			object.type = RATIONAL;
			decimal = i;
		}
		
		switch(object.type){
		case INTEGER:
			tmpInt *= 10;
			tmpInt += (IntegerType) char_to_int(literal[i]);
			break;
		case RATIONAL:
			tmpRat *= 10;
			tmpRat += (RationalType) char_to_int(literal[i]);
			break;
		default:
			break;
		}
	}
	
	if (object.type == RATIONAL){
		RationalType denom = 1;
		for (unsigned int i = 0; i < decimal; i++, denom *= 10);
		
		tmpRat /= denom;
		tmpRat += (RationalType) tmpInt;
		
		object.data.d = tmpRat;
	}else{
		object.data.n = tmpInt;
	}
	
	return object;
}

void Assembler::assemble_argument(const Argument& arg){
	if (arg.type == VALUE){
		Data_Object_Cast_Union castUnion;
		castUnion.data = read_literal(arg.arg);
		
		std::cout << "created Data_Object {type=" << castUnion.data.type << ", data=" << castUnion.data.data.n << "} for literal " << arg.arg << std::endl; ///DEBUG
		
		for (unsigned int i = 0; i < DATA_OBJECT_SIZE; i++){
			outStream << castUnion.bytes[i];
		}
		
	}else if (arg.type == REGISTER){
		Byte reg = get_register(arg.arg);
		
		if (error != AERROR_REGLIMIT){
			std::cout << "\tsuccessfully mapped " << arg.arg << " to register " << (int) reg << std::endl; /// DEBUG
			outStream << reg;
		}
	}else if(arg.type == REGISTER_P){
		Byte reg = get_register(arg.arg, true);
		if (error != AERROR_REGLIMIT){
			std::cout << "\tsuccessfully mapped " << arg.arg << " to persistent register " << (int) reg << std::endl; /// DEBUG
			outStream << reg;
		}
	}
}

Byte Assembler::get_opcode_hex(std::string& opcode) const{
		
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
	else if(opcode == "INPUT"){
		return 0x09 << 2;
	}
	else if(opcode == "JUMP"){
		return 0x0A << 2;
	}
	else if(opcode == "LOAD"){
		return 0x0B << 2;
	}
	else if(opcode == "LSH"){
		return 0x0C << 2;
	}
	else if(opcode == "LT"){
		return 0x0D << 2;
	}
	else if(opcode == "MALLOC"){
		return 0x0E << 2;
	}
	else if(opcode == "MFREE"){
		return 0x0F << 2;
	}
	else if(opcode == "MLOAD"){
		return 0x10 << 2;
	}
	else if(opcode == "MOD"){
		return 0x11 << 2;
	}
	else if(opcode == "MOVE"){
		return 0x12 << 2;
	}
	else if(opcode == "MSTORE"){
		return 0x13 << 2;
	}
	else if(opcode == "MUL"){
		return 0x14 << 2;
	}
	else if(opcode == "NOT"){
		return 0x15 << 2;
	}
	else if(opcode == "OR"){
		return 0x16 << 2;
	}
	else if(opcode == "POPFRAME"){
		return 0x17 << 2;
	}
	else if(opcode == "PUSHFRAME"){
		return 0x18 << 2;
	}
	else if(opcode == "POW"){
		return 0x19 << 2;
	}
	else if(opcode == "PRINT"){
		return 0x1A << 2;
	}
	else if(opcode == "RETURN"){
		return 0x1B << 2;
	}
	else if(opcode == "RSH"){
		return 0x1C << 2;
	}
	else if(opcode == "SETDO"){
		return 0x1D << 2;
	}
	else if(opcode == "SUB"){
		return 0x1E << 2;
	}
	else if(opcode == "XOR"){
		return 0x1F << 2;
	}else{
		return 0xFF;
	}
}

Byte Assembler::get_funct(Byte opcodeShifted, std::vector<Argument> args, char subfunction){
	
	switch(opcodeShifted >> 2){
	case POPFRAME:
	case PUSHFRAME:
	case HALT:
	case RETURN:
	
		if (args.size() != 0){
			std::cout << "\tA" << std::endl;
			error = AERROR_WRONGARGS;
		}
		
		return 0x00;
		break;
		
	case SETDO:
	case ABS:
	case MOVE:
	case NOT:
	case INPUT:
		if (args.size() != 1){
			error = AERROR_WRONGARGS;
			std::cout << "\tB" << std::endl;
		}
		
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
		if ((args[0].type == REGISTER || args[0].type == REGISTER_P) 
				&& (args[1].type == REGISTER || args[1].type == REGISTER_P)){ // GG
			return 0x00;
		}else if ((args[0].type == REGISTER || args[0].type == REGISTER_P) 
				&& (args[1].type == VALUE)){ // GV
			return 0x01;
		}else if ((args[0].type == VALUE) 
				&& (args[1].type == REGISTER || args[1].type == REGISTER_P)){ // VG
			return 0x02;
		}else{
			std::cout << "\tB" << std::endl;
			error = AERROR_WRONGARGS;
			return 0x00;
		}
		break;
	
	case MLOAD:
	case MSTORE:
		if (subfunction == 'D'){
			if (args.size() == 2)
				return 0x02;
			else if (args.size() == 3)
				return 0x03;
			else{
				error = AERROR_WRONGARGS;
				std::cout << "\tD" << std::endl;
				return 0x00;
			}
		}else{
			if (args.size() == 2)
				return 0x00;
			else if (args.size() == 3)
				return 0x01;
			else{
				std::cout << "\tE" << std::endl;
				error = AERROR_WRONGARGS;
				return 0x00;
			}
		}
		break;
	
	case BRANCH:
		if (args.size() != 2){
			std::cout << "\tF" << std::endl;
			error = AERROR_WRONGARGS;
		}
		
		if (subfunction == 'T'){
			return 0x00;
		}else if (subfunction == 'F'){
			return 0x01;
		}else{
			return 0x00;
		}
	case CALL:
		return 0x00;
		break;
	case JUMP:
		if (args.size() != 1){
			std::cout << "\tG" << std::endl;
			error = AERROR_WRONGARGS;
		}
		
		if (subfunction == 'L'){
			return 0x01;
		}else{
			return 0x00;
		}
	case LOAD:
		if (args.size() != 1){
			std::cout << "\tH" << std::endl;
			error = AERROR_WRONGARGS;
		}
		
		return 0x00;
		break;
	case MALLOC:
		return 0x00;
		break;
	case MFREE:
		return 0x00;
		break;
	case PRINT:
		if (subfunction == 'D'){
			return 0x02;
		}else{
			if (args.size() != 1){
				std::cout << "\tI" << std::endl;
				error = AERROR_WRONGARGS;
			}
			
			if (args[0].type == REGISTER || args[0].type == REGISTER_P)
				return 0x00;
			else if (args[0].type == VALUE)
				return 0x01;
		}
		break;
	default:
		return 0x00;
	}
}

unsigned int Assembler::char_to_int(char c){
	return ((unsigned int) c) - 48;
}
