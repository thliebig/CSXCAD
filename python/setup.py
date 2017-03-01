from distutils.core import setup
from distutils.extension import Extension
from Cython.Build import cythonize

import os

extensions = [
    Extension("*", [os.path.join(os.path.dirname(__file__), "CSXCAD","*.pyx")],
        language="c++",             # generate C++ code
        libraries = ['CSXCAD',]),
]

setup(
  name="CSXCAD",
  version = '0.6.2',
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
