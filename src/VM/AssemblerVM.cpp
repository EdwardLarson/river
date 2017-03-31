#include <iostream>
#include <fstream>
#include <vector>

#include "VM.h"
#include "Assembler.h"

#define INTEGRATE_ASSEMBLER 1
#define INTEGRATE_VM 1


int main(int argc, char** argv){
	if (argc < 2){
		std::cout << "Usage: runassembly <assembly file>" << std::endl;
		return 1;
	}
	
	std::ifstream inStream(argv[1]);
	if (!inStream.is_open()){
		std::cout << "Unable to open " << argv[1] << " for reading" << std::endl;
	}
	
	Assembler assembler(inStream);
	
	if (assembler.assemble()){
		PCType progLength = (PCType) assembler.get_bytecode().size();
		const Byte* bytecodeBegin = &(assembler.get_bytecode()[0]);
		execute(bytecodeBegin, &progLength);
	}
	
	
	return 0;
}
