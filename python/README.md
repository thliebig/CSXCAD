# CSXCAD Python Interface

CSXCAD Python extension, providing a binding to the C++ CSXCAD library.

## Latest documentation

This file may be outdated. For the latest documentation, check
the online web page.

* [CSXCAD/openEMS Python Interface Installation](https://openems.readthedocs.io/en/latest/python/install.html)

### Dependencies

This file assumes readers have already installed required C++
and Python dependencies (including compiling and installing
the CSXCAD C++ library into the system). If not, follow the
online manual.

* [Requirements of Building CSXCAD/openEMS](https://openems.readthedocs.io/en/latest/install/index.html)

The manual includes instructions for multiple systems (Alpine, AlmaLinux,
CentOS, Debian, Fedora, Ubuntu, FreeBSD, macOS), they're not repeated here
for brevity.

## Quick Start

If the CSXCAD library was installed into `~/opt/physics`, install this
package with:

```bash
# create an isolated venv in ~/opt/physics/venv and activate it
python3 -m venv $HOME/opt/physics/venv
source $HOME/opt/physics/venv/bin/activate

# build CSXCAD Python extension (CSXCAD_INSTALL_PATH must be set!)
# Only CSXCAD is needed here, openEMS itself is optional
export CSXCAD_INSTALL_PATH=$HOME/opt/physics
pip3 install .
```

Replace `$HOME/opt/physics` with the path prefix to CSXCAD.

Once installed, test CSXCAD from a neutral directory. Don't test
CSXCAD in the source code directory `CSXCAD/python` to avoid importing
local files.

```bash
$ cd /  # Important: always leave "CSXCAD/python" first.

$ cd / && python3 -c "import CSXCAD; print(CSXCAD.ContinuousStructure())"
<CSXCAD.CSXCAD.ContinuousStructure object at 0x7f5957943fd0>

$ cd / && python3 -c "import CSXCAD; print(CSXCAD.__version__)"
0.6.4.dev76+gccb4c218e
```

## Environment Variables

The following environment variables control the behaviors of the
Python extension installation.

1. **(Required)** `CSXCAD_INSTALL_PATH`:
path prefix of the CSXCAD installation. Without these
variables, installation is terminated with an error.

3. **(Optional)** `VIRTUAL_ENV`: path prefix of the Python `venv`,
set automatically if a Python `venv` is activated. If `venv` exists
in the C++ path prefix's `/venv` subdirectory
(`VIRTUAL_ENV=$CSXCAD_INSTALL_PATH/venv`), or overlaps
(`VIRTUAL_ENV=$CSXCAD_INSTALL_PATH`),
`CSXCAD_INSTALL_PATH` can be omitted, activating the venv is sufficient.

4. **(Optional)** `CSXCAD_INSTALL_PATH_IGNORE`:
disable `CSXCAD_INSTALL_PATH` usages and error checking. Useful
only if their installation paths are specified manually through
other methods, such as `CXXFLAGS` or `LDFLAGS`.

5. **(Optional)** `CSXCAD_NOSCM`: pip no
longer downloads `setuptools_scm`, git-based version numbers are
no longer generated.

## Basic Install

### Step 1: Set `CSXCAD_INSTALL_PATH`

The environment variable `CSXCAD_INSTALL_PATH` must be set to ensure a
successful installation. If `CSXCAD_INSTALL_PATH` is not set, a
`RuntimeError` is generated.

If the CSXCAD library was installed into `~/opt/physics`, set
`CSXCAD_INSTALL_PATH` with:

```bash
export CSXCAD_INSTALL_PATH="$HOME/opt/physics"
```

Replace `$HOME/opt/physics` with the path prefix to CSXCAD.

You
should be able to find `/lib` and `/include` in this prefix:

```bash
$ ls $CSXCAD_INSTALL_PATH
bin  include  lib  lib64  share

$ ls $CSXCAD_INSTALL_PATH/include
CSXCAD  fparser.hh

$ ls $CSXCAD_INSTALL_PATH/lib
libCSXCAD.so  libCSXCAD.so.0  libCSXCAD.so.0.6.3  libfparser.so  libfparser.so.4  libfparser.so.4.5.1
```

`CSXCAD_INSTALL_PATH` is used by both CSXCAD and openEMS, since
both projects are typically used together (although CSXCAD is
an independent library).

As a hardcoded special case, the path of the current Python venv
(`VIRTUAL_ENV`) is also considered as a search path prefix by default.
If your C++ and Python `venv` paths exactly overlap, one doesn't need
to set any environment variables if a Python venv is activated prior
to installation. We still use `$CSXCAD_INSTALL_PATH` throughout the
documentation for consistency.

### Step 2: venv

Installing Python packages into Python's default search paths
(such as `/usr/` in the base system, or `~/.local` in the home
directory) is discouraged by most operating systems, because
there's a risk of dependency conflicts between a system-supplied
and a user-installed Python package.

To ensure Python packages are installed in a conflict-free manner,
it's suggested by most systems to create an isolated environment
for Python packages, known as a *virtual environment* (`venv`).

If you have never created your own `venv` before, create a
`venv` specifically for CSXCAD (and optionally openEMS) now:

```bash
python3 -m venv $HOME/opt/physics/venv/
```

This creates the Python venv in a subdirectory of `$CSXCAD_INSTALL_PATH`.
But if you prefer separation, you can use a different path, such
as `~/venvs/csxcad`, or activate an existing `venv` you already
have.

Remember, if the Python extension has been installed to an
isolated `venv`, all Python scripts that use CSXCAD
can only be executed inside this venv while it's activated.
Likewise, only Python packages installed into the venv can be
seen.

The venv can be entered via:

```bash
source $HOME/opt/physics/venv/bin/activate

# leave the venv with "deactivate"
```

Once the venv is activated, follow the next steps.

### Step 3: pip

Assuming that the correct `CSXCAD_INSTALL_PATH` have already been set, and a
`venv` has been activated, run:

```bash
pip3 install .
```

When installing packages inside a `venv`, avoid using `--user` because
it doesn't respect the activated `venv`, effectively undoing it.

## Advanced Install

This section is written for experienced users, sysadmins and
developers who need to troubleshoot or customize their installation.
Ordinary users can skip this section.

### Suppress `RuntimeError`

CSXCAD is usually installed to a non-standard location such as
`~/opt/physics` in the user home directory (along with openEMS).
By default, system compilers are unable to find necessary CSXCAD
C++ library, because only global path prefixes such as `/usr/`
or `/usr/local` are considered.

A `RuntimeError` is generated if `CSXCAD_INSTALL_PATH` is not
set. If you know what you're doing (e.g., the CSXCAD library is
already added to the compiler search paths manually), you can bypass
this error with:

```bash
export CSXCAD_INSTALL_PATH_IGNORE=1
```

### Search Path Management

By default, all necessary search paths are configured automatically
by `CSXCAD_INSTALL_PATH` or `VIRTUAL_ENV`. On all Unix-like system,
`/usr/local` is also always added into the search paths as another
hardcoded special case, regardless of the system's default search paths.
Likewise, on macOS, the prefix reported by `brew --prefix` is
automatically added to the search paths.

If manual control is needed, set `CXXFLAGS` and `LDFLAGS` instead of
`CSXCAD_INSTALL_PATH`. These flags include the following arguments:

* `-I`: header include path, including the `/include` suffix.
* `-L`: library linking path, including the `/lib` suffix.
* `-Wl,-rpath,`: library runtime path, including the `/lib` suffix.

The following example assumes the CSXCAD installation prefix is
`$HOME/opt/physics`, and some dependent libraries have been installed
to `/usr/local`.

```bash
export CSXCAD_INSTALL_PATH_IGNORE=1

export CXXFLAGS="-I$HOME/opt/physics/include -I/usr/local/include $CXXFLAGS"
export LDFLAGS="-L$HOME/opt/physics/lib -L/usr/local/lib -Wl,-rpath,$HOME/opt/physics/lib $LDFLAGS"
```

To use these options properly, one needs to understand the motivation
behind specifying them. Basically, building a Python module requires
headers and libraries from three distinct sources:

1. Standard global headers and libraries provided by the system,
   and used by compilers by default. Typical paths are `/usr/include`
   and `/usr/lib`.
   They paths *do not* need any special listing, since they're used
   by default. All dependencies installed by the system's package
   manager typically also belong to this category, without special
   treatment (but exceptions exist, such as macOS Homebrew).

2. Non-standard global headers and libraries installed by the user
   (usually dependencies such as a custom Boost or VTK newer than the
   system's own version). They're outside the system's control, and
   not used by compilers by default.
   For example, on CentOS, the paths
   `-L/usr/local/include` and `-Wl,-rpath,/usr/local/lib` *must be*
   listed if any custom packages are installed to `/usr/local`.
   Unlike other system-wide package managers, macOS's Homebrew
   also belong to this category, because it's a 3rd-party package
   manager, thus it requires
   `-L$(brew --prefix)/include` and `-Wl,-rpath,$(brew --prefix)/lib`.

3. Non-standard local CSXCAD headers and libraries.
   These files are usually installed to an arbitrary prefix in the user's
   home directory, not used by any compilers by default, as such
   `-L$HOME/opt/physics/include` and `-Wl,-rpath,$HOME/opt/physics/lib`.
   These paths *must be* listed.

If multiple paths are needed, repeat the option for each path, and
separate each option by spaces.

### Expose System Packages in venv

In an isolated `venv`, only Python packages installed into the `venv` can
be seen. Optionally, one can expose external system-wide packages to a
`venv` via `--system-site-packages` during `venv` creation:

```bash
# create venv, expose system packages
# (run once during installation)
python3 -m venv --system-site-packages $HOME/opt/physics
```

In this `venv`, the packages within `venv` stays within
the `venv`, but system-wide packages are also available. Activation
is still needed prior to using CSXCAD in Python.

```bash
source $HOME/opt/physics/venv/bin/activate
```

### Install Python Extension to Home Directory Instead of venv

It's possible to install a package via `pip` into the default
Python search path under a user's home directory via `--user`.

After 2021, this practice is deprecated on most systems by
[PEP 668](https://peps.python.org/pep-0668/), since it bypasses
a system's own package manager, risking dependency conflicts.
The new option `--break-system-packages` is required.

```bash
pip3 install . --user --break-system-packages
```

This is still the default behavior of CSXCAD and openEMS when using
the `./update_openEMS.sh` script from `openEMS-Project.git`. If this
is undesirable, activate a `venv` manually prior to calling it.
We plan to improve the script in the future.

As suggested by the option `--break-system-packages`, it has the
risk of creating dependency conflicts between the same package
from the system and from `pip`. Using `--break-system-packages` is
only considered safe if all Python dependencies are installed via
your system's package manager (e.g. `apt`, `dnf`), as recommended in
the [documentation](https://openems.readthedocs.io/en/latest/install/requirements.html),
prior to running `pip3 install .`. Otherwise, `pip` may attempt to
install dependent packages on its own, risking dependency conflicts
with system packages.

### Installation without Re-downloading

By default, `pip` prefers to ignore existing packages in the
system, aggressively redownloading them, either for constructing
a fresh user `venv`, or for constructing the isolated build
environment. It can be problematic if you want to manage
packages external to `pip`, or if Internet access is not always
online.

It's possible to suppress most redownloading behaviors, making
it a useful solution for external package management.

#### Install Dependencies

To manage packages manually, ensure that all dependencies have
been installed via your system's package manager. In theory,
one can use a DVD as the software repository. A full list
of package manager dependencies on various systems can be
found in the [documentation](https://openems.readthedocs.io/en/latest/install/requirements.html):

#### Expose System Packages to `venv`

Create a `venv`, and expose existing system packages, so we don't
need to install anything into the venv if the system already has
them (only needed to run once during installation).

```bash
python3 -m venv --system-site-packages $HOME/opt/physics
```

#### Disable Build Isolation

During installation via `pip`, `pip` will redownload build-time
dependencies such as `setuptools`, or `cython`, even when those
dependencies are already available to the system or a `venv`.

This is the result of the *build isolation* feature in `pip`.
When building packages, `pip` creates an internal `venv` for itself,
isolated from both the base system and a user's own `venv`. This way,
it allows users to install packages with conflicting build-time
dependencies.

By default, CSXCAD also uses `setuptools_scm` to automatically create
a version number based on the current `git` history. Since a fairly
new version is required, pre-installation via a system's package
manager may be impractical.

Both behaviors can disabled via:

```bash
export CSXCAD_NOSCM=1
pip3 install . --no-build-isolation
```

The variable `CSXCAD_NOSCM` is specific to CSXCAD.

### Legacy Installation via `setup.py`

`setup.py` method was the traditional way of building and installing
Python extensions. It has been deprecated by Python developers in favor
of `pip`. Follow this section only for the purpose of debugging a build.

Assuming that the correct `CSXCAD_INSTALL_PATH` have already been set
(or have been bypassed via `CSXCAD_INSTALL_PATH_IGNORE`), this extension
can be built manually via:

```bash
export CSXCAD_INSTALL_PATH=$HOME/opt/physics

python setup.py build_ext

# install to user's home directory, equivalent to
# pip3 install . --user --break-system-packages
python setup.py install --user

# if using a venv, remove --user so the venv path is respected
# python setup.py install
```

#### Advanced: setup.py search path management

On Unix-like systems, one can use the standard `CXXFLAGS` and
`LDFLAGS` to control compiler headers and libraries paths for
both `pip` and `setup.py` in the beginning of this tutorial.

In `setup.py`, it also provides its own custom options. Their
uses are not necessary, they're introduced there:

* `-I`: header include path, including the `/include` suffix, colon-separated.
* `-L`: library linking path, including the `/lib` suffix, colon-separated.
* `-R`: library runtime path, including the `/lib` suffix, colon-separated.

The following example assumes the CSXCAD installation prefix is
`$HOME/opt/physics/`, and some libraries have been installed to
`/usr/local`.

```bash
export CSXCAD_INSTALL_PATH_IGNORE=1

python3 setup.py build_ext \
  -I "$HOME/opt/physics/include:/usr/local/include" \
  -L "$HOME/opt/physics/lib:/usr/local/lib" \
  -R "$HOME/opt/physics/lib"
```

## Troubleshooting

### ModuleNotFoundError: No module named 'CSXCAD.CSXCAD'

Ensure you're not running `python` under `CSXCAD/python` of the
source code tree. Otherwise, Python attempts to import the incomplete
source code instead of the complied Python extension.

```bash
cd /

source $HOME/opt/physics/venv/bin/activate
python3 -m "import CSXCAD"
```

If the error persists, debug the installation by running `pip` in
verbose mode.

```bash
export CSXCAD_INSTALL_PATH="$HOME/opt/physics"

pip3 install . --verbose
```

The `setup.py` method can also be used for troubleshooting.

If you are unable to solve the problem, create a post in the
[discussion forum](https://github.com/thliebig/openEMS-Project/discussions).
Make sure to provide detailed information about your system
(operating systems name and version, any error messages and
debugging outputs).

### AttributeError: module 'CSXCAD.CSRectGrid' has no attribute...

Ensure your CSXCAD and Python installation matches.

There could be an old copy of CSXCAD inside your system somewhere, which
is built with a newer Python extension that uses then non-existent properties.
Delete all older copies of CSXCAD from your system, and reinstall CSXCAD and
the Python extension.

## Tests

Unit tests are available in the `python/tests` directory. They
can be executed via:

```bash
cd tests
python3 -m unittest
```

## Examples

Although CSXCAD is often used together with openEMS, it's a general-purpose
geometry library on its own. A CSXCAD model contains material and excitation
source parameters. In principle, You can develop your own solver backend
based on CSXCAD's input. A single example is available in `python/examples`.
