#include <iostream>
#include <cstring>
#include <fstream>
#include <list>
#include "AST.h"
#include "Lexer.h"

#define VERSION 0.0.1

void printUsage();

AST* parseFile(char* filename);
int runAST(AST* tree);

int main(int argc, char** argv){
	std::cout << "Version: VERSION" << std::endl; 
	if (argc == 1) printUsage();
	
	int flagCount = 0;
	
	//std::string source = argv[1];
	
	for (char i = 1; i < argc; i++){
		char* arg = argv[i];
		if (arg[0] == '-'){
			// argument is a flag
			std::cout << "flag detected: " << arg << std::endl;
		}else{
			//argument is a filename
			
			//parse file
			
			std::cout << "filename detected: " << arg << std::endl;
			
			std::ifstream inStream(arg);
			
			if (!inStream.is_open()){
				std::cerr << "Unable to open file " << arg << " for parsing!" << std::endl;
			}else{
				std::string line;
				Lexer lex;
				while (std::getline(inStream, line)){
					lex.interpret_line(line);
				}
				lex.finish();
				
				std::list<TokenPair> tokenStream = lex.get_token_stream();
				
				std::cout << "tokenStream: " << std::endl;
				
				std::list<TokenPair>::iterator iter;
				for (iter = tokenStream.begin(); iter != tokenStream.end(); ++iter){
					std::cout << iter->type << " : " << iter->token << std::endl;
				}
				
			}
			
			
			
			
			
			/* This is all dumb
			//get the file extension
			char* extension = NULL;
			for (int j = 0; arg[j] != '\0'; j++){
				std::cout << arg[j] << std::endl;
				if (arg[j] == '.'){
					extension = &arg[j + 1];
					if (*extension == '\0'){
						extension = NULL;
					}
				}
			}
			
			std::cout << "extension: " << extension << std::endl;
			
			if (std::strcmp(extension, "scd")){
				//code file .scd
				std::cout << ".scd extension: " << arg << std::endl;
			}else if (std::strcmp(extension, "str")){
				//ast file .str
				std::cout << ".str extension: " << arg << std::endl;
			}
			
			*/
		}
	}
}

void printUsage(){
	std::cout << "usage: app.exe <sources>" << std::endl;
}