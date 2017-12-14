#include <iostream>
#include <vector>
#include <stdexcept>
#include <memory>

using std::unique_ptr;
using std::shared_ptr;
using std::weak_ptr;
using std::make_shared;

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
		switch (t) {
			case Tokens::LeftParen:
				value = '(';
				break;
			case Tokens::RightParen:
				value = ')';
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

std::ostream &operator<<(std::ostream &os, shared_ptr<Token> const &t) { 
	return os << "Token(" << t->type << "," << t->value << ")";
}
// std::ostream &operator<<(std::ostream &os, weak_ptr<Token> const &t) { 
// 	return os << "Token(" << t->type << "," << t->value << ")";
// }

class Type {
public:
	virtual void print() {
		std::cout << "[base]";
	};
};
class AtomType : public Type {
public:
	std::string value;
	AtomType(std::string v) : value(v) { };
	void print() {
		std::cout << "[Atom " << value << "]";
	}
};
class ListType : public Type {
public:
	std::vector<shared_ptr<Type>> data;
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
};

class Reader {
	shared_ptr<Token> previous;

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

	shared_ptr<Type> read_form() {
		auto p = peek();
		trace("read_form peek: " << p << "\n");
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
	shared_ptr<ListType> read_list() {
		auto l = shared_ptr<ListType>(new ListType());
		while (true) {
			auto p = peek();
			trace("read_list peek: " << p << "\n");
			switch (p->type) {
				case Tokens::RightParen:
					get_token();
					trace("read_list found closing paren: " << p << "\n");
					return l;
					break;
				default:
					trace("read_list reading atom " << p << "\n");
					auto tp = shared_ptr<Type>(read_form());
					l->data.push_back(tp);
			}
		}
	}
	shared_ptr<AtomType> read_atom() {
		auto t = get_token();
		trace("read_atom token: " << t << "\n");
		return shared_ptr<AtomType>(new AtomType(t->value));
	}
};

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
			t->print();
			std::cout << "\n";
		}
	} while (!std::cin.eof());
}
