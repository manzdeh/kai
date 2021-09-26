/**************************************************
 * Copyright (c) 2021 Amanch Esmailzadeh
 * See LICENSE for details
 **************************************************/

#include "gltf2_parser.h"

#include <stdint.h>
#include <string.h>
#include <vector>

static inline bool is_delimiter(const char c) {
    return c == ' ' || c == '\t' || c == '\n' || c == '\r';
}

namespace kai::gltf2 {
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

    struct Scene {
        std::vector<uint32_t> node_indices;
    };

    struct Node {
        union {
            struct {
                float rotation[4];
                float scale[3];
                float translation[3];
            };
            float matrix[4][4] = {};
        };
        std::vector<uint32_t> child_indices;
        uint32_t mesh_index;
        bool uses_trs;
    };

    struct Mesh {
        struct Primitive {
            enum {
                position,
                normal,
                tangent,
                texcoord,
                color,
                joint,
                weight
            } type;
            uint32_t index = 0;
        };
        std::vector<Primitive> primitives;
    };

    struct Accessor {
        uint32_t buffer_view_index;
        uint32_t byte_offset;
        uint32_t count;

        enum class ComponentType {
            sbyte = 5120,
            ubyte = 5121,
            sshort = 5122,
            ushort = 5123,
            uint = 5125,
            float32 = 5126
        } component_type;

        enum class Type {
            scalar,
            vec2,
            vec3,
            vec4,
            mat2,
            mat3,
            mat4
        } type;

        std::vector<float> max;
        std::vector<float> min;
        bool normalized;
    };

    struct BufferView {
        uint32_t index;
        uint32_t offset;
        uint32_t length;
        uint32_t stride;
        enum class Target {
            array_buffer = 34962,
            element_array_buffer = 34963
        } target;
    };

    struct Buffer {
        size_t count;
        unsigned char *data;
    };

    struct Info {
        struct {
            uint32_t major: 16;
            uint32_t minor: 16;
        } version;

        std::vector<Scene> scenes;
        std::vector<Node> nodes;
        std::vector<Mesh> meshes;
        std::vector<Accessor> accessors;
        std::vector<BufferView> buffer_views;
        std::vector<Buffer> buffers;
    };

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
            size_t literal_start = 0;
            bool is_literal = false;
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
                    default:
                        if(!is_literal) {
                            literal_start = j;
                        }
                        is_literal = true;
                        token = TokenType::literal;
                        break;
                }

                if(is_literal && token != TokenType::literal) {
                    is_literal = false;
                    tokens.push_back({TokenType::literal, std::string(&buffer[literal_start], j - literal_start)});
                }

                if(!is_literal) {
                    tokens.push_back({token, std::string(&buffer[j], 1)});
                }
            }

            if(is_literal) {
                tokens.push_back({TokenType::literal, std::string(&buffer[literal_start], j - literal_start)});
            }

            j--; // Decrement because we're on a delimiter

            i = j;
        }

        tokens.push_back({TokenType::eof});
    }
}
