#include "Assembler.h"

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
		Assembler a(inStream);
		a.assemble();
		
		std::cout << std::hex;
		
		for (std::vector<Byte>::const_iterator iter = a.get_bytecode().begin(); iter != a.get_bytecode().end(); ++iter){
			outStream << (*iter);
			std::cout << int (*iter) << std::endl;
		}
		
		std::cout << std::dec;
		
		std::cout << std::dec << "Wrote " << (outStream.tellp() * sizeof(Byte)) << " bytes to " << argv[2] << std::endl;
		
		
		
		outStream.close();
		inStream.close();
	}
	
	std::cout << "before end" << std::endl;
	
	return 0;
}