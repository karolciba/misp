#include <iostream>
#include <fstream>
#include <memory>

#include "reader.h"
#include "utils.h"

using std::unique_ptr;
using std::shared_ptr;
using std::make_shared;

int main(int argc, char** argv) {
	shared_ptr<Env> env = std::make_shared<Env>();

    // Initialize self extenstions from core file
	std::ifstream core;
	core.open("core.mp");

    if (core.is_open()) {
		Reader core_reader(core);
		while (!core.eof()) {
			auto t = core_reader.read_form();
			if (t) {
				trace("Print ast\n");
				t->print();
				std::cout << "\n"; trace("Eval ast\n");
				auto ret = t->eval(env);
				ret->print();
				std::cout << "\n";
				std::cout << "user> ";
			}
		}
        std::cout << "\n";
	}

	Reader reader;
	std::cout << "user> ";
	do {
		auto t = reader.read_form();
		if (t) {
			trace("Print ast\n");
			// t->print();
			// std::cout << "\n";
			trace("Eval ast\n");
			auto ret = t->eval(env);
			ret->print();
			std::cout << "\n";
			std::cout << "user> ";
		}
	} while (!std::cin.eof());
}
