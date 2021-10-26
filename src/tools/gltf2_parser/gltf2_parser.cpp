/**************************************************
 * Copyright (c) 2021 Amanch Esmailzadeh
 * See LICENSE for details
 **************************************************/

// TODO: This is not a full glTF2 parser (yet). At the moment it only handles glTF files that
// contain a single mesh and it assumes that the data contained in the buffer is non-interleaved.

#include "gltf2_parser.h"

#include <stdio.h>
#include <string.h>

#include <array>
#include <optional>
#include <string>
#include <unordered_map>
#include <utility>
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

static char * copy_to_str(const std::string &source) {
    static char str_buffer[9999];
    size_t len = source.length();

    if(len < (sizeof(str_buffer) - 1)) {
        memcpy(str_buffer, source.c_str(), len);
        str_buffer[len] = '\0';
        return str_buffer;
    }

    return nullptr;
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
            enum class Attribute {
                position,
                normal,
                tangent,
                texcoord,
                color,
                joint,
                weight,

                count
            };
            std::vector<std::pair<Attribute, uint32_t>> attributes;
            uint32_t indices;
        };
        std::vector<Primitive> primitives;
    };

    struct Accessor {
        using ComponentValues = std::vector<std::array<uint8_t, 4>>;

        void set_component_type(const std::string &str) {
            if(str == "SCALAR") {
                type = Accessor::Type::scalar;
            } else if(str == "VEC2") {
                type = Accessor::Type::vec2;
            } else if(str == "VEC3") {
                type = Accessor::Type::vec3;
            } else if(str == "VEC4") {
                type = Accessor::Type::vec4;
            } else if(str == "MAT2") {
                type = Accessor::Type::mat2;
            } else if(str == "MAT3") {
                type = Accessor::Type::mat3;
            } else if(str == "MAT4") {
                type = Accessor::Type::mat4;
            }
        }

        template<typename T, typename U = int(const char *)>
        void parse_accessor_type(const char *str, ComponentValues &component, U pred = atoi) {
            T val = static_cast<T>(pred(str));
            memcpy(component.back().data(), &val, sizeof(val));
        }

        uint32_t buffer_view;
        uint32_t byte_offset;
        uint32_t count;

        enum class ComponentType {
            unset = 0,
            sbyte = 5120,
            ubyte = 5121,
            sshort = 5122,
            ushort = 5123,
            uint = 5125,
            float32 = 5126
        } component_type;

        constexpr size_t get_component_size(void) const {
            switch(component_type) {
                case ComponentType::sbyte: return sizeof(int8_t);
                case ComponentType::ubyte: return sizeof(uint8_t);
                case ComponentType::sshort: return sizeof(int16_t);
                case ComponentType::ushort: return sizeof(uint16_t);
                case ComponentType::uint: return sizeof(uint32_t);
                case ComponentType::float32: return sizeof(float);
                default: return 0;
            }
        }

        enum class Type {
            scalar,
            vec2,
            vec3,
            vec4,
            mat2,
            mat3,
            mat4
        } type;

        constexpr uint32_t get_type_count(void) const {
            switch(type) {
                case Type::scalar: return 1;
                case Type::vec2: return 2;
                case Type::vec3: return 3;
                case Type::vec4: return 4;
                case Type::mat2: return 4;
                case Type::mat3: return 9;
                case Type::mat4: return 16;
                default: return 0;
            }
        }

        ComponentValues max;
        ComponentValues min;
        bool normalized;
    };

    struct BufferView {
        uint32_t buffer;
        uint32_t byte_offset;
        uint32_t byte_length;
        enum class Target {
            unspecified = 0,
            array_buffer = 34962,
            element_array_buffer = 34963
        } target;

        std::optional<uint32_t> byte_stride;
    };

    struct Buffer {
        uint64_t byte_length;
        std::string uri;
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
        static constexpr uint32_t top_level_nodes_count = 7;
        static const std::array<const char *, Parser::top_level_nodes_count> properties;

        Parser(const std::vector<Token> &tokens) : tokens(tokens) {
            top_level_nodes.reserve(top_level_nodes_count);
            for(size_t i = 0; i < tokens.size(); i++) {
                if(tokens[i].type == Token::Type::literal) {
                    auto it = std::find(properties.begin(), properties.end(), tokens[i].value);
                    Token::Type open_type;
                    std::pair<size_t, size_t> range;

                    if(it != properties.end() && get_open_type(tokens[i], open_type) && find_scope_depth(range, i, open_type)) {
                        top_level_nodes.insert({tokens[i].value, range});
                        i = range.second;
                    }
                }
            }
        }

        bool find_next(Token::Type type, size_t &out_index);
        bool get_open_type(const Token &tok, Token::Type &out_type) const;
        bool find_scope_depth(std::pair<size_t, size_t> &out_indices, size_t index_start, Token::Type open_type) const;

        void parse_asset(void);
        void parse_scenes(void);
        void parse_nodes(void);
        void parse_meshes(void);
        void parse_accessors(void);
        void parse_buffer_views(void);
        void parse_buffers(void);

        Info info;
        const std::vector<Token> &tokens;

        std::unordered_map<std::string, std::pair<size_t, size_t>> top_level_nodes;
    };

    // TODO: Handle the "scene" property as well if scenes are going to be supported
    const std::array<const char *, Parser::top_level_nodes_count> Parser::properties = {
        "asset",
        "scenes",
        "nodes",
        "meshes",
        "accessors",
        "bufferViews",
        "buffers"
    };

    MeshData * load(const char *buffer, const char *dir, size_t size) {
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

        Parser p(tokens);
        p.parse_asset();

        if(p.info.version.major == 2 && p.info.version.minor == 0) {
            p.parse_scenes();
            p.parse_nodes();
            p.parse_meshes();
            p.parse_accessors();
            p.parse_buffer_views();
            p.parse_buffers();
        } else {
            fprintf(stderr, "Only glTF 2.0 files are supported!\n");
            return nullptr;
        }

        if(p.info.meshes.size() > 0 && p.info.buffers.size() > 0) {
            // NOTE: For now we only assume 1 mesh and 1 buffer per file

            static char file_path[9999];
            const char *file = p.info.buffers.front().uri.c_str();
            if(dir) {
                snprintf(file_path, sizeof(file_path), "%s/%s", dir, file);
                file = file_path;
            }

            FILE *f = fopen(file, "rb");

            if(f) {
                KAI_PUSH_DISABLE_COMPILER_WARNINGS(4815)
                MeshData m;
                KAI_POP_COMPILER_WARNINGS

                using PrimAttrib = Mesh::Primitive::Attribute;

                int32_t buffer_indices[static_cast<uint32_t>(PrimAttrib::count)];
                for(uint32_t i = 0; i < KAI_ARRAY_COUNT(buffer_indices); i++) {
                    buffer_indices[i] = -1;
                }

                for(const Mesh::Primitive &primitive : p.info.meshes.front().primitives) {
                    bool processed_index_buffer = false;

                    for(uint32_t i = 0; i < primitive.attributes.size(); i++) {
                        const auto &attribute = primitive.attributes[i];

                        KAI_ASSERT(attribute.second < p.info.accessors.size());
                        KAI_ASSERT(p.info.accessors[attribute.second].buffer_view < p.info.buffer_views.size());

                        const Accessor &accessor = p.info.accessors[attribute.second];
                        const BufferView &bv = p.info.buffer_views[accessor.buffer_view];

                        KAI_ASSERT(!bv.byte_stride.has_value()); // TODO: Stride values aren't handled yet

                        switch(attribute.first) {
                            case Mesh::Primitive::Attribute::position:
                                m.vertex_count = accessor.count;
                                m.attributes.position = static_cast<int32_t>(m.vertex_stride);
                                buffer_indices[static_cast<uint32_t>(PrimAttrib::position)] = accessor.buffer_view;
                                break;
                            case Mesh::Primitive::Attribute::normal:
                                m.attributes.normal = static_cast<int32_t>(m.vertex_stride);
                                buffer_indices[static_cast<uint32_t>(PrimAttrib::normal)] = accessor.buffer_view;
                                break;
                            case Mesh::Primitive::Attribute::texcoord:
                                m.attributes.texcoord = static_cast<int32_t>(m.vertex_stride);
                                buffer_indices[static_cast<uint32_t>(PrimAttrib::texcoord)] = accessor.buffer_view;
                                break;
                        }

                        m.vertex_byte_size += bv.byte_length;
                        m.vertex_stride += accessor.get_component_size() * accessor.get_type_count();

                        if(!processed_index_buffer) {
                            KAI_ASSERT(primitive.indices < p.info.accessors.size());
                            KAI_ASSERT(p.info.accessors[primitive.indices].buffer_view < p.info.buffer_views.size());

                            const Accessor &ibuffer_accessor = p.info.accessors[primitive.indices];
                            const BufferView &ibuffer_view = p.info.buffer_views[ibuffer_accessor.buffer_view];

                            m.index_count = ibuffer_accessor.count;
                            m.index_byte_size = ibuffer_view.byte_length;
                            m.index_byte_offset = ibuffer_view.byte_offset;

                            processed_index_buffer = true;
                        }
                    }
                }

                KAI_ASSERT(m.attributes.position >= 0); // We always expect to have vertex positions

                m.vertex_byte_size = m.vertex_stride * m.vertex_count;

                MeshData *mesh_data = reinterpret_cast<MeshData *>(malloc(sizeof(*mesh_data) + m.vertex_byte_size + m.index_byte_size));
                memcpy(mesh_data, &m, sizeof(m));

                // NOTE: This works for non-interleaved data, but it won't work once the stride parameter is used
                for(uint32_t i = 0; i < KAI_ARRAY_COUNT(buffer_indices); i++) {
                    if(m.attributes.data[i] > -1) {
                        const BufferView &view = p.info.buffer_views[buffer_indices[i]];
                        fseek(f, view.byte_offset, SEEK_SET);
                        fread(mesh_data->data + m.attributes.data[i], 1, view.byte_length, f);
                    }
                }

                fseek(f, static_cast<long int>(m.index_byte_offset), SEEK_SET);
                fread(mesh_data->data + m.vertex_byte_size, 1, m.index_byte_size, f);

                fclose(f);

                return mesh_data;
            }
        }

        return nullptr;
    }

    void destroy(MeshData *data) {
        free(data);
    }

    // -------------------------------------------------- Parser --------------------------------------------------
    bool Parser::find_next(Token::Type type, size_t &out_index) {
        size_t index = out_index + 1;
        for(; index < tokens.size(); index++) {
            if(tokens[index].type == type) {
                out_index = index;
                return true;
            }
        }

        return false;
    }

    bool Parser::get_open_type(const Token &tok, Token::Type &out_type) const {
        if(tok.type == Token::Type::literal) {
#define MAP_OPEN_TYPE(name, type) \
            do { \
                if(strcmp(name, tok.value.c_str()) == 0) { \
                    out_type = type; \
                    return true; \
                } \
            } while(0)

            MAP_OPEN_TYPE("asset", Token::Type::open_brace);
            MAP_OPEN_TYPE("scenes", Token::Type::open_bracket);
            MAP_OPEN_TYPE("nodes", Token::Type::open_bracket);
            MAP_OPEN_TYPE("meshes", Token::Type::open_bracket);
            MAP_OPEN_TYPE("accessors", Token::Type::open_bracket);
            MAP_OPEN_TYPE("bufferViews", Token::Type::open_bracket);
            MAP_OPEN_TYPE("buffers", Token::Type::open_bracket);
#undef MAP_OPEN_TYPE
        }

        return false;
    }


    bool Parser::find_scope_depth(std::pair<size_t, size_t> &out_indices, size_t index_start, Token::Type open_type) const {
        bool is_open_brace = open_type == Token::Type::open_brace;

        if(is_open_brace || open_type == Token::Type::open_bracket) {
            Token::Type close_type = (is_open_brace) ? Token::Type::close_brace : Token::Type::close_bracket;
            int32_t depth = 0;
            bool start_set = false;

            size_t scope_start = 0;

            for(size_t i = index_start; i < tokens.size(); i++) {
                if(tokens[i].type == open_type) {
                    depth++;
                    if(!start_set) {
                        scope_start = i;
                        start_set = true;
                    }
                } else if(tokens[i].type == close_type) {
                    depth--;
                }

                if(depth == 0 && tokens[i].type == close_type) {
                    out_indices = std::pair(scope_start, i);
                    return true;
                }
            }
        }

        return false;
    }

    void Parser::parse_asset(void) {
        auto it = top_level_nodes.find("asset");

        if(it != top_level_nodes.end()) {
            for(size_t i = it->second.first; i < it->second.second; i++) {
                if(tokens[i].type == Token::Type::literal && tokens[i].value == "version" && find_next(Token::Type::literal, i)) {
                    const char *delims = ".";

                    char *str = copy_to_str(tokens[i].value);
                    if(str) {
                        char *tok = strtok(str, delims);
                        info.version.major = atol(tok);
                        tok = strtok(nullptr, delims);
                        info.version.minor = atol(tok);
                    }
                }
            }
        }
    }

    void Parser::parse_scenes(void) {
        auto it = top_level_nodes.find("scenes");

        if(it != top_level_nodes.end()) {
            for(size_t i = it->second.first; i < it->second.second; i++) {
                if(tokens[i].type == Token::Type::open_brace) {
                    info.scenes.push_back({});
                    continue;
                }

                if(tokens[i].type == Token::Type::literal && tokens[i].value == "nodes" && find_next(Token::Type::open_bracket, i)) {
                    while(tokens[i].type != Token::Type::close_bracket) {
                        if(tokens[i].type == Token::Type::literal) {
                            info.scenes.back().node_indices.push_back(atol(tokens[i].value.c_str()));
                        }

                        i++;
                    }
                }
            }
        }
    }

    void Parser::parse_nodes(void) {
        auto it = top_level_nodes.find("nodes");

        if(it != top_level_nodes.end()) {
            for(size_t i = it->second.first; i < it->second.second; i++) {
                if(tokens[i].type == Token::Type::open_brace) {
                    info.nodes.push_back({});
                    continue;
                }

                if(tokens[i].type == Token::Type::literal && tokens[i].value == "mesh" && find_next(Token::Type::literal, i)) {
                    info.nodes.back().mesh_index = atol(tokens[i].value.c_str());
                }
            }
        }
    }

    // TODO: Check if this can be simplified a bit
    void Parser::parse_meshes(void) {
        auto it = top_level_nodes.find("meshes");

        if(it != top_level_nodes.end()) {
            for(size_t i = it->second.first; i < it->second.second; i++) {
                if(tokens[i].type == Token::Type::open_brace) {
                    info.meshes.push_back({});
                    continue;
                }

                if(tokens[i].type == Token::Type::literal && tokens[i].value == "primitives" && find_next(Token::Type::open_bracket, i)) {
                    while(tokens[i].type != Token::Type::close_bracket) {
                        if(tokens[i].type == Token::Type::open_brace) {
                            info.meshes.back().primitives.push_back({});

                            while(tokens[i].type != Token::Type::close_brace) {

                                if(tokens[i].type == Token::Type::literal) {
                                    if(tokens[i].value == "attributes" && find_next(Token::Type::open_brace, i)) {
                                        static const std::array<const char *, 7> attribute_types = {
                                            "POSITION",
                                            "NORMAL",
                                            "TANGENT",
                                            "TEXCOORD",
                                            "COLOR",
                                            "JOINT",
                                            "WEIGHT"
                                        };

                                        while(tokens[i].type != Token::Type::close_brace) {
                                            auto retval = std::find_if(attribute_types.begin(), attribute_types.end(), [&strval = tokens[i].value](const char *str) {
                                                return strval.rfind(str, 0) == 0;
                                            });

                                            if(retval != attribute_types.end()) {
                                                using Attribute = Mesh::Primitive::Attribute;

                                                ptrdiff_t attribute_idx = retval - attribute_types.begin();
                                                find_next(Token::Type::literal, i);
                                                uint32_t data_idx = atol(tokens[i].value.c_str());
                                                info.meshes.back().primitives.back().attributes.push_back({static_cast<Attribute>(attribute_idx), data_idx});
                                            }

                                            i++;
                                        }

                                    } else if(tokens[i].value == "indices" && find_next(Token::Type::literal, i)) {
                                        info.meshes.back().primitives.back().indices = atol(tokens[i].value.c_str());
                                    }
                                }

                                i++;
                            }
                        }

                        i++;
                    }
                }
            }
        }
    }

    void Parser::parse_accessors(void) {
        auto it = top_level_nodes.find("accessors");

        if(it != top_level_nodes.end()) {
            for(size_t i = it->second.first; i < it->second.second; i++) {
                if(tokens[i].type == Token::Type::open_brace) {
                    info.accessors.push_back({});
                    continue;
                }

                if(tokens[i].type == Token::Type::literal) {
                    if(tokens[i].value == "bufferView" && find_next(Token::Type::literal, i)) {
                        info.accessors.back().buffer_view = atol(tokens[i].value.c_str());
                        continue;
                    }

                    if(tokens[i].value == "byte_offset" && find_next(Token::Type::literal, i)) {
                        info.accessors.back().byte_offset = atol(tokens[i].value.c_str());
                        continue;
                    }

                    if(tokens[i].value == "count" && find_next(Token::Type::literal, i)) {
                        info.accessors.back().count = atol(tokens[i].value.c_str());
                        continue;
                    }

                    if(tokens[i].value == "componentType" && find_next(Token::Type::literal, i)) {
                        info.accessors.back().component_type = static_cast<Accessor::ComponentType>(atol(tokens[i].value.c_str()));
                        continue;
                    }

                    if(tokens[i].value == "type" && find_next(Token::Type::literal, i)) {
                        info.accessors.back().set_component_type(tokens[i].value);
                        continue;
                    }

                    if(tokens[i].value == "normalized" && find_next(Token::Type::literal, i)) {
                        info.accessors.back().normalized = atol(tokens[i].value.c_str());
                        continue;
                    }

                    auto parse_component_values = [this](const char *str, Accessor::ComponentValues &component, size_t &i) {
                        size_t index = i;
                        bool found_str = tokens[index].value == str;
                        while(!found_str) {
                            if(index >= tokens.size()) {
                                break;
                            }

                            find_next(Token::Type::literal, index);
                            found_str = tokens[index].value == str;
                        }

                        if(found_str && find_next(Token::Type::open_bracket, index)) {
                            i = index;

                            const Accessor::ComponentType type = info.accessors.back().component_type;

                            if(type != Accessor::ComponentType::unset) {
                                while(tokens[i].type != Token::Type::close_bracket) {
                                    if(tokens[i].type == Token::Type::literal) {
                                        component.push_back({});

                                        switch(type) {
                                            case Accessor::ComponentType::sbyte:
                                                info.accessors.back().parse_accessor_type<int8_t>(tokens[i].value.c_str(), component);
                                                break;
                                            case Accessor::ComponentType::ubyte:
                                                info.accessors.back().parse_accessor_type<uint8_t>(tokens[i].value.c_str(), component);
                                                break;
                                            case Accessor::ComponentType::sshort:
                                                info.accessors.back().parse_accessor_type<int16_t>(tokens[i].value.c_str(), component);
                                                break;
                                            case Accessor::ComponentType::ushort:
                                                info.accessors.back().parse_accessor_type<uint16_t>(tokens[i].value.c_str(), component);
                                                break;
                                            case Accessor::ComponentType::uint:
                                                info.accessors.back().parse_accessor_type<uint32_t, long int(const char *)>
                                                    (tokens[i].value.c_str(), component, atol);
                                                break;
                                            case Accessor::ComponentType::float32: {
                                                info.accessors.back().parse_accessor_type<float, double(const char *)>
                                                    (tokens[i].value.c_str(), component, atof);
                                                break;
                                            }
                                            default:
                                                break;
                                        }
                                    }

                                    i++;
                                }
                            } else {
                                fprintf(stderr, "Can't parse the \"%s\" component for accessor %d, because the \"componentType\" is unspecified!\n", str, static_cast<uint32_t>(info.accessors.size() - 1));
                            }
                        }
                    };

                    parse_component_values("max", info.accessors.back().max, i);
                    parse_component_values("min", info.accessors.back().min, i);
                }
            }
        }
    }

    void Parser::parse_buffer_views(void) {
        auto it = top_level_nodes.find("bufferViews");
        if(it != top_level_nodes.end()) {
            for(size_t i = it->second.first; i < it->second.second; i++) {
                if(tokens[i].type == Token::Type::open_brace) {
                    info.buffer_views.push_back({});

                    while(tokens[i].type != Token::Type::close_brace) {
                        if(tokens[i].type == Token::Type::literal) {
#define MAP_VALUE(str, member, type) \
                            if(tokens[i].value == str && find_next(Token::Type::literal, i)) { \
                                info.buffer_views.back().member = static_cast<type>(atol(tokens[i].value.c_str())); \
                                goto next; \
                            }

                            MAP_VALUE("buffer", buffer, uint32_t);
                            MAP_VALUE("byteOffset", byte_offset, uint32_t);
                            MAP_VALUE("byteLength", byte_length, uint32_t);
                            MAP_VALUE("byteStride", byte_stride, uint32_t);
                            MAP_VALUE("target", target, BufferView::Target);

#undef MAP_VALUE
                        }
next:
                        i++;
                    }
                }
            }
        }
    }

    void Parser::parse_buffers(void) {
        auto it = top_level_nodes.find("buffers");
        if(it != top_level_nodes.end()) {
            for(size_t i = it->second.first; i < it->second.second; i++) {
                if(tokens[i].type == Token::Type::open_brace) {
                    info.buffers.push_back({});
                    continue;
                }

                if(tokens[i].type == Token::Type::literal) {
                    if(tokens[i].value == "byteLength") {
                        find_next(Token::Type::literal, i);
                        info.buffers.back().byte_length = atoll(tokens[i].value.c_str());
                    } else if(tokens[i].value == "uri") {
                        find_next(Token::Type::literal, i);
                        info.buffers.back().uri = tokens[i].value;
                    }
                }
            }
        }
    }

}
