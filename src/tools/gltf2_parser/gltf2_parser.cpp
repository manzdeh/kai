/**************************************************
 * Copyright (c) 2021 Amanch Esmailzadeh
 * See LICENSE for details
 **************************************************/

#include "gltf2_parser.h"

#include <array>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <vector>

struct Token {
    enum class Type {
        open_brace,
        close_brace,
        open_bracket,
        close_bracket,
        colon,
        comma,
        double_quote,
        literal
    };

    Token(Type type) : type(type) {}
    Token(Type type, std::string &&str) : type(type), value(std::move(str)) {}

    Type type;
    std::string value;
};

static inline bool is_delimiter(const char c) {
    return c == ' ' || c == '\t' || c == '\n' || c == '\r';
}

namespace kai::gltf2 {
    struct Info;
    void parse_asset(Info &info, const std::vector<Token> &tokens, size_t &out_index);

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
            float matrix[4][4];
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

                Token::Type token;

                switch(buffer[j]) {
                    case '{': token = Token::Type::open_brace; break;
                    case '}': token = Token::Type::close_brace; break;
                    case '[': token = Token::Type::open_bracket; break;
                    case ']': token = Token::Type::close_bracket; break;
                    case ':': token = Token::Type::colon; break;
                    case ',': token = Token::Type::comma; break;
                    case '"': token = Token::Type::double_quote; break;
                    default:
                        if(!is_literal) {
                            literal_start = j;
                        }
                        is_literal = true;
                        token = Token::Type::literal;
                        break;
                }

                if(is_literal && token != Token::Type::literal) {
                    is_literal = false;
                    tokens.push_back({Token::Type::literal, std::string(&buffer[literal_start], j - literal_start)});
                }

                if(!is_literal) {
                    tokens.push_back({token, std::string(&buffer[j], 1)});
                }
            }

            if(is_literal) {
                tokens.push_back({Token::Type::literal, std::string(&buffer[literal_start], j - literal_start)});
            }

            j--; // Decrement because we're on a delimiter

            i = j;
        }

        enum ParseStep {
            NONE,
            ASSET,
            SCENES,
            NODES,
            MESHES,
            ACCESSORS,
            BUFFER_VIEWS,
            BUFFERS
        } parse_step = NONE;

        auto find_matching_property = [](const std::string &name) -> ParseStep {
            static const std::array<const char *, 7> properties = {
                "asset",
                "scenes",
                "nodes",
                "meshes",
                "accessors",
                "bufferViews",
                "buffers"
            };

            auto it = std::find(properties.begin(), properties.end(), name);
            if(it != properties.end()) {
#define MAP_PROPERTY(name, retval) if(strcmp(*it, name) == 0) return retval

                MAP_PROPERTY(properties[0], ASSET);
                MAP_PROPERTY(properties[1], SCENES);
                MAP_PROPERTY(properties[2], NODES);
                MAP_PROPERTY(properties[3], MESHES);
                MAP_PROPERTY(properties[4], ACCESSORS);
                MAP_PROPERTY(properties[5], BUFFER_VIEWS);
                MAP_PROPERTY(properties[6], BUFFERS);

#undef MAP_PROPERTY
            }

            return NONE;
        };

        Info model_info = {};

        for(size_t i = 0; i < tokens.size(); i++) {
            const Token &token = tokens[i];
            switch(parse_step) {
                case NONE:
                    if(token.type == Token::Type::literal) {
                        parse_step = find_matching_property(token.value);
                    }
                    break;
                case ASSET:
                    parse_asset(model_info, tokens, i);
                    parse_step = NONE;
                    if(model_info.version.major != 2) {
                        fprintf(stderr, "Only glTF 2.0 files are supported!\n");
                        goto end;
                    }
                    break;
                default:
                    break;
            }
        }

end:
        ;
    }

    void find_next(Token::Type type, const std::vector<Token> &tokens, size_t &out_index) {
        for(size_t i = out_index + 1; i < tokens.size(); i++) {
            if(tokens[i].type == type) {
                out_index = i;
                return;
            }
        }
    }

    void parse_asset(Info &info, const std::vector<Token> &tokens, size_t &out_index) {
        find_next(Token::Type::open_brace, tokens, out_index);
        size_t i = out_index;

        int32_t scope_depth = 0;

        for(; i < tokens.size(); i++) {
            const Token &token = tokens[i];

            switch(token.type) {
                case Token::Type::open_brace:
                    scope_depth++;
                    break;
                case Token::Type::close_brace:
                    scope_depth--;
                    break;
                default:
                    break;
            }

            if(scope_depth == 0) {
                break;
            }

            if(token.type == Token::Type::literal && token.value == "version") {
                find_next(Token::Type::literal, tokens, i);

                size_t len = tokens[i].value.length();

                const char *delims = ".";

                char *value = reinterpret_cast<char *>(malloc(len + 1));
                value[len] = '\0';
                const char *source = tokens[i].value.c_str();
                memcpy(value, source, len);

                char *tok = strtok(value, delims);
                info.version.major = atol(tok);
                tok = strtok(nullptr, delims);
                info.version.minor = atol(tok);

                free(value);
            }
        }

        out_index = i;
    }
}
