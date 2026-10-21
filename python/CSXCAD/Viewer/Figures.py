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
#
"""
Matplotlib figures shown next to the 3D view (:meth:`Viewer.AddFigure`).

A :class:`ViewFigure` is one figure with named curves. In the scene tree it
is a node of kind ``figure`` below the group ``figures`` (visible: shown as
a tab or window of the GUI), its curves are child nodes of kind ``curve``
(visible: drawn). Only the GUI shows figures; offscreen they are written as
images. Matplotlib is needed only by whoever creates the figures.
"""

import re

import numpy as np

_SUB = dict(zip('0123456789+-=()aehijklmnoprstuvx', '₀₁₂₃₄₅₆₇₈₉₊₋₌₍₎ₐₑₕᵢⱼₖₗₘₙₒₚᵣₛₜᵤᵥₓ'))
_SUP = dict(zip('0123456789+-=()in', '⁰¹²³⁴⁵⁶⁷⁸⁹⁺⁻⁼⁽⁾ⁱⁿ'))
# commands not listed (e.g. \Re, \log) keep their name
_SYMBOLS = {
    'pi': 'π', 'Omega': 'Ω', 'omega': 'ω', 'mu': 'µ', 'epsilon': 'ε',
    'varepsilon': 'ε', 'lambda': 'λ', 'beta': 'β', 'alpha': 'α', 'gamma': 'γ', 'Gamma': 'Γ',
    'theta': 'θ', 'phi': 'φ', 'varphi': 'φ', 'sigma': 'σ', 'tau': 'τ', 'delta': 'δ',
    'Delta': 'Δ', 'eta': 'η', 'rho': 'ρ', 'nu': 'ν', 'kappa': 'κ', 'infty': '∞',
    'cdot': '·', 'times': '×', 'pm': '±', 'deg': '°', 'circ': '°', 'rightarrow': '→',
    'leftarrow': '←', 'approx': '≈', 'sqrt': '√', 'angle': '∠', 'mathrm': '', 'rm': '', 'mathit': '', 'it': '', 'mathbf': '', 'bf': '',
    'text': '', 'left': '', 'right': '',
}


def _Script(text, table, mark):
    """ Sub- or superscript `text` in Unicode where every character has one,
    else written with `mark` (``_`` or ``^``). """
    if text and all(c in table for c in text):
        return ''.join(table[c] for c in text)
    return mark + ('(' + text + ')' if re.search(r'[\s,]', text) else text)


def _PlainMath(s):
    s = re.sub(r'\\([{}|%$&#_])', lambda m: {'{': '(', '}': ')'}.get(m.group(1), m.group(1)), s)
    s = re.sub(r'\\([A-Za-z]+)\s*', lambda m: _SYMBOLS.get(m.group(1), m.group(1)), s)
    s = s.replace('\\,', ' ').replace('\\ ', ' ').replace('\\;', ' ').replace('\\!', '').replace('~', ' ')
    # innermost groups first: x_{in} -> xᵢₙ, then plain braces
    for _ in range(10):
        t = re.sub(r'([_^])\{([^{}]*)\}', lambda m: _Script(m.group(2), _SUB if m.group(1) == '_' else _SUP,
                                                             m.group(1)), s)
        t = re.sub(r'\{([^{}]*)\}', r'\1', t)
        if t == s:
            break
        s = t
    s = re.sub(r'([_^])(\w)', lambda m: _Script(m.group(2), _SUB if m.group(1) == '_' else _SUP, m.group(1)), s)
    return s


def PlainLabel(text):
    """ PlainLabel(text)

    A matplotlib label as plain text for the tree: the math parts (between
    ``$``) with Greek letters and Unicode sub- and superscripts, e.g.
    ``'$\\Re\\{Z_{in}\\}$'`` -> ``'Re(Zᵢₙ)'``, ``'$S_{11}$'`` -> ``'S₁₁'``.
    """
    text = str(text)
    if '$' not in text:
        return text
    parts = re.split(r'(?<!\\)\$', text)
    out = ''.join(_PlainMath(p) if n % 2 else p.replace('\\$', '$') for n, p in enumerate(parts))
    return re.sub(r'\s+', ' ', out).strip()


class ViewFigure:
    """ ViewFigure(figure, name=None, curves=None)

    A matplotlib figure for the viewer, see :meth:`Viewer.AddFigure`.

    Parameters
    ----------
    figure : matplotlib.figure.Figure
    name : str, optional
        Default: the figure's suptitle, else ``'Figure'``.
    curves : dict, optional
        ``{name: artist or [artists]}``, the parts that can be shown and
        hidden. Default: the labelled lines, collections and images of all
        axes (maps without a label are named after their axes' title).
        Mathtext names become plain text (:func:`PlainLabel`).
    """

    def __init__(self, figure, name=None, curves=None):
        self._figure = figure
        self._name = name or _Title(figure) or 'Figure'
        # mathtext labels as plain text (see PlainLabel), made unique
        self._curves = {}
        for label, artists in (_AutoCurves(figure) if curves is None else curves.items()):
            base, n = PlainLabel(label), 1
            key = base
            while key in self._curves:
                n += 1
                key = '{} ({})'.format(base, n)
            self._curves[key] = list(artists) if isinstance(artists, (list, tuple)) else [artists]
        self._path = None
        self._shown = True
        self._highlight = {}   # {curve: [(artist, line width before)]}

    def __repr__(self):
        return '<ViewFigure {!r}: {} curves>'.format(self._name, len(self._curves))

    def GetName(self):
        return self._name

    def GetPath(self):
        """ Path in the scene tree, None while not added. """
        return self._path

    def GetFigure(self):
        """ The matplotlib figure. """
        return self._figure

    def GetCurves(self):
        """ The curve names, in order. """
        return list(self._curves)

    def GetCurveArtists(self, name):
        return list(self._curves[name])

    def GetCurveVisible(self, name):
        return any(a.get_visible() for a in self._curves[name])

    def SetCurveVisible(self, name, val):
        """ Draw or hide the curve `name` (use :meth:`Viewer.SetVisible` on
        its tree path to keep the GUI in sync). """
        for a in self._curves[name]:
            a.set_visible(bool(val))
        self.Redraw()

    def SetHighlight(self, names):
        """ Draw the curves `names` (a list, empty: none) with a thicker
        line, e.g. while they are selected in the tree. Maps keep theirs. """
        names = [n for n in names if n in self._curves]
        for name in [n for n in self._highlight if n not in names]:
            for a, lw in self._highlight.pop(name):
                a.set_linewidth(lw)
        for name in names:
            if name not in self._highlight:
                lines = [(a, a.get_linewidth()) for a in self._curves[name]
                         if hasattr(a, 'get_linewidth') and np.ndim(a.get_linewidth()) == 0]
                self._highlight[name] = lines
                for a, lw in lines:
                    a.set_linewidth(max(2 * lw, lw + 2))
        self.Redraw()

    def GetHighlight(self):
        return list(self._highlight)

    def IsShown(self):
        """ Shown as a tab or window of the GUI. """
        return self._shown

    def Redraw(self):
        canvas = getattr(self._figure, 'canvas', None)
        if canvas is not None:
            canvas.draw_idle()


def _Title(figure):
    st = getattr(figure, '_suptitle', None)
    return st.get_text() if st is not None else ''


def _AutoCurves(figure):
    """ ``[(label, artist)]`` of the labelled artists and the maps of all axes. """
    curves = []
    for k, ax in enumerate(figure.axes):
        if getattr(ax, '_colorbar', None) is not None:
            continue   # the axes of a colour bar
        for a in list(ax.get_lines()) + list(ax.collections) + list(ax.images):
            label = a.get_label()
            if label and not label.startswith('_'):
                curves.append((label, a))
            elif a in ax.collections or a in ax.images:
                if getattr(a, 'get_array', None) is not None and a.get_array() is not None:
                    curves.append((ax.get_title() or 'map {}'.format(k + 1), a))
    return curves
