# -*- coding: utf-8 -*-
#
# Shortcut CSXCAD import

AppCSXCAD_BIN = 'AppCSXCAD'

import os
if os.name == 'nt':
    _install_path = os.environ.get('CSXCAD_INSTALL_PATH', None)
    if _install_path is None:
        # openEMS is the primary user of CSXCAD, check if it has set an install path (fallback)
        _install_path = os.environ.get('OPENEMS_INSTALL_PATH', None)
    if _install_path and os.path.exists(_install_path):
        try:  # to use Python 3.8's DLL handling
            os.add_dll_directory(_install_path)
        except AttributeError:  # <3.8, use PATH
            if _install_path not in os.environ['PATH']:
                os.environ['PATH'] += os.pathsep + _install_path

        AppCSXCAD_BIN = os.path.join(_install_path, 'AppCSXCAD')

try:
    from importlib.metadata import version
    __version__ = version("CSXCAD")
except ImportError:
    try:
        from CSXCAD.__fallback_version__ import __fallback_version__
        __version__ = __fallback_version__
    except ImportError:
        __version__ = "0.0.0"

from CSXCAD.CSXCAD import ContinuousStructure
