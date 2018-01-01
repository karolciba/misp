//
// Created by karol on 30.12.17.
//

#include <sstream>
#include <iostream>

#include "types.h"
#include "utils.h"

using std::make_shared;


void Type::print() const {
    std::cout << repr();
}
bool Type::is_nil() const {
    return false;
}


AtomType::AtomType(std::string v) : value{v} { };

std::string AtomType::repr() const {
    std::ostringstream os;
    os << "" << value << "";
    return os.str();
}

shared_ptr<Type> AtomType::eval(shared_ptr<Env> env, shared_ptr<Type> args = nullptr) const {
    shared_ptr<Type> env_value = env->get(value);
    if (env_value) {
        trace("AtomType#eval [" << repr() << ": " << env_value->repr() << "] " << env->repr() << "\n");
        return env_value;
    }
    trace("AtomType#eval [" << repr() << "] " << env->repr() << "\n");
    return std::make_shared<AtomType>(value);
}



ConsType::ConsType(shared_ptr<Type> x, shared_ptr<Type> y) : head{x}, tail{y} { };

std::string ConsType::repr() const {
    std::ostringstream os;
    os << "(cons ";
    os << (head ? head->repr() : "nil");
    os << " ";
    os << (tail ? tail->repr() : "nil" );
    os << ")";
    return os.str();
}

shared_ptr<Type> ConsType::car() const {
    return head;
}

shared_ptr<Type> ConsType::cdr() const {
    return tail;
}

bool ConsType::is_nil() const {
    return !head && !tail;
}

shared_ptr<Type> ConsType::eval(shared_ptr<Env> env, shared_ptr<Type> args) const {
    trace("ConsType#eval [" << repr() << ": " << (args ? args->repr() : " ") << "] " << env->repr() << "\n");
    if (is_nil()) {
        return make_shared<ConsType>();
    }
    shared_ptr<Type> op = head->eval(env);
    if (tail) {
        return op->eval(env, tail);
    } else {
        return op;
    }
}


LambdaType::LambdaType(shared_ptr<Env> env, shared_ptr<Type> binds, shared_ptr<Type> expr): env(env), binds(binds), expr(expr) { }

std::string LambdaType::repr() const {
    std::ostringstream os;
    //os << "(#lambda " << binds->repr() << " " << expr->repr() << " " << env->repr() << ")";
    os << "(#lambda " << binds->repr() << " " << expr->repr()  << ")";
    return os.str();
}

shared_ptr<Type> LambdaType::eval(shared_ptr<Env> ienv, shared_ptr<Type> args) const {
    trace("LambdaType#eval [" << repr() << ": " << (args ? args->repr() : " ") << "] " << ienv->repr() << "\n");

    shared_ptr<Env> new_env = make_shared<Env>(env);

    auto fcar = env->get("car");
    auto fcdr = env->get("cdr");

    /* local copy not to destroy original parameters names list */
    shared_ptr<Type> lbinds = binds;
    while (!lbinds->is_nil() && !args->is_nil()) {
        shared_ptr<AtomType> key = std::dynamic_pointer_cast<AtomType>(fcar->eval(env, lbinds));
        new_env->set(key->value, fcar->eval(env,args)->eval(ienv));
        lbinds = fcdr->eval(env, lbinds);
        args = fcdr->eval(env, args);
    }

    shared_ptr<Type> ret = expr->eval(new_env);

    return ret;
}

std::string ConsFunctionType::repr() const {
    std::ostringstream os;
    os << "(#cons)";
    return os.str();
}

shared_ptr<Type> ConsFunctionType::eval(shared_ptr<Env> env, shared_ptr<Type> inargs) const {
    shared_ptr<ConsType> args = std::dynamic_pointer_cast<ConsType>(inargs);
    if (!args) {
        error("expected object <ConsType> got " << typeid(inargs).name() << "\n");
    }
    return make_shared<ConsType>(args->car(), args->cdr());
}


std::string CarFunctionType::repr() const {
    std::ostringstream os;
    os << "(#car) ";
    return os.str();
}
shared_ptr<Type> CarFunctionType::eval(shared_ptr<Env> env, shared_ptr<Type> inargs) const {
    shared_ptr<ConsType> args = std::dynamic_pointer_cast<ConsType>(inargs);
    if (!args) {
        error("expected object <ConsType> got " << typeid(inargs).name() << "\n");
    }
    return args->car();
}


std::string CdrFunctionType::repr() const {
    std::ostringstream os;
    os << "(#cdr) ";
    return os.str();
};
shared_ptr<Type> CdrFunctionType::eval(shared_ptr<Env> env, shared_ptr<Type> inargs) const {
    shared_ptr<ConsType> args = std::dynamic_pointer_cast<ConsType>(inargs);
    if (!args) {
        error("expected object <ConsType> got " << typeid(inargs).name() << "\n");
    }
    return args->cdr();
}

std::string FnFunctionType::repr() const {
    std::ostringstream os;
    os << "(#fn*) ";
    return os.str();
};
shared_ptr<Type> FnFunctionType::eval(shared_ptr<Env> env, shared_ptr<Type> args) const {
    trace("FnFunctionType#eval [" << repr() << ": " << args->repr() << "] " << env->repr() << "\n");

    auto fcar = env->get("car");
    auto fcdr = env->get("cdr");
    shared_ptr<Type> binds = fcar->eval(env, args);
    shared_ptr<Type> expr = fcdr->eval(env, args);

    shared_ptr<LambdaType> ret = make_shared<LambdaType>(env, std::move(binds), std::move(expr));

    return ret;
}

std::string DefFunctionType::repr() const {
    std::ostringstream os;
    os << "(#def!)";
    return os.str();
};
shared_ptr<Type> DefFunctionType::eval(shared_ptr<Env> env, shared_ptr<Type> args) const {
    trace("DefFunctionType#eval [" << repr() << ": " << args->repr() << "] " << env->repr() << "\n");

    auto fcar = env->get("car");
    auto fcdr = env->get("cdr");
    // Evaluate name
    // either it is atom already - then take it as it is, otherwise
    // evaluate it first
    shared_ptr<Type> name_arg = fcar->eval(env, args);

    trace("DefFunctionType#evaluating name " << name_arg->repr() << "\n");
    if (!std::dynamic_pointer_cast<AtomType>(name_arg)) {
        name_arg = name_arg->eval(env);
    }
    shared_ptr<AtomType> name = std::dynamic_pointer_cast<AtomType>(name_arg);
    if (!name) {
        error("DefFunction expected object <AtomType> got " << typeid(name_arg).name() << "\n");
    }
    trace("DefFunctionType#evaluated name " << name->repr() << "\n");

    // Evaluate value
    trace("DefFunctionType#evaluating value\n");
    shared_ptr<Type> arg = fcdr->eval(env, args);
    trace("DefFunctionType#arg " << arg->repr() << "\n");
    shared_ptr<Type> value = arg->eval(env);
    trace("DefFunctionType#value " << arg->repr() << "\n");

    //trace("DefFunctionType#evaluated value" << value->repr() << "\n");

    env->set(name->value, value);

    return value;
}


std::string CondFunctionType::repr() const {
    std::ostringstream os;
    os << "(#cond?)";
    return os.str();
};
shared_ptr<Type> CondFunctionType::eval(shared_ptr<Env> env, shared_ptr<Type> args) const {
    trace("CondFunctionType#eval [" << repr() << ": " << args->repr() << "] " << env->repr() << "\n");
    auto fcar = env->get("car");
    auto fcdr = env->get("cdr");

    shared_ptr<Type> last = make_shared<ConsType>();
    //shared_ptr<Type> arg = fcar->eval(env, args);
    while(args) {
        shared_ptr<Type> arg = fcar->eval(env, args);
        trace("CondFunctionType#cond " << arg->repr() << "\n");
        shared_ptr<Type> cond = fcar->eval(env, arg)->eval(env);
        trace("CondFunctionType#test " << cond->repr() << "\n");
        if (!cond || cond->is_nil()) {
            trace("CondFunctionType#tested false\n");
            break;
        }
        trace("CondFunctionType#tested true\n");
        last = fcdr->eval(env, arg);
        args = fcdr->eval(env, args);
    }

    return last->eval(env);
}


std::string UIntFunctionType::repr() const {
    std::ostringstream os;
    os << "(#uint) ";
    return os.str();
};
shared_ptr<Type> UIntFunctionType::eval(shared_ptr<Env> env, shared_ptr<Type> inargs) const {
    auto fcar = env->get("car");
    shared_ptr<Type> number = fcar->eval(env, inargs);

    shared_ptr<AtomType> name = std::dynamic_pointer_cast<AtomType>(number);
    if (!name) {
        error("UIntFunction expected object <AtomType> got " << typeid(number).name() << "\n");
    }

    unsigned int num = atoi(name->value.c_str());

    shared_ptr<ConsType> list = make_shared<ConsType>();
    for(int i = 0; i < num; i++) {
        list = make_shared<ConsType>(list, list);
    }

    return list;
}


std::string LenFunctionType::repr() const {
    std::ostringstream os;
    os << "(#len) ";
    return os.str();
};
shared_ptr<Type> LenFunctionType::eval(shared_ptr<Env> env, shared_ptr<Type> inargs) const {
    auto fcar = env->get("car");
    auto fcdr = env->get("cdr");
    int len = 0;

    shared_ptr<Type> next = fcar->eval(env,inargs)->eval(env);
    while (next = fcdr->eval(env, next)) {
        trace("LenFunction#eval left " << next->repr() << "\n");
        len++;
    }

    return make_shared<AtomType>(std::to_string(len));
}
