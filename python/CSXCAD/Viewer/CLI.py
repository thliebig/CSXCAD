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
"""

import argparse
import json
import sys

DESCRIPTION = 'View a CSXCAD structure (XML file), or render images and exports from it.'


def _Size(text):
    try:
        w, h = text.lower().split('x')
        return int(w), int(h)
    except ValueError:
        raise argparse.ArgumentTypeError('size must be WIDTHxHEIGHT, e.g. 1200x800')


def _Parser():
    p = argparse.ArgumentParser(prog='csxcad-view', description=DESCRIPTION)
    p.add_argument('file', nargs='?', help='structure XML file (a ContinuousStructure anywhere in it)')
    p.add_argument('--view', metavar='VIEW.json', help='view state to apply (see Viewer.SaveState)')
    p.add_argument('--size', type=_Size, metavar='WxH', help='render size in pixels')
    p.add_argument('--png', metavar='OUT.png', help='write a screenshot of the view')
    p.add_argument('--views', metavar='OUT.png', help='write a contact sheet of all sides')
    p.add_argument('--export', nargs=2, action='append', metavar=('FMT', 'PATH'),
                   help='export, FMT one of stl, ply, vtp, gltf, glb, svg, pdf, eps, png (repeatable)')
    p.add_argument('--describe', nargs='?', const='text', choices=('text', 'json'),
                   help='print a summary of the structure')
    p.add_argument('--offscreen', action='store_true', help='never open a window')
    p.add_argument('--no-gui', action='store_true', help='use the plain VTK window instead of the GUI')
    p.add_argument('--gui', action='store_true', help='open the window even after writing outputs')
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
    exports = list(args.export or [])
    if args.export_STL:
        exports.append(('stl', args.export_STL))
    if args.export_polydata_vtk:
        exports.append(('vtp', args.export_polydata_vtk))
    outputs = bool(args.png or args.views or exports or args.describe)
    want_window = (not outputs or args.gui) and not args.offscreen

    if args.file is None:
        if outputs or not want_window:
            print('csxcad-view: an input file is required for outputs', file=sys.stderr)
            return 2
    from CSXCAD.Viewer import Viewer
    view = {}
    if args.view:
        with open(args.view) as f:
            view = json.load(f)
    if args.size:
        view['size'] = args.size
    try:
        v = Viewer(args.file, offscreen=not want_window, **view)
    except (FileNotFoundError, ValueError) as err:
        print('csxcad-view: {}'.format(err), file=sys.stderr)
        return 1
    for fmt, path in exports:
        res = v.Export(fmt, path)
        for r in res['report']:
            print('csxcad-view: {}: {}'.format(r['file'], r['reason']), file=sys.stderr)
    if args.png:
        v.Screenshot(args.png)
    if args.views:
        v.RenderViews(args.views)
    if args.describe == 'json':
        print(json.dumps(v.Describe('dict'), indent=1))
    elif args.describe:
        print(v.Describe('text'))
    if want_window:
        v.Show(gui=False if args.no_gui else None)
    return 0


if __name__ == '__main__':
    sys.exit(Main())
