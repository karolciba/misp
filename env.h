//
// Created by karol on 30.12.17.
//
#ifndef MISP_ENV_H
#define MISP_ENV_H

#include <memory>
#include <unordered_map>

using std::shared_ptr;

class Type;

class Env : public std::enable_shared_from_this<Env> {
public:
    shared_ptr<Env> outer;
    std::unordered_map<std::string, shared_ptr<Type>> env;

    Env();
    Env(shared_ptr<Env> iouter) : outer(iouter) { };

    void set(std::string key, shared_ptr<Type> value);
    std::string repr();
    shared_ptr<Env> find(std::string key);
    shared_ptr<Type> get(std::string key);
};

#endif //MISP_ENV_H
