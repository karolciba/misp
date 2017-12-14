#include <iostream>
#include <vector>
#include <stdexcept>
#include <memory>

using std::unique_ptr;
using std::shared_ptr;
using std::weak_ptr;

int LEVEL = 5;
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
	std::string value;
	Tokens type;

	Token(Tokens t, std::string v = "") : type(t), value(v) {
		trace("Constr on Token(" << t << "," << v << ")\n");
	};
	~Token() {
		trace("Destr on Token(" << type << "," << value << ")\n");
	}
	void print() {
		trace("Token(" << type << "," << value << ")\n");
	}
};

std::ostream &operator<<(std::ostream &os, shared_ptr<Token> const &t) { 
	return os << "Token(" << t->type << "," << t->value << ")";
}
// std::ostream &operator<<(std::ostream &os, weak_ptr<Token> const &t) { 
// 	return os << "Token(" << t->type << "," << t->value << ")";
// }

class Type {
};
class AtomType : public Type {
public:
	std::string value;
	AtomType(std::string v) : value(v) { };
};
class ListType : public Type {
public:
	std::vector<Type* > data;
};

class Reader {
	shared_ptr<Token> previous;
public:
	Reader() : previous(nullptr) {};

	bool is_space(char c) {
		return c == ' ' or c == '\t';
	}

	bool is_paren(char c) {
		return c == '(' or c == ')';
	}

	bool is_newline(char c) {
		return c == '\n';
	}

	shared_ptr<Token> get_token() {
		if (previous) {
			return std::move(previous);
		}

		char c;

		do {
			c = std::cin.get();
		} while (!std::cin.eof() && is_space(c));

		if (std::cin.eof()) {
			return shared_ptr<Token>(new Token(Tokens::Eol));
		}
		if (is_newline(c)) {
			return shared_ptr<Token>(new Token(Tokens::NewLine));
		}
		else if (c == '(') {
			return shared_ptr<Token>(new Token(Tokens::LeftParen));
		} else if (c == ')') {
			return shared_ptr<Token>(new Token(Tokens::RightParen));
		} else {
			Token* t = new Token(Tokens::Atom);
			while (!std::cin.eof() && !is_space(c) && !is_paren(c)) {
				t->value += c;
				c = std::cin.get();
			}
			std::cin.putback(c);
			return shared_ptr<Token>(t);
		}
	}
	void put_back(shared_ptr<Token> t) {
		previous = std::move(t);
	}
	shared_ptr<Token> peek() {
		if (previous) {
			return previous;
		}
		auto t = get_token();
		previous = t;
		return t;
	}

	Type* read_form() {
		auto p = peek();
		trace("read_form peek: " << p << "\n");
		switch (p->type) {
			case Tokens::Atom:
				trace("Atom Token\n");
				read_atom();
				break;
			case Tokens::LeftParen:
				trace("LeftParen Token\n");
				// consume left parent token
				get_token();
				read_list();
				break;
			default:
				throw std::runtime_error("Syntax error");
		}
	}
	ListType* read_list() {
		ListType* l;
		while (true) {
			auto p = peek();
			trace("read_list peek: " << p << "\n");
			switch (p->type) {
				case Tokens::RightParen:
					trace("read_list found closing paren: " << p << "\n");
					return l;
					break;
				default:
					trace("read_list reading atom " << p << "\n");
					Type* tp = read_form();
					trace("read_list read atom " << p << "\n");
					l->data.push_back(tp);
			}
		}
	}
	AtomType* read_atom() {
		auto t = get_token();
		trace("read_atom token: " << t << "\n");
		return new AtomType(t->value);
	}
};

int main(int argc, char** argv) {
	Reader reader;
	do {
		std::cout << "user> ";
		reader.read_form();
	} while (!std::cin.eof());
}
