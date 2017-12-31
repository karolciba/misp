//
// Created by karol on 30.12.17.
//

#ifndef MISP_TYPES_H
#define MISP_TYPES_H

#include <memory>

#include "env.h"

class Type : public std::enable_shared_from_this<Type> {
public:
    virtual void print() const;
    virtual std::string repr() const = 0;
    virtual bool is_nil() const;
    virtual shared_ptr<Type> eval(shared_ptr<Env> env, shared_ptr<Type> args = nullptr) const = 0;
};

class AtomType : public Type {
public:
    std::string value;

    explicit AtomType(std::string v);

    std::string repr() const override;
    shared_ptr<Type> eval(shared_ptr<Env> env, shared_ptr<Type> args) const override;
};

class ConsType : public Type {
public:
    shared_ptr<Type> head;
    shared_ptr<Type> tail;

    ConsType(shared_ptr<Type> x = nullptr, shared_ptr<Type> y = nullptr);

    std::string repr() const override;
    bool is_nil() const override;
    shared_ptr<Type> eval(shared_ptr<Env> env, shared_ptr<Type> args = nullptr) const override;

    shared_ptr<Type> car() const;
    shared_ptr<Type> cdr() const;
};

class LambdaType : public Type {
public:
    /* Envirionment cought at definition time */
    shared_ptr<Env> env;
    /* List of parameters names */
    shared_ptr<Type> binds;
    /* Expression to be evaluated at calll time */
    shared_ptr<Type> expr;

    LambdaType(shared_ptr<Env> env, shared_ptr<Type> binds, shared_ptr<Type> expr);

    std::string repr() const override;
    shared_ptr<Type> eval(shared_ptr<Env> ienv, shared_ptr<Type> args = nullptr) const override;
};

class ConsFunctionType : public Type {
public:
    std::string repr() const override;
    shared_ptr<Type> eval(shared_ptr<Env> env, shared_ptr<Type> inargs = nullptr) const override;

};

class CarFunctionType : public Type {
public:
    std::string repr() const override;
    shared_ptr<Type> eval(shared_ptr<Env> env, shared_ptr<Type> inargs) const override;
};

class CdrFunctionType : public Type {
public:
    std::string repr() const override;
    shared_ptr<Type> eval(shared_ptr<Env> env, shared_ptr<Type> inargs) const override;
};

class FnFunctionType : public Type {
public:
    std::string repr() const override;
    shared_ptr<Type> eval(shared_ptr<Env> env, shared_ptr<Type> args) const override;
};

class DefFunctionType : public Type {
public:
    std::string repr() const override;
    shared_ptr<Type> eval(shared_ptr<Env> env, shared_ptr<Type> args) const override;
};

class CondFunctionType : public Type {
public:
    std::string repr() const override;
    shared_ptr<Type> eval(shared_ptr<Env> env, shared_ptr<Type> args) const override;
};

class UIntFunctionType : public Type {
public:
    std::string repr() const override;
    shared_ptr<Type> eval(shared_ptr<Env> env, shared_ptr<Type> inargs) const override;
};

class LenFunctionType : public Type {
public:
    std::string repr() const override;
    shared_ptr<Type> eval(shared_ptr<Env> env, shared_ptr<Type> inargs) const override;
};

#endif //MISP_TYPES_H
