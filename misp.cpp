#include <iostream>
#include <typeinfo>
#include <vector>
#include <deque>
#include <stdexcept>
#include <unordered_map>
#include <memory>
#include <string>

using std::unique_ptr;
using std::shared_ptr;
using std::weak_ptr;
using std::make_shared;

int LEVEL = 5;
// int LEVEL = 0;
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
	shared_ptr<Env>find(std::string key);
	shared_ptr<Type> get(std::string key);
};

class Type : public std::enable_shared_from_this<Type> {
public:
	virtual void print() {
		std::cout << "[base]";
	};
	virtual shared_ptr<Type> eval(Env &env, Type* args=nullptr) {
		trace("Type#eval\n");
		return nullptr;
	}
};

class AtomType : public Type {
public:
	std::string value;
	AtomType(std::string v) : value(v) { };
	void print() {
		std::cout << "" << value << "";
	}
	shared_ptr<Type> eval(Env &env, Type *args=nullptr) {
        shared_ptr<Type> env_value = env.get(value);
		if (env_value) {
			return env_value;
		}
		return std::make_shared<AtomType>(value);
	}
	bool operator==(const AtomType& other) {
		return value == other.value;
	}
};

class ConsType : public Type {
public:
	shared_ptr<Type> next;
	shared_ptr<Type> value;
	ConsType() {
		trace("Constructing ConsType\n");
	}
    shared_ptr<Type> car() {
		return value;
	}
	shared_ptr<Type> cdr() {
		return next;
	}
	shared_ptr<Type> eval(Env &env, Type* inargs=nullptr) {
		return shared_from_this();
	}
};

class ListType : public Type {
public:
	std::deque<shared_ptr<Type>> data;
	ListType() {
		trace("Conctruting ListType\n");
	}
	void print() {
		std::cout << "(";
		for (auto &i : data) {
			i->print();
            std::cout << " ";
		}
		std::cout << ")";
	}
	shared_ptr<Type> car() {
		shared_ptr<Type> first = *data.begin();
        return first;
	}
	shared_ptr<ListType> cdr() {
        auto it = data.begin();
		it++;
        shared_ptr<ListType> ret = make_shared<ListType>();
		for(;it != data.end(); it++) {
            ret->data.push_back(*it);
		}
		return ret;
	}
	shared_ptr<Type> eval(Env &env, Type *inargs=nullptr) {
		trace("ListType#eval\n");

		if (data.size() == 0) {
			trace("ListType#eval empty list\n");
			return shared_from_this();
		}

        shared_ptr<Type> op = car();
        shared_ptr<Type> ev_op = op->eval(env);

		shared_ptr<Type> args = cdr();

		shared_ptr<Type> ret = ev_op->eval(env, args.get());

		trace("ListType#eval got evaluated params\n");
		return ret;
	}
};

class FunctionType : public Type {
public:
	Env* env;

	FunctionType() : env() { }

	void print() {
        std::cout << "[#function]";
    }

    shared_ptr<Type> eval(Env &env, Type* oargs) {
        trace("FunctionType#eval\n");
        return nullptr;
    }
};

class LambdaFunctionType : public FunctionType {
public:
	shared_ptr<Env> env;
    shared_ptr<ListType> binds;
	shared_ptr<ListType> expr;

	LambdaFunctionType(shared_ptr<Env> env, shared_ptr<ListType> binds, shared_ptr<ListType> expr) : FunctionType() {
		this->env = env;
		this->binds = binds;
		this->expr = expr;
	}

    shared_ptr<Type> eval(Env &ienv, Type* oargs) {
		trace("FunctionType#eval\n");
		ListType* args = dynamic_cast<ListType*>(oargs);
        auto b = binds->data.begin();
		auto a = args->data.begin();
		for (;
				b != binds->data.end();
				b++, a++) {
            AtomType* k = dynamic_cast<AtomType*>((*b).get());
			env->set(k->value, *a);
		}

		return expr->eval(*env);

	}

};

class FnStarFunctionType : public FunctionType {
public:
	Env* env;

	FnStarFunctionType() : FunctionType() { }

	void print() {
		std::cout << "[#function]";
	}

    shared_ptr<Type> eval(Env &env, Type* oargs) {
		shared_ptr<Env> lenv = std::make_shared<Env>(env);
        ListType* args = dynamic_cast<ListType*>(oargs);
		shared_ptr<ListType> binds = std::dynamic_pointer_cast<ListType>(args->car());
		shared_ptr<ListType> expr = std::dynamic_pointer_cast<ListType>(args->cdr()->car());

		return make_shared<LambdaFunctionType>(lenv, binds, expr);
	}
};

class PlusFunctionType : public FunctionType {
public:

    PlusFunctionType() : FunctionType() { }

    void print() {
        std::cout << "[#+ function]";
    }

    shared_ptr<Type> eval(Env &env, Type* oargs) {
        trace("PlusFunctionType#eval\n");
        ListType* args = dynamic_cast<ListType*>(oargs);
        int value = 0;
        for (auto it = args->data.begin(); it != args->data.end(); it++) {
            AtomType* a = dynamic_cast<AtomType*>((*it).get()->eval(env).get());
            value += std::stoi( a->value);
        }
        return std::make_shared<AtomType>(std::to_string(value));
	}
};

class ConsFunctionType : public FunctionType {
public:
	shared_ptr<Type> eval(Env& env, Type* inargs) {
//        ListType* args = dynamic_cast<ListType*>(inargs);
        shared_ptr<ListType> ret = std::make_shared<ListType>();

		return ret;
	}
};

class DefFunctionType : public FunctionType {
public:
	DefFunctionType() : FunctionType() { }
	void print() {
		std::cout << "[#def function]";
	}
	shared_ptr<Type> eval(Env &env, Type* oargs) {
		trace("DefFunctionType#eval\n");
		ListType* args = dynamic_cast<ListType*>(oargs);

		shared_ptr<Type> oname = *args->data.begin();
		// unique_ptr<Type> eop = op->eval();
		AtomType* name = dynamic_cast<AtomType*>(oname.get());
		if (!name) {
			std::cout << "SYNTAX ERROR" << "\n";
		}
		args->data.pop_front();

		AtomType* value = (AtomType*)args->eval(env).get();

		env.set(name->value, std::make_shared<AtomType>(value->value));

		return std::make_shared<AtomType>(value->value);
	}
};

Env::Env() {
    env["+"] = std::make_shared<PlusFunctionType>();
	env["-"] = std::make_shared<PlusFunctionType>();
	env["def!"] = std::make_shared<DefFunctionType>();
	env["fn*"] = std::make_shared<FnStarFunctionType>();
    // PTDB
	env["let*"] = std::make_shared<ConsFunctionType>();
	// TBD
    env["cons"] = std::make_shared<ConsFunctionType>();
	env["car"] = std::make_shared<PlusFunctionType>();
	env["cdr"] = std::make_shared<PlusFunctionType>();
	env["cond"] = std::make_shared<PlusFunctionType>();
}

void Env::set(std::string key, shared_ptr<Type> value) {
	env[key] = value;
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
