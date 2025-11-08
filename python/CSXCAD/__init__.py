# -*- coding: utf-8 -*-
#
# Shortcut CSXCAD import

from __future__ import absolute_import

AppCSXCAD_BIN = 'AppCSXCAD'

import os
if os.name == 'nt' and 'CSXCAD_INSTALL_PATH' in os.environ and os.path.exists(os.environ['CSXCAD_INSTALL_PATH']):
    try:  # to use Python 3.8's DLL handling
        os.add_dll_directory(os.environ['CSXCAD_INSTALL_PATH'])
    except AttributeError:  # <3.8, use PATH
        if os.environ['CSXCAD_INSTALL_PATH'] not in os.environ['PATH']:
            os.environ['PATH'] += os.pathsep + os.environ['CSXCAD_INSTALL_PATH']
    
    AppCSXCAD_BIN = os.path.join(os.environ['CSXCAD_INSTALL_PATH'], 'AppCSXCAD')

try:
    from CSXCAD.__version__ import __version__
except ImportError:
    try:
        from CSXCAD.__fallback_version__ import __fallback_version__
        __version__ = __fallback_version__
    except ImportError:
        __version__ = "0.0.0"

from CSXCAD.CSXCAD import ContinuousStructure
