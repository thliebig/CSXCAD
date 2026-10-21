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

"""CSXCAD.Viewer.GUI
PySide6 GUI around :class:`CSXCAD.Viewer.Viewer`.

A thin wrapper: toolbars and menus are generated from the action registry
(:mod:`CSXCAD.Viewer.Actions`), and every widget calls core methods only.
"""

import sys

from CSXCAD.Viewer import _PrepareQt

_OTHER_BINDINGS = ('PyQt5', 'PyQt6', 'PySide2')
_WINDOWS = []


def CanUseGUI():
    """ CanUseGUI()

    Whether the PySide6 GUI can be used in this process.

    Returns
    -------
    ok : bool
    reason : str or None
        Why not, if worth a warning (None if PySide6 is simply missing).
    """
    for mod in _OTHER_BINDINGS:
        if mod in sys.modules:
            return False, '{} is already loaded (e.g. by matplotlib); a second Qt binding could crash'.format(mod)
    try:
        import PySide6  # noqa: F401
    except ImportError:
        return False, None
    app = _ExistingApp()
    if app is not None and app.platformName() == 'wayland':
        return False, ('the running Qt application uses Wayland, which cannot host the VTK window; '
                       'import CSXCAD.Viewer before the first matplotlib figure')
    from CSXCAD.Viewer.Core import DisplayAvailable
    if not DisplayAvailable():
        return False, 'no usable display'
    # let a later matplotlib QtAgg backend pick PySide6 too
    _PrepareQt()
    return True, None


def _ExistingApp():
    if 'PySide6.QtWidgets' not in sys.modules:
        return None
    from PySide6.QtWidgets import QApplication
    return QApplication.instance()


def GetApp():
    """ The QApplication, created if none exists (never a second one). """
    from PySide6.QtWidgets import QApplication
    app = QApplication.instance()
    if app is None:
        # VTK's Qt widget embeds a native X11 window; on a Wayland session
        # use XWayland for it
        _PrepareQt()
        app = QApplication(sys.argv[:1] or ['csxcad-view'])
        app.setApplicationName('csxcad-view')
        app.setOrganizationName('CSXCAD')
    return app


def ShowGUI(viewer, block=True, window_class=None):
    """ ShowGUI(viewer, block=True, window_class=None)

    Open a GUI window on `viewer`. Blocking returns the final view state when
    the window is closed; otherwise returns at once (see :func:`Exec`).
    `window_class` is a :class:`~CSXCAD.Viewer.GUI.MainWindow.MainWindow`
    subclass to use instead.
    """
    GetApp()
    if window_class is None:
        from CSXCAD.Viewer.GUI.MainWindow import MainWindow as window_class
    win = window_class(viewer)
    _WINDOWS.append(win)
    win.show()
    win.Start()
    if not block:
        return None
    from PySide6.QtCore import QEventLoop
    loop = QEventLoop()
    win.closed.connect(loop.quit)
    if win.isVisible():
        loop.exec()
    return win.final_state


def Exec():
    """ Exec()

    Run the Qt event loop until the last viewer window is closed.
    """
    if not _WINDOWS:
        return
    from PySide6.QtCore import QEventLoop
    GetApp()
    loop = QEventLoop()

    def check():
        if not any(w.isVisible() for w in _WINDOWS):
            loop.quit()
    for w in list(_WINDOWS):
        w.closed.connect(check)
    if any(w.isVisible() for w in _WINDOWS):
        loop.exec()


def _Forget(win):
    if win in _WINDOWS:
        _WINDOWS.remove(win)
