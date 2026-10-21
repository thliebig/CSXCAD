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

"""CSXCAD.Viewer.GUI.Tree
Tree view mirroring the viewer's scene tree. It refreshes from the core and
calls core methods only.
"""

from PySide6.QtCore import Qt
from PySide6.QtGui import QColor, QIcon, QPixmap, QPainter, QBrush, QPen
from PySide6.QtWidgets import (QTreeWidget, QTreeWidgetItem, QMenu, QColorDialog, QWidgetAction, QSlider,
                               QWidget, QHBoxLayout, QLabel, QAbstractItemView, QHeaderView)

ROLE_PATH = Qt.ItemDataRole.UserRole
ROLE_KIND = Qt.ItemDataRole.UserRole + 1
ROLE_PRIM = Qt.ItemDataRole.UserRole + 2


def Swatch(rgba, edge=None):
    pm = QPixmap(14, 14)
    pm.fill(Qt.GlobalColor.transparent)
    p = QPainter(pm)
    p.setBrush(QBrush(QColor(rgba[0], rgba[1], rgba[2], max(60, rgba[3]))))
    p.setPen(QPen(QColor(*(edge[:3] if edge is not None else (90, 90, 90)))))
    p.drawRect(1, 1, 12, 12)
    p.end()
    return QIcon(pm)


class SceneTree(QTreeWidget):
    """ SceneTree(main_window) """

    def __init__(self, win):
        super().__init__(win)
        self.win = win
        self.setColumnCount(2)
        self.setHeaderLabels(['Name', 'Type'])
        # the names always fit; the type takes what is left, elided (full
        # text as tooltip), and a narrow dock scrolls sideways
        self.header().setSectionResizeMode(0, QHeaderView.ResizeMode.ResizeToContents)
        self.header().setStretchLastSection(True)
        self.setTextElideMode(Qt.TextElideMode.ElideRight)
        self.setSelectionMode(QAbstractItemView.SelectionMode.ExtendedSelection)
        self.setContextMenuPolicy(Qt.ContextMenuPolicy.CustomContextMenu)
        self.customContextMenuRequested.connect(self._ContextMenu)
        self.itemChanged.connect(self._OnItemChanged)
        self.itemExpanded.connect(self._OnExpanded)
        self.itemSelectionChanged.connect(self._OnSelection)
        self.itemDoubleClicked.connect(self._OnDoubleClick)
        self._items = {}
        self._updating = False

    @property
    def viewer(self):
        return self.win.viewer

    # ------------------------------------------------------------------
    def Refresh(self):
        """ Rebuild or update the rows from the core scene tree. """
        v = self.viewer
        tree = v.GetTree()
        self._updating = True
        self.blockSignals(True)
        try:
            seen = set()
            for i, node in enumerate(tree):
                self._Sync(None, node, i, seen)
            # deepest first: removing a row deletes the rows below it
            for path in sorted(self._items, key=lambda p: -p.count('/')):
                if path not in seen:
                    it = self._items.pop(path)
                    parent = it.parent()
                    if parent is None:
                        self.takeTopLevelItem(self.indexOfTopLevelItem(it))
                    else:
                        parent.removeChild(it)
            if not v.GetHighlight():
                # cleared in the core (Esc, picking off): drop the matching rows
                for it in self.selectedItems():
                    if it.data(0, ROLE_KIND) in ('property', 'primitive'):
                        it.setSelected(False)
                if not self.selectedItems():
                    self.win.info.clear()
        finally:
            self.blockSignals(False)
            self._updating = False

    def _Sync(self, parent_item, node, index, seen):
        if node['kind'] == 'grid':
            return   # the grid has its own dock
        path = node['path']
        seen.add(path)
        it = self._items.get(path)
        if it is None:
            it = QTreeWidgetItem()
            it.setData(0, ROLE_PATH, path)
            it.setData(0, ROLE_KIND, node['kind'])
            # user checkable, but not auto tri-state: the mixed state comes from the core
            it.setFlags(it.flags() | Qt.ItemFlag.ItemIsUserCheckable)
            # the index counts the grid nodes, which have no rows; Qt drops
            # an insert beyond the end
            if parent_item is None:
                self.insertTopLevelItem(min(index, self.topLevelItemCount()), it)
            else:
                parent_item.insertChild(min(index, parent_item.childCount()), it)
            self._items[path] = it
            if node['kind'] == 'group' and path.count('/') < 2:
                # a group with only hidden content starts collapsed
                it.setExpanded(node['visible'] is not False)
        it.setText(0, node['label'])
        vis = node['visible']
        it.setCheckState(0, Qt.CheckState.PartiallyChecked if vis is None else
                         (Qt.CheckState.Checked if vis else Qt.CheckState.Unchecked))
        if node['kind'] == 'property':
            # primitive rows only while primitives can be selected, created on expand
            prims = self.viewer.GetPrimitiveSelection()
            it.setChildIndicatorPolicy(QTreeWidgetItem.ChildIndicatorPolicy.ShowIndicator if prims else
                                       QTreeWidgetItem.ChildIndicatorPolicy.DontShowIndicator)
            if not prims and it.childCount():
                it.takeChildren()
            try:
                e = self.viewer._scene.Entry(path)
                it.setIcon(0, Swatch(e.prop.GetFillColor(), e.prop.GetEdgeColor()))
                it.setText(1, e.prop.GetTypeString())
                it.setForeground(1, QBrush(QColor(128, 128, 128)))
            except KeyError:
                pass
        elif node['kind'] == 'item':
            item = self.viewer.GetItems().get(path)
            desc = None if item is None else self.viewer._CallHook(item.GetDescription)
            it.setText(1, desc or 'item')
            it.setToolTip(1, desc or '')
            it.setForeground(1, QBrush(QColor(128, 128, 128)))
        elif node['kind'] == 'figure':
            it.setText(1, 'figure')
            it.setForeground(1, QBrush(QColor(128, 128, 128)))
        # empty groups are hidden, except Materials
        empty = node['kind'] == 'group' and not node['children']
        it.setHidden(empty and path != 'properties/materials')
        for i, child in enumerate(node['children']):
            self._Sync(it, child, i, seen)

    def _OnExpanded(self, it):
        if it.data(0, ROLE_KIND) != 'property' or it.childCount() or not self.viewer.GetPrimitiveSelection():
            return
        e = self.viewer._scene.Entry(it.data(0, ROLE_PATH))
        self.blockSignals(True)
        for idx, prim in enumerate(e.prop.GetAllPrimitives()):
            # numbered as in the view state (highlight, focus): by position
            child = QTreeWidgetItem(['{} #{}'.format(prim.GetTypeName(), idx),
                                     'prio {}'.format(prim.GetPriority())])
            child.setData(0, ROLE_KIND, 'primitive')
            child.setData(0, ROLE_PATH, e.path)
            child.setData(0, ROLE_PRIM, (prim.GetID(), idx))
            child.setForeground(1, QBrush(QColor(128, 128, 128)))
            it.addChild(child)
        self.blockSignals(False)

    def _OnItemChanged(self, it, column):
        if self._updating or column != 0:
            return
        kind = it.data(0, ROLE_KIND)
        if kind == 'primitive':
            return
        state = it.checkState(0)
        self.viewer.SetVisible(it.data(0, ROLE_PATH), state != Qt.CheckState.Unchecked)
        self.Refresh()
        self.win.RefreshActions()
        self.win.grid_panel.Refresh()

    def _OnSelection(self):
        items = self.selectedItems()
        v = self.viewer
        if not items:
            v.Highlight(None)
            v.HighlightCurves([])
            self.win.info.clear()
            return
        v.HighlightCurves([i.data(0, ROLE_PATH) for i in items if i.data(0, ROLE_KIND) == 'curve'])
        it = self.currentItem() if self.currentItem() in items else items[-1]
        kind = it.data(0, ROLE_KIND)
        path = it.data(0, ROLE_PATH)
        if kind not in ('property', 'primitive'):
            v.Highlight(None)   # an item, group or grid row has no highlight
        # a figure or curve row brings its figure to the front, any other row the 3D view
        if kind == 'figure':
            v.RaiseFigure(path)
        elif kind == 'curve':
            v.RaiseFigure(v._scene.tree.Get(path).parent.path)
        elif not (kind == 'group' and path == 'figures'):
            v.RaiseFigure(None)
        if kind == 'property':
            names = [i.data(0, ROLE_PATH) for i in items if i.data(0, ROLE_KIND) == 'property']
            v.Highlight([v._scene.Entry(n).name for n in names])
            self.win.info.ShowProperty(v._scene.Entry(path).prop)
        elif kind == 'primitive':
            e = v._scene.Entry(path)
            _, idx = it.data(0, ROLE_PRIM)
            v.Highlight((e.name, idx))
            self.win.info.ShowPrimitive(e.prop.GetPrimitive(idx))
        elif kind == 'item':
            self.win.info.ShowItem(v.GetItems()[path])
        elif kind == 'part':   # the settings of its item
            self.win.info.ShowItem(v.GetItems()[v._scene.tree.Get(path).parent.path])
        elif kind == 'grid':
            self.win.info.ShowGrid()

    def _OnDoubleClick(self, it, column):
        kind = it.data(0, ROLE_KIND)
        v = self.viewer
        try:
            if kind == 'property':
                v.Focus(v._scene.Entry(it.data(0, ROLE_PATH)).name)
            elif kind == 'primitive':
                _, idx = it.data(0, ROLE_PRIM)
                v.Focus((v._scene.Entry(it.data(0, ROLE_PATH)).name, idx))
        except (ValueError, KeyError):
            pass

    def SelectItem(self, path):
        """ Select the row of an item (by path), showing its info and settings. """
        it = self._items.get(path)
        if it is None:
            return
        self.blockSignals(True)
        self.clearSelection()
        it.setSelected(True)
        self.blockSignals(False)
        self.scrollToItem(it)
        self.win.info.ShowItem(self.viewer.GetItems()[path])

    def Select(self, prop_name, index=None):
        """ Select the row of a property (and of its primitive number
        `index`), or clear with None. """
        self.blockSignals(True)
        self.clearSelection()
        self.blockSignals(False)
        if prop_name is None:
            self.win.info.clear()
            return
        v = self.viewer
        e = v._scene.Entry(prop_name)
        it = self._items.get(e.path)
        if it is None:
            return
        target = it
        if index is not None and index >= 0 and v.GetPrimitiveSelection():
            it.setExpanded(True)
            self._OnExpanded(it)
            for i in range(it.childCount()):
                c = it.child(i)
                if c.data(0, ROLE_PRIM)[1] == index:
                    target = c
                    break
        self.blockSignals(True)
        target.setSelected(True)
        self.blockSignals(False)
        self.scrollToItem(target)
        if target is it:
            self.win.info.ShowProperty(e.prop)
        else:
            self.win.info.ShowPrimitive(e.prop.GetPrimitive(target.data(0, ROLE_PRIM)[1]))

    # ------------------------------------------------------------------
    def _SelectedProps(self):
        v = self.viewer
        return [v._scene.Entry(i.data(0, ROLE_PATH)).name for i in self.selectedItems()
                if i.data(0, ROLE_KIND) == 'property']

    def _ContextMenu(self, pos):
        it = self.itemAt(pos)
        if it is None:
            return
        kind = it.data(0, ROLE_KIND)
        v = self.viewer
        menu = QMenu(self)
        if kind == 'property':
            if not it.isSelected():
                self.clearSelection()
                it.setSelected(True)
            names = self._SelectedProps()
            first = v._scene.Entry(names[0]).prop
            menu.addAction('Fill colour...', lambda: self._ChooseColor(names, 'fill'))
            menu.addAction('Edge colour...', lambda: self._ChooseColor(names, 'edge'))
            menu.addAction(self._OpacityAction(menu, names, first.GetFillColor()[3]))
            mode_menu = menu.addMenu('Render mode')
            current = first.GetRenderMode()
            for mode, label in (('solid', 'Solid'), ('edges', 'Edges'), ('solid+edges', 'Solid and edges'),
                                (None, 'Default')):
                a = mode_menu.addAction(label, lambda m=mode: self._SetMode(names, m))
                a.setCheckable(True)
                a.setChecked(current == mode)
            menu.addSeparator()
            menu.addAction('Focus', lambda: v.Focus(names))
            menu.addAction('Isolate', lambda: (v.Isolate(names), self.Refresh()))
            menu.addAction('Show all', lambda: (v.Isolate(None), v.SetVisible('properties', True), self.Refresh()))
            exp = menu.addMenu('Export')
            for fmt in ('stl', 'ply', 'vtp'):
                exp.addAction(fmt.upper() + '...', lambda f=fmt: self.win.ExportDialog(f, prop=names))
        elif kind in ('group', 'grid', 'item', 'part'):
            path = it.data(0, ROLE_PATH)
            menu.addAction('Show', lambda: (v.SetVisible(path, True), self.Refresh()))
            menu.addAction('Hide', lambda: (v.SetVisible(path, False), self.Refresh()))
            if kind == 'item':
                menu.addAction('Focus', lambda: self._FocusItem(path))
                item = v.GetItems()[path]
                try:
                    extra = item.GetActions() or []
                except Exception as err:
                    extra = []
                    self.win.statusBar().showMessage('item actions failed: {}'.format(err), 5000)
                if extra:
                    menu.addSeparator()
                for label, fn in extra:
                    menu.addAction(label, lambda fn=fn, label=label: self._RunItemAction(label, fn))
        else:
            return
        menu.exec(self.viewport().mapToGlobal(pos))

    def _RunItemAction(self, label, fn):
        try:
            fn()
        except Exception as err:
            self.win.statusBar().showMessage('{} failed: {}'.format(label, err), 5000)
        self.viewer.Render()
        self.Refresh()
        self.win.item_editor.Sync()

    def _FocusItem(self, path):
        b = self.viewer.GetItems()[path].GetBounds()
        if b is not None:
            self.viewer.Focus(b)

    def _ChooseColor(self, names, which):
        v = self.viewer
        prop = v._scene.Entry(names[0]).prop
        rgba = prop.GetFillColor() if which == 'fill' else prop.GetEdgeColor()
        c = QColorDialog.getColor(QColor(*rgba[:3]), self, 'Fill colour' if which == 'fill' else 'Edge colour')
        if not c.isValid():
            return
        with v.Batch():
            for n in names:
                if which == 'fill':
                    v.SetPropertyColor(n, fill=c.name(), render=False)
                else:
                    v.SetPropertyColor(n, edge=c.name(), render=False)
        self.Refresh()

    def _OpacityAction(self, menu, names, alpha):
        w = QWidget(menu)
        lay = QHBoxLayout(w)
        lay.setContentsMargins(24, 2, 8, 2)
        lay.addWidget(QLabel('Opacity'))
        s = QSlider(Qt.Orientation.Horizontal)
        s.setRange(0, 255)
        s.setValue(alpha)
        lay.addWidget(s)

        def changed(val):
            with self.viewer.Batch():
                for n in names:
                    self.viewer.SetPropertyColor(n, opacity=val / 255.0, render=False)
        s.valueChanged.connect(changed)
        wa = QWidgetAction(menu)
        wa.setDefaultWidget(w)
        return wa

    def _SetMode(self, names, mode):
        with self.viewer.Batch():
            for n in names:
                self.viewer.SetPropertyRenderMode(n, mode, render=False)
