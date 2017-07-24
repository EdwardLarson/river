#include <iostream>
#include <fstream>
#include <string>

#include "VM/VM.h"
#include "VM/Assembler.h"
#include "Lexer.h"

#define RIVER_VERSION_MAIN	0
#define RIVER_VERSION_SUB	1

bool read_flag_args(int argc, char** argv, int flagIndex, std::vector<std::string>& argsRead);

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
		std::cout << "Available commands:\n";
		
		std::cout << "lex -i <input file(s)> [-o <output file> -log]\n";
		std::cout << "assemble -i <input file(s)> [-o <output file> -log]\n";
		std::cout << "run -i <input file(s)> [-safe -log]\n";
		std::cout << "runassembly -i <input file(s)> [-o <assembly output file> -log]\n";
		std::cout << "compile -i <input file(s)> [-o <output file> -op <parse output file> -oa <assembly output file> -log]";
		std::cout << std::endl;
		
	}else if (funct == "lex"){
		// -i <input file(s)> [-o <output file> -log <logging file>]
		
		if (argc < 4){
			std::cout << "Usage: lex -i <input file(s)> [-o <output file> -log <logging file>]" << std::endl;
			return 0;
		}
		
		std::vector<std::string> inputFiles;
		std::ostream* outStream = &std::cout;
		
		bool givenInput = false;
		bool givenOutput = false;
		bool log = false;
		
		std::string currentArg;
		
		for (unsigned int i = 2; i < argc; i++){
			currentArg = argv[i];
			
			// read input files to a vector
			if (currentArg == "-i"){
				
				givenInput = read_flag_args(argc, argv, i, inputFiles);
				
			// read output file
			}else if (currentArg == "-o" && i < argc - 1){
				
				// attempt to create an output stream for the output argument
				std::ofstream* fileOut = new std::ofstream(argv[i + 1]);
				if (!(fileOut->good())){
					std::cout << "Error: Unable to open " << argv[i + 1] << " for editing" << std::endl;
					continue;
				}
				
				// delete outStream if an output argument previously created an output stream
				if (givenOutput) delete outStream;
				
				outStream = fileOut;
				givenOutput = true;
				
			}else if (currentArg == "-log"){
				log = true;
			}
		}
	
		///std::cout << "args processed" << std::endl;
		
		
		if (givenInput) {
			std::ifstream inStream;
			std::list<TokenPair> tknStream;
			
			// open and lex each input file
			for (std::vector<std::string>::const_iterator iter = inputFiles.begin(); iter != inputFiles.end(); ++iter){
				inStream.open(iter->c_str());
				if (!inStream.good()){
					std::cout << "Error: unable to open " << *iter << " for reading" << std::endl;
					continue;
				}
				
				Lexer lexer('\t');
				
				std::string line;
				while (getline(inStream, line)){
					lexer.interpret_line(line);
				}
				lexer.finish();
				inStream.close();
				
				// save all tokens lexed from this file to a tokenstream
				tknStream.insert(tknStream.end(), lexer.get_token_stream().begin(), lexer.get_token_stream().end());
			}
			
			// print tokenstream from all input files
			for (std::list<TokenPair>::const_iterator iter = tknStream.begin(); iter != tknStream.end(); ++iter){
				(*outStream) << iter->type << ": " << iter->token << '\n';
			}
			(*outStream) << std::endl;
			
		}
		
	}else if (funct == "assemble"){
		// -i <input file(s)> [-o <output file> -log <logging file>]
		
		if (argc < 4){
			std::cout << "Usage: assemble -i <input file(s)> [-o <output file> -log <logging file>]" << std::endl;
			return 0;
		}
		
		std::vector<std::string> inputFiles;
		std::ostream* outStream = &std::cout;
		
		bool givenInput = false;
		bool givenOutput = false;
		bool log = false;
		bool printBytes = false;
		
		std::string currentArg;
		
		for (unsigned int i = 2; i < argc; i++){
			currentArg = argv[i];
			if (currentArg == "-i"){
				givenInput = read_flag_args(argc, argv, i, inputFiles);
			}else if(currentArg == "-o" && i < argc - 1){
				
				// attempt to create an output stream for the output argument
				std::ofstream* fileOut = new std::ofstream(argv[i + 1]);
				if (!(fileOut->good())){
					std::cout << "Error: Unable to open " << argv[i + 1] << " for editing" << std::endl;
					continue;
				}
				
				// delete outStream if an output argument previously created an output stream
				if (givenOutput) delete outStream;
				
				outStream = fileOut;
				givenOutput = true;
			}else if(currentArg == "-log"){
				log = true;
			}else if(currentArg == "-print-bytes"){
				printBytes = true;
			}
		}
		
		///std::cout << "args processed" << std::endl;
		
		if (givenInput){
			std::ifstream inStream;
			std::vector<Byte> byteStream;
			
			// open and assemble each input file
			for (std::vector<std::string>::const_iterator iter = inputFiles.begin(); iter != inputFiles.end(); ++iter){
				inStream.open(iter->c_str());
				if (!inStream.good()){
					std::cout << "Error: unable to open " << *iter << " for reading" << std::endl;
					continue;
				}
				
				Assembler assembler(inStream);
				assembler.set_log(log);
				
				assembler.assemble();
				inStream.close();
				
				// save assembly from this file to a bytestream
				byteStream.insert(byteStream.end(), assembler.get_bytecode().begin(), assembler.get_bytecode().end());
			}
			
			if (printBytes) std::cout << std::hex;
			
			// print tokenstream from all input files
			for (std::vector<Byte>::const_iterator iter = byteStream.begin(); iter != byteStream.end(); ++iter){
				if (givenOutput) (*outStream) << *iter;
				if (printBytes) std::cout << (unsigned int) *iter << '-';
			}
			(*outStream) << std::endl; 
			if (printBytes) std::cout << std::dec << std::endl;
		}
	}else if (funct == "run"){
		// -i <input file(s)> [-safe -log <logging file>]
		
		if (argc < 4){
			std::cout << "Usage: run -i <input file(s)> [-safe -log <logging file>]" << std::endl;
			return 0;
		}
		
		std::vector<std::string> inputFiles;
		
		bool givenInput = false;
		bool log = false;
		
		std::string currentArg;
		
		for (unsigned int i = 2; i < argc; i++){
			currentArg = argv[i];
			if (currentArg == "-i"){
				givenInput = read_flag_args(argc, argv, i, inputFiles);
			}else if (currentArg == "-log"){
				log = true;
			}
		}
		
		///std::cout << "args processed" << std::endl;
		
		if (givenInput){
			std::ifstream inStream;
			
			// open and assemble each input file
			for (std::vector<std::string>::const_iterator iter = inputFiles.begin(); iter != inputFiles.end(); ++iter){
				inStream.open(iter->c_str(), std::ios::binary);
				if (!inStream.good()){
					std::cout << "Error: unable to open " << *iter << " for reading" << std::endl;
					continue;
				}
				
				inStream.seekg(0, inStream.end);
				// subtract 1 from the length to remove the endline character (used to find the end of the file)
				PCType progLength = inStream.tellg();
				progLength--;
				inStream.seekg(0, inStream.beg);
				
				Byte* byteStream = new Byte[progLength];
				
				inStream.read((char*) byteStream, progLength);
				
				execute(byteStream, &progLength, log);
				
				delete[] byteStream;
			}
		}
	}else if (funct == "runassembly"){
		// -i <input file(s)> [-o <assembly output file> -log <logging file>]
		
		if (argc < 4){
			std::cout << "Usage: assemble -i <input file(s)> [-o <output file> -log <logging file>]" << std::endl;
			return 0;
		}
		
		std::vector<std::string> inputFiles;
		std::ostream* outStream = NULL;
		
		bool givenInput = false;
		bool givenOutput = false;
		bool log = false;
		
		std::string currentArg;
		
		for (unsigned int i = 2; i < argc; i++){
			currentArg = argv[i];
			if (currentArg == "-i"){
				givenInput = read_flag_args(argc, argv, i, inputFiles);
			}else if(currentArg == "-o" && i < argc - 1){
				
				// attempt to create an output stream for the output argument
				std::ofstream* fileOut = new std::ofstream(argv[i + 1]);
				if (!(fileOut->good())){
					std::cout << "Error: Unable to open " << argv[i + 1] << " for editing" << std::endl;
					continue;
				}
				
				// delete outStream if an output argument previously created an output stream
				if (givenOutput) delete outStream;
				
				outStream = fileOut;
				givenOutput = true;
			}else if (currentArg == "-log"){
				log = true;
			}
		}
		
		///std::cout << "args processed" << std::endl;
		
		if (givenInput){
			std::ifstream inStream;
			std::vector<Byte> byteStream;
			
			// open and assemble each input file
			for (std::vector<std::string>::const_iterator iter = inputFiles.begin(); iter != inputFiles.end(); ++iter){
				inStream.open(iter->c_str());
				if (!inStream.good()){
					std::cout << "Error: unable to open " << *iter << " for reading" << std::endl;
					continue;
				}
				
				Assembler assembler(inStream);
				assembler.set_log(log);
				
				assembler.assemble();
				inStream.close();
				
				// save assembly from this file to a bytestream
				byteStream.insert(byteStream.end(), assembler.get_bytecode().begin(), assembler.get_bytecode().end());
			}
			
			// print tokenstream from all input files
			if (givenOutput){
				for (std::vector<Byte>::const_iterator iter = byteStream.begin(); iter != byteStream.end(); ++iter){
					(*outStream) << *iter;
				}
				(*outStream) << std::endl;
			}
			
			// run assembled bytestream
			PCType progLength = byteStream.size();
			execute(&byteStream[0], &progLength, log);
		}
	}else if (funct == "compile"){
		// -i <input file(s)> [-o <output file> -op <parse output file> -oa <assembly output file> -log <loging file>]
		std::cout << "Currently unimplemented" << std::endl;
	}else{
		std::cout << "Unknown command. Type \"river help\" for available commands." << std::endl;
	}
	
	
	
	
	return 0;
}

bool read_flag_args(int argc, char** argv, int flagIndex, std::vector<std::string>& argsRead){
	bool foundArgs = false;
	int i = 1;
	// stop when reaching end of arguments or the next flag
	while (flagIndex + i < argc && argv[flagIndex + i][0] != '-'){
		foundArgs = true;
		argsRead.push_back(argv[flagIndex + i]);
		++i;
	}
	return foundArgs;
}