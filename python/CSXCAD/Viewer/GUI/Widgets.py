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

"""CSXCAD.Viewer.GUI.Widgets
Grid plane sliders, section-plane controls, the animation bar and the item
settings form.
"""

import math

from PySide6.QtCore import Qt
from PySide6.QtGui import QColor
from PySide6.QtWidgets import (QWidget, QVBoxLayout, QGridLayout, QGroupBox, QCheckBox, QSlider, QLabel,
                               QComboBox, QDoubleSpinBox, QPushButton, QColorDialog, QToolBar, QSpinBox,
                               QFormLayout, QLineEdit, QHBoxLayout, QScrollArea, QFrame)

from CSXCAD.Viewer.ViewState import ParseColor


class GridPanel(QWidget):
    """ GridPanel(main_window): mesh grid planes and the section plane. """

    def __init__(self, win):
        super().__init__(win)
        self.win = win
        self._updating = False
        lay = QVBoxLayout(self)

        box = QGroupBox('Mesh grid')
        g = QGridLayout(box)
        self.visible = QCheckBox('Show grid')
        self.visible.setToolTip('Show the mesh: grid planes and outline.')
        self.visible.toggled.connect(lambda on: self._Set(visible=on))
        g.addWidget(self.visible, 0, 0, 1, 3)
        self.planes = {}
        for row, d in enumerate('xyz', start=1):
            cb = QCheckBox(d)
            cb.setToolTip('Show the grid plane normal to {}.'.format(d))
            sl = QSlider(Qt.Orientation.Horizontal)
            sl.setToolTip('Position of the {} plane: the mesh line it lies on.'.format(d))
            lab = QLabel()
            lab.setToolTip('Coordinate of the {} plane.'.format(d))
            lab.setMinimumWidth(90)
            cb.toggled.connect(lambda on, d=d: self._Set(planes={d: dict(visible=on)}))
            sl.valueChanged.connect(lambda val, d=d: self._Set(planes={d: dict(index=val)}))
            g.addWidget(cb, row, 0)
            g.addWidget(sl, row, 1)
            g.addWidget(lab, row, 2)
            self.planes[d] = (cb, sl, lab)
        g.addWidget(QLabel('Opacity'), 4, 0)
        self.opacity = QSlider(Qt.Orientation.Horizontal)
        self.opacity.setRange(0, 100)
        self.opacity.setToolTip('Opacity of the grid lines.')
        self.opacity.valueChanged.connect(lambda val: self._Set(opacity=val / 100.0))
        g.addWidget(self.opacity, 4, 1)
        self.color = QPushButton('Colour')
        self.color.setToolTip('Colour of the grid lines.')
        self.color.clicked.connect(self._ChooseColor)
        g.addWidget(self.color, 4, 2)
        self.outline = QCheckBox('Outline with ticks')
        self.outline.setToolTip('The simulation domain as a box, with a tick at every mesh line.')
        self.outline.toggled.connect(lambda on: self._Set(outline=on))
        g.addWidget(self.outline, 5, 0, 1, 3)
        g.addWidget(QLabel('Quality'), 6, 0)
        self.quality = QComboBox()
        self.quality.addItems(['off', 'ratio', 'size'])
        self.quality.setToolTip('Mesh quality on the grid planes: "ratio" marks lines whose neighbouring cells\n'
                                'differ in size by more than the threshold, "size" colours the lines by cell size.')
        self.quality.currentTextChanged.connect(lambda t: self._Set(quality=None if t == 'off' else t))
        g.addWidget(self.quality, 6, 1)
        self.threshold = QDoubleSpinBox()
        self.threshold.setRange(1.0, 10.0)
        self.threshold.setSingleStep(0.1)
        self.threshold.setToolTip('Size ratio of neighbouring cells above which "ratio" marks a line.')
        self.threshold.valueChanged.connect(lambda val: self._Set(quality_threshold=val))
        g.addWidget(self.threshold, 6, 2)
        lay.addWidget(box)

        box = QGroupBox('Section plane')
        g = QGridLayout(box)
        self.clip_on = QCheckBox('Clip')
        self.clip_on.setToolTip('Cut the structure open with a section plane.')
        self.clip_on.toggled.connect(self._ClipChanged)
        self.clip_normal = QComboBox()
        self.clip_normal.addItems(['x', 'y', 'z'])
        self.clip_normal.setToolTip('Normal direction of the section plane.')
        self.clip_normal.setCurrentText('z')
        self.clip_normal.currentTextChanged.connect(self._ClipChanged)
        self.clip_invert = QCheckBox('Invert')
        self.clip_invert.setToolTip('Keep the other side of the section plane.')
        self.clip_invert.toggled.connect(self._ClipChanged)
        self.clip_pos = QSlider(Qt.Orientation.Horizontal)
        self.clip_pos.setRange(0, 1000)
        self.clip_pos.setValue(500)
        self.clip_pos.setToolTip('Position of the section plane across the structure.')
        self.clip_pos.valueChanged.connect(self._ClipChanged)
        self.clip_label = QLabel()
        g.addWidget(self.clip_on, 0, 0)
        g.addWidget(self.clip_normal, 0, 1)
        g.addWidget(self.clip_invert, 0, 2)
        g.addWidget(self.clip_pos, 1, 0, 1, 2)
        g.addWidget(self.clip_label, 1, 2)
        lay.addWidget(box)
        lay.addStretch(1)

    @property
    def viewer(self):
        return self.win.viewer

    def _Set(self, **kw):
        if self._updating:
            return
        self.viewer.SetGrid(**kw)
        self.Refresh()
        self.win.tree.Refresh()
        self.win.RefreshActions()

    def _ChooseColor(self):
        rgb, _ = ParseColor(self.viewer.GetGrid()['color'])
        c = QColorDialog.getColor(QColor.fromRgbF(*rgb), self, 'Grid colour')
        if c.isValid():
            self._Set(color=c.name())

    def Refresh(self):
        v = self.viewer
        g = v.GetGrid()
        info = v.GetGridPlaneInfo()
        layer = v._scene.grid_layer
        names = ('r', 'a', 'z') if layer.cylindrical else ('x', 'y', 'z')
        self._updating = True
        try:
            self.visible.setChecked(bool(g['visible']))
            for n, d in enumerate('xyz'):
                cb, sl, lab = self.planes[d]
                cb.setText(names[n])
                cb.setChecked(bool(g['planes'][d]['visible']))
                if layer.lines is not None and d in info:
                    idx, coord, N = info[d][:3]
                    sl.setRange(0, N - 1)
                    sl.setValue(idx)
                    if names[n] == 'a':
                        lab.setText('{} = {:.4g}°'.format('α', math.degrees(coord)))
                    else:
                        lab.setText('{} = {:.4g}'.format(names[n], coord))
                    sl.setEnabled(True)
                else:
                    sl.setEnabled(False)
                    lab.setText('-')
            self.opacity.setValue(int(round(g['opacity'] * 100)))
            self.outline.setChecked(bool(g['outline']))
            self.quality.setCurrentText(g['quality'] or 'off')
            self.threshold.setValue(g['quality_threshold'])
            clip = v.GetClip()
            self.clip_on.setChecked(clip is not None)
            if clip is not None:
                if isinstance(clip['normal'], str):
                    self.clip_normal.setCurrentText(clip['normal'])
                self.clip_invert.setChecked(bool(clip['invert']))
                lo, hi = self._ClipRange()
                if hi > lo:
                    self.clip_pos.setValue(int(round(1000 * (clip['position'] - lo) / (hi - lo))))
                self.clip_label.setText('{:.4g}'.format(clip['position']))
        finally:
            self._updating = False

    def _ClipRange(self):
        b = self.viewer._Bounds()
        if b is None:
            return 0.0, 1.0
        n = 'xyz'.index(self.clip_normal.currentText())
        return b[2 * n], b[2 * n + 1]

    def _ClipChanged(self, *args):
        if self._updating:
            return
        v = self.viewer
        if not self.clip_on.isChecked():
            v.SetClip(None)
        else:
            lo, hi = self._ClipRange()
            pos = lo + (hi - lo) * self.clip_pos.value() / 1000.0
            v.SetClip(normal=self.clip_normal.currentText(), position=pos, invert=self.clip_invert.isChecked())
            self.clip_label.setText('{:.4g}'.format(pos))
        self.win.RefreshActions()


class AnimationBar(QToolBar):
    """ AnimationBar(main_window): play/pause, stop, stepping, step slider, fps. """

    def __init__(self, win):
        super().__init__('Animation', win)
        self.win = win
        self._updating = False
        for name in ('play', 'stop', 'step_back', 'step_forward'):
            self.addAction(win.Action(name))
        self.slider = QSlider(Qt.Orientation.Horizontal)
        self.slider.setMinimumWidth(200)
        self.slider.setToolTip('Animation step: drag to go through the animation.')
        self.slider.valueChanged.connect(self._OnSlider)
        self.addWidget(self.slider)
        self.label = QLabel()
        self.label.setMinimumWidth(110)
        self.addWidget(self.label)
        self.addWidget(QLabel(' fps '))
        self.fps = QSpinBox()
        self.fps.setRange(1, 120)
        self.fps.setToolTip('Animation speed in frames per second; also the frame rate of exported videos.')
        self.fps.valueChanged.connect(lambda val: None if self._updating else self.win.viewer.SetFps(val))
        self.addWidget(self.fps)

    def _OnSlider(self, val):
        if not self._updating:
            self.win.viewer.SetAnimationStep(val)

    def Refresh(self):
        v = self.win.viewer
        N = v.GetAnimationSteps()
        # it animates the 3D view: hidden while a figure is in front
        self.setVisible(N > 0 and getattr(self.win, '_view_in_front', True))
        if N == 0:
            return
        a = v.GetAnimation()
        self._updating = True
        try:
            self.slider.setRange(0, N - 1)
            self.slider.setValue(a['step'])
            self.label.setText('{}/{}  ({:.0f}%)'.format(a['step'], N, 100.0 * a['step'] / N))
            self.fps.setValue(int(round(a['fps'])))
        finally:
            self._updating = False
        self.win.RefreshActions()


class SliderField(QWidget):
    """ A slider over a list of numbers, with the value as readout. """

    def __init__(self, values, value, apply):
        super().__init__()
        self.values = [float(v) for v in values]
        lay = QHBoxLayout(self)
        lay.setContentsMargins(0, 0, 0, 0)
        self.slider = QSlider(Qt.Orientation.Horizontal)
        self.slider.setRange(0, max(len(self.values) - 1, 0))
        # apply on release: a step may read data
        self.slider.setTracking(False)
        self.label = QLabel()
        self.label.setMinimumWidth(60)
        lay.addWidget(self.slider, 1)
        lay.addWidget(self.label)
        self.slider.sliderMoved.connect(self._Show)
        self.slider.valueChanged.connect(lambda i: apply(self.values[i]))
        self.SetValue(value)

    def _Show(self, i):
        self.label.setText('{:.4g}'.format(self.values[i]) if self.values else '')

    def SetValue(self, value):
        if not self.values:
            return
        i = min(range(len(self.values)), key=lambda n: abs(self.values[n] - float(value)))
        self.slider.blockSignals(True)
        self.slider.setValue(i)
        self.slider.blockSignals(False)
        self._Show(i)


class ItemEditor(QWidget):
    """ ItemEditor(main_window): settings of the selected item.

    A form built from :meth:`CSXCAD.Viewer.Viewer.GetItemParameters`
    (grouped by their ``section``), plus the item's own widget
    (:meth:`~CSXCAD.Viewer.Scene.ViewItem.CreateWidget`), in its own dock;
    a hint while no item with settings is selected.
    """

    def __init__(self, win):
        super().__init__(win)
        self.win = win
        self.item = None
        self._fields = {}
        lay = QVBoxLayout(self)
        lay.setContentsMargins(0, 0, 0, 0)
        # long forms scroll instead of stretching the window
        self._scroll = QScrollArea()
        self._scroll.setWidgetResizable(True)
        self._scroll.setHorizontalScrollBarPolicy(Qt.ScrollBarPolicy.ScrollBarAlwaysOff)
        self._scroll.setFrameShape(QFrame.Shape.NoFrame)
        lay.addWidget(self._scroll)
        self._box = None
        self._Hint('Select an item to change its settings.')

    def _Params(self):
        return [p for p in self.win.viewer.GetItemParameters(self.item) if p.get('form', True)]

    def _Hint(self, text):
        hint = QLabel(text)
        hint.setWordWrap(True)
        hint.setAlignment(Qt.AlignmentFlag.AlignTop | Qt.AlignmentFlag.AlignLeft)
        hint.setEnabled(False)
        self._box = hint
        self._scroll.setWidget(hint)

    def SetItem(self, item):
        """ Show the settings of `item` (None clears). """
        if self._box is not None:
            self._scroll.takeWidget()
            self._box.deleteLater()
            self._box = None
        self.item = item
        self._fields = {}
        if item is None:
            self._Hint('Select an item to change its settings.')
            return
        try:
            params = self._Params()
            custom = item.CreateWidget(self)
        except Exception as err:
            self.win.statusBar().showMessage('item settings failed: {}'.format(err), 5000)
            params, custom = [], None
        if not params and custom is None:
            self._Hint('{} has no settings.'.format(item.GetName()))
            return
        self._box = QGroupBox(item.GetName())
        lay = QVBoxLayout(self._box)
        if params:
            form = QFormLayout()
            section = None
            for p in params:
                if p.get('section') != section:
                    section = p.get('section')
                    head = QLabel('<b>{}</b>'.format(section or ''))
                    form.addRow(head)
                w = self._Field(p)
                self._fields[p['name']] = (p, w)
                label = QLabel(p.get('label', p['name']))
                if p.get('help'):
                    label.setToolTip(p['help'])
                    w.setToolTip(p['help'])
                form.addRow(label, w)
            lay.addLayout(form)
        if custom is not None:
            lay.addWidget(custom)
        self._scroll.setWidget(self._box)

    def _Field(self, p):
        kind = p.get('type', 'str')
        name = p['name']
        val = p.get('value')
        if kind == 'bool':
            w = QCheckBox()
            w.setChecked(bool(val))
            w.toggled.connect(lambda on: self._Apply(name, on))
        elif kind == 'slider':
            w = SliderField(p.get('values', ()), val, lambda v: self._Apply(name, v))
        elif kind in ('int', 'float'):
            if kind == 'int':
                w = QSpinBox()
                w.setRange(int(p.get('min', -2**31)), int(p.get('max', 2**31 - 1)))
                w.setSingleStep(int(p.get('step', 1)))
                w.setValue(int(val))
            else:
                w = QDoubleSpinBox()
                w.setDecimals(int(p.get('decimals', 4)))
                w.setRange(float(p.get('min', -1e12)), float(p.get('max', 1e12)))
                w.setSingleStep(float(p.get('step', 0.1)))
                w.setValue(float(val))
            # apply on Enter, arrows or focus loss, not on every key stroke
            w.setKeyboardTracking(False)
            w.valueChanged.connect(lambda v: self._Apply(name, v))
        elif kind == 'choice':
            w = QComboBox()
            choices = list(p.get('choices', []))
            labels = list(p.get('labels') or [str(c) for c in choices])
            for c, label in zip(choices, labels):
                w.addItem(label, c)
            if val in choices:
                w.setCurrentIndex(choices.index(val))
            w.currentIndexChanged.connect(lambda i, w=w: self._Apply(name, w.itemData(i)))
        elif kind == 'color':
            w = QPushButton()
            self._SetSwatch(w, val)
            w.clicked.connect(lambda checked=False, w=w: self._ChooseColor(name, w))
        else:
            w = QLineEdit('' if val is None else str(val))
            w.editingFinished.connect(lambda w=w: self._Apply(name, w.text()))
        return w

    @staticmethod
    def _SetSwatch(button, val):
        rgb, _ = ParseColor(val if val is not None else 'black')
        c = QColor.fromRgbF(*rgb)
        button.setText(c.name())
        button.setStyleSheet('background-color: {}; color: {}'.format(
            c.name(), 'black' if c.lightnessF() > 0.5 else 'white'))

    def _ChooseColor(self, name, button):
        c = QColorDialog.getColor(QColor(button.text()), self, name)
        if c.isValid():
            self._Apply(name, c.name())

    def _Apply(self, name, value):
        item = self.item
        if item is None:
            return
        try:
            self.win.viewer.SetItemParameter(item, name, value)
        except Exception as err:
            self.win.statusBar().showMessage('{}: {}'.format(name, err), 5000)
        self.Sync()
        self.win.info.ShowItem(item)

    def Sync(self):
        """ Re-read the values from the item (a setting may change others);
        rebuild the form when the set of settings changed. """
        if self.item is None:
            return
        try:
            params = {p['name']: p for p in self._Params()}
        except Exception:
            return
        if list(params) != list(self._fields):
            self.SetItem(self.item)
            return
        for name, (p, w) in self._fields.items():
            if name not in params:
                continue
            val = params[name].get('value')
            w.blockSignals(True)
            try:
                if isinstance(w, QCheckBox):
                    w.setChecked(bool(val))
                elif isinstance(w, QSpinBox):
                    w.setValue(int(val))
                elif isinstance(w, QDoubleSpinBox):
                    w.setValue(float(val))
                elif isinstance(w, QComboBox):
                    i = w.findData(val)
                    if i >= 0:
                        w.setCurrentIndex(i)
                elif isinstance(w, SliderField):
                    w.SetValue(val)
                elif isinstance(w, QPushButton):
                    self._SetSwatch(w, val)
                elif isinstance(w, QLineEdit):
                    w.setText('' if val is None else str(val))
            finally:
                w.blockSignals(False)
