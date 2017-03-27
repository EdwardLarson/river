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
	
	std::vector<Byte> byteVec;
	
	Assembler assembler(inStream, &byteVec);
	
	if (assembler.assemble()){
		//Byte* bytes;
		PCType progLength = (PCType) byteVec.size();
		
		std::cout << "Assembled " << progLength << " bytes, starting VM" << std::endl;
		
		execute(&(byteVec[0]), &progLength);
	}
	
	
	return 0;
}
