#ifndef TOKEN_H
#define TOKEN_H

#include <string>
enum TokenType { TKN_T1, TKN_T2, TKN_T3, TKN_ERR, TKN_EOF };

struct Token {
    TokenType type;
    std::string* instance;
    int lineNum;
};

#endif
