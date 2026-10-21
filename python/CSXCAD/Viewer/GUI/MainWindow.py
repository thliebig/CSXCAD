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

"""CSXCAD.Viewer.GUI.MainWindow
The main window of the viewer GUI.
"""

import os
import time

import numpy as np

from PySide6.QtCore import Qt, QTimer, Signal, QSettings, QByteArray, QSize, QObject, QEvent
from PySide6.QtGui import QAction, QActionGroup, QIcon, QKeySequence, QPixmap, QPainter, QPalette
from PySide6.QtWidgets import (QMainWindow, QDockWidget, QFileDialog, QMessageBox, QWidget, QToolBar,
                               QApplication, QInputDialog, QColorDialog, QDialog, QFormLayout, QSpinBox,
                               QCheckBox, QDialogButtonBox, QWidgetAction, QSlider, QHBoxLayout, QLabel,
                               QMenu, QProgressDialog, QMdiArea, QMdiSubWindow, QVBoxLayout, QTabBar)
from PySide6.QtSvg import QSvgRenderer

from CSXCAD.Viewer import Actions
from CSXCAD.Viewer.Core import ViewerHost
from CSXCAD.Viewer.GUI import _Forget

ICON_DIR = os.path.join(os.path.dirname(os.path.dirname(os.path.abspath(__file__))), 'icons')
_ICON_CACHE = {}


def Icon(name, widget=None):
    """ Tabler-style SVG icon, tinted with the palette's text colour. """
    if name is None:
        return QIcon()
    fn = os.path.join(ICON_DIR, name + '.svg')
    if not os.path.exists(fn):
        return QIcon()
    pal = (widget.palette() if widget is not None else QApplication.palette())
    color = pal.color(QPalette.ColorRole.WindowText).name()
    key = (name, color)
    if key in _ICON_CACHE:
        return _ICON_CACHE[key]
    with open(fn) as f:
        svg = f.read().replace('currentColor', color)
    renderer = QSvgRenderer(QByteArray(svg.encode('utf-8')))
    icon = QIcon()
    for size in (16, 24, 32, 48):
        pm = QPixmap(size, size)
        pm.fill(Qt.GlobalColor.transparent)
        p = QPainter(pm)
        renderer.render(p)
        p.end()
        icon.addPixmap(pm)
    _ICON_CACHE[key] = icon
    return icon


def _RenderWidgetClass():
    import vtkmodules.qt
    vtkmodules.qt.PyQtImpl = 'PySide6'
    from vtkmodules.qt.QVTKRenderWindowInteractor import QVTKRenderWindowInteractor
    return QVTKRenderWindowInteractor


class _CloseFilter(QObject):
    """ Event filter on a sub-window of the central area: a close is
    ignored and handed to `on_close` (None: nothing). QMdiArea would delete
    the sub-window with its widget; the 3D view must stay, a figure only
    hides. """
    def __init__(self, parent, on_close=None):
        super().__init__(parent)
        self._on_close = on_close

    def eventFilter(self, obj, ev):
        if ev.type() == QEvent.Type.Close:
            ev.ignore()
            if self._on_close is not None:
                self._on_close()
            return True
        return False


class MainWindow(QMainWindow, ViewerHost):
    """ MainWindow(viewer, render_widget=None)

    GUI window around a :class:`CSXCAD.Viewer.Viewer`, its
    :class:`~CSXCAD.Viewer.Core.ViewerHost` while open.

    An application building on the viewer can subclass it: set
    :attr:`TITLE` and :attr:`SETTINGS`, add its docks, menus and toolbars in
    :meth:`Customize` (so the saved window layout covers them), and pass the
    class to :meth:`CSXCAD.Viewer.Viewer.Show` as ``window_class``. Actions
    go to the viewer (:meth:`CSXCAD.Viewer.Viewer.AddAction`), so they also
    work in the plain VTK window.
    """
    closed = Signal()

    #: window title
    TITLE = 'CSXCAD Viewer'
    #: (organisation, application) of the QSettings holding the window layout
    SETTINGS = ('CSXCAD', 'csxcad-view')

    def __init__(self, viewer, render_widget=None):
        super().__init__()
        self.viewer = viewer
        self.final_state = None
        self._settings = QSettings(*self.SETTINGS)
        self._last_hover = 0.0
        self._actions = {}
        self._menus = {}
        self._toolbars = {}
        self.setWindowTitle(self._Title())
        self.setWindowIcon(Icon('render-solid-edges'))

        if render_widget is None:
            render_widget = _RenderWidgetClass()(self)
            self._own_render_widget = True
        else:
            self._own_render_widget = False
        self.vtk_widget = render_widget
        # the 3D view and the figures are windows of the central area,
        # shown as tabs or as sub-windows (ApplyWindowMode)
        self.mdi = QMdiArea(self)
        self.mdi.setTabsMovable(True)
        self.mdi.setDocumentMode(True)
        # windows are added in sub-window mode, the saved mode (default tabs)
        # is applied once all are there (_RestoreSettings)
        self.setCentralWidget(self.mdi)
        self._figure_subs = {}   # {figure path: (sub-window, toolbar)}
        render_widget.setWindowTitle('3D view')
        self._view_sub = self._AddSubWindow(render_widget, None)
        self.mdi.subWindowActivated.connect(self._OnSubWindow)

        from CSXCAD.Viewer.GUI.Tree import SceneTree
        from CSXCAD.Viewer.GUI.InfoPanel import InfoPanel
        from CSXCAD.Viewer.GUI.Widgets import GridPanel, AnimationBar, ItemEditor
        self.tree = SceneTree(self)
        self.info = InfoPanel(self)
        self.grid_panel = GridPanel(self)
        self.item_editor = ItemEditor(self)
        self.AddDock('tree', 'Structure', self.tree)
        self.AddDock('info', 'Info', self.info)
        self.AddDock('grid', 'Grid && Section', self.grid_panel, Qt.DockWidgetArea.RightDockWidgetArea)
        self.AddDock('settings', 'Settings', self.item_editor, Qt.DockWidgetArea.RightDockWidgetArea)
        # the grid controls are short, item settings can be long
        self.resizeDocks([self.dock_grid, self.dock_settings], [1, 3], Qt.Orientation.Vertical)

        self._BuildActions()
        self._BuildMenus()
        self._BuildToolbars()
        self.anim_bar = AnimationBar(self)
        self.anim_bar.setObjectName('toolbar_animation')
        self.addToolBar(Qt.ToolBarArea.BottomToolBarArea, self.anim_bar)

        self.status_pos = QLabel()
        self.statusBar().addPermanentWidget(self.status_pos)

        viewer.SetHost(self)
        # figures added before this window opened (or shown in an earlier one)
        for figure in viewer.GetFigures().values():
            self.FigureAdded(figure)
        self.Customize()
        self._RestoreSettings()

    def Customize(self):
        """ Hook for subclasses: add docks (:meth:`AddDock`), menus and
        toolbars here. Called at the end of construction, before the saved
        window layout is restored. """

    # ------------------------------------------------------------------
    def _Title(self):
        name = self.viewer.GetFileName() or self.viewer._name
        return self.TITLE + (' - ' + os.path.basename(name) if name else '')

    def AddDock(self, name, title, widget, area=Qt.DockWidgetArea.LeftDockWidgetArea):
        """ AddDock(name, title, widget, area=Qt.DockWidgetArea.LeftDockWidgetArea)

        Add `widget` in a dock. `name` identifies it in the saved window
        layout and as attribute ``dock_<name>``. Returns the QDockWidget.
        """
        dock = QDockWidget(title, self)
        dock.setObjectName('dock_' + name)
        dock.setWidget(widget)
        self.addDockWidget(area, dock)
        setattr(self, 'dock_' + name, dock)
        return dock

    def Start(self):
        """ Attach the viewer to the render widget (after show()). """
        v = self.viewer
        w = self.vtk_widget
        if self._own_render_widget:
            rw = w.GetRenderWindow()
            iren = rw.GetInteractor()
            v.Attach(rw, iren)
            w.Initialize()
            iren.AddObserver('MouseMoveEvent', self._OnMove, 10.0)
        # a relative size and position refer to this window's screen
        v.SetSize(v._size_spec, render=False)
        if v._window_position is not None:
            self.ApplyPosition(v._window_position)
        self.RefreshAll()
        self.mdi.setActiveSubWindow(self._view_sub)   # the 3D view starts in front
        self._OnSubWindow()
        w.setFocus()
        v.Render()

    # ------------------------------------------------------------------
    # ViewerHost: what the viewer asks of its window
    handles_input = True   # keys go through the actions, values to the status bar

    def Schedule(self, ms, fn):
        QTimer.singleShot(int(ms), fn)
        return True

    def ShowStatus(self, message):
        self.statusBar().showMessage(message, 5000 if message else 1)

    def TreeChanged(self):
        self.tree.Refresh()

    def Close(self):
        self.close()
        return True

    def AnimationChanged(self):
        self.anim_bar.Refresh()
        # an item's settings may follow the clock (not every played frame)
        if not self.viewer.GetAnimation()['playing']:
            self.item_editor.Sync()

    # ------------------------------------------------------------------
    # figures and the central area
    def WindowMode(self):
        """ ``'tabs'`` or ``'windows'``, see :meth:`ApplyWindowMode`. """
        return 'tabs' if self.mdi.viewMode() == QMdiArea.ViewMode.TabbedView else 'windows'

    def ApplyWindowMode(self, mode):
        """ ApplyWindowMode(mode)

        ``'tabs'``: the 3D view and every figure fill the central area, one
        tab each. ``'windows'``: framed sub-windows inside the area, to place
        side by side (Tile, Cascade). Set it with
        :meth:`CSXCAD.Viewer.Viewer.SetWindowMode`, it is part of the view
        state. """
        tabs = mode == 'tabs'
        self.mdi.setViewMode(QMdiArea.ViewMode.TabbedView if tabs else QMdiArea.ViewMode.SubWindowView)
        for sub in self.mdi.subWindowList():
            if not sub.isHidden():
                self._StyleSubWindow(sub)
        if not tabs:
            self.mdi.tileSubWindows()
        self._mode_actions[mode].setChecked(True)
        self._tile_act.setEnabled(not tabs)
        self._cascade_act.setEnabled(not tabs)
        self._SyncShown()

    def _SyncShown(self):
        """ A hidden figure is not in the central area at all: QMdiArea
        cannot hide single tabs (a hidden tab leaves a gap and mixes up the
        order). A shown one is, at its place: the 3D view first, then the
        figures in their order. The 3D view itself always stays. """
        figures = self.viewer._figures
        attached = self.mdi.subWindowList()
        for path, (sub, tb) in self._figure_subs.items():
            shown = path not in figures or figures[path].IsShown()
            if not shown and sub in attached:
                if self.mdi.currentSubWindow() is sub:
                    self.mdi.setActiveSubWindow(self._view_sub)
                self.mdi.removeSubWindow(sub)   # kept, with its figure
                sub.hide()
            elif shown and sub not in attached:
                self.mdi.addSubWindow(sub)
                self._StyleSubWindow(sub)
                self._PlaceTab(sub)
        self._OnSubWindow()

    def _PlaceTab(self, sub):
        """ Move the tab of a re-added `sub` (appended at the end) to its place. """
        bar = self.mdi.findChild(QTabBar)
        if bar is None:
            return
        order = [self._view_sub] + [s for s, tb in self._figure_subs.values()]
        tabs = self.mdi.subWindowList(QMdiArea.WindowOrder.CreationOrder)
        want = sum(1 for s in tabs if s is not sub and s in order and order.index(s) < order.index(sub))
        have = tabs.index(sub)
        if have != want and have < bar.count():
            bar.moveTab(have, want)   # QMdiArea follows the tab bar's order

    def _StyleSubWindow(self, sub):
        """ Show `sub` as the mode wants it: filling and frameless as a tab,
        framed (the 3D view without a close button) as a sub-window. """
        if self.WindowMode() == 'tabs':
            sub.setWindowFlags(Qt.WindowType.FramelessWindowHint)
            sub.showMaximized()
            return
        flags = Qt.WindowType.SubWindow | Qt.WindowType.WindowTitleHint | Qt.WindowType.WindowMinMaxButtonsHint
        if sub.widget() is not self.vtk_widget:
            flags |= Qt.WindowType.WindowCloseButtonHint
        sub.setWindowFlags(flags)
        sub.showNormal()

    def _AddSubWindow(self, widget, figure):
        # The render widget is a native (X11) window, VTK draws into it. Its
        # containers must be native too: Qt cannot reliably move and resize
        # a native child with an alien (non-native) parent, the picture then
        # stays where it was. All sub-windows are native, so that they also
        # stack among each other through the window system.
        sub = QMdiSubWindow()
        sub.setAttribute(Qt.WidgetAttribute.WA_NativeWindow)
        sub.setWidget(widget)
        self.mdi.addSubWindow(sub)
        sub.setAttribute(Qt.WidgetAttribute.WA_DeleteOnClose, False)
        sub.setWindowTitle(widget.windowTitle())
        on_close = None if figure is None else (lambda p=figure.GetPath(): self.viewer.SetVisible(p, False))
        sub.installEventFilter(_CloseFilter(sub, on_close))
        self._StyleSubWindow(sub)
        return sub

    def _OnSubWindow(self, *args):
        """ Only the current figure's toolbar is shown, none for the 3D view.
        As a tab, the render widget is hidden while another tab is in front:
        it is a native window, which the other (non-native) tabs cannot cover
        (QMdiArea only raises the current tab, it hides none). """
        cur = self.mdi.currentSubWindow()
        for sub, tb in self._figure_subs.values():
            tb.setVisible(sub is cur and not sub.isHidden())
        show_3d = self.WindowMode() == 'windows' or cur is None or cur is self._view_sub
        if self.vtk_widget.isHidden() == show_3d:   # hidden but wanted, or the other way round
            self.vtk_widget.setVisible(show_3d)
            if show_3d and self.viewer.GetHost() is self:
                self.viewer.Render()

    def FigureAdded(self, figure):
        from matplotlib.backends.backend_qtagg import FigureCanvasQTAgg, NavigationToolbar2QT
        fig = figure.GetFigure()
        if getattr(fig.canvas, 'manager', None) is not None:
            import matplotlib.pyplot as plt
            plt.close(fig)   # a pyplot figure: no window of its own
        canvas = FigureCanvasQTAgg(fig)
        w = QWidget()
        lay = QVBoxLayout(w)
        lay.setContentsMargins(0, 0, 0, 0)
        lay.addWidget(canvas)
        w.setWindowTitle(figure.GetName())
        tb = NavigationToolbar2QT(canvas, self)
        tb.setObjectName('figure_toolbar_' + figure.GetPath())
        tb.setWindowTitle(figure.GetName() + ' figure')
        tb.toggleViewAction().setVisible(False)   # not in the toolbar menu
        tb.setIconSize(QSize(20, 20))   # as the viewer's own, or the toolbar area changes height
        self.addToolBar(Qt.ToolBarArea.TopToolBarArea, tb)
        cur = self.mdi.currentSubWindow()
        sub = self._AddSubWindow(w, figure)
        self._figure_subs[figure.GetPath()] = (sub, tb)
        if cur is not None:   # a new figure does not take the front
            self.mdi.setActiveSubWindow(cur)
        self._SyncShown()

    def FigureRemoved(self, figure):
        sub, tb = self._figure_subs.pop(figure.GetPath(), (None, None))
        if sub is None:
            return
        self.removeToolBar(tb)
        tb.deleteLater()
        if sub in self.mdi.subWindowList():
            self.mdi.removeSubWindow(sub)
        sub.deleteLater()
        self._OnSubWindow()

    def FigureShown(self, figure, shown):
        sub = self._figure_subs[figure.GetPath()][0]
        self._SyncShown()
        if shown:
            self.mdi.setActiveSubWindow(sub)
            self._OnSubWindow()

    def RaiseFigure(self, figure):
        self._SyncShown()   # Viewer.RaiseFigure shows a hidden figure first
        sub = self._view_sub if figure is None else self._figure_subs[figure.GetPath()][0]
        self.mdi.setActiveSubWindow(sub)
        self._OnSubWindow()

    def _Screen(self):
        return self.screen() or QApplication.primaryScreen()

    def ScreenSize(self):
        """ Size of the window's screen in pixels. """
        scr = self._Screen()
        g, dpr = scr.geometry(), scr.devicePixelRatio()
        return int(round(g.width() * dpr)), int(round(g.height() * dpr))

    def ApplyPosition(self, pos):
        """ Move the window's top-left corner to `pos`, fractions of the available screen area. """
        a = self._Screen().availableGeometry()
        self.move(a.x() + int(round(pos[0] * a.width())), a.y() + int(round(pos[1] * a.height())))

    def WindowPosition(self):
        """ The window's top-left corner in fractions of the available screen area. """
        if not self.isVisible():
            return None
        a = self._Screen().availableGeometry()
        f = self.frameGeometry()
        return (round((f.x() - a.x()) / max(a.width(), 1), 3), round((f.y() - a.y()) / max(a.height(), 1), 3))

    def RenderSize(self):
        """ Size of the render area in pixels (None before the window is shown). """
        if not self.isVisible():
            return None
        w = self.vtk_widget
        dpr = w.devicePixelRatioF() if hasattr(w, 'devicePixelRatioF') else 1.0
        return int(round(w.width() * dpr)), int(round(w.height() * dpr))

    def ApplySize(self, size):
        """ Resize the window so that the render area gets `size` pixels. """
        if self.isMaximized() or self.isFullScreen():
            return True
        dpr = self.vtk_widget.devicePixelRatioF() if hasattr(self.vtk_widget, 'devicePixelRatioF') else 1.0
        w = int(round(size[0] / dpr))
        h = int(round(size[1] / dpr))
        dw = w - self.vtk_widget.width()
        dh = h - self.vtk_widget.height()
        if dw or dh:
            self.resize(self.width() + dw, self.height() + dh)
        return True

    def RefreshAll(self):
        self.tree.Refresh()
        self.grid_panel.Refresh()
        self.anim_bar.Refresh()
        self.RefreshActions()
        self.setWindowTitle(self._Title())

    # ------------------------------------------------------------------
    # actions
    def _BuildActions(self):
        self._radio_groups = {}
        for a in self.viewer.GetActions():
            self._MakeQAction(a)
        self._BuildAppActions()

    def _MakeQAction(self, a):
        old = self._actions.get(a.name)
        if old is not None:
            # replaced by an action of the same name
            old[1].setVisible(False)
            old[1].setEnabled(False)
        qa = QAction(Icon(a.icon, self), a.label, self)
        if a.shortcut:
            qa.setShortcut(QKeySequence(a.shortcut))
            qa.setToolTip('{} ({})'.format(a.label, a.shortcut))
            if not a.shortcut.startswith('Ctrl+') and a.shortcut != 'Esc':
                # plain keys only while the 3D view has the focus, so
                # they do not steal keys from the tree or text fields;
                # Esc deselects from anywhere (e.g. after a tree selection)
                qa.setShortcutContext(Qt.ShortcutContext.WidgetWithChildrenShortcut)
                self.vtk_widget.addAction(qa)
        qa.setCheckable(a.checkable)
        if a.radio:
            if a.radio not in self._radio_groups:
                self._radio_groups[a.radio] = QActionGroup(self)
            self._radio_groups[a.radio].addAction(qa)
        qa.triggered.connect(lambda checked=False, act=a: self._Trigger(act))
        self._actions[a.name] = (a, qa)
        return qa

    def _BuildAppActions(self):
        def add(name, label, icon, slot, shortcut=None):
            qa = QAction(Icon(icon, self), label, self)
            if shortcut:
                qa.setShortcut(QKeySequence(shortcut))
            qa.triggered.connect(slot)
            self._actions[name] = (None, qa)
            return qa
        add('open', 'Open...', 'open', self.OpenFile, QKeySequence.StandardKey.Open)
        add('reload', 'Reload', 'reload', self.Reload, 'F5')
        add('save_xml', 'Save XML', 'save', self.SaveXML, QKeySequence.StandardKey.Save)
        add('save_xml_as', 'Save XML as...', None, self.SaveXMLAs)
        add('save_view', 'Save view...', 'save-view', self.SaveView)
        add('load_view', 'Load view...', 'load-view', self.LoadView)
        add('copy_view_qt', 'Copy view as Python', 'copy', self.CopyView, 'Ctrl+Shift+C')
        add('screenshot_qt', 'Screenshot...', 'camera', self.ScreenshotDialog, 'Ctrl+P')
        add('views_qt', 'Render all views...', 'views', self.RenderViewsDialog)
        add('export_animation_qt', 'Export animation...', 'export', self.ExportAnimationDialog)
        add('background', 'Background colour...', 'background', self.ChooseBackground)
        add('edge_color', 'Edge colour...', None, self.ChooseEdgeColor)
        add('arc_resolution', 'Arc resolution...', None, self.ChooseArcResolution)
        add('show_all', 'Show all properties', 'eye', lambda: self.viewer.SetVisible('properties', True))
        add('quit', 'Close', None, self.close, QKeySequence.StandardKey.Close)
        add('about', 'About', 'info', self.About)
        # the registry's print-based ones are replaced by dialogs here
        self._actions['screenshot'][1].setVisible(False)
        self._actions['copy_view'][1].setVisible(False)

    def _Trigger(self, act):
        try:
            act.Trigger(self.viewer)
        except Exception as err:
            self.statusBar().showMessage('{} failed: {}'.format(act.label, err), 5000)
        self.RefreshActions()
        self.tree.Refresh()
        self.grid_panel.Refresh()

    def RefreshActions(self):
        if 'export_animation_qt' in self._actions:
            self._actions['export_animation_qt'][1].setEnabled(self.viewer.GetAnimationSteps() > 0)
        for name, (a, qa) in self._actions.items():
            if a is None:
                continue
            if a.checkable and a.get is not None:
                qa.blockSignals(True)
                qa.setChecked(bool(a.get(self.viewer)))
                qa.blockSignals(False)
            if a.enabled is not None:
                qa.setEnabled(bool(a.enabled(self.viewer)))

    def Action(self, name):
        return self._actions[name][1]

    def _BuildMenus(self):
        mb = self.menuBar()
        m = mb.addMenu('&File')
        for n in ('open', 'reload', 'save_xml', 'save_xml_as', None, 'save_view', 'load_view', 'copy_view_qt', None,
                  'screenshot_qt', 'views_qt', 'export_animation_qt'):
            m.addSeparator() if n is None else m.addAction(self.Action(n))
        self.recent_menu = m.addMenu('Recent files')
        self.recent_menu.aboutToShow.connect(self._FillRecent)
        exp = m.addMenu(Icon('export', self), 'Export')
        for fmt, label in (('stl', 'STL (per property)...'), ('ply', 'PLY (per property)...'),
                           ('vtp', 'VTP (per property)...'), (None, None), ('gltf', 'glTF scene...'),
                           ('glb', 'GLB scene...'), (None, None), ('svg', 'SVG image...'), ('pdf', 'PDF image...')):
            if fmt is None:
                exp.addSeparator()
                continue
            exp.addAction(label, lambda f=fmt: self.ExportDialog(f))
        m.addSeparator()
        m.addAction(self.Action('quit'))
        self._menus['File'] = m
        for group in ('View', 'Display', 'Grid', 'Animation'):
            self._menus[group] = mb.addMenu('&' + group)
        self._help_menu = mb.addMenu('&Help')
        for a in self.viewer.GetActions():
            if a.group != 'Help':
                self.Menu(a.group).addAction(self.Action(a.name))
        menu = self._menus['Display']
        menu.addSeparator()
        menu.addAction(self._TransparencySliderAction(menu))
        for n in ('background', 'edge_color', 'arc_resolution', 'show_all'):
            menu.addAction(self.Action(n))
        menu = self._menus['Animation']
        menu.addSeparator()
        menu.addAction(self.Action('export_animation_qt'))
        self._actions['help'][1].triggered.disconnect()
        self._actions['help'][1].triggered.connect(
            lambda: QMessageBox.information(self, 'Keyboard shortcuts',
                                            '<pre>' + Actions.HelpText(self.viewer) + '</pre>'))
        self._help_menu.addAction(self.Action('help'))
        self._help_menu.addAction(self.Action('about'))
        self._BuildWindowMenu()

    def _BuildWindowMenu(self):
        menu = self.Menu('Window')
        group = QActionGroup(self)
        self._mode_actions = {}
        for text, mode in (('Tabs', 'tabs'), ('Windows', 'windows')):
            act = QAction(text, self, checkable=True)
            act.triggered.connect(lambda checked=False, m=mode: self.viewer.SetWindowMode(m))
            group.addAction(act)
            menu.addAction(act)
            self._mode_actions[mode] = act
        self._mode_actions['tabs'].setChecked(True)
        menu.addSeparator()
        self._tile_act = menu.addAction('Tile', self.mdi.tileSubWindows)
        self._cascade_act = menu.addAction('Cascade', self.mdi.cascadeSubWindows)
        self._tile_act.setEnabled(False)
        self._cascade_act.setEnabled(False)
        menu.addSeparator()
        menu.addAction('3D view', lambda: self.viewer.RaiseFigure(None))

    def Menu(self, name):
        """ The menu `name`, created before Help if new. """
        if name not in self._menus:
            m = QMenu('&' + name, self)
            self.menuBar().insertMenu(self._help_menu.menuAction(), m)
            self._menus[name] = m
        return self._menus[name]

    def Toolbar(self, name):
        """ The toolbar `name`, created if new. """
        if name not in self._toolbars:
            tb = QToolBar(name, self)
            tb.setObjectName('toolbar_' + name.lower())
            tb.setIconSize(QSize(20, 20))
            self.addToolBar(tb)
            self._toolbars[name] = tb
        return self._toolbars[name]

    def ActionAdded(self, action, toolbar):
        qa = self._MakeQAction(action)
        self.Menu(action.group).addAction(qa)
        if toolbar:
            self.Toolbar(toolbar).addAction(qa)
        self.RefreshActions()

    def _TransparencySliderAction(self, parent):
        w = QWidget(parent)
        lay = QHBoxLayout(w)
        lay.setContentsMargins(24, 2, 8, 2)
        lay.addWidget(QLabel('See-through level'))
        s = QSlider(Qt.Orientation.Horizontal)
        s.setRange(0, 100)
        s.setValue(30)
        lay.addWidget(s)
        s.valueChanged.connect(lambda val: (self.viewer.SetTransparency(val / 100.0), self.RefreshActions()))
        wa = QWidgetAction(parent)
        wa.setDefaultWidget(w)
        return wa

    def _BuildToolbars(self):
        layout = {
            'File': ['open', 'screenshot_qt', 'views_qt'],
            'View': ['fit', 'view_+x', 'view_-x', 'view_+y', 'view_-y', 'view_+z', 'view_-z', 'view_iso_z',
                     'view_iso_x', 'view_iso_y', 'flip', 'roll_left', 'roll_right', 'azimuth_left', 'azimuth_right',
                     'mode_2d', 'parallel'],
            'Display': ['render_solid', 'render_edges', 'render_solid_edges', 'transparency', 'clip', 'labels', 'picking',
                        'select_primitives'],
            'Grid': ['grid', 'grid_outline', 'grid_quality'],
        }
        for name, items in layout.items():
            for n in items:
                self.Toolbar(name).addAction(self.Action(n))
        for a in self.viewer.GetActions():
            toolbar = self.viewer.GetActionToolbar(a.name)
            if toolbar:
                self.Toolbar(toolbar).addAction(self.Action(a.name))

    # ------------------------------------------------------------------
    # picking and hover
    def OnClick(self, x, y):
        res = self.viewer.Pick(x, y)
        if res is None:
            self.viewer.Highlight(None)
            self.tree.Select(None)
            return
        if 'item' in res:
            self.viewer.Highlight(None)
            self.tree.SelectItem(res['item'])
            return
        pid = res['primitive_id'] if self.viewer.GetPrimitiveSelection() else -1
        self.viewer.Highlight((res['prop'], pid) if pid >= 0 else res['prop'])
        self.tree.Select(res['prop'], pid)
        self.statusBar().showMessage('{}{} at ({})'.format(
            res['prop'], ' #{}'.format(pid) if pid >= 0 else '',
            ', '.join('{:.4g}'.format(c) for c in res['world_pos'])), 8000)

    def _OnMove(self, obj, event):
        now = time.monotonic()
        if now - self._last_hover < 0.05:
            return
        self._last_hover = now
        x, y = obj.GetEventPosition()
        ren = self.viewer.GetRenderer()
        cam = ren.GetActiveCamera()
        ren.SetWorldPoint(*cam.GetFocalPoint(), 1.0)
        ren.WorldToDisplay()
        z = ren.GetDisplayPoint()[2]
        ren.SetDisplayPoint(x, y, z)
        ren.DisplayToWorld()
        w = ren.GetWorldPoint()
        if w[3] == 0:
            return
        pos = np.array(w[:3]) / w[3]
        text = 'x={:.4g}  y={:.4g}  z={:.4g}'.format(*pos)
        near = self.viewer.GetNearestGridLines(pos)
        if near:
            text += '   lines: ' + '  '.join('{}[{}]={:.4g}'.format(k, i, c) for k, (i, c) in near.items())
        value = None if self.viewer._interacting else self.viewer.GetItemValueText(x, y)
        if value:
            text = value + '   ' + text
        self.status_pos.setText(text)

    # ------------------------------------------------------------------
    # file handling
    def OpenFile(self):
        fn, _ = QFileDialog.getOpenFileName(self, 'Open structure', self._LastDir(), 'XML files (*.xml);;All files (*)')
        if fn:
            self.LoadFile(fn)

    def LoadFile(self, fn):
        try:
            self.viewer.SetCSX(fn)
        except Exception as err:
            QMessageBox.critical(self, 'Open', str(err))
            return
        self.viewer.Fit()
        self._AddRecent(fn)
        self.RefreshAll()

    def Reload(self):
        fn = self.viewer.GetFileName()
        if not fn:
            return
        state = self.viewer.GetState()
        self.viewer.SetCSX(fn)
        self.viewer.SetState(state)
        self.RefreshAll()

    def SaveXML(self):
        fn = self.viewer.GetFileName()
        if not fn:
            return self.SaveXMLAs()
        if self.viewer.CSX is not None:
            self.viewer.CSX.Write2XML(fn)
            self.statusBar().showMessage('saved {}'.format(fn), 5000)

    def SaveXMLAs(self):
        if self.viewer.CSX is None:
            return
        fn, _ = QFileDialog.getSaveFileName(self, 'Save structure', self._LastDir(), 'XML files (*.xml)')
        if fn:
            self.viewer.CSX.Write2XML(fn)
            self.viewer.SetFileName(fn)
            self._AddRecent(fn)
            self.setWindowTitle(self._Title())

    def SaveView(self):
        fn, _ = QFileDialog.getSaveFileName(self, 'Save view', self._LastDir(), 'View (*.json)')
        if fn:
            self.viewer.SaveState(fn)

    def LoadView(self):
        fn, _ = QFileDialog.getOpenFileName(self, 'Load view', self._LastDir(), 'View (*.json)')
        if fn:
            try:
                self.viewer.LoadState(fn)
            except Exception as err:
                QMessageBox.critical(self, 'Load view', str(err))
            self.RefreshAll()

    def CopyView(self):
        QApplication.clipboard().setText(self.viewer.GetStateAsPython() + '\n')
        self.statusBar().showMessage('view copied to the clipboard as Python', 5000)

    def ScreenshotDialog(self):
        fn, _ = QFileDialog.getSaveFileName(self, 'Screenshot', self._LastDir(), 'PNG image (*.png)')
        if not fn:
            return
        dlg = QDialog(self)
        dlg.setWindowTitle('Screenshot')
        form = QFormLayout(dlg)
        size = self.viewer.GetSize()
        w_spin, h_spin, s_spin = QSpinBox(), QSpinBox(), QSpinBox()
        for sp, val, hi in ((w_spin, size[0], 16384), (h_spin, size[1], 16384), (s_spin, 1, 8)):
            sp.setRange(1, hi)
            sp.setValue(val)
        transp = QCheckBox()
        form.addRow('Width', w_spin)
        form.addRow('Height', h_spin)
        form.addRow('Scale', s_spin)
        form.addRow('Transparent background', transp)
        bb = QDialogButtonBox(QDialogButtonBox.StandardButton.Ok | QDialogButtonBox.StandardButton.Cancel)
        bb.accepted.connect(dlg.accept)
        bb.rejected.connect(dlg.reject)
        form.addRow(bb)
        if dlg.exec() != QDialog.DialogCode.Accepted:
            return
        self.viewer.Screenshot(fn, size=(w_spin.value(), h_spin.value()), scale=s_spin.value(),
                               transparent_background=transp.isChecked())
        self.viewer.Render()
        self.statusBar().showMessage('saved {}'.format(fn), 5000)

    def ExportAnimationDialog(self):
        """ Render the animation into a GIF, a video or PNG frames. """
        from CSXCAD.Viewer.Animation import ExportFormats
        v = self.viewer
        names = {'gif': 'GIF animation (*.gif)', 'ogv': 'Ogg Theora video (*.ogv)', 'mp4': 'MP4 video (*.mp4)',
                 'png': 'PNG frames into a folder (*)'}
        fmts = [f for f in ('gif', 'ogv', 'mp4', 'png') if f in ExportFormats()]
        filters = [names[f] for f in fmts]
        fn, flt = QFileDialog.getSaveFileName(self, 'Export animation', self._LastDir(), ';;'.join(filters))
        if not fn:
            return
        fmt = fmts[filters.index(flt)] if flt in filters else fmts[0]
        ext = os.path.splitext(fn)[1].lower().lstrip('.')
        if ext in fmts:
            fmt = ext
        elif fmt != 'png':
            fn += '.' + fmt
        dlg = QDialog(self)
        dlg.setWindowTitle('Export animation')
        form = QFormLayout(dlg)
        size = v.GetSize()
        spins = {}
        for key, label, val, lo, hi in (('w', 'Width', size[0], 1, 16384), ('h', 'Height', size[1], 1, 16384),
                                        ('scale', 'Scale', 1, 1, 8), ('loops', 'Loops', 1, 1, 100),
                                        ('fps', 'Frames per second', int(round(v.GetFps())), 1, 120)):
            sp = QSpinBox()
            sp.setRange(lo, hi)
            sp.setValue(val)
            form.addRow(label, sp)
            spins[key] = sp
        form.addRow(QLabel('{} frames per loop'.format(v.GetAnimationSteps())))
        bb = QDialogButtonBox(QDialogButtonBox.StandardButton.Ok | QDialogButtonBox.StandardButton.Cancel)
        bb.accepted.connect(dlg.accept)
        bb.rejected.connect(dlg.reject)
        form.addRow(bb)
        if dlg.exec() != QDialog.DialogCode.Accepted:
            return
        progress = QProgressDialog('Rendering the animation...', None, 0, 1, self)
        progress.setWindowModality(Qt.WindowModality.WindowModal)
        progress.setMinimumDuration(0)

        def step(i, n):
            progress.setMaximum(n)
            progress.setValue(i)
            QApplication.processEvents()
        try:
            v.SetFps(spins['fps'].value())
            v.ExportAnimation(fn, fmt, size=(spins['w'].value(), spins['h'].value()),
                              scale=spins['scale'].value(), loops=spins['loops'].value(), progress=step)
        except Exception as err:
            QMessageBox.warning(self, 'Export animation', str(err))
            return
        finally:
            progress.close()
            v.Render()
        self.statusBar().showMessage('saved {}'.format(fn), 5000)

    def RenderViewsDialog(self):
        fn, _ = QFileDialog.getSaveFileName(self, 'Render all views', self._LastDir(), 'PNG image (*.png)')
        if fn:
            self.viewer.RenderViews(fn)
            self.statusBar().showMessage('saved {}'.format(fn), 5000)

    def ExportDialog(self, fmt, prop=None):
        if fmt in ('stl', 'ply', 'vtp'):
            path = QFileDialog.getExistingDirectory(self, 'Export {} into directory'.format(fmt.upper()), self._LastDir())
        else:
            path, _ = QFileDialog.getSaveFileName(self, 'Export {}'.format(fmt.upper()), self._LastDir(),
                                                  '{} (*.{})'.format(fmt.upper(), fmt))
        if not path:
            return
        try:
            if prop is not None:
                saved = self.viewer.GetIsolate()
                self.viewer.Isolate(prop, render=False)
            try:
                res = self.viewer.Export(fmt, path)
            finally:
                if prop is not None:
                    self.viewer.Isolate(saved)
        except Exception as err:
            QMessageBox.critical(self, 'Export', str(err))
            return
        msg = 'exported {} file(s)'.format(len(res['files']))
        if res['report']:
            msg += '; ' + '; '.join('{}: {}'.format(r['file'], r['reason']) for r in res['report'])
        self.statusBar().showMessage(msg, 10000)

    def ChooseBackground(self):
        c = QColorDialog.getColor(parent=self, title='Background colour')
        if c.isValid():
            self.viewer.SetBackground(c.name())

    def ChooseEdgeColor(self):
        c = QColorDialog.getColor(parent=self, title='Edge colour for all properties (cancel: property colours)')
        self.viewer.SetEdgeColor(c.name() if c.isValid() else None)

    def ChooseArcResolution(self):
        val, ok = QInputDialog.getInt(self, 'Arc resolution', 'Segments per full circle',
                                      self.viewer.GetArcResolution(), 8, 512)
        if ok:
            self.viewer.SetArcResolution(val)

    def About(self):
        import CSXCAD
        import vtkmodules.vtkCommonCore as vc
        import PySide6
        from PySide6.QtCore import qVersion
        QMessageBox.about(self, 'About CSXCAD Viewer',
                          'CSXCAD Viewer<br>CSXCAD {}<br>VTK {}<br>PySide6 {}<br>Qt {}<br><br>'
                          'Icons: Tabler Icons (MIT)'.format(CSXCAD.__version__, vc.vtkVersion.GetVTKVersion(),
                                                             PySide6.__version__, qVersion()))

    # ------------------------------------------------------------------
    # settings
    def _LastDir(self):
        fn = self.viewer.GetFileName()
        return os.path.dirname(fn) if fn else os.getcwd()

    def _AddRecent(self, fn):
        files = self._settings.value('recent_files', []) or []
        if isinstance(files, str):
            files = [files]
        fn = os.path.abspath(fn)
        files = [fn] + [f for f in files if f != fn]
        self._settings.setValue('recent_files', files[:10])

    def _FillRecent(self):
        self.recent_menu.clear()
        files = self._settings.value('recent_files', []) or []
        if isinstance(files, str):
            files = [files]
        for f in files:
            self.recent_menu.addAction(f, lambda fn=f: self.LoadFile(fn))

    def _RestoreSettings(self):
        geo = self._settings.value('geometry')
        if geo is not None:
            self.restoreGeometry(geo)
        st = self._settings.value('window_state')
        if st is not None:
            self.restoreState(st)
        self.viewer.SetPicking(self._settings.value('picking', True, type=bool))
        self.viewer.SetPrimitiveSelection(self._settings.value('select_primitives', False, type=bool))
        self.ApplyWindowMode(self.viewer.GetWindowMode())   # part of the view state
        self.RefreshActions()

    def closeEvent(self, event):
        self._settings.setValue('geometry', self.saveGeometry())
        self._settings.setValue('window_state', self.saveState())
        self._settings.setValue('picking', self.viewer.GetPicking())
        self._settings.setValue('select_primitives', self.viewer.GetPrimitiveSelection())
        v = self.viewer
        self.final_state = v.GetState()
        v.Pause()
        v.SetHost(None)
        v.Detach()
        if self._own_render_widget:
            self.vtk_widget.Finalize()
        _Forget(self)
        super().closeEvent(event)
        self.closed.emit()
