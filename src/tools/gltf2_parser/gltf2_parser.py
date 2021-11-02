import sys
import os.path
import json

class VertexData:
    def __init__(self):
        self.count = 0
        self.start = 0
        self.size = 0
        self.stride = 0

class IndexData:
    def __init__(self):
        self.count = 0
        self.start = 0
        self.size = 0

class Texcoord:
    def __init__(self):
        self.count = 0
        self.start = 0

# NOTE: The order here needs to match the definition of the MeshHeader type defined in the engine
class Mesh:
    def __init__(self):
        self.buffer_size = 0
        self.buffer_start = 0

        self.vertices = VertexData()
        self.indices = IndexData()
        self.texcoords = Texcoord()

def copy_buffer_contents(data, value, bin_buffer, buf):
    buffer_view = data["bufferViews"][data["accessors"][value]["bufferView"]]
    start = buffer_view["byteOffset"]
    end = start + buffer_view["byteLength"]
    buf.append(bin_buffer[start:end])

def get_element_count(elem):
    match elem:
        case "SCALAR": return 1
        case "VEC2": return 2
        case "VEC3": return 3
        case "VEC4": return 4
        case "MAT2": return 4
        case "MAT3": return 9
        case "MAT4": return 16

def get_component_size(component):
    match component:
        case 5120: return 1 # BYTE
        case 5121: return 1 # UNSIGNED_BYTE
        case 5122: return 2 # SHORT
        case 5123: return 2 # UNSIGNED_SHORT
        case 5125: return 4 # UNSIGNED_INT
        case 5126: return 4 # FLOAT

def add_attribute_data(data, mesh_buffer, attr_str, attr_data, it):
    attributes = data["meshes"][0]["primitives"][0]["attributes"]
    accessor = data["accessors"][attributes[attr_str]]
    end = get_element_count(accessor["type"]) * get_component_size(accessor["componentType"])
    start = it * end
    end += start
    mesh_buffer += attr_data[start : end]

def parse_json_data(data, dir_path, file_name):
    if data["asset"]["version"] != "2.0":
        print("ERROR: Only glTF 2.0 files can be parsed!")
        quit()

    if len(data["meshes"]) != 1 or len(data["meshes"][0]["primitives"]) != 1:
        print("ERROR: Currently only glTF files with a single mesh and primitive are supported!")
        quit()

    mesh = Mesh()

    position_data = []
    texcoords_data = []
    ibuffer_data = []

    primitive = data["meshes"][0]["primitives"][0]
    attributes = primitive["attributes"]

    # NOTE: We always assume we only have 1 buffer in the file and we always read from that one
    with open(dir_path + "/" + data["buffers"][0]["uri"], "rb") as f:
        bin_buffer = f.read()

    set_texcoord_start = False

    for key in attributes:
        if key == "POSITION":
            copy_buffer_contents(data, attributes[key], bin_buffer, position_data)
        elif key.startswith("TEXCOORD"):
            texcoords_data.append([])
            copy_buffer_contents(data, attributes[key], bin_buffer, texcoords_data[len(texcoords_data) - 1])
            if not set_texcoord_start:
                mesh.texcoords.start = mesh.vertices.stride
                set_texcoord_start = True


        accessor = data["accessors"][attributes[key]]

        mesh.vertices.stride += (get_element_count(accessor["type"]) * get_component_size(accessor["componentType"]))
        mesh.vertices.count = max(mesh.vertices.count, int(accessor["count"]))

    if "indices" in primitive:
        indices_accessor_idx = primitive["indices"]
        copy_buffer_contents(data, indices_accessor_idx, bin_buffer, ibuffer_data)

        accessor = data["accessors"][indices_accessor_idx]

        mesh.indices.count = accessor["count"]
        mesh.indices.size = mesh.indices.count * get_element_count(accessor["type"]) * get_component_size(accessor["componentType"])

    mesh.vertices.start = 0
    mesh.vertices.size = mesh.vertices.stride * mesh.vertices.count

    mesh.indices.start = mesh.vertices.size

    mesh.texcoords.count = len(texcoords_data)

    mesh_buffer = []
    for i in range(0, mesh.vertices.count):
        if len(position_data) > 0:
            add_attribute_data(data, mesh_buffer, "POSITION", position_data[0], i)

        if len(texcoords_data) > 0:
            for j in range(0, len(texcoords_data)):
                add_attribute_data(data, mesh_buffer, "TEXCOORD_{}".format(j), texcoords_data[j][0], i)

    if len(ibuffer_data) > 0:
        mesh_buffer += ibuffer_data[0]

    output_dir = os.path.join(os.path.dirname(os.path.realpath(__file__)), "output")

    if not os.path.exists(output_dir):
        os.makedirs(output_dir)

    mesh_path = os.path.join(output_dir, file_name + ".bin")

    mesh_file = open(mesh_path, "wb")
    for i in range(0, len(mesh_buffer)):
        mesh_file.write(bytes([mesh_buffer[i]]))
    mesh_file.close()

def print_usage():
    print("    Usage: gltf2_parser.py \"<path to gltf file>\"")

if __name__ == "__main__":
    argc = len(sys.argv)

    if argc > 2:
        print("ERROR: Too many arguments supplied!")
        print_usage();
        quit()
    elif argc == 1:
        print("ERROR: No arguments supplied!")
        print_usage();
        quit()

    file_path = sys.argv[1]

    f = open(file_path, "r")

    parse_json_data(json.load(f), os.path.dirname(file_path), os.path.basename(file_path).split(".")[0])
