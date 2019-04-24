#!/usr/bin/env python

import os
import stat
from sys import platform
from shutil import rmtree
from subprocess import check_call

def get_platform_type():
    if platform == "linux" or platform == "linux2" or platform == "darwin":
        return "unix"
    elif platform == "win32":
        return "windows"
    else:
        raise ValueError("Unknown platform.")

def resolve_path(rel_path):
    return os.path.abspath(os.path.join(os.path.dirname(__file__), rel_path))

def makedirs_silent(root):
    try:
        os.makedirs(root)
    except:
        pass

if __name__ == "__main__":
    platform_type = get_platform_type()

    if platform_type == "unix":
        build_dir = resolve_path("bin/web")
    elif platform_type == "windows":
        build_dir = resolve_path(".\\bin\\web")

    makedirs_silent(build_dir)
    os.chdir(build_dir)

    if platform_type == "unix":
        os.system("emcmake cmake ../.. -DEMSCRIPTEN=ON -G \"Unix Makefiles\"")
        os.system("make")
    elif platform_type == "windows":
        os.system("emcmake cmake ../.. -DEMSCRIPTEN=ON -G \"NMake Makefiles\"")
        os.system("nmake")
