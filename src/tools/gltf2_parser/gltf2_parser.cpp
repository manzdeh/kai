/**************************************************
 * Copyright (c) 2021 Amanch Esmailzadeh
 * See LICENSE for details
 **************************************************/

#include "gltf2_parser.h"

#include <string.h>
#include <vector>

static inline bool is_delimiter(const char c) {
    return c == ' ' || c == '\t' || c == '\n' || c == '\r';
}

namespace kai::gltf2 {
    void load(const char *buffer, size_t size) {
        size = (size > 0) ? size : strlen(buffer);

        std::vector<Token> tokens;

        // Lexer
        for(size_t i = 0; i < size; i++) {
            if(buffer[i] == '\0') {
                break;
            }

            if(is_delimiter(buffer[i])) {
                continue;
            }

            size_t j = i;
            for(; j < size; j++) {
                if(is_delimiter(buffer[j])) {
                    break;
                }

                TokenType token;

                switch(buffer[j]) {
                    case '{': token = TokenType::open_brace; break;
                    case '}': token = TokenType::close_brace; break;
                    case '[': token = TokenType::open_bracket; break;
                    case ']': token = TokenType::close_bracket; break;
                    case ':': token = TokenType::colon; break;
                    case ',': token = TokenType::comma; break;
                    case '"': token = TokenType::double_quote; break;
                    default: token = TokenType::literal; break;
                }

                tokens.push_back({token, ""});
            }
            j--; // Decrement because we're on a delimiter

            i = j;
        }

        tokens.push_back({TokenType::eof, ""});
    }
}
