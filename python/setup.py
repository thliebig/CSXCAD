from setuptools import Extension, setup
from setuptools import __version__ as setuptools_version

import sys
import math
import os
import platform
import subprocess
import pathlib
import glob


def normalize_incorrect_path(path_str):
    path = pathlib.Path(path_str)
    if path.stem == "bin" and (path / "../include/CSXCAD").exists():
        # user error
        return (path / "../").resolve()
    else:
        return path


def determine_build_options():
    cpu_bits = str(int(math.log2(sys.maxsize) + 1))

    local_prefix_list = []
    use_prefix = True
    user_entered_prefix = False

    if "CSXCAD_INSTALL_PATH_IGNORE" is os.environ:
        use_prefix = False

    if use_prefix and 'CSXCAD_INSTALL_PATH' in os.environ:
        # Because CSXCAD and openEMS are usually installed to a non-standard
        # location such as the user's home directory rather than a standard
        # directory, their paths must be told to the compilers explicitly.
        # Since a general solution is impossible (the actual installation path
        # is unpredictable), users should set CSXCAD_INSTALL_PATH to ensure a
        # successful installation.
        local_prefix_list.append(
            normalize_incorrect_path(os.environ["CSXCAD_INSTALL_PATH"])
        )
        user_entered_prefix = True
    if 'VIRTUAL_ENV' in os.environ:
        # if Python venv used, our documentation recommends using the same
        # path as CSXCAD_INSTALL_PATH, so that the custom C++ and Python prefix
        # overlaps.
        local_prefix_list.append(
            pathlib.Path(os.environ["VIRTUAL_ENV"])
        )
        user_entered_prefix = True

    if use_prefix and (not user_entered_prefix):
        raise RuntimeError(
            "No environment variable CSXCAD_INSTALL_PATH or VIRTUAL_ENV found, "
            "installation may fail due to missing headers and libraries! "
            "Please set the environment variable CSXCAD_INSTALL_PATH to the path "
            "of CSXCAD/openEMS installation, check documentation for details."
            "If you know what you're doing, set CSXCAD_INSTALL_PATH_IGNORE=1 to "
            "suppress this error."
        )

    if platform.system() == "Darwin":
        # In additional to libraries in $CSXCAD_INSTALL_PATH and $VIRTUAL_ENV, we
        # also need to list custom headers and libraries installed to the local
        # system but are not used by compilers by default (such as a custom Boost).
        # On macOS, the required prefix are -L $(brew --prefix)/include and
        # -R $(brew --prefix)/lib respectively. Hardcode it as a special treatment.
        try:
            status = subprocess.run(["brew", "--prefix"], capture_output=True)
            path = status.stdout.decode("UTF-8").replace("\n", "")
            local_prefix_list.append(
                pathlib.Path(path)
            )
        except FileNotFoundError:
            pass
    if os.name == "posix":
        # The path /usr/local is also too common on Unix systems, so we hardcode it
        # as a special treatment on Unix-like systems. For example, on CentOS, the
        # paths -L /usr/local/include and -R /usr/local/lib must be listed if a
        # custom Boost is installed here.
        local_prefix_list.append(pathlib.Path("/usr/local"))

    build_options = {
        "extra_compile_args": [],
        "include_dirs": [],
        "library_dirs": [],
        "runtime_library_dirs": [],
    }

    for prefix in local_prefix_list:
        prefix_path = pathlib.Path(prefix)
        if os.name == 'nt':
            build_options["library_dirs"] += [str(prefix_path)]

        build_options["include_dirs"] += [
            str(prefix_path / "include")
        ]
        build_options["library_dirs"] += [
            str(prefix_path / "lib"),
            str(prefix_path / "lib" / cpu_bits)
        ]
        build_options["runtime_library_dirs"] += [
            str(prefix_path / "lib"),
            str(prefix_path / "lib" / cpu_bits)
        ]

    # Strictly speaking we should detect compiler, not platform,
    # unfortunately there's no easy way to do so without implementing
    # full compiler detection logic. This is good enough for 90% of
    # use cases.
    if os.name == "posix":
        build_options["extra_compile_args"].append("-std=c++11")

    # Setting this will cause an exception during build on Windows platforms.
    if os.name != "posix":
        del build_options["runtime_library_dirs"]

    return build_options


def get_modules_list(module_prefix, path_glob_pattern, build_options):
    output_list = []

    if int(setuptools_version.split(".")[0]) < 18:
        from Cython.Build import cythonize
        output_list = cythonize(
            Extension(name="*", sources=[path_glob_pattern], **build_options)
        )
    else:
        # Above setuptools 18, setuptools contains a special case for `.pyx`
        # files if setup_requires=["cython"] is set, pyx sources are auto-
        # Cythonized. This is recommended since setup.py / pip still runs
        # even if Cython is not installed. But wildcards are not supported,
        # we collect modules by hand.

        # e.g. [CSXCAD/CSPrimitives.pyx, CSXCAD/CSXCAD.pyx, ...]
        filepath_list = glob.glob(path_glob_pattern)
        for filepath in filepath_list:
            # e.g. CSXCAD/CSPrimitives.pyx
            filename = os.path.basename(filepath)
            # e.g. CSXCAD.CSPrimitives
            module_name = module_prefix + filename.replace(".pyx", "")

            output_list.append(
                Extension(name=module_name, sources=[filepath], **build_options)
            )
    return output_list


build_opt = determine_build_options()
build_opt["language"] = "c++"
build_opt["libraries"] = ["CSXCAD"]

extensions = get_modules_list(
    module_prefix="CSXCAD.",
    path_glob_pattern="CSXCAD/*.pyx",
    build_options=build_opt
)

setup(
  name="CSXCAD",
  packages=["CSXCAD", ],
  package_data={'CSXCAD': ['*.pxd']},
  # DO NOT add any new build-time dependency in setup_requires.
  # We should use pyproject.toml exclusively. The only item
  # "cython" is meant to activate auto-Cython feature in
  # setuptools 18.
  setup_requires=[
    'cython'
  ],
  # DO add new run-time dependencies here to keep it in sync with
  # pyproject.toml.
  install_requires=[
    # BSD 3-Clause (https://github.com/numpy/numpy/blob/main/LICENSE.txt)
    'numpy',
    # Matplotlib license (https://github.com/matplotlib/matplotlib/blob/main/LICENSE/LICENSE)
    "matplotlib >= 2.1.0"
  ],
  ext_modules=extensions
)
