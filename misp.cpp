#include <iostream>
#include <typeinfo>
#include <vector>
#include <deque>
#include <stdexcept>
#include <unordered_map>
#include <memory>
#include <string>
#include <sstream>

using std::unique_ptr;
using std::shared_ptr;
using std::weak_ptr;
using std::make_shared;

int LEVEL = 5;
//int LEVEL = 0;
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

class Type;

class Env : public std::enable_shared_from_this<Env> {
public:
	shared_ptr<Env> outer;
	std::unordered_map<std::string, shared_ptr<Type>> env;

	Env();
    Env(Env* iouter) : outer(iouter) { };

	void set(std::string key, shared_ptr<Type> value);
    std::string repr();
	shared_ptr<Env>find(std::string key);
	shared_ptr<Type> get(std::string key);
};

class Type : public std::enable_shared_from_this<Type> {
public:
	virtual void print() {
		std::cout << repr();
	};
    virtual std::string repr() {
		std::ostringstream os;
		os << "[ERROR base type]";
		return os.str();
	};
	virtual shared_ptr<Type> eval(Env &env, Type* args=nullptr) {
		trace("[ERROR] Type#eval\n");
		return nullptr;
	}
};

class AtomType : public Type {
public:
	std::string value;
	AtomType(std::string v) : value(v) { };

	std::string repr() {
		std::ostringstream os;
		os << "" << value << "";
		return os.str();
	};

	shared_ptr<Type> eval(Env &env, Type *args=nullptr) {
		shared_ptr<Type> env_value = env.get(value);
		if (env_value) {
			trace("AtomType#eval [" << repr() << ": " << env_value->repr() << "] " << env.repr() << "\n");
			return env_value;
		}
		trace("AtomType#eval [" << repr() << "] " << env.repr() << "\n");
		return std::make_shared<AtomType>(value);
	}

	bool operator==(const AtomType& other) {
		return value == other.value;
	}
};

class ListType : public Type {
public:

	std::deque<shared_ptr<Type>> data;
	ListType() {
		trace("Constructing ListType\n");
	}

	std::string repr() {
		std::ostringstream os;
		os << "(";
		for (auto &i : data) {
			os << i->repr();
			os << " ";
		}
		os << ")";
		return os.str();
	};

	shared_ptr<Type> car() {
		auto it = data.begin();
		if (it == data.end()) {
			return nullptr;
		}
		shared_ptr<Type> first = *it;
		return first;
	}

	shared_ptr<ListType> cdr() {
		// skip first
		auto it = data.begin();
		if (it == data.end()) {
			return nullptr;
		}
		it++;
		if (it == data.end()) {
			return nullptr;
		}

		// create list from the rest
		shared_ptr<ListType> ret = make_shared<ListType>();
		for(;it != data.end(); it++) {
			ret->data.push_back(*it);
		}
		return ret;
	}

	shared_ptr<Type> eval_args(Env &env, Type *inargs=nullptr) {
		unique_ptr<ListType> empty_inargs;
		if (!inargs) {
			empty_inargs = std::make_unique<ListType>();
			inargs = empty_inargs.get();
		}
		trace("ListType#eval_args [" << repr() << ": " << inargs->repr() << "] " << env.repr() << "\n");

		if (data.size() == 0) {
			trace("ListType#eval empty list\n");
			return shared_from_this();
		}
		shared_ptr<ListType> ret = make_shared<ListType>();

		for (auto it = data.begin(); it != data.end(); it++) {
			ret->data.push_back((*it)->eval(env));
		}

		trace("ListType#eval got evaluated params\n");
		return ret;
	}

	shared_ptr<Type> eval(Env &env, Type *inargs=nullptr) {
		unique_ptr<ListType> empty_inargs;
		if (!inargs) {
			empty_inargs = std::make_unique<ListType>();
			inargs = empty_inargs.get();
		}
		trace("ListType#eval [" << repr() << ": " << inargs->repr() << "] " << env.repr() << "\n");

		shared_ptr<Type> op = car();
		shared_ptr<Type> ev_op = op->eval(env);

		shared_ptr<Type> args = cdr();

		shared_ptr<Type> ret = ev_op->eval(env, args.get());

		trace("ListType#eval return: " << ret->repr() << "\n");
		return ret;
	}
};

class FunctionType : public Type {
public:
	Env* env;

	FunctionType() : env() { }

	std::string repr() {
		std::ostringstream os;
		os << "(#function)";
		return os.str();
	};

	shared_ptr<Type> eval(Env &env, Type* oargs) {
		trace("FunctionType#eval\n");
		return nullptr;
	}
};

class PlusFunctionType : public FunctionType {
public:

	PlusFunctionType() : FunctionType() { }

	std::string repr() {
		std::ostringstream os;
		os << "(#+)";
		return os.str();
	};

	shared_ptr<Type> eval(Env &env, Type* oargs) {
		trace("PlusFunctionType#eval [" << repr() << ": " << oargs->repr() << "] " << env.repr() << "\n");
		ListType* args = dynamic_cast<ListType*>(oargs);
		shared_ptr<ListType> ev_args = std::dynamic_pointer_cast<ListType>(args->eval_args(env));
		trace("PlusFunctionType#evalated args " << oargs->repr() << "\n");
		int value = 0;
		for (auto it = args->data.begin(); it != args->data.end(); it++) {
			//AtomType* at = dynamic_cast<AtomType*>((*it).get());
			shared_ptr<Type> t = *it;
			shared_ptr<AtomType> at = std::dynamic_pointer_cast<AtomType>(t);
			shared_ptr<AtomType> eat = std::dynamic_pointer_cast<AtomType>(at->eval(env));
			trace("PlusFunctionType#arg " << eat->repr() << "\n");
			value += std::stoi(eat->value);
		}
		return std::make_shared<AtomType>(std::to_string(value));
	}
};

class DefFunctionType : public FunctionType {
public:
	DefFunctionType() : FunctionType() { }
	std::string repr() {
		std::ostringstream os;
		os << "(#def!)";
		return os.str();
	};
	shared_ptr<Type> eval(Env &env, Type* oargs) {
		trace("DefFunctionType#eval [" << repr() << ": " << oargs->repr() << "] " << env.repr() << "\n");
		ListType* args = dynamic_cast<ListType*>(oargs);

		// Evaluate name
		// either it is atom already - then take it as it is, otherwise
		// evaluate it first
		shared_ptr<Type> name_arg = args->car();
		trace("DefFunctionType#evaluating name " << name_arg->repr() << "\n");
		if (!std::dynamic_pointer_cast<AtomType>(name_arg)) {
			name_arg = name_arg->eval(env);
		}
		shared_ptr<AtomType> name = std::dynamic_pointer_cast<AtomType>(name_arg);
		trace("DefFunctionType#evaluated name " << name->repr() << "\n");

		// Evaluate value
		trace("DefFunctionType#evaluating value\n");
		shared_ptr<Type> value = args->cdr()->car()->eval(env);
		trace("DefFunctionType#evaluated value" << value->repr() << "\n");

		env.set(name->value, value);

		return value;
	}
};

Env::Env() {
	env["+"] = std::make_shared<PlusFunctionType>();
	env["def!"] = std::make_shared<DefFunctionType>();
	//env["fn*"] = std::make_shared<FnStarFunctionType>();
    // PTDB
	//env["let*"] = std::make_shared<ConsFunctionType>();
	// TBD
    //env["cons"] = std::make_shared<FunctionType>();
	//env["car"] = std::make_shared<FunctionType>();
	//env["cdr"] = std::make_shared<FunctionType>();
	//env["cond"] = std::make_shared<FunctionType>();
}

void Env::set(std::string key, shared_ptr<Type> value) {
	env[key] = value;
}

std::string Env::repr() {
	std::ostringstream os;
	os << "{";
	for (auto &it : env) {
		os << "\"" << (it).first;
		os << "\": " << ((it).second)->repr();
		os << ", ";
	}
	os << "}";
	return os.str();
}

shared_ptr<Env> Env::find(std::string key) {
	auto it = env.find(key);
	if (it != env.end()) {
		return shared_from_this();
	} else if (outer) {
		return outer->find(key);
	}

	return nullptr;
}

shared_ptr<Type> Env::get(std::string key) {
	auto it = env.find(key);
	if (it != env.end()) {
		return it->second;
	} else {
		return nullptr;
	}
}

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
            auto cpy = previous;
            previous = nullptr;
			return cpy;
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
		} else if (c == '(') {
			return shared_ptr<Token>(new Token(Tokens::LeftParen));
		} else if (c == ')') {
			return shared_ptr<Token>(new Token(Tokens::RightParen));
		} else {
			Token* t = new Token(Tokens::Atom);
			while (!std::cin.eof() && !is_newline(c) && !is_space(c) && !is_paren(c)) {
				t->value += c;
				c = std::cin.get();
			}
			std::cin.putback(c);
			return shared_ptr<Token>(t);
		}
	}

	void put_back(shared_ptr<Token> t) {
		previous = t;
	}

	shared_ptr<Token>& peek() {
		if (previous) {
			return previous;
		}
		previous = get_token();
		return previous;
	}

	shared_ptr<Type> read_form() {
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
		//auto l = shared_ptr<ListType>(new ListType());
		//investigate
		auto l = std::make_shared<ListType>();
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
					auto tp = shared_ptr<Type>(read_form());
					l->data.push_back(tp);
			}
		}
	}
	shared_ptr<AtomType> read_atom() {
		auto t = get_token();
		trace("read_atom token: " << &t << "\n");
		return shared_ptr<AtomType>(new AtomType(t->value));
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
	unique_ptr<Env> env = std::make_unique<Env>();
	do {
		std::cout << "user> ";
		auto t = reader.read_form();
		if (t) {
			trace("Print ast\n");
			// t->print();
			// std::cout << "\n";
			trace("Eval ast\n");
			auto ret = t->eval(*env);
			ret->print();
			std::cout << "\n";
		}
	} while (!std::cin.eof());
}
