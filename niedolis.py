import operator as op
import functools


def tokenize(chars: str):
    chars = chars.replace("'(", "(quote ")
    return [t for t in chars.replace('(', ' ( ').replace(')', ' ) ').split()]


def atom(token: str):
    try:
        return int(token)
    except ValueError:
        return token


def parse(tokens: list):
    token = tokens.pop(0)
    if token == '(':
        tree = []
        while tokens[0] != ')':
            tree.append(parse(tokens))
        tokens.pop(0)
        return tree

    return atom(token)


def proc(params, def_env):
    args = params[0]
    body = params[1]

    def call_fun(args, call_env):
        n_env = {k: def_env[k] for k in def_env}
        n_env.update(zip(params, args))
        return eval(body, n_env)

    return call_fun


ENV = {
    '+': lambda args, _: functools.reduce(op.add, args),
    'concat': lambda args, _: functools.reduce(op.concat, args),
    'fn*': proc
}


def eval(ast, env=ENV):
    if type(ast) is int:
        return ast

    if type(ast) is list and ast[0] == "quote":
        return ast[1:]
    if type(ast) is list and ast[0] == "quote":
        return ast[1:]
    if type(ast) is list and ast[0] == "null?":
        return "'true" if len(ast[1]) == 0 else []
    if type(ast) is list and ast[0] == "if":
        (_, predicate, default, alternative) = ast
        exp = default if eval(predicate, env) else alternative
        return eval(exp, env)

    if type(ast) is str and ast[0] == "'":
        return ast[1:]
    if type(ast) is str:
        return env[ast]
    if len(ast) == 0:
        return ["(", ")"]
    if ast[0] == "exit":
        return ["(", "exit", ")"]

    fun = eval(ast[0], env)
    args = [eval(arg, env) for arg in ast[1:]] if len(ast) > 1 else []
    return fun(args, env)


if __name__ == "__main__":
    #print(eval(parse(tokenize("(+ 'A 'B ( + 'E 'C) 'A (+ 'D 'L 'O))"))))
    while True:
        try:
            ipt = "(+ 1 2)"
            ipt = input(">")
            val = eval(parse(tokenize(ipt)))
            if val is None or val == ["(", "exit", ")"]:
                break
            print(val)
        except Exception as e:
            print(e)

