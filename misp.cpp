#include <iostream>
#include <typeinfo>
#include <vector>
#include <deque>
#include <stdexcept>
#include <memory>
#include <string>

using std::unique_ptr;
using std::shared_ptr;
using std::weak_ptr;
using std::make_shared;

// int LEVEL = 5;
int LEVEL = 0;
int ERROR = 1;
int WARN  = 2;
int INFO  = 3;
int TRACE = 4;

#define trace(msg) (LEVEL >= TRACE && std::cerr << msg)
#define info(msg) (LEVEL >= INFO && std::cerr << msg)


enum Tokens {
	Eol,
	LeftParen,
	RightParen,
	Atom,
	NewLine,
};

class Token {
public:
	Tokens type;
	std::string value;

	Token(Tokens t, std::string v = "") : type(t), value(v) {
		switch (t) {
			case Tokens::LeftParen:
				value = '(';
				break;
			case Tokens::RightParen:
				value = ')';
				break;
			default:
				break;
		}
		trace("Constr on Token(" << t << ", \"" << value << "\")\n");
	};
	~Token() {
		trace("Destr on Token(" << type << ", \"" << value << "\")\n");
	}
	void print() {
		trace("Token(" << type << ", \"" << value << "\")\n");
	}
};

std::ostream &operator<<(std::ostream &os, Token* const &t) { 
	return os << "Token(" << t->type << "," << t->value << ")";
}
std::ostream &operator<<(std::ostream &os, unique_ptr<Token> t) { 
	return os << "Token(" << t->type << "," << t->value << ")";
}

class Type {
public:
	virtual void print() {
		std::cout << "[base]";
	};
	virtual unique_ptr<Type> eval(Type* args=nullptr) {
		trace("Type#eval\n");
		return nullptr;
	}
};

class AtomType : public Type {
public:
	std::string value;
	AtomType(std::string v) : value(v) { };
	void print() {
		std::cout << "[Atom " << value << "]";
	}
	unique_ptr<Type> eval(Type *args=nullptr) {
		trace("AtomType#eval\n");
		return unique_ptr<Type>(new AtomType(value));
	}
};

class FunctionType : public Type {
public:
	unique_ptr<Type> eval(Type *args) {
		trace("FunctionType#eval\n");
	}
};

class ListType : public Type {
public:
	std::deque<unique_ptr<Type>> data;
	ListType() {
		trace("Conctruting ListType\n");
	}
	void print() {
		std::cout << "[List ";
		for (auto &i : data) {
			i->print();
		}
		std::cout << "]";
	}
	unique_ptr<Type> eval(Type *inargs=nullptr) {
		trace("ListType#eval\n");

		if (data.size() == 0) {
			trace("ListType#eval empty list\n");
			return unique_ptr<Type> (new ListType());
		}

		unique_ptr<Type> op = std::move(*data.begin());
		// unique_ptr<Type> eop = op->eval();
		AtomType* aop = dynamic_cast<AtomType*>(op.get());
		if (!aop) {
			std::cout << "SYNTAX ERROR" << "\n";
		}
		data.pop_front();
		trace("ListType#eval got operator\n");

		auto args = unique_ptr<ListType>(new ListType());
		for (auto it = data.begin(); it != data.end(); it++) {
			args->data.push_back( (*it)->eval() );
		}
		trace("ListType#eval got evaluated params\n");

		if (aop->value == "+") {
			int value = 0;
			for (auto it = args->data.begin(); it != args->data.end(); it++) {
				value += std::stoi( (dynamic_cast<AtomType*>((*it).get()))->value);
			}
			return unique_ptr<Type>(new AtomType(std::to_string(value)));
		}

		return std::move(aop->eval(args.get()));
	}
};

class Reader {
	unique_ptr<Token> previous;

	bool is_space(char c) {
		return c == ' ' or c == '\t';
	}

	bool is_paren(char c) {
		return c == '(' or c == ')';
	}

	bool is_newline(char c) {
		return c == '\n';
	}

public:
	Reader() : previous(nullptr) {};
	unique_ptr<Token> get_token() {
		if (previous) {
			return std::move(previous);
		}

		char c;

		do {
			c = std::cin.get();
		} while (!std::cin.eof() && is_space(c));

		if (std::cin.eof()) {
			return unique_ptr<Token>(new Token(Tokens::Eol));
		}
		if (is_newline(c)) {
			return unique_ptr<Token>(new Token(Tokens::NewLine));
		}
		else if (c == '(') {
			return unique_ptr<Token>(new Token(Tokens::LeftParen));
		} else if (c == ')') {
			return unique_ptr<Token>(new Token(Tokens::RightParen));
		} else {
			Token* t = new Token(Tokens::Atom);
			while (!std::cin.eof() && !is_newline(c) && !is_space(c) && !is_paren(c)) {
				t->value += c;
				c = std::cin.get();
			}
			std::cin.putback(c);
			return unique_ptr<Token>(t);
		}
	}

	void put_back(unique_ptr<Token> t) {
		previous = std::move(t);
	}

	unique_ptr<Token>& peek() {
		if (previous) {
			return previous;
		}
		previous = get_token();
		return previous;
	}

	unique_ptr<Type> read_form() {
		auto& p = peek();
		trace("read_form peek: " << &p << "\n");
		switch (p->type) {
			case Tokens::Atom:
				trace("read_form found: Atom Token\n");
				return read_atom();
			case Tokens::LeftParen:
				trace("read_form found: LeftParen Token\n");
				// consume left parent token
				get_token();
				return read_list();
				break;
			case Tokens::NewLine:
				get_token();
				trace("read_form found: NewLine\n");
				return nullptr;
			case Tokens::Eol:
				get_token();
				trace("read_form found: EOL\n");
				return nullptr;
			default:
				throw std::runtime_error("Syntax error");
		}
	}
	unique_ptr<ListType> read_list() {
		auto l = unique_ptr<ListType>(new ListType());
		while (true) {
			auto& p = peek();
			trace("read_list peek: " << &p << "\n");
			switch (p->type) {
				case Tokens::RightParen:
					get_token();
					trace("read_list found closing paren: " << &p << "\n");
					return l;
					break;
				default:
					trace("read_list reading atom " << &p << "\n");
					auto tp = unique_ptr<Type>(read_form());
					l->data.push_back(std::move(tp));
			}
		}
	}
	unique_ptr<AtomType> read_atom() {
		auto t = get_token();
		trace("read_atom token: " << &t << "\n");
		return unique_ptr<AtomType>(new AtomType(t->value));
	}
};

// class Eval {
// public:
// 	unique_ptr<Type> eval(Type *ast);
// 	unique_ptr<Type> eval_ast(Type* ast) {
// 		if (dynamic_cast<AtomType*>(ast.get())) {
// 			std::cout << "atom" << "\n";
// 		}
// 		if (dynamic_cast<ListType*>(ast.get())) {
// 			std::cout << "list" << "\n";
// 		}
// 	}
// };

class Printer {
public:
	void print(Type t) {
	}
};

int main(int argc, char** argv) {
	Reader reader;
	do {
		std::cout << "user> ";
		auto t = reader.read_form();
		if (t) {
			trace("Print ast\n");
			t->print();
			std::cout << "\n";
			trace("Eval ast\n");
			auto ret = t->eval();
			ret->print();
			std::cout << "\n";
		}
	} while (!std::cin.eof());
}
