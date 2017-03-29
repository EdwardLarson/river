#include <iostream>
#include <fstream>
#include <string>

#include "VM/VM.h"
#include "VM/Assembler.h"
#include "Lexer.h"

#define RIVER_VERSION_MAIN	0
#define RIVER_VERSION_SUB	1

int main(int argc, char** argv){
	
	if (argc == 1){
		std::cout << "River version " << RIVER_VERSION_MAIN << ":" << RIVER_VERSION_SUB << std::endl
			<< "By Edward Larson 2017" << std::endl
			<< "Type \"river help\" for available commands." << std::endl;
			
		return 0;
	}
	
	std::string funct = argv[1];
	
	if (funct == "version"){
		std::cout << "River version " << RIVER_VERSION_MAIN << ":" << RIVER_VERSION_SUB << std::endl;
	}else if (funct == "help"){
		
	}else if (funct == "lex"){
		// -i <input file(s)> [-o <output file> -log]
		
		std::ifstream inStream;
		std::ostream& outStream = std::cout;
		Lexer lexer('\t');
		
		bool good = false;
		bool log = false;
		
		for (unsigned int i = 2; i < argc; i++){
			std::cout << i << "th arg " << argv[i] << std::endl;
			if (argv[i] == "-i"){ std::cout << "AYY" << std::endl; if (i < argc - 1){
				good = true;
				inStream.open(argv[i + 1]);
				std::cout << "attempted to open " << argv[i + 1] << std::endl;
				
			}else if (argv[i] == "-o" && i < argc - 1){
				//std::ofstream ostr(argv[i + 1]);
				//outStream = ostr;
			}else if (argv[i] == "-log"){
				log = true;
			}}
		}
	
		std::cout << "args processed" << std::endl;
		
		if (good && inStream.good()){
			std::string line;
			
			std::cout << "both good" << std::endl;
				
			while(getline(inStream, line)){
				lexer.interpret_line(line);
			}
			lexer.finish();
			
			std::list<TokenPair>& tknStream = lexer.get_token_stream();
			
			for (	std::list<TokenPair>::const_iterator 
					iter = tknStream.begin(); iter != tknStream.end(); ++iter){
				
				outStream << iter->type << ": " << iter->token << std::endl;
			}
		}
		
	}else if (funct == "assemble"){
		// -i <input file(s)> [-o <output file> -log]
	}else if (funct == "run"){
		// -i <input file(s)> [-safe -log]
	}else if (funct == "runassembly"){
		// -i <input file(s)> [-o <assembly output file> -log]
	}else if (funct == "compile"){
		// -i <input file(s)> [-o <output file> -op <parse output file> -oa <assembly output file> -log]
		std::cout << "Currently unimplemented" << std::endl;
	}else{
		std::cout << "Unknown command. Type \"river help\" for available commands." << std::endl;
	}
	
	
	
	
	return 0;
}