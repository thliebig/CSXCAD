from distutils.core import setup
from distutils.extension import Extension
from Cython.Build import cythonize

VERSION = '0.6.0'

VTK_LIBS = '''vtkCommon vtkFiltering vtkImaging vtkGraphics vtkGenericFiltering vtkIO vtkRendering vtkVolumeRendering vtkHybrid vtkWidgets
vtkParallel vtkInfovis vtkGeovis vtkViews vtkCharts'''.split()

extensions = [
    Extension("*", ["CSXCAD/*.pyx"],
        language="c++",             # generate C++ code
        include_dirs = ['.', '../src/', ],
        libraries = ['CSXCAD',] + VTK_LIBS),
]

setup(
  name="CSXCAD",
  version = VERSION,
  description = "Python interface for the CSXCAD library",
  classifiers = [
      'Development Status :: 3 - Alpha',
      'Intended Audience :: Developers',
      'Intended Audience :: Information Technology',
      'Intended Audience :: Science/Research',
      'License :: OSI Approved :: GNU Lesser General Public License v3 or later (LGPLv3+)',
      'Programming Language :: Python',
      'Topic :: Scientific/Engineering',
      'Topic :: Software Development :: Libraries :: Python Modules',
      'Operating System :: POSIX :: Linux',
      'Operating System :: Microsoft :: Windows',
  ],
  author = 'Thorsten Liebig',
  author_email = 'Thorsten.Liebig@gmx.de',
  maintainer = 'Thorsten Liebig',
  maintainer_email = 'Thorsten.Liebig@gmx.de',
  url = 'http://openEMS.de',
  packages=["CSXCAD", ],
  package_data={'CSXCAD': ['*.pxd']},
  ext_modules = cythonize(extensions)
 )
