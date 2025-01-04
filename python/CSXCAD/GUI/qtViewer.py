import os, sys
import numpy as np

from CSXCAD.GUI.qtSettings import Qt, QApplication, QMainWindow
from CSXCAD.GUI.qtSettings import createApp

# # load implementations for rendering and interaction factory classes
import vtkmodules.vtkRenderingOpenGL2

from vtkmodules.qt.QVTKRenderWindowInteractor import PyQtImpl, QVTKRenderWindowInteractor
from vtkmodules.vtkInteractionStyle import vtkInteractorStyleTrackballCamera

from vtkmodules.vtkRenderingCore import (
    vtkActor,
    vtkPolyDataMapper,
    vtkDataSetMapper,
    vtkRenderWindow,
    vtkRenderWindowInteractor,
    vtkRenderer
)

from vtkmodules.vtkCommonCore import vtkDoubleArray, vtkFloatArray

from CSXCAD import CSProperties, CSRectGrid
from CSXCAD.GUI.vtkPrimitives import renderPrimitives

def array2vtkFloat(a):
    # this is an easy way to convert small arrays to vtk
    # for large arrays use:
    # from vtk.util.numpy_support import numpy_to_vtk  #<--- this pulls in way to many dependencies?
    out = vtkFloatArray()
    for i,x in enumerate(a):
        out.InsertNextValue(x)
    return out

class qtViewer(QMainWindow):
    arc_resolution = 32
    def __init__(self, **kw):
        QMainWindow.__init__(self, **kw)
        self.setWindowTitle('Simple CSXCAD Viewer')
        self.resize(800, 600)

        self.prop_actors = {}
        self.rect_grid = None
        self.grid_plane_actor = [None]*3
        self.grid_plane_filter = [None]*3

        self.ren_widget = QVTKRenderWindowInteractor(self)
        self.setCentralWidget(self.ren_widget)

        app = QApplication.instance()
        self.ren_widget.AddObserver("ExitEvent", lambda o, e, a=app: a.quit())

        self.ren = vtkRenderer()
        self.ren_win = self.ren_widget.GetRenderWindow()

        self.ren_win.GetInteractor().SetInteractorStyle(vtkInteractorStyleTrackballCamera())

        self.ren_win.AddRenderer(self.ren)

        self.addAxes()
        self.setGridOpacity(30, render=False)
        self.setBackgroundColor('white', render=False)

        self.ren_widget.Initialize()
        self.ren_widget.Start()

    def render(self, force=False):
        self.ren_win.GetInteractor().Render()

    def resetView(self):
        self.ren.ResetCamera()
        self.render()

    def addAxes(self):
        from vtkmodules.vtkRenderingAnnotation import vtkAxesActor
        from vtkmodules.vtkInteractionWidgets import vtkOrientationMarkerWidget
        from vtkmodules.vtkRenderingCore import vtkPropAssembly
        self.axes = vtkAxesActor()
        self.axes.SetTotalLength(4,4,4)
        self.axes_marker = vtkOrientationMarkerWidget()
        assembly = vtkPropAssembly()
        assembly.AddPart(self.axes)

        self.axes_marker.SetOrientationMarker(assembly)
        self.axes_marker.SetViewport(0.0,0.0,0.25,0.25)

        self.axes_marker.SetInteractor(self.ren_win.GetInteractor())
        self.axes_marker.SetEnabled(1)
        self.axes_marker.InteractiveOff()

    def setBackgroundColor(self, color, render=True):
        color = CSProperties.hex2color(color)
        color = [c/255 for c in color]
        self.ren.SetBackground(color)

        iColor = [0,0,0] if np.sum(color)/3>0.5 else [1,1,1]

        self.axes.GetXAxisCaptionActor2D().GetCaptionTextProperty().SetColor(iColor)
        self.axes.GetYAxisCaptionActor2D().GetCaptionTextProperty().SetColor(iColor)
        self.axes.GetZAxisCaptionActor2D().GetCaptionTextProperty().SetColor(iColor)

        if render:
            self.render()

    def clear(self):
        self.prop_actors = {}

    def renderCSX(self, CSX):
        self.clear()
        self.CSX = CSX
        self.updateCSX()
        self.renderGrid()

    def removeProperty(self, prop):
        if prop not in self.prop_actors:
            return
        actor = self.prop_actors[prop]
        self.ren.RemoveActor(actor)
        del self.prop_actors[prop]
        del actor

    def updateCSX(self, force=False):
        default_CS = self.CSX.GetCoordInputType()
        for prop in self.CSX.GetAllProperties():
            if force:
                self.removeProperty(prop)
            if not prop.GetVisibility():
                if prop in self.prop_actors:
                    self.prop_actors[prop].SetVisibility(False)
                continue
            actor = renderPrimitives(self.ren, prop, default_CS=default_CS, arc_resolution=self.arc_resolution)
            self.prop_actors[prop] = actor

    def clearGrid(self):
        for n in range(3):
            if self.grid_plane_actor[n] is not None:
                actor = self.grid_plane_actor[n]
                self.ren.RemoveActor(actor)
                del actor
            if self.grid_plane_filter[n] is not None:
                flt = self.grid_plane_filter[n]
                del flt
        self.grid_plane_actor  = [None]*3
        self.grid_plane_filter = [None]*3
        if self.rect_grid is not None:
            del self.rect_grid
            self.rect_grid = None

    def renderGrid(self):
        self.clearGrid()
        if self.CSX is None:
            return
        grid = self.CSX.GetGrid()
        if not grid.IsValid():
            return
        Ng = [grid.GetQtyLines(n) for n in range(3)]
        if grid.GetMeshType()==CSRectGrid.CARTESIAN:
            from vtkmodules.vtkCommonDataModel import vtkRectilinearGrid
            self.rect_grid = vtkRectilinearGrid()
            self.rect_grid.SetDimensions(Ng)
            self.rect_grid.SetXCoordinates( array2vtkFloat(grid.GetLines(0, do_sort=True)) )
            self.rect_grid.SetYCoordinates( array2vtkFloat(grid.GetLines(1, do_sort=True)) )
            self.rect_grid.SetZCoordinates( array2vtkFloat(grid.GetLines(2, do_sort=True)) )
        elif grid.GetMeshType()==CSRectGrid.CYLINDRICAL:
            from vtkmodules.vtkCommonDataModel import vtkStructuredGrid
            from vtkmodules.vtkCommonCore import vtkPoints
            self.rect_grid = vtkStructuredGrid()
            self.rect_grid.SetDimensions(Ng)
            points = vtkPoints()
            points.SetNumberOfPoints(np.prod(Ng))
            pos = [0,0,0]
            lines = [grid.GetLines(n, do_sort=True) for n in range(3)]
            n = 0
            for z in grid.GetLines(2, do_sort=True):
                pos[2] = z;
                for a in grid.GetLines(1, do_sort=True):
                    cosj = np.cos(a)
                    sinj = np.sin(a)
                    for r in grid.GetLines(0, do_sort=True):
                        pos[0] = r * cosj
                        pos[1] = r * sinj
                        points.SetPoint(n, pos)
                        n += 1
            self.rect_grid.SetPoints(points)

        self.renderGridDir(0, 0)
        self.renderGridDir(1, 0)
        self.renderGridDir(2, 0)

    def renderGridDir(self, ny, plane_pos):
        if self.CSX is None or self.rect_grid is None:
            return

        grid = self.CSX.GetGrid()
        Ng = [grid.GetQtyLines(n) for n in range(3)]

        if plane_pos<0:
            plane_pos = 0
        elif plane_pos>=Ng[ny]:
            plane_pos = Ng[ny]-1

        if self.grid_plane_filter[ny] is None:
            if grid.GetMeshType()==CSRectGrid.CARTESIAN:
                from vtkmodules.vtkFiltersGeometry import vtkRectilinearGridGeometryFilter
                grid_plane_flt = vtkRectilinearGridGeometryFilter()
            elif grid.GetMeshType()==CSRectGrid.CYLINDRICAL:
                from vtkmodules.vtkFiltersGeometry import vtkStructuredGridGeometryFilter
                grid_plane_flt = vtkStructuredGridGeometryFilter()
            grid_plane_flt.SetInputData(self.rect_grid)
            self.grid_plane_filter[ny] = grid_plane_flt
        else:
            grid_plane_flt = self.grid_plane_filter[ny]

        if ny == 2:
            grid_plane_flt.SetExtent(0, Ng[0]-1, 0, Ng[1]-1, plane_pos, plane_pos)
        elif ny == 1:
            grid_plane_flt.SetExtent(0, Ng[0]-1, plane_pos, plane_pos, 0, Ng[2]-1)
        elif ny == 0:
            grid_plane_flt.SetExtent(plane_pos, plane_pos, 0, Ng[1]-1, 0, Ng[2]-1)

        if self.grid_plane_actor[ny] is None:
            self.grid_plane_actor[ny] = vtkActor()
            grid_mapper = vtkPolyDataMapper()
            grid_mapper.SetInputConnection(grid_plane_flt.GetOutputPort())
            self.grid_plane_actor[ny].SetMapper(grid_mapper)
            self.grid_plane_actor[ny].GetProperty().SetColor(0,0,0)
            self.grid_plane_actor[ny].GetProperty().SetDiffuse(0)
            self.grid_plane_actor[ny].GetProperty().SetAmbient(1)
            self.grid_plane_actor[ny].GetProperty().SetRepresentationToWireframe()
            self.grid_plane_actor[ny].GetProperty().SetOpacity(self.grid_opacity)
            self.ren.AddActor(self.grid_plane_actor[ny])

    def setGridOpacity(self, val, render=True):
        self.grid_opacity = float(val)/255
        for n in range(3):
            if self.grid_plane_actor[n]:
                self.grid_plane_actor[n].GetProperty().SetOpacity(self.grid_opacity)
        if render:
            self.render()





def showCSX(CSX):
    app = createApp(sys.argv)

    main = qtViewer()
    main.show()

    main.renderCSX(CSX)
    main.resetView()

    try:
        app.exec()
    except AttributeError:
        app.exec_()

if __name__ == '__main__':
    if len(sys.argv)<2:
        print('Usage: python qtViewer.py <csxcad_export>.xml')
        sys.exit(1)

    fn = sys.argv[-1]

    from CSXCAD.CSXCAD import ContinuousStructure
    CSX = ContinuousStructure()
    CSX.ReadFromXML(fn)
    showCSX(CSX)
