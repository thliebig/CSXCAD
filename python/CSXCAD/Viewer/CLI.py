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

"""CSXCAD.Viewer.CLI
The ``csxcad-view`` command.

Without output options it opens the GUI (or a plain VTK window). With only
output options it renders off screen, writes the outputs and exits.
AppCSXCAD's options are accepted too, so existing callers keep working.

The view is a view state (see :mod:`CSXCAD.Viewer.ViewState`): from a file
(``--view``), single keys (``--set``) or both; ``--list-view-keys`` lists
them, ``--print-view`` prints the resulting state.
"""

import argparse
import json
import sys

DESCRIPTION = 'View a CSXCAD structure (XML file), or render images and exports from it.'
EPILOG = """\
The view: --view FILE.json and/or --set KEY=VALUE (repeatable, VALUE as JSON,
plain words as text); --list-view-keys lists the keys, --print-view prints the
resulting view as a template. Tree paths for "visible" come from --describe.

examples:
  csxcad-view model.xml --describe
  csxcad-view model.xml --views all_sides.png
  csxcad-view model.xml --png feed.png --set view=iso --set focus=port --set elevation=20
  csxcad-view model.xml --png cut.png --set view=+y --set 'clip={"normal": "y", "position": 0}'
  csxcad-view model.xml --png patch.png --set 'highlight=["patch"]' --set render_mode=solid+edges
  csxcad-view sim/openEMS_setup.xml --file sim/PEC_dump.vtp --set isolate=type:Metal --png pec.png
"""


def _SetArg(text):
    """ KEY=VALUE of --set: VALUE as JSON, else as text. """
    key, sep, val = text.partition('=')
    if not sep or not key:
        raise argparse.ArgumentTypeError('--set needs KEY=VALUE, e.g. view=iso')
    try:
        return key.strip(), json.loads(val)
    except ValueError:
        return key.strip(), val


def _Size(text):
    try:
        w, h = text.lower().split('x')
        return int(w), int(h)
    except ValueError:
        raise argparse.ArgumentTypeError('size must be WIDTHxHEIGHT, e.g. 1200x800')


def AddViewArguments(p):
    """ AddViewArguments(parser)

    The view and output options of ``csxcad-view`` on an
    ``argparse.ArgumentParser``, for a command building on it (e.g.
    ``openems-view``); see :func:`ViewFromArgs` and :func:`WriteOutputs`.
    """
    p.add_argument('--view', metavar='VIEW.json', help='view state to apply (e.g. from --print-view or '
                                                        'Viewer.SaveState)')
    p.add_argument('--set', type=_SetArg, action='append', metavar='KEY=VALUE',
                   help='one view state key, after --view (repeatable), e.g. view=+z')
    p.add_argument('--list-view-keys', action='store_true', help='list the view state keys and exit')
    p.add_argument('--print-view', action='store_true', help='print the resulting view state as JSON')
    p.add_argument('--file', dest='files', action='append', metavar='FILE.vtp',
                   help='show a VTK file next to the structure (repeatable), e.g. openEMS\'s PEC_dump.vtp; '
                        'its item path is files/<name>')
    p.add_argument('--size', type=_Size, metavar='WxH', help='render size in pixels')
    p.add_argument('--png', metavar='OUT.png', help='write a screenshot of the view')
    p.add_argument('--views', metavar='OUT.png', help='write a contact sheet of all sides')
    p.add_argument('--export', nargs=2, action='append', metavar=('FMT', 'PATH'),
                   help='export, FMT one of stl, ply, vtp, gltf, glb, svg, pdf, eps, png (repeatable)')
    p.add_argument('--describe', nargs='?', const='text', choices=('text', 'json'),
                   help='print a summary: properties, tree paths, items and their settings')
    p.add_argument('--offscreen', action='store_true', help='never open a window')
    p.add_argument('--no-gui', action='store_true', help='use the plain VTK window instead of the GUI')
    p.add_argument('--gui', action='store_true', help='open the window even after writing outputs')


def ViewFromArgs(args):
    """ The view state dict of the options of :func:`AddViewArguments`. """
    view = {}
    if args.view:
        with open(args.view) as f:
            view = json.load(f)
    for key, val in args.set or []:
        view[key] = val
    if args.size:
        view['size'] = args.size
    return view


def AddFiles(v, args):
    """ Add the files of ``--file`` to the viewer `v`. """
    for fn in args.files or []:
        v.AddFile(fn, render=False)


def HasOutputs(args):
    """ Whether the options ask for outputs (then no window opens without --gui). """
    return bool(args.png or args.views or args.export or args.describe or args.print_view)


def WriteOutputs(v, args, prog='csxcad-view'):
    """ Write the outputs the options of :func:`AddViewArguments` ask for. """
    for fmt, path in args.export or []:
        res = v.Export(fmt, path)
        for r in res['report']:
            print('{}: {}: {}'.format(prog, r['file'], r['reason']), file=sys.stderr)
    if args.png:
        v.Screenshot(args.png)
    if args.views:
        v.RenderViews(args.views)
    if args.print_view:
        from CSXCAD.Viewer.ViewState import Dumps
        print(Dumps(v.GetState()))
    if args.describe == 'json':
        print(json.dumps(v.Describe('dict'), indent=1))
    elif args.describe:
        print(v.Describe('text'))


class ItemSettings:
    """ Context manager collecting the item settings a view state could not
    apply (see :class:`~CSXCAD.Viewer.Core.ItemSettingWarning`): a command
    reports them as errors, where the GUI only warns. Other warnings pass. """

    def __enter__(self):
        import warnings
        self._cm = warnings.catch_warnings(record=True)
        self._log = self._cm.__enter__()
        warnings.simplefilter('always')
        self.errors = []
        return self

    def __exit__(self, *exc):
        import warnings
        from CSXCAD.Viewer.Core import ItemSettingWarning
        self._cm.__exit__(*exc)
        for w in self._log:
            if issubclass(w.category, ItemSettingWarning):
                self.errors.append(str(w.message))
            else:
                warnings.warn_explicit(w.message, w.category, w.filename, w.lineno)
        return False


def UnknownPaths(v, view):
    """ Messages for the tree paths of `view`'s ``visible`` and ``items``
    that are not in the viewer `v` (the viewer keeps them for items added
    later; a command adds none later). """
    out = []
    paths = set(v._scene.tree.nodes)
    bad = [p for p in (view.get('visible') or {}) if p not in paths]
    if bad:
        out.append('no tree path {}; the paths: {}'.format(', '.join(bad), ', '.join(sorted(paths))))
    have = list(v.GetItems()) + list(v._figures)
    bad = [p for p in (view.get('items') or {}) if p not in have]
    if bad:
        out.append('no item or figure {}; the items and figures: {}'.format(', '.join(bad), ', '.join(have) or 'none'))
    return out


def ErrorText(err):
    """ The message of an error from a wrong view or name (a KeyError's
    text is its quoted argument). """
    return err.args[0] if isinstance(err, KeyError) and err.args else str(err)


def _Parser():
    p = argparse.ArgumentParser(prog='csxcad-view', description=DESCRIPTION, epilog=EPILOG,
                                formatter_class=argparse.RawDescriptionHelpFormatter)
    p.add_argument('file', nargs='?', help='structure XML file (a ContinuousStructure anywhere in it)')
    AddViewArguments(p)
    # AppCSXCAD compatibility
    p.add_argument('--disableEdit', action='store_true', help=argparse.SUPPRESS)
    p.add_argument('-v', dest='verbose', action='store_true', help=argparse.SUPPRESS)
    p.add_argument('--RenderDiscMaterial', action='store_true', help=argparse.SUPPRESS)
    p.add_argument('--export-STL', metavar='DIR', help=argparse.SUPPRESS)
    p.add_argument('--export-polydata-vtk', metavar='DIR', help=argparse.SUPPRESS)
    return p


def Main(argv=None):
    """ Entry point of ``csxcad-view``; returns the exit code. """
    args = _Parser().parse_args(argv)
    if args.list_view_keys:
        print(KeyList())
        return 0
    args.export = list(args.export or [])
    if args.export_STL:
        args.export.append(('stl', args.export_STL))
    if args.export_polydata_vtk:
        args.export.append(('vtp', args.export_polydata_vtk))
    want_window = (not HasOutputs(args) or args.gui) and not args.offscreen

    if args.file is None and (HasOutputs(args) or not want_window):
        print('csxcad-view: an input file is required for outputs', file=sys.stderr)
        return 2
    from CSXCAD.Viewer import Viewer
    try:
        with ItemSettings() as wrong:
            v = Viewer(args.file, offscreen=not want_window, **ViewFromArgs(args))
            AddFiles(v, args)
    except (FileNotFoundError, ValueError, KeyError) as err:
        print('csxcad-view: {}'.format(ErrorText(err)), file=sys.stderr)
        return 1
    errors = wrong.errors + UnknownPaths(v, ViewFromArgs(args))
    if errors:
        for e in errors:
            print('csxcad-view: {}'.format(e), file=sys.stderr)
        return 1
    WriteOutputs(v, args)
    if want_window:
        v.Show(gui=False if args.no_gui else None)
    return 0


def KeyList():
    """ The view state keys with their default and meaning, for --list-view-keys. """
    from CSXCAD.Viewer.ViewState import DEFAULTS, KEY_HELP, ToJSON
    lines = ['View state keys (--set KEY=VALUE, VALUE as JSON; or a --view file):', '']
    for key, default in DEFAULTS.items():
        if key == 'version':
            continue
        d = json.dumps(ToJSON(default))
        if len(d) > 40:
            d = '{...}' if isinstance(default, dict) else d[:37] + '...'
        lines.append('  {:<20} default {}'.format(key, d))
        lines.append('      {}'.format(KEY_HELP[key]))
    return '\n'.join(lines)


if __name__ == '__main__':
    sys.exit(Main())
