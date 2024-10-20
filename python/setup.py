from setuptools import Extension, setup
from Cython.Build import cythonize
from pathlib import Path

PATH_TO_OPEN_EMS_INSTALLATION = Path.home()/'opt/openEMS'

extensions = [
    Extension(
        '*', 
        ['CSXCAD/*.pyx'],
        include_dirs = [str(Path(PATH_TO_OPEN_EMS_INSTALLATION)/'include')],
        library_dirs = [str(Path(PATH_TO_OPEN_EMS_INSTALLATION)/'lib')],
        runtime_library_dirs = [str(Path(PATH_TO_OPEN_EMS_INSTALLATION)/'lib')],
        language = 'c++',
        libraries = ['CSXCAD'],
    )
]

setup(
    packages = ['CSXCAD'],
    ext_modules = cythonize(extensions),
)

