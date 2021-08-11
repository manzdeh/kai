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
    print("Kai game project generator\n")

    project_name = input("Project name: ")
    project_name = re.sub("[^A-Za-z0-9]+", "_", project_name)

    print("Generating a project with name: {}".format(project_name))

    project_dir = input("Parent directory for {}: ".format(project_name))
    project_dir = "{}/{}".format(project_dir, project_name)
    try:
        os.mkdir(project_dir)
    except OSError:
        print("Failed to create directory {}. Aborting...".format(project_dir))
        sys.exit(-1)

    src_dir = "{}/src".format(project_dir)
    os.mkdir(src_dir)

    shutil.copytree("../../core/includes", "{}/kai".format(src_dir))
    shutil.copyfile("build_template.txt", "{}/build.bat".format(project_dir))
    shutil.copyfile("build_and_fetch_engine_template.txt", "{}/build_and_fetch_engine.bat".format(project_dir))
    shutil.copyfile("../../../bin/kai.exe", "{}/kai.exe".format(project_dir))
    shutil.copyfile("../../../bin/kai.lib", "{}/kai.lib".format(project_dir))

    create_cpp_file(src_dir)
