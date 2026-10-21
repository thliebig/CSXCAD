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

"""CSXCAD.Viewer.Files
VTK data files shown next to the structure, e.g. the metal edges openEMS
writes with ``--debug-PEC`` (``PEC_dump.vtp``).

The coordinates of a file are in metres unless it says otherwise: a field
data array :data:`LENGTH_UNIT` holds the metres per coordinate unit (the
viewer's VTP export and openEMS's PEC dump write it).
"""

import importlib
import os

from vtkmodules.vtkCommonTransforms import vtkTransform
from vtkmodules.vtkFiltersGeneral import vtkTransformFilter
from vtkmodules.vtkFiltersGeometry import vtkDataSetSurfaceFilter
from vtkmodules.vtkRenderingCore import vtkActor, vtkPolyDataMapper

from CSXCAD.Viewer.Scene import ViewItem
from CSXCAD.Viewer.ViewState import ParseColor

#: name of the field data array with the metres per coordinate unit
LENGTH_UNIT = 'LengthUnit'
#: name of a cell array of mesh directions 0, 1, 2, coloured like the axes
#: (x red, y green, z blue) whichever of them occur
DIRECTION = 'direction'

#: file extension -> (VTK module, reader class)
READERS = {
    '.vtp': ('vtkIOXML', 'vtkXMLPolyDataReader'),
    '.vtu': ('vtkIOXML', 'vtkXMLUnstructuredGridReader'),
    '.vtr': ('vtkIOXML', 'vtkXMLRectilinearGridReader'),
    '.vts': ('vtkIOXML', 'vtkXMLStructuredGridReader'),
    '.vti': ('vtkIOXML', 'vtkXMLImageDataReader'),
    '.vtk': ('vtkIOLegacy', 'vtkDataSetReader'),
    '.vtkhdf': ('vtkIOHDF', 'vtkHDFReader'),
}


def ReadFile(filename):
    """ ReadFile(filename)

    The data set of a VTK file (one of :data:`READERS`).
    """
    ext = os.path.splitext(filename)[1].lower()
    if ext not in READERS:
        raise ValueError('{}: not a VTK file the viewer reads; the extensions: {}'.format(
            filename, ', '.join(READERS)))
    if not os.path.isfile(filename):
        raise FileNotFoundError('{}: no such file'.format(filename))
    module, cls = READERS[ext]
    reader = getattr(importlib.import_module('vtkmodules.' + module), cls)()
    reader.SetFileName(filename)
    reader.Update()
    data = reader.GetOutputDataObject(0)
    if data is None or data.GetNumberOfPoints() == 0 and data.GetNumberOfCells() == 0:
        raise ValueError('{}: no data read'.format(filename))
    return data


def FileUnit(data):
    """ Metres per coordinate unit of `data` from its :data:`LENGTH_UNIT`
    field data, or None. """
    arr = data.GetFieldData().GetArray(LENGTH_UNIT)
    if arr is None or arr.GetNumberOfTuples() < 1:
        return None
    return float(arr.GetTuple1(0))


class FileItem(ViewItem):
    """ FileItem(filename, name=None, unit=None, color='red', line_width=2)

    A VTK file (see :data:`READERS`) as an item, its surface, lines and
    points scaled into the structure's drawing units. Add it with
    :meth:`Viewer.AddFile`.

    Parameters
    ----------
    filename : str
    name : str, optional
        Item name, default the file name.
    unit : str or float, optional
        Length unit of the file's coordinates: ``'m'``, ``'drawing'`` (the
        structure's) or metres per unit. Default: the file's
        :data:`LENGTH_UNIT` field data, else metres.
    color, line_width
        Initial settings (see :meth:`GetParameters`).
    """

    DEFAULT_GROUP = 'files'
    CLIPPED = True
    HELP = dict(
        color='colour of the file content, unless coloured by an array',
        line_width='width of the lines in pixels',
        color_by='colour by a data array of the file (one component), or solid; "direction" '
                 '(the PEC dump\'s) colours x, y, z edges red, green, blue',
    )

    def __init__(self, filename, name=None, unit=None, color='red', line_width=2):
        self._filename = os.path.abspath(filename)
        self._data = ReadFile(filename)
        if unit is None:
            unit = FileUnit(self._data)
        if unit is None:
            unit = 'm'
        if isinstance(unit, str) and unit not in ('m', 'drawing'):
            raise ValueError('unit "{}": "m", "drawing" or metres per coordinate unit'.format(unit))
        self._unit = unit if isinstance(unit, str) else float(unit)
        if self._data.IsA('vtkPolyData'):
            src = self._data
        else:
            surf = vtkDataSetSurfaceFilter()
            surf.SetInputData(self._data)
            surf.Update()
            src = surf.GetOutput()
        self._poly = src
        self._transform = vtkTransform()
        tf = vtkTransformFilter()
        tf.SetTransform(self._transform)
        tf.SetInputData(src)
        self._mapper = vtkPolyDataMapper()
        self._mapper.SetInputConnection(tf.GetOutputPort())
        # lines on a surface (e.g. edges of a metal sheet) stay in front of it
        self._mapper.SetRelativeCoincidentTopologyLineOffsetParameters(-2, -8)
        self._mapper.SetRelativeCoincidentTopologyPointOffsetParameter(-8)
        actor = vtkActor()
        actor.SetMapper(self._mapper)
        super().__init__(name or os.path.basename(filename), actor)
        self._actor = actor
        self._color = 'red'
        self._color_by = 'solid'
        self.SetParameter('color', color)
        self.SetParameter('line_width', line_width)
        self.SetParameter('color_by', 'solid')

    def GetFileName(self):
        return self._filename

    def GetData(self):
        """ The data set read from the file, in the file's units. """
        return self._data

    def GetScale(self):
        """ Drawing units per coordinate unit of the file. """
        if self._unit == 'drawing':
            return 1.0
        metres = 1.0 if self._unit == 'm' else self._unit
        v = self.GetViewer()
        if v is None or v.CSX is None:
            return 1.0
        return metres / v.CSX.GetGrid().GetDeltaUnit()

    def _Arrays(self):
        """ Names of the one-component cell and point arrays. """
        out = []
        for data in (self._poly.GetCellData(), self._poly.GetPointData()):
            for i in range(data.GetNumberOfArrays()):
                a = data.GetArray(i)
                if a is not None and a.GetNumberOfComponents() == 1 and a.GetName() and a.GetName() not in out:
                    out.append(a.GetName())
        return out

    def _Array(self, name):
        a = self._poly.GetCellData().GetArray(name)
        if a is not None:
            return a, True
        return self._poly.GetPointData().GetArray(name), False

    def OnAdded(self, viewer):
        s = self.GetScale()
        self._transform.Identity()
        self._transform.Scale(s, s, s)

    def GetInfo(self):
        d = self._poly
        info = dict(file=self._filename, points=d.GetNumberOfPoints(), lines=d.GetNumberOfLines(),
                    polygons=d.GetNumberOfPolys(), unit=self._unit, scale=self.GetScale())
        if self._Arrays():
            info['arrays'] = ', '.join(self._Arrays())
        return info

    def GetDescription(self):
        d = self._poly
        parts = [(d.GetNumberOfLines(), 'lines'), (d.GetNumberOfPolys(), 'polygons'), (d.GetNumberOfVerts(), 'points')]
        return ', '.join('{} {}'.format(n, what) for n, what in parts if n)

    def GetParameters(self):
        return [
            dict(name='color', type='color', value=self._color, default='red'),
            dict(name='line_width', type='float', value=self._actor.GetProperty().GetLineWidth(),
                 min=1, max=20, step=1, default=2.0),
            dict(name='color_by', type='choice', value=self._color_by, choices=['solid'] + self._Arrays(),
                 default='solid'),
        ]

    def SetParameter(self, name, value):
        p = self._actor.GetProperty()
        if name == 'color':
            p.SetColor(*ParseColor(value)[0])
            self._color = value
        elif name == 'line_width':
            p.SetLineWidth(float(value))
            p.SetPointSize(float(value) + 2)
        elif name == 'color_by':
            if value != 'solid' and value not in self._Arrays():
                raise ValueError('color_by "{}": one of {}'.format(value, ', '.join(['solid'] + self._Arrays())))
            self._color_by = value
            if value == 'solid':
                self._mapper.ScalarVisibilityOff()
                return
            arr, cells = self._Array(value)
            if cells:
                self._mapper.SetScalarModeToUseCellFieldData()
            else:
                self._mapper.SetScalarModeToUsePointFieldData()
            self._mapper.SelectColorArray(value)
            self._mapper.SetColorModeToMapScalars()   # an unsigned char array is no colour
            if value == DIRECTION:
                self._mapper.SetScalarRange(0, 2)
            else:
                self._mapper.SetScalarRange(arr.GetRange())
            self._mapper.ScalarVisibilityOn()
        else:
            super().SetParameter(name, value)

    def GetValueText(self, world_pos, prop, cell_id):
        if self._color_by == 'solid' or cell_id is None or cell_id < 0:
            return None
        arr, cells = self._Array(self._color_by)
        if not cells or cell_id >= arr.GetNumberOfTuples():
            return None
        val = arr.GetTuple1(cell_id)
        if self._color_by == DIRECTION and val in (0, 1, 2):
            return '{} {}'.format(self._color_by, 'xyz'[int(val)])
        return '{} = {:g}'.format(self._color_by, val)
