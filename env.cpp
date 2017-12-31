//
// Created by karol on 30.12.17.
//
#include <sstream>

#include "env.h"
#include "types.h"

Env::Env() {
    env["def!"] = std::make_shared<DefFunctionType>();
    env["fn*"] = std::make_shared<FnFunctionType>();
    env["cond?"] = std::make_shared<CondFunctionType>();
    env["car"] = std::make_shared<CarFunctionType>();
    env["cdr"] = std::make_shared<CdrFunctionType>();
    env["cons"] = std::make_shared<ConsFunctionType>();
    env["uint"] = std::make_shared<UIntFunctionType>();
    env["len"] = std::make_shared<LenFunctionType>();
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
    /*
    if (outer) {
        os << outer->repr();
    }
    */
    os << "}";
    return os.str();
}

/**
 * Search chain of environment to find one containing key
 * @param string key
 * @return shared_ptr<Env> containing env
 */
shared_ptr<Env> Env::find(std::string key) {
    auto it = env.find(key);
    if (it != env.end()) {
        return shared_from_this();
    } else if (outer) {
        return outer->find(key);
    }
    return nullptr;
}

/**
 * Search for key in current environment, otherwise look in outer scope.
 *
 * @param string key to look for
 * @return shared_ptr<Type>
 */
shared_ptr<Type> Env::get(std::string key) {
    auto it = env.find(key);
    if (it != env.end()) {
        return it->second;
    } else {
        auto cont = find(key);
        if (cont) {
            return cont->get(key);
        } else {
            return nullptr;
        }
    }
}
