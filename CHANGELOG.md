# CSXCAD Changelog

Notable, user-visible changes in CSXCAD. The format follows
[Keep a Changelog](https://keepachangelog.com/en/1.1.0/); versions follow
`0.MINOR.PATCH`.

CSXCAD is an independent library and is versioned independently of openEMS.
The rendered version of this file, together with the openEMS changelog, is part
of the [openEMS documentation](https://docs.openems.de/).

**When you change something user-visible, add a bullet under `Unreleased` in the
same commit**, creating that section if it is not there — it exists only while
there are unreleased changes. It becomes the next release entry.

## Unreleased

### Added

- `CSPropDiscMaterial::GetPolyDataModel()` returns the discrete-material
  surface as plain point and quad arrays, in structure coordinates or in the
  mesh coordinates of the data file, and as numpy arrays in Python.
- Python: `SetRenderMode()`/`GetRenderMode()` on properties (stored as the
  `RenderMode` attribute), bulk `GetVertices()`/`GetFaces()`/`GetFacesValid()`
  on polyhedra, probe/dump `Get/SetStartTime()`, `Get/SetStopTime()` and
  `Get/SetMultiGridLevel()`, and `CSTransform.GetTransformList()` with the
  individual transformation steps (C++: `GetNumberOfTransforms()`,
  `GetTransformName()`, `GetTransformArguments()`). `GetAllPrimitives()` no
  longer fails on a user-defined primitive read from XML.
- `ReadFromXML()` finds the `ContinuousStructure` element anywhere in the
  document, not only directly below the root, so files that embed a structure
  load directly.

### Changed

- The Python bindings need Python 3.6 or newer (was 3.4), the oldest version
  CI still tests; CentOS 7 and Ubuntu 14.04 are no longer tested.
- C++: `CSPropDiscMaterial::CreatePolyDataModel()` is removed, use
  `GetPolyDataModel()`.
- Polyhedron vertices are double precision (were single), also when read
  from an ASCII STL or a PLY file. C++: `CSPrimPolyhedron::AddVertex()` takes
  and `GetVertex()` returns `double`.
- The inside of a polyhedron (also one read from an STL or PLY file) is found
  more robustly. Its surface counts as inside, as for a box, and so does a
  point within the drawing tolerance of it (`SetDrawingTolerance()`, now also
  in Python; openEMS sets it from the mesh). No face is dropped any more:
  bodies that touch, overlap or lie inside each other, cavities, faces of more
  than three corners and surfaces with small holes work. Results change for
  models with mesh lines on a polyhedron's surface, which used to be metal or
  not at random.
- **All primitives obey the drawing tolerance:** a point within it of a box,
  cylinder, sphere, shell, wire, polygon, linear or rotational polygon counts
  as inside, so coordinates rounded in the XML or computed (e.g. `sin(pi)`
  instead of 0) still meet the mesh lines they were meant for. The tolerance
  is a length also along the angle of cylindrical coordinates and for scaled
  primitives. The surface of spheres, spherical shells, wires and the stop
  angle of a rotational polygon now count as inside too, as for boxes, and so
  do the corners of polygons. User-defined primitives ignore the tolerance.

### Fixed

- A broken PLY file for a polyhedron could crash or give wrong faces; it now
  fails, and the message says what is wrong. ASCII STL files with upper-case
  keywords, facets of more than three corners, or no `endsolid` now load.

### Build

- C++17 is now required (was C++11), e.g. GCC 8 or newer, also for the
  Python bindings.
- VTK, CGAL and Boost (and with them GMP and MPFR) are no longer needed.

## 0.7.0 — 2026-10-09

### Added

- **Waveguide mode excitation and probe from an HDF5 mode file.** The new
  `CSModeData` class reads a 2-D mode field (`Vx`/`Vy`) from HDF5 and
  interpolates it bilinearly; the generic components serve both excitation and
  probe. Octave: `SetExcitationWeightFile.m` and the `ModeFileName` argument of
  `AddProbe.m`. Python: `GetWeightedExcitation()`.
- **New property `CSPropAbsorbingBC`** describing an absorbing boundary
  condition on a primitive, including Python bindings and geometry display.
- **Lumped RLC elements**: `CSPropLumpedElement` gained parallel and series
  R/L/C combinations, with Python support.
- **Djordjevic-Sarkar material model** for Octave/Matlab:
  `AddDjordjevicSarkarMaterial.m` and `CalcDjordjevicSarkarApprox.m`.
- **Python API for classes that were C++-only**: `CSBackgroundMaterial`,
  `CSPropDiscMaterial` (including coordinate/data queries and
  `SetTransform()`/`GetTransform()`) and `CSPrimMultiBox`.
- Copying support: `GetCopy()` on properties and primitives, and
  `CSTransform.copy()`.
- An excitation can be enabled and disabled (`SetEnabled()`/`GetEnabled()`).
- `SetCoordInputType()`/`GetCoordInputType()` on `CSProperties`, and colour and
  visibility accessors, from Python.
- `matlab/WriteCSX.m`, a user-friendly wrapper around `struct_2_xml`.
- `python/examples/StaticSolver.py`, a small electrostatic field solver example.
- Test suites: Python unit tests under `python/tests/` (including XML
  round-trip and wrapper-lifetime tests) and C++ tests under `tests/`, both run
  from CTest and CI.
- **Per-probe/dump `OverSampling` factor.** `CSPropProbeBox`, and thus
  `CSPropDumpBox`, can carry an optional integer oversampling factor, which
  CSXCAD stores and serializes but leaves the simulator to interpret — see the
  openEMS changelog. Octave: the `OverSampling` key of
  `AddProbe.m`/`AddDump.m`. Python: `over_sampling=` or
  `SetOverSampling()`/`GetOverSampling()`.

### Changed

- **Object lifetime (#83).** A new `CSObject` base class gives every object a
  destruction callback, an owner and a kind. Python wrappers no longer go stale:
  a wrapper whose C++ object has been destroyed raises `RuntimeError` instead of
  operating on freed memory, and `GetProperty()` no longer returns a wrapper of
  the wrong type after an address has been reused. `ContinuousStructure` now
  frees its C++ instance instead of leaking it. See *Upgrade notes*.
- **Mesh lines are always sorted and free of duplicates.** Sorting is deferred
  to the next read access, so an increasing `AddDiscLine()` costs nothing.
  `Sort()` and the `sorted` argument of `GetLines()`/`Write2XML()` are
  deprecated and have no effect any more. This also fixes `Snap2LineNumber()`,
  which returned garbage for lines added out of order. From Python, `GetLine()`
  now raises `IndexError` instead of returning `0` and accepts negative indices.
- The `INVALID` value was removed from the `CSPropLumpedElement` type enum.
- **Python packaging modernised**: `pyproject.toml`, installable with `pip`,
  dynamic versioning via `setuptools_scm`. The environment variable
  `OPENEMS_INSTALL_PATH` was renamed to `CSXCAD_INSTALL_PATH`, and the virtual
  environment now lives in `$CSXCAD_INSTALL_PATH/venv`.
- Octave/Matlab docstrings were reformatted as Markdown so that the online
  function reference can be generated from them.

### Fixed

- Python: `SetMaterialProperty()` and `SetDispersiveMaterialProperty()` accept
  numpy scalars, e.g. an element indexed from an array. `SetMaterialWeight()`
  and `SetDispersiveMaterialWeight()` no longer split a three-character `str`
  subclass into one weighting function per direction.
- XML writing is no longer affected by the numeric locale — under locales with a
  decimal comma the written files were malformed.
- `SmoothMeshLines` preserves user-supplied lines exactly through the symmetric
  reconstruction path, which perturbed them by about one ULP and silently
  dropped zero-thickness PEC primitives (openEMS-Project #470). A
  `check_symmetry` flag disables symmetry detection, and loop termination
  tolerates floating-point noise.
- A missing `RectilinearGrid` in `ReadFromXML()` is no longer fatal; an empty
  grid is valid.
- `CSRectGrid::Clone()` no longer drops the mesh type.
- Discrete materials: backward-compatible XML reading restored, data loaded
  before the VTK surface model is built, surfaces touching the data-set boundary
  are rendered, and `filetype` defaults to HDF5 in the Python binding when a
  filename is given.
- Serialisation fixes: `bVisible` is written; Lorentz material weights are
  encoded correctly; `Scale` and probe values are no longer truncated to `int`;
  the `CSPrimMultiBox` type name `Multi Box` produced invalid XML and is now
  `MultiBox`; `CSPropProbeBox` wrote a wrong `NormDir`/`uiNumber`.
- Polyhedron face handling, rotational-polygon copy initialisation, property
  copy constructors and dispersive-material handling.
- Assorted memory leaks, null dereferences and XML-safety problems found in
  code review.
- Windows: `CSBackgroundMaterial` symbols are exported, and the DLL is installed
  to `bin/`.
- `hdf5.h` is no longer pulled in by the public headers.
- `IsInside()` of cylinders, cylindrical shells, polygons, linear and
  rotational polygons and wires no longer returns `False` everywhere when the
  primitive was created or changed without a following `Update()`, e.g. from
  Python. Several setters also left the cached bounding box stale.

### Build

- C++11 is now required, and CMake 3.1 or newer.
- Boost is no longer a dependency.
- CGAL 6 is supported, and VTK is located with a modern `find_package()`.
- Windows builds via vcpkg manifest, with MSVC and clang-cl under Visual
  Studio 2022.
- CI covers Linux, macOS, FreeBSD and Windows and runs the smoke test and both
  test suites.

### Upgrade notes

- **Rebuild all components together.** Every class deriving from `CSObject`
  changed its layout. The soname is unchanged and will not catch a partial
  rebuild.
- A Python script that used a wrapper after its C++ object had been destroyed
  now stops with a `RuntimeError`. It previously appeared to work while reading
  freed memory. This is the one change users are most likely to notice.
- `Sort()` and `sorted=` are accepted but do nothing; mesh lines are always
  sorted. Code that relied on retrieving lines in insertion order needs to be
  adapted.
- Set `CSXCAD_INSTALL_PATH` where `OPENEMS_INSTALL_PATH` was used for the Python
  module.

## Older releases

Releases v0.5.2 (2014-08-16) through v0.6.3 (2023-10-22) were not recorded here.
See the [commit history](https://github.com/thliebig/CSXCAD/commits/master) or
compare two tags, for example
[`v0.6.2...v0.6.3`](https://github.com/thliebig/CSXCAD/compare/v0.6.2...v0.6.3).

The entries below are the original `NEWS` file, kept verbatim.

### v0.5.1

- matlab interface: throw an error when re-adding an already existing property (fix)

### v0.5.0

- new file-format for discrete materials
- new create function for discrete materials
- fix for curve primitive in cylindrical coordinates
- full multi-pole Lorentz/Drude/Debye material models
- extensive code-cleanup
- bug fixes
