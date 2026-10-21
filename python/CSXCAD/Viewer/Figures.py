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

A figure has settings like an item (:meth:`ViewFigure.GetParameters`): its
title, per axes the labels, limits, scales, grid and legend, per curve its
label, colour and line. The GUI edits them in its Settings dock, the view
state stores them under ``items``.
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


# the settings of a figure: {option: (type, help)}; see ViewFigure.GetParameters
_FIGURE_OPTIONS = {
    'title': ('str', 'Title of the figure, above all axes.'),
}
_AXES_OPTIONS = {
    'axes_title': ('str', 'Title of the axes.'),
    'xlabel': ('str', 'Label of the x axis.'),
    'ylabel': ('str', 'Label of the y axis.'),
    'xlim': ('range', 'Range of the x axis: two numbers, e.g. [1, 3]; empty: automatic.'),
    'ylim': ('range', 'Range of the y axis (polar: the radius): two numbers; empty: automatic.'),
    'xscale': ('choice', 'Scale of the x axis.'),
    'yscale': ('choice', 'Scale of the y axis.'),
    'ylabel_right': ('str', 'Label of the second y axis at the right.'),
    'ylim_right': ('range', 'Range of the second y axis: two numbers; empty: automatic.'),
    'theta_zero': ('choice', 'Where the angle 0 is (polar axes).'),
    'theta_direction': ('choice', 'Direction of increasing angles: 1 counterclockwise, -1 clockwise (polar axes).'),
    'grid': ('bool', 'Grid lines.'),
    'legend': ('bool', 'The legend of the curves.'),
}
_CURVE_OPTIONS = {
    'label': ('str', 'Text of the curve in the legend (its tree row keeps its name).'),
    'color': ('color', 'Colour of the curve.'),
    'linestyle': ('choice', 'Line style of the curve.'),
    'linewidth': ('float', 'Line width of the curve in points.'),
    'cmap': ('str', 'Colour map of the map, a matplotlib name such as viridis or jet.'),
    'clim': ('range', 'Colour range of the map: two numbers; empty: automatic.'),
}
_CHOICES = {
    'xscale': ['linear', 'log', 'symlog'],
    'yscale': ['linear', 'log', 'symlog'],
    'theta_zero': ['E', 'NE', 'N', 'NW', 'W', 'SW', 'S', 'SE'],
    'theta_direction': [1, -1],
    'linestyle': ['-', '--', '-.', ':', 'None'],
}


def ParseRange(value):
    """ ParseRange(value)

    Two numbers from a list or a text such as ``'1, 3'`` or ``'[1 3]'``;
    None for automatic (None, an empty text or ``'auto'``). """
    if value is None:
        return None
    if isinstance(value, str):
        text = value.strip().strip('[]()').strip()
        if text.lower() in ('', 'auto', 'none'):
            return None
        value = [v for v in re.split(r'[\s,;]+', text) if v]
    if len(value) != 2:
        raise ValueError('a range needs two numbers, not {!r}'.format(value))
    return [float(value[0]), float(value[1])]


def _Round(v):
    return float('{:.6g}'.format(v))


def FigureAxes(figure):
    """ FigureAxes(figure)

    ``([axes], {n: twin})``: the axes of a matplotlib figure that hold
    curves or maps (not those of colour bars), and the second y axis at the
    right (``twinx``) of axes number n. """
    main, twins = [], {}
    for ax in figure.axes:
        if getattr(ax, '_colorbar', None) is not None:
            continue   # the axes of a colour bar
        pos = ax.get_position().bounds
        host = next((n for n, m in enumerate(main) if n not in twins and m.get_position().bounds == pos
                     and m.get_shared_x_axes().joined(m, ax)), None)
        if host is None:
            main.append(ax)
        else:
            twins[host] = ax
    return main, twins


def _Legend(ax, twin):
    """ The legend of `ax` with the curves of its `twin` too, as one. """
    h, labels = ax.get_legend_handles_labels()
    if twin is not None:
        h2, l2 = twin.get_legend_handles_labels()
        h, labels = h + h2, labels + l2
    return ax.legend(h, labels)


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
        self._curves = self._MakeCurves(curves)
        self._path = None
        self._shown = True
        self._highlight = {}   # {curve: [(artist, line width before)]}
        # the settings as created: "Copy view as Python" leaves those out
        self._defaults = {p['name']: p['value'] for p in self._Params()}

    def _MakeCurves(self, curves):
        """ ``{name: [artists]}`` from the `curves` given (default: found
        in the figure); mathtext labels as plain text (see PlainLabel), made
        unique. """
        out = {}
        for label, artists in (_AutoCurves(self._figure) if curves is None else curves.items()):
            base, n = PlainLabel(label), 1
            key = base
            while key in out:
                n += 1
                key = '{} ({})'.format(base, n)
            out[key] = list(artists) if isinstance(artists, (list, tuple)) else [artists]
        return out

    def _Redrawn(self, curves=None):
        """ After the matplotlib figure was drawn anew (e.g. by its
        creator, from a changed description): take its new artists, keeping
        the names, which curves are hidden and which highlighted. """
        hidden = [c for c in self._curves if not self.GetCurveVisible(c)]
        highlight = list(self._highlight)
        self._highlight = {}
        self._curves = self._MakeCurves(curves)
        for c in hidden:
            if c in self._curves:
                for a in self._curves[c]:
                    a.set_visible(False)
        self.SetHighlight(highlight)   # and redraws

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

    # ------------------------------------------------------------------
    # settings
    def GetParameters(self):
        """ GetParameters()

        The settings of the figure, in the format of
        :meth:`CSXCAD.Viewer.Scene.ViewItem.GetParameters`: ``title`` of the
        figure; per axes ``axes_title``, ``xlabel``, ``ylabel``, ``xlim``,
        ``ylim``, ``xscale``, ``yscale``, ``grid``, ``legend`` (and the
        second y axis' ``ylabel_right``, ``ylim_right``; polar axes
        ``theta_zero``, ``theta_direction``), numbered ``xlabel.1`` from
        the second axes on; per curve ``<curve>/label``, ``color``,
        ``linestyle``, ``linewidth`` (maps: ``cmap``, ``clim``). Ranges are
        lists of two numbers. Change them with
        :meth:`Viewer.SetItemParameter` on the figure's tree path. """
        out = []
        for p in self._Params():
            if p['name'] in self._defaults:
                p['default'] = self._defaults[p['name']]
            out.append(p)
        return out

    def SetParameter(self, name, value):
        """ SetParameter(name, value)

        Change the setting `name` (see :meth:`GetParameters`) of the
        matplotlib figure and redraw it. """
        target = self._Target(name)
        self._Apply(target, value)
        self.Redraw()

    def CreateWidget(self, parent):
        """ No widget of its own: the GUI shows the settings as a form. """
        return None

    def _Targets(self):
        """ ``{name: (opt, section, what)}`` of all settings; `what` is
        None (the figure), ``('axes', n)`` or ``('curve', name)``. """
        out = {'title': ('title', 'Figure', None)}
        main, twins = FigureAxes(self._figure)
        for n, ax in enumerate(main):
            suffix = '' if n == 0 else '.{}'.format(n)
            section = 'Axes' if len(main) == 1 else 'Axes {}'.format(n)
            if ax.name == 'polar':
                opts = ['axes_title', 'ylim', 'theta_zero', 'theta_direction', 'grid', 'legend']
            else:
                opts = ['axes_title', 'xlabel', 'ylabel', 'xlim', 'ylim', 'xscale', 'yscale', 'grid', 'legend']
                if n in twins:
                    opts[3:3] = ['ylabel_right', 'ylim_right']
            for opt in opts:
                out[opt + suffix] = (opt, section, ('axes', n))
        for c, artists in self._curves.items():
            a = artists[0]
            if hasattr(a, 'get_cmap'):
                opts = ['cmap', 'clim']
            elif hasattr(a, 'get_linestyle') and hasattr(a, 'get_xydata'):
                opts = ['label', 'color', 'linestyle', 'linewidth']
            else:
                continue
            for opt in opts:
                out['{}/{}'.format(c, opt)] = (opt, 'Curve ' + c, ('curve', c))
        return out

    def _Target(self, name):
        targets = self._Targets()
        if name not in targets:
            raise KeyError('figure "{}" has no setting "{}"; the settings: {}'.format(
                self._name, name, ', '.join(targets)))
        return name, targets[name]

    def _Params(self):
        main, twins = FigureAxes(self._figure)
        out = []
        for name, (opt, section, what) in self._Targets().items():
            kind, text = dict(_FIGURE_OPTIONS, **dict(_AXES_OPTIONS, **_CURVE_OPTIONS))[opt]
            p = dict(name=name, label=opt.replace('_', ' '), section=section, help=text,
                     value=self._Read(opt, what, main, twins))
            if kind == 'range':
                p['type'] = 'str'
            else:
                p['type'] = kind
            if kind == 'choice':
                p['choices'] = list(_CHOICES[opt])
            if opt == 'linewidth':
                p.update(min=0.0, step=0.5, decimals=2)
            out.append(p)
        return out

    def _Read(self, opt, what, main, twins):
        fig = self._figure
        if what is None:
            st = getattr(fig, '_suptitle', None)
            return st.get_text() if st is not None else ''
        if what[0] == 'curve':
            a = self._curves[what[1]][0]
            if opt == 'label':
                return a.get_label()
            if opt == 'color':
                from matplotlib.colors import to_hex
                return to_hex(a.get_color())
            if opt == 'linestyle':
                ls = a.get_linestyle()
                return ls if ls in _CHOICES['linestyle'] else '-'
            if opt == 'linewidth':
                # while highlighted, the width it gets back
                lw = dict(self._highlight.get(what[1], [])).get(a, a.get_linewidth())
                return float(lw)
            if opt == 'cmap':
                return a.get_cmap().name
            if opt == 'clim':
                return [_Round(v) for v in a.get_clim()]
        ax = main[what[1]]
        if opt.endswith('_right'):
            ax = twins[what[1]]
            opt = opt[:-len('_right')]
        if opt == 'axes_title':
            return ax.get_title()
        if opt in ('xlabel', 'ylabel', 'xscale', 'yscale'):
            return getattr(ax, 'get_' + opt)()
        if opt in ('xlim', 'ylim'):
            return [_Round(v) for v in getattr(ax, 'get_' + opt)()]
        if opt == 'theta_zero':
            off = round(np.rad2deg(ax.get_theta_offset())) % 360
            return dict(zip((0, 45, 90, 135, 180, 225, 270, 315), _CHOICES['theta_zero'])).get(off, 'E')
        if opt == 'theta_direction':
            return int(ax.get_theta_direction())
        if opt == 'grid':
            return any(g.get_visible() for g in ax.get_xgridlines() + ax.get_ygridlines())
        if opt == 'legend':
            leg = ax.get_legend()
            return leg is not None and leg.get_visible()
        raise KeyError(opt)

    def _Apply(self, target, value):
        """ Apply the setting `target` (from :meth:`_Target`) to the
        matplotlib figure. """
        name, (opt, section, what) = target
        fig = self._figure
        if what is None:
            fig.suptitle(str(value))
            return
        if what[0] == 'curve':
            c = what[1]
            for a in self._curves[c]:
                if opt == 'label':
                    a.set_label(str(value))
                elif opt == 'color':
                    from CSXCAD.Viewer.ViewState import ParseColor
                    a.set_color(ParseColor(value)[0])
                elif opt == 'linestyle':
                    a.set_linestyle(self._Choice(opt, value))
                elif opt == 'linewidth':
                    lw = float(value)
                    if c in self._highlight:   # thicker while selected, this width afterwards
                        self._highlight[c] = [(h, lw if h is a else w) for h, w in self._highlight[c]]
                        a.set_linewidth(max(2 * lw, lw + 2))
                    else:
                        a.set_linewidth(lw)
                elif opt == 'cmap':
                    a.set_cmap(str(value))
                elif opt == 'clim':
                    rng = ParseRange(value)
                    if rng is None:
                        a.autoscale()
                    else:
                        a.set_clim(*rng)
            if opt == 'label':
                main, twins = FigureAxes(fig)
                for n, ax in enumerate(main):
                    if ax.get_legend() is not None and ax.get_legend().get_visible():
                        _Legend(ax, twins.get(n))
            return
        main, twins = FigureAxes(fig)
        n = what[1]
        ax = main[n]
        if opt.endswith('_right'):
            ax = twins[n]
            opt = opt[:-len('_right')]
        if opt == 'axes_title':
            ax.set_title(str(value))
        elif opt in ('xlabel', 'ylabel'):
            getattr(ax, 'set_' + opt)(str(value))
        elif opt in ('xscale', 'yscale'):
            getattr(ax, 'set_' + opt)(self._Choice(opt, value))
        elif opt in ('xlim', 'ylim'):
            rng = ParseRange(value)
            if rng is None:
                ax.autoscale(axis=opt[0])
            else:
                getattr(ax, 'set_' + opt)(*rng)
        elif opt == 'theta_zero':
            ax.set_theta_zero_location(self._Choice(opt, value))
        elif opt == 'theta_direction':
            ax.set_theta_direction(self._Choice(opt, int(value)))
        elif opt == 'grid':
            ax.grid(bool(value))
        elif opt == 'legend':
            leg = ax.get_legend()
            if value and leg is None:
                _Legend(ax, twins.get(n))
            elif leg is not None:
                leg.set_visible(bool(value))

    @staticmethod
    def _Choice(opt, value):
        if value not in _CHOICES[opt]:
            raise ValueError('{} must be one of {}, not {!r}'.format(opt, _CHOICES[opt], value))
        return value


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
