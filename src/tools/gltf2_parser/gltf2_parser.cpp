/**************************************************
 * Copyright (c) 2021 Amanch Esmailzadeh
 * See LICENSE for details
 **************************************************/

#include "gltf2_parser.h"

#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include <array>
#include <string>
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

    struct Parser {
        Parser(const std::vector<Token> &tokens) : tokens(tokens) {}

        void find_next(Token::Type type);

        void parse_asset(void);
        void parse_scenes(void);
        void parse_nodes(void);

        Info info;
        const std::vector<Token> &tokens;
        size_t index = 0;
    };

    void load(const char *buffer, size_t size) {
        size = (size > 0) ? size : strlen(buffer);

        std::vector<Token> tokens;

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
            // TODO: Handle the "scene" property as well if scenes are going to be supported
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

        Parser p(tokens);

        for(; p.index < p.tokens.size(); p.index++) {
            const Token &token = p.tokens[p.index];
            switch(parse_step) {
                case NONE:
                    if(token.type == Token::Type::literal && ((parse_step = find_matching_property(token.value)) != NONE)) {
                        continue;
                    }
                    break;
                case ASSET:
                    p.parse_asset();
                    if(p.info.version.major != 2 && p.info.version.minor != 0) {
                        fprintf(stderr, "Only glTF 2.0 files are supported!\n");
                        goto end;
                    }
                    break;
                case SCENES:
                    p.parse_scenes();
                    break;
                case NODES:
                    break;
                default:
                    break;
            }

            parse_step = NONE;
        }

end:
        ;
    }

    void Parser::find_next(Token::Type type) {
        index++;
        for(; index < tokens.size(); index++) {
            if(tokens[index].type == type) {
                return;
            }
        }
    }

    char * copy_to_str(const std::string &source) {
        static char str_buffer[9999];
        size_t len = source.length();

        if(len < (sizeof(str_buffer) - 1)) {
            memcpy(str_buffer, source.c_str(), len);
            str_buffer[len] = '\0';
            return str_buffer;
        }

        return nullptr;
    }

    void Parser::parse_asset(void) {
        find_next(Token::Type::open_brace);

        int32_t scope_depth = 0;

        for(; index < tokens.size(); index++) {
            const Token &token = tokens[index];

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
                find_next(Token::Type::literal);

                const char *delims = ".";

                char *str = copy_to_str(tokens[index].value);
                if(str) {
                    char *tok = strtok(str, delims);
                    info.version.major = atol(tok);
                    tok = strtok(nullptr, delims);
                    info.version.minor = atol(tok);
                }
            }
        }
    }

    void Parser::parse_scenes(void) {
        size_t scene_index = 0;

        for(; index < tokens.size(); index++) {
            if(tokens[index].type == Token::Type::close_bracket) {
                break;
            }

            if(tokens[index].type == Token::Type::open_brace) {
                info.scenes.push_back({});

                while(tokens[index].type != Token::Type::close_brace) {
                    if(tokens[index].type == Token::Type::literal && tokens[index].value == "nodes") {
                        find_next(Token::Type::open_bracket);

                        while(tokens[index].type != Token::Type::close_bracket) {
                            if(tokens[index].type == Token::Type::literal) {
                                info.scenes[scene_index].node_indices.push_back(atol(tokens[index].value.c_str()));
                            }

                            index++;
                        }
                    }

                    index++;
                }

                scene_index++;
            }
        }
    }

    void Parser::parse_nodes(void) {
    }
}
