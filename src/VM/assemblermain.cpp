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
		Assembler a(inStream, outStream);
		a.assemble();
		
		//std::cout << "Wrote " << (outStream.tellp() * sizeof(Byte)) << " bytes to " << argv[2] << std::endl;
		//outStream.close();
		//inStream.close();
	}
	
	std::cout << "before end" << std::endl;
	
	return 0;
}