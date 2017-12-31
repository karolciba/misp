//
// Created by karol on 30.12.17.
//
#include <sstream>
#include <memory>
#include <iostream>
#include <deque>

using std::shared_ptr;
using std::make_shared;

#include "reader.h"
#include "utils.h"


Token::Token(Tokens t, std::string v) : type{t}, value{v} { }

std::ostream& operator<<(std::ostream &os, const Token *t) {
    return os << "Token(" << t->type << "," << t->value << ")";
}


bool Reader::is_space(char c) {
    return c == ' ' or c == '\t';
}

bool Reader::is_paren(char c) {
    return c == '(' or c == ')';
}

bool Reader::is_newline(char c) {
    return c == '\n';
}

Reader::Reader(std::istream &source) : source{source}, previous(nullptr) {};

shared_ptr<Token> Reader::get_token() {
    if (previous) {
        auto cpy = previous;
        previous = nullptr;
        return cpy;
    }

    char c;

    do {
        c = source.get();
    } while (!source.eof() && is_space(c));

    if (source.eof()) {
        return shared_ptr<Token>(new Token(Tokens::Eol));
    }
    if (is_newline(c)) {
        return shared_ptr<Token>(new Token(Tokens::NewLine));
    } else if (c == '(') {
        balance++;
        return shared_ptr<Token>(new Token(Tokens::LeftParen));
    } else if (c == ';') {
        return shared_ptr<Token>(new Token(Tokens::Comment));
    } else if (c == ')') {
        balance--;
        return shared_ptr<Token>(new Token(Tokens::RightParen));
    } else {
        Token* t = new Token(Tokens::Atom);
        while (!source.eof() && !is_newline(c) && !is_space(c) && !is_paren(c)) {
            t->value += c;
            c = source.get();
        }
        source.putback(c);
        return shared_ptr<Token>(t);
    }
}

void Reader::put_back(shared_ptr<Token> t) {
    previous = t;
}

shared_ptr<Token>& Reader::peek() {
    if (previous) {
        return previous;
    }
    previous = get_token();
    return previous;
}

shared_ptr<Type> Reader::read_form() {
    if (source.eof()) {
        return nullptr;
    }

    auto& p = peek();
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
        case Tokens::NewLine:
            get_token();
            trace("read_form found: NewLine\n");
            return nullptr;
        case Tokens::Comment:
            get_comment();
            return nullptr;
        case Tokens::Eol:
            get_token();
            trace("read_form found: EOL\n");
            return nullptr;
        default:
            throw std::runtime_error("Syntax error");
    }
}

/**
 * Consumes comment till end of line
 */
void Reader::get_comment() {
    while(get_token()->type != Tokens::NewLine);
}

shared_ptr<ConsType> Reader::read_list() {
    //auto l = shared_ptr<ListType>(new ListType());
    // items will be stored in reversed order
    // to ease cons-list construction
    std::deque<shared_ptr<Type>> list;
    shared_ptr<ConsType> head;
    while (true) {
        auto& p = peek();
        trace("read_list peek: " << &p << "\n");
        switch (p->type) {
            case Tokens::RightParen:
                trace("read_list found closing paren: " << p << "\n");
                get_token();
                for (auto it : list) {
                    head = make_shared<ConsType>(it, head);
                }
                // if now list then return empty list instead of nullptr
                if (!head) {
                    head = make_shared<ConsType>();
                }

                return head;
                // while parens are not balanced ignore newlines
            case Tokens::NewLine:
                if (balance) {
                    get_token();
                    break;
                }
            default:
                trace("read_list reading atom " << &p << " ");
                auto tp = shared_ptr<Type>(read_form());
                list.push_front(tp);
        }
    }
}
shared_ptr<AtomType> Reader::read_atom() {
    auto t = get_token();
    trace("read_atom token: " << &t << "\n");
    return shared_ptr<AtomType>(new AtomType(t->value));
}
