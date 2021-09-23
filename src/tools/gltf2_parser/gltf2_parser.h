/**************************************************
 * Copyright (c) 2021 Amanch Esmailzadeh
 * See LICENSE for details
 **************************************************/

#ifndef GLTF2_PARSER_H
#define GLTF2_PARSER_H

#include <string>

namespace kai {
    namespace gltf2 {
        enum class TokenType {
            open_brace,
            close_brace,
            open_bracket,
            close_bracket,
            colon,
            comma,
            double_quote,
            literal,

            eof
        };

        struct Token {
            Token(TokenType type) : type(type) {}
            Token(TokenType type, std::string &&str) : type(type), value(std::move(str)) {}

            TokenType type;
            std::string value;
        };

        void load(const char *buffer, size_t size = 0);
    }
}


#endif /* GLTF2_PARSER_H */
