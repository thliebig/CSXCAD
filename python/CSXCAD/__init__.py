# -*- coding: utf-8 -*-
#
# Shortcut CSXCAD import

from __future__ import absolute_import

import os
if os.name == 'nt' and 'OPENEMS_INSTALL_PATH' in os.environ and os.path.exists(os.environ['OPENEMS_INSTALL_PATH']):
    try:  # to use Python 3.8's DLL handling
        os.add_dll_directory(os.environ['OPENEMS_INSTALL_PATH'])
    except AttributeError:  # <3.8, use PATH
        if os.environ['OPENEMS_INSTALL_PATH'] not in os.environ['PATH']:
            os.environ['PATH'] += os.pathsep + os.environ['OPENEMS_INSTALL_PATH']

from CSXCAD.CSXCAD import ContinuousStructure
