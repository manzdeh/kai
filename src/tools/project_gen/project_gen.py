import os
import re
import shutil
import sys

def create_cpp_file(path):
    with open("{}/main.cpp".format(path), "wb") as main_file:
        main_file.write("#include \"kai/kai.h\"\n\n".encode("ascii"))
        main_file.write("KAI_GAME_INIT {\n}\n\n".encode("ascii"))
        main_file.write("KAI_GAME_UPDATE {\n}\n\n".encode("ascii"))
        main_file.write("KAI_GAME_DESTROY {\n}".encode("ascii"))

if __name__ == "__main__":
    print("Kai Game project generation\n")

    project_name = input("Project Name: ")
    project_name = re.sub("[^A-Za-z0-9]+", "_", project_name)

    print("Generating a project with name: {}".format(project_name))

    game_project_dir = input("Parent directory for {}: ".format(project_name))
    game_project_dir = "{}/{}".format(game_project_dir, project_name)
    try:
        os.mkdir(game_project_dir)
    except OSError:
        print("Failed to create directory {}. Aborting...".format(game_project_dir))
        sys.exit(-1)

    game_src_dir = "{}/src".format(game_project_dir)
    os.mkdir(game_src_dir)

    shutil.copytree("../../core/includes", "{}/kai".format(game_src_dir))

    create_cpp_file(game_src_dir)
