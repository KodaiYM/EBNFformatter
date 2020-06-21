#include "codegen.h"
#include "parser.h"
#include "print.h"
#include "tokenizer.h"
#include <fstream>
#include <iostream>

int main(int argc, char *argv[]) {
	if (argc != 2) {
		std::cerr << "There are not enough arguments.\n";
		return EXIT_FAILURE;
	}
	std::ifstream ifs(argv[1]);
	if (ifs.fail()) {
		std::cerr << "file " << argv[1] << "couldn't open.\n";
		return EXIT_FAILURE;
	}
	auto file = std::string(std::istreambuf_iterator<char>(ifs),
	                        std::istreambuf_iterator<char>());
	ifs.close();

	Tokenizer     tokenizer(file);
	Parser        parser(tokenizer);
	std::ofstream token_file(".token.txt");
	token_file << tokenizer;
	token_file.close();

	auto AST = parser.makeAST(); // Abstract Syntax Tree

	std::ofstream AST_file(".AST.txt");
	AST_file << AST;
	AST_file.close();

	// calculate whole node
	gen(AST);

	return EXIT_SUCCESS;
}
