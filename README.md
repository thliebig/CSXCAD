# CSXCAD

CSXCAD (Continuous Structure XML CAD) is a C++ library for describing geometrical objects and their physical or non-physical properties. It is designed as a standalone, reusable geometry and material description library suitable for electromagnetic simulation tools and similar applications.

Licensed under the [LGPLv3](https://www.gnu.org/licenses/lgpl-3.0.html).

**Documentation:** [docs.openems.de](https://docs.openems.de)

---

## Dependencies

- CMake ≥ 3.0
- C++11 compiler (GCC, Clang, MSVC)
- [fparser](http://warp.povusers.org/FunctionParser/) — parametric expression evaluation
- [TinyXML](https://sourceforge.net/projects/tinyxml/) — XML serialization
- [HDF5](https://www.hdfgroup.org/solutions/hdf5/) — discrete material file I/O
- [VTK](https://vtk.org/) — geometry export
- [CGAL](https://www.cgal.org/) — polyhedron operations

> **Note:** The git clone must be **deep** (not shallow). CMake uses `git describe --tags --always` for the version string and will fail on shallow clones.

---

## Build and Install (C++ Library)

```bash
mkdir build && cd build
cmake .. -DCMAKE_INSTALL_PREFIX=~/opt -DFPARSER_ROOT_DIR=~/opt
make -j$(nproc)
make install
```

Optional local CMake overrides can be placed in `localConfig.cmake` at the repo root (gitignored).

### Windows

On Windows, [vcpkg](https://vcpkg.io) is used for TinyXML, HDF5, VTK, and CGAL (see `vcpkg.json`):

```bash
cmake .. -DCMAKE_TOOLCHAIN_FILE=$VCPKG_ROOT/scripts/buildsystems/vcpkg.cmake
```

---

## Python Bindings

The `python/` directory contains Cython bindings. The C++ library must be built and installed first.

```bash
export CSXCAD_INSTALL_PATH=~/opt
cd python
pip3 install .
```

Always run tests or scripts from **outside** the `python/` source directory to avoid accidentally importing the uncompiled source instead of the installed extension.

### Quick Example

```python
from CSXCAD.CSXCAD import ContinuousStructure

csx = ContinuousStructure()

# Add a metal property with a box primitive
metal = csx.AddMetal('copper')
metal.AddBox(start=[-1, -1, -1], stop=[1, 1, 1])

# Set background material
bg = csx.GetBackgroundMaterial()
bg.SetEpsilon(1.0)

# Serialize to XML (readable by any tool that supports the CSXCAD format)
csx.Write2XML('my_structure.xml')
```

### Running the Tests

```bash
cd python/tests
python3 -m unittest
```

Run a single module or test case:

```bash
python3 -m unittest test_CSPrimitives
python3 -m unittest test_CSPrimitives.TestCSPrimitives.test_box
```

---

## Matlab / Octave Interface

The `matlab/` directory contains a pure M-file interface (no compilation required). It builds the CSX data structure as a Matlab/Octave struct and writes it to XML for consumption by CSXCAD.

Add the `matlab/` directory to your Matlab/Octave path:

```matlab
addpath('/path/to/CSXCAD/matlab');
```

Basic usage:

```matlab
CSX = InitCSX();
CSX = AddMetal(CSX, 'copper');
CSX = AddBox(CSX, 'copper', 10, [-1 -1 -1], [1 1 1]);
WriteCSX('my_structure.xml', CSX);
```

---

## Related Projects

- [openEMS](https://github.com/thliebig/openEMS) — FDTD simulator using CSXCAD for geometry input
- [openEMS-Project](https://github.com/thliebig/openEMS-Project) — meta-repository combining openEMS and all its dependencies including CSXCAD
