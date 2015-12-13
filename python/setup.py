from distutils.core import setup
from distutils.extension import Extension
from Cython.Build import cythonize

import os

SRC_PATH = os.path.join(os.path.dirname(__file__), '../src')
print(SRC_PATH)

VTK_LIBS = '''vtkCommon vtkFiltering vtkImaging vtkGraphics vtkGenericFiltering vtkIO vtkRendering vtkVolumeRendering vtkHybrid vtkWidgets
vtkParallel vtkInfovis vtkGeovis vtkViews vtkCharts'''.split()

extensions = [
    Extension("*", ["pyCSXCAD/*.pyx"],
        language="c++",             # generate C++ code
        include_dirs = ['.', '../src/', ],
        libraries = ['CSXCAD',] + VTK_LIBS,
        library_dirs = ['../lib/', ]),
]

setup(
  name="pyCSXCAD",
  packages=["pyCSXCAD", ],
 ext_modules = cythonize(extensions)
 )
