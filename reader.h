//
// Created by karol on 30.12.17.
//

#ifndef MISP_READER_H
#define MISP_READER_H

#include "types.h"

enum Tokens {
    Eol = '0',
    Comment = ';',
    LeftParen = '(',
    RightParen = ')',
    Atom = 'a',
    NewLine = '\n',
};

class Token {
public:
    Tokens type;
    std::string value;

    explicit Token(Tokens t, std::string v = "");

    //friend std::ostream& operator <<(std::ostream &os, const Token *t);
};

class Reader {
    std::istream& source;
    shared_ptr<Token> previous;
    int balance = 0;

    bool is_space(char c);
    bool is_paren(char c);
    bool is_newline(char c);

public:
    explicit Reader(std::istream &source = std::cin);

    shared_ptr<Token> get_token();
    void put_back(shared_ptr<Token> t);

    shared_ptr<Token>& peek();

    shared_ptr<Type> read_form();

    /**
     * Consumes comment till end of line
     */
    void get_comment();

    shared_ptr<ConsType> read_list();
    shared_ptr<AtomType> read_atom();
};
#endif //MISP_READER_H
