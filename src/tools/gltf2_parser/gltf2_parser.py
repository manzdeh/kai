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
        self.size = 0

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

def parse_json_data(data, dir_path):
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

    for key in attributes:
        if key == "POSITION":
            copy_buffer_contents(data, attributes[key], bin_buffer, position_data)
        elif key.startswith("TEXCOORD"):
            texcoords_data.append([])
            copy_buffer_contents(data, attributes[key], bin_buffer, texcoords_data[len(texcoords_data) - 1])

        accessor = data["accessors"][attributes[key]]

        mesh.vertices.stride += (int(accessor["count"]) * get_element_count(accessor["type"]))
        mesh.vertices.count = max(mesh.vertices.count, int(data["accessors"][attributes[key]]["count"]))

    if "indices" in primitive:
        copy_buffer_contents(data, primitive["indices"], bin_buffer, ibuffer_data)

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

    parse_json_data(json.load(f), os.path.dirname(file_path))
