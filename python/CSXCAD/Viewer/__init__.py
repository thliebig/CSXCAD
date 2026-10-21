# -*- coding: utf-8 -*-
#
# Copyright (C) 2026 Thorsten Liebig (Thorsten.Liebig@gmx.de)
#
# This program is free software: you can redistribute it and/or modify
# it under the terms of the GNU Lesser General Public License as published
# by the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU Lesser General Public License for more details.
#
# You should have received a copy of the GNU Lesser General Public License
# along with this program.  If not, see <http://www.gnu.org/licenses/>.

"""CSXCAD.Viewer
Viewer for CSXCAD structures, based on VTK.

The core needs only ``vtk`` besides CSXCAD's own dependencies; the optional GUI
(:mod:`CSXCAD.Viewer.GUI`) adds ``PySide6``. Everything is imported lazily:
``import CSXCAD.Viewer`` loads neither VTK nor Qt. Imported before the first
matplotlib figure, it lets matplotlib start Qt the way the GUI needs it (X11,
also on Wayland, and PySide6), so the GUI can show up after the figures.

Examples
--------
Show a structure, with a view baked into the script:

>>> from CSXCAD.Viewer import Show
>>> Show(CSX, render_mode='solid+edges', camera=dict(position=(-80, -120, 90)))

Render images without a window:

>>> from CSXCAD.Viewer import Viewer
>>> v = Viewer(CSX, offscreen=True)
>>> v.RenderViews('all_sides.png')
>>> print(v.Describe('text'))
"""

__all__ = ['Show', 'Viewer', 'ViewItem', 'Exec']


def __getattr__(name):
    if name in ('Show', 'Viewer'):
        from CSXCAD.Viewer import Core
        return getattr(Core, name)
    if name == 'ViewItem':
        from CSXCAD.Viewer.Scene import ViewItem
        return ViewItem
    if name == 'Exec':
        return Exec
    raise AttributeError("module 'CSXCAD.Viewer' has no attribute '{}'".format(name))


def _PrepareQt():
    """ Before anything starts Qt (e.g. a matplotlib figure), make it use
    X11, on Wayland through XWayland, which VTK's render window needs (Wayland
    stays the fallback), and matplotlib's Qt backend use PySide6 like the
    viewer. Only with vtk and PySide6 installed (the GUI needs both);
    explicit settings are kept. """
    import os
    import sys
    import importlib.util
    if importlib.util.find_spec('vtkmodules') is None or importlib.util.find_spec('PySide6') is None:
        return
    if 'PySide6.QtWidgets' in sys.modules:
        from PySide6.QtWidgets import QApplication
        if QApplication.instance() is not None:
            return
    os.environ.setdefault('QT_API', 'pyside6')
    if sys.platform.startswith('linux') and os.environ.get('DISPLAY'):
        os.environ.setdefault('QT_QPA_PLATFORM', 'xcb;wayland')


# at import: a script importing the viewer early can show the GUI after its
# matplotlib figures
_PrepareQt()


def Exec():
    """ Exec()

    Run the GUI event loop until the last viewer window is closed, after
    ``Show(block=False)``. ``plt.show()`` does the same for matplotlib and
    viewer windows together.
    """
    from CSXCAD.Viewer import GUI
    return GUI.Exec()
