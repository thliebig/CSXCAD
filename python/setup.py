from setuptools import Extension, setup
from Cython.Build import cythonize
from pathlib import Path
import os

def looks_like_openEMS_is_installed_there(path_to_some_folder:Path):
    path_to_some_folder = Path(path_to_some_folder)
    FILES_THAT_SHOULD_EXIST = [
        path_to_some_folder/'bin/openEMS',
        path_to_some_folder/'include/openEMS/openems.h',
    ]
    if any([not p.is_file() for p in FILES_THAT_SHOULD_EXIST]):
       return False
    return True

if 'OPENEMS_INSTALL_PATH' not in os.environ:
    raise SystemExit('Please set the environment variable OPENEMS_INSTALL_PATH to point to where openEMS was installed. ')

path_to_openEMS_installation = Path(os.environ['OPENEMS_INSTALL_PATH']) # Environment variables are always strings, so this should never raise any error.

if not looks_like_openEMS_is_installed_there(path_to_openEMS_installation):
    raise SystemExit(f'I was expecting to find openEMS installed in {path_to_openEMS_installation}, but it does not look like it is installed there. ')

# Strictly speaking we should detect compiler, not platform,
# unfortunately there's no easy way to do so without implementing
# full compiler detection logic. This is good enough for 90% of
# use cases.
cxxflags = []
if os.name == "posix":
    cxxflags.append("-std=c++11")

extensions = [
    Extension(
        '*',
        ['CSXCAD/*.pyx'],
        include_dirs = [str(path_to_openEMS_installation/'include')],
        library_dirs = [str(path_to_openEMS_installation/'lib')],
        runtime_library_dirs = [str(path_to_openEMS_installation/'lib')],
        language = 'c++',
        libraries = ['CSXCAD'],
        extra_compile_args = cxxflags,
    )
]

setup(
    packages = ['CSXCAD'],
    ext_modules = cythonize(extensions),
)
