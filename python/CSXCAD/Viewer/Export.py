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

"""CSXCAD.Viewer.Export
Exporter registry for geometry and scenes.

``EXPORTERS`` maps a format name to ``(function, extensions, description)``.
Each function is called as ``function(viewer, path, **opts)``; register a new
one with :func:`RegisterExporter`.

Mesh formats (stl, ply, vtp) write one file per property into a directory,
in metres by default (``scale='m'``), or a single merged file with
``merged=True``. They also work without any window:
:func:`ExportCSX` takes a structure directly.
"""

import os
import re

import numpy as np

from vtkmodules.vtkCommonDataModel import vtkPolyData
from vtkmodules.vtkCommonTransforms import vtkTransform
from vtkmodules.vtkFiltersCore import vtkAppendPolyData, vtkTriangleFilter
from vtkmodules.vtkFiltersGeneral import vtkTransformFilter
from vtkmodules.vtkCommonCore import vtkUnsignedCharArray

EXPORTERS = {}


def RegisterExporter(name, func, extensions, description):
    """ RegisterExporter(name, func, extensions, description)

    Add an export format: ``func(viewer, path, **opts)``.
    """
    EXPORTERS[name] = (func, tuple(extensions), description)


def SafeName(name, used=None):
    """ File-system safe version of a property name, unique within `used`. """
    s = re.sub(r'[\\/:*?"<>|\x00-\x1f]', '_', name).strip().strip('.') or 'unnamed'
    if used is not None:
        base, n = s, 1
        while s.lower() in used:
            n += 1
            s = '{}_{}'.format(base, n)
        used.add(s.lower())
    return s


def _Scaled(pd, factor):
    if factor == 1:
        return pd
    tr = vtkTransform()
    tr.Scale(factor, factor, factor)
    flt = vtkTransformFilter()
    flt.SetInputData(pd)
    flt.SetTransform(tr)
    flt.Update()
    out = vtkPolyData()
    out.ShallowCopy(flt.GetOutput())
    return out


def _ScaleFactor(viewer, scale):
    if scale in ('m', 'meter', 'metre'):
        return viewer.CSX.GetGrid().GetDeltaUnit()
    if scale in ('drawing', None, 1):
        return 1.0
    return float(scale)


def _PropertyData(viewer, include_hidden):
    """ [(entry, polydata incl. disc model)] of the properties to export. """
    out = []
    for e in viewer._scene.entries:
        if not include_hidden and not e.IsVisible():
            continue
        parts = [pd for pd in (e.raw, e.disc_pd if viewer.GetDiscMaterial() else None) if pd is not None]
        if not parts:
            continue
        if len(parts) == 1:
            pd = parts[0]
        else:
            app = vtkAppendPolyData()
            for p in parts:
                app.AddInputData(p)
            app.Update()
            pd = app.GetOutput()
        out.append((e, pd))
    return out


def _Surface(pd):
    """ Triangles only (STL/PLY have no lines or points). """
    tri = vtkTriangleFilter()
    tri.SetInputData(pd)
    tri.PassLinesOff()
    tri.PassVertsOff()
    tri.Update()
    out = vtkPolyData()
    out.ShallowCopy(tri.GetOutput())
    return out


def _MeshExport(viewer, path, fmt, merged=False, include_hidden=False, scale='m', binary=True):
    from vtkmodules.vtkIOGeometry import vtkSTLWriter
    from vtkmodules.vtkIOPLY import vtkPLYWriter
    from vtkmodules.vtkIOXML import vtkXMLPolyDataWriter
    factor = _ScaleFactor(viewer, scale)
    data = _PropertyData(viewer, include_hidden)
    report = []
    files = []

    def write(pd, fn, rgba=None):
        if fmt in ('stl', 'ply'):
            n_lines = pd.GetNumberOfLines() + pd.GetNumberOfVerts()
            pd = _Surface(pd)
            if n_lines:
                report.append(dict(file=os.path.basename(fn), reason='{} line/point cells dropped'.format(n_lines)))
            if pd.GetNumberOfPolys() == 0:
                report.append(dict(file=os.path.basename(fn), reason='no surface, not written'))
                return
        pd = _Scaled(pd, factor)
        if fmt == 'stl':
            w = vtkSTLWriter()
            w.SetFileTypeToBinary() if binary else w.SetFileTypeToASCII()
        elif fmt == 'ply':
            w = vtkPLYWriter()
            w.SetFileTypeToBinary() if binary else w.SetFileTypeToASCII()
            if rgba is not None:
                pd = _WithCellArray(pd, _ColorArray(pd, rgba))
            if pd.GetCellData().GetArray('colors') is not None:
                w.SetArrayName('colors')
                w.SetEnableAlpha(True)
        else:
            w = vtkXMLPolyDataWriter()
            w.SetDataModeToBinary() if binary else w.SetDataModeToAscii()
        w.SetFileName(fn)
        w.SetInputData(pd)
        w.Write()
        files.append(fn)

    if merged:
        if not path.lower().endswith('.' + fmt):
            path = path + '.' + fmt
        d = os.path.dirname(os.path.abspath(path))
        os.makedirs(d, exist_ok=True)
        app = vtkAppendPolyData()
        for e, pd in data:
            if fmt == 'ply':
                pd = _WithCellArray(pd, _ColorArray(pd, e.prop.GetFillColor()))
            app.AddInputData(pd)
        if not data:
            return dict(files=[], report=[dict(file=path, reason='nothing to export')])
        app.Update()
        write(app.GetOutput(), path)
    else:
        os.makedirs(path, exist_ok=True)
        used = set()
        for e, pd in data:
            fn = os.path.join(path, SafeName(e.name, used) + '.' + fmt)
            write(pd, fn, rgba=e.prop.GetFillColor())
    return dict(files=files, report=report)


def _ColorArray(pd, rgba):
    colors = vtkUnsignedCharArray()
    colors.SetName('colors')
    colors.SetNumberOfComponents(4)
    colors.SetNumberOfTuples(pd.GetNumberOfCells())
    for c in range(4):
        colors.FillComponent(c, rgba[c])
    return colors


def _WithCellArray(pd, arr):
    out = vtkPolyData()
    out.ShallowCopy(pd)
    cd = out.GetCellData()
    cd.Initialize()
    for i in range(pd.GetCellData().GetNumberOfArrays()):
        cd.AddArray(pd.GetCellData().GetArray(i))
    cd.AddArray(arr)
    return out


def ExportSTL(viewer, path, **opts):
    """ One binary STL per visible property in directory `path`. """
    return _MeshExport(viewer, path, 'stl', **opts)


def ExportPLY(viewer, path, **opts):
    """ One PLY (with the fill colour per face) per visible property. """
    return _MeshExport(viewer, path, 'ply', **opts)


def ExportVTP(viewer, path, **opts):
    """ One VTK XML polydata file per visible property (keeps ``primitive_id``). """
    return _MeshExport(viewer, path, 'vtp', **opts)


def ExportGLTF(viewer, path, include_items=True, include_hidden=False, scale='m', binary=None):
    """ ExportGLTF(viewer, path, include_items=True, include_hidden=False, scale='m')

    glTF scene with one named mesh per property and its fill colour and
    opacity as material, e.g. for Blender. Writes ``.glb`` (one binary
    file) if `path` ends with it or `binary` is True, else ``.gltf`` with the
    data embedded.
    """
    from vtkmodules.vtkIOExport import vtkGLTFExporter
    from vtkmodules.vtkRenderingCore import vtkActor, vtkPolyDataMapper, vtkRenderer
    from vtkmodules.vtkFiltersCore import vtkPolyDataNormals
    from CSXCAD.Viewer.Core import NewOffscreenWindow
    if binary is None:
        binary = path.lower().endswith('.glb')
    factor = _ScaleFactor(viewer, scale)
    ren = vtkRenderer()
    keep = []
    names = []
    for e, pd in _PropertyData(viewer, include_hidden):
        surf = _Surface(pd)
        if surf.GetNumberOfPolys() == 0:
            continue
        norm = vtkPolyDataNormals()
        norm.SetInputData(_Scaled(surf, factor))
        norm.SplittingOn()
        norm.Update()
        out = vtkPolyData()
        out.ShallowCopy(norm.GetOutput())
        out.GetCellData().Initialize()
        m = vtkPolyDataMapper()
        m.SetInputData(out)
        m.ScalarVisibilityOff()
        a = vtkActor()
        a.SetMapper(m)
        rgba = e.prop.GetFillColor()
        a.GetProperty().SetColor(rgba[0] / 255.0, rgba[1] / 255.0, rgba[2] / 255.0)
        a.GetProperty().SetOpacity(rgba[3] / 255.0)
        ren.AddActor(a)
        keep.append((a, m, out))
        names.append(e.name)
    if include_items:
        for it in viewer._scene.items.values():
            if not it.IsExportable() or not it.GetVisible():
                continue
            for p in it.GetProps():
                if not isinstance(p, vtkActor) or p.GetMapper() is None:
                    continue
                # an own mapper: the item's one keeps its OpenGL buffers in
                # the viewer's window, the export renders in another
                m = p.GetMapper().NewInstance()
                m.ShallowCopy(p.GetMapper())
                q = vtkActor()
                q.ShallowCopy(p)
                q.SetMapper(m)
                q.SetScale(*(np.array(p.GetScale()) * factor))
                q.SetPosition(*(np.array(p.GetPosition()) * factor))
                q.SetOrigin(*(np.array(p.GetOrigin()) * factor))
                ren.AddActor(q)
                keep.append((q, m))
                names.append(it.GetName())
    rw = NewOffscreenWindow()
    rw.SetSize(64, 64)
    rw.AddRenderer(ren)
    rw.Render()
    exp = vtkGLTFExporter()
    exp.SetRenderWindow(rw)
    exp.SetFileName(path)
    exp.InlineDataOn()
    if hasattr(exp, 'SetSaveNormal'):
        exp.SetSaveNormal(True)
    exp.Write()
    rw.Finalize()
    import json
    with open(path) as f:
        doc = json.load(f)
    _ApplyNames(doc, names)
    if binary:
        _WriteGLB(doc, path)
    else:
        with open(path, 'w') as f:
            json.dump(doc, f)
    return dict(files=[path], report=[])


def _WriteGLB(doc, path):
    """ Pack a glTF document with base64 buffers into a binary .glb file. """
    import base64
    import json
    import struct
    blobs = []
    offsets = []
    pos = 0
    for buf in doc.get('buffers', []):
        uri = buf.pop('uri', '')
        data = base64.b64decode(uri.split(',', 1)[1]) if uri.startswith('data:') else b''
        data += b'\0' * ((4 - len(data) % 4) % 4)
        offsets.append(pos)
        blobs.append(data)
        pos += len(data)
    for bv in doc.get('bufferViews', []):
        bv['byteOffset'] = bv.get('byteOffset', 0) + offsets[bv.get('buffer', 0)]
        bv['buffer'] = 0
    binary = b''.join(blobs)
    doc['buffers'] = [dict(byteLength=len(binary))] if binary else []
    js = json.dumps(doc).encode('utf-8')
    js += b' ' * ((4 - len(js) % 4) % 4)
    total = 12 + 8 + len(js) + (8 + len(binary) if binary else 0)
    with open(path, 'wb') as f:
        f.write(b'glTF' + struct.pack('<II', 2, total))
        f.write(struct.pack('<I', len(js)) + b'JSON' + js)
        if binary:
            f.write(struct.pack('<I', len(binary)) + b'BIN\0' + binary)


def _ApplyNames(doc, names):
    """ Give the glTF nodes, meshes and materials the property names. """
    meshes = doc.get('meshes', [])
    mats = doc.get('materials', [])
    nodes = [n for n in doc.get('nodes', []) if 'mesh' in n]
    for node, name in zip(nodes, names):
        node['name'] = name
        mesh = meshes[node['mesh']]
        mesh['name'] = name
        for prim in mesh.get('primitives', []):
            if 'material' in prim and prim['material'] < len(mats):
                mats[prim['material']]['name'] = name


def ExportGL2PS(viewer, path, title='CSXCAD', fmt=None):
    """ Vector image (svg/pdf/eps) of the current view via GL2PS. """
    try:
        from vtkmodules.vtkIOExportGL2PS import vtkGL2PSExporter
        import vtkmodules.vtkRenderingGL2PSOpenGL2  # noqa: F401
    except ImportError:
        raise RuntimeError('this VTK build has no GL2PS support, vector export is not available')
    if fmt is None:
        fmt = os.path.splitext(path)[1].lstrip('.').lower()
    rw = viewer._EnsureWindow() if viewer.GetRenderWindow() is None else viewer.GetRenderWindow()
    rw.Render()
    exp = vtkGL2PSExporter()
    exp.SetRenderWindow(rw)
    base = os.path.splitext(path)[0]
    exp.SetFilePrefix(base)
    {'svg': exp.SetFileFormatToSVG, 'pdf': exp.SetFileFormatToPDF, 'eps': exp.SetFileFormatToEPS,
     'ps': exp.SetFileFormatToPS}[fmt]()
    exp.SetTitle(title)
    exp.CompressOff()
    exp.DrawBackgroundOn()
    exp.Write3DPropsAsRasterImageOff()
    exp.Write()
    return dict(files=[base + '.' + fmt], report=[])


def ExportPNG(viewer, path, size=None, scale=1, transparent_background=False):
    """ Screenshot of the current view. """
    viewer.Screenshot(path, size=size, scale=scale, transparent_background=transparent_background)
    return dict(files=[path], report=[])


RegisterExporter('stl', ExportSTL, ('.stl',), 'STL surface per property')
RegisterExporter('ply', ExportPLY, ('.ply',), 'PLY surface per property, with colours')
RegisterExporter('vtp', ExportVTP, ('.vtp',), 'VTK XML polydata per property')
RegisterExporter('gltf', ExportGLTF, ('.gltf',), 'glTF scene (e.g. for Blender)')
RegisterExporter('glb', lambda v, p, **o: ExportGLTF(v, p, binary=True, **o), ('.glb',), 'glTF scene, single file')
RegisterExporter('svg', lambda v, p, **o: ExportGL2PS(v, p, fmt='svg', **o), ('.svg',), 'SVG vector image of the view')
RegisterExporter('pdf', lambda v, p, **o: ExportGL2PS(v, p, fmt='pdf', **o), ('.pdf',), 'PDF vector image of the view')
RegisterExporter('eps', lambda v, p, **o: ExportGL2PS(v, p, fmt='eps', **o), ('.eps',), 'EPS vector image of the view')
RegisterExporter('png', ExportPNG, ('.png',), 'PNG image of the view')


def Export(viewer, fmt, path, **opts):
    """ Export(viewer, fmt, path, **opts)

    Export with the registered exporter `fmt`.

    Returns
    -------
    dict
        ``files`` written and a ``report`` of what was dropped or skipped.
    """
    fmt = fmt.lower()
    if fmt not in EXPORTERS:
        raise ValueError('unknown export format "{}", available: {}'.format(fmt, ', '.join(EXPORTERS)))
    return EXPORTERS[fmt][0](viewer, os.fspath(path), **opts)


def ExportCSX(CSX, fmt, path, **opts):
    """ ExportCSX(CSX, fmt, path, **opts)

    Export a structure without any window, e.g.
    ``ExportCSX(CSX, 'stl', 'out_dir')``.
    """
    from CSXCAD.Viewer.Core import Viewer
    view = opts.pop('view', {})
    v = Viewer(CSX, offscreen=fmt in ('svg', 'pdf', 'eps', 'png', 'gltf', 'glb'), **view)
    return Export(v, fmt, path, **opts)
