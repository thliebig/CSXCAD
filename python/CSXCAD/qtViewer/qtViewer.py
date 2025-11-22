import os, sys
import numpy as np

if os.name == 'posix':
    # should not hurt on any other system either
    os.environ['QT_QPA_PLATFORM'] = 'xcb'

from CSXCAD.qtViewer.qtSettings import Qt, QApplication, QMainWindow
from CSXCAD.qtViewer.qtSettings import createApp

from CSXCAD.qtViewer.qtSettings import QToolBar, QAction

from CSXCAD.qtViewer.qtSettings import QTreeWidget, QTreeWidgetItem, QDockWidget

# # load implementations for rendering and interaction factory classes
import vtkmodules.vtkRenderingOpenGL2

import vtk

from vtkmodules.qt.QVTKRenderWindowInteractor import PyQtImpl, QVTKRenderWindowInteractor

from CSXCAD import CSProperties, CSRectGrid
from CSXCAD.qtViewer.vtkPrimitives import renderPrimitives

def array2vtkFloat(a):
    # this is an easy way to convert small arrays to vtk
    # for large arrays use:
    # from vtk.util.numpy_support import numpy_to_vtk  #<--- this pulls in way to many dependencies?
    out = vtk.vtkFloatArray()
    for i,x in enumerate(a):
        out.InsertNextValue(x)
    return out

class Viewer(QMainWindow):
    arc_resolution = 32
    def __init__(self, **kw):
        QMainWindow.__init__(self, **kw)
        self.setWindowTitle('Simple CSXCAD Viewer')
        self.resize(800, 600)

        self.cam_data = None
        self.prop_actors = {}
        self.rect_grid = None
        self.grid_plane_actor = [None]*3
        self.grid_plane_filter = [None]*3

        self.ren_widget = QVTKRenderWindowInteractor(self)
        self.setCentralWidget(self.ren_widget)

        self.addActions()
        self.createNavTree()

        app = QApplication.instance()
        self.ren_widget.AddObserver("ExitEvent", lambda o, e, a=app: a.quit())

        self.ren = vtk.vtkRenderer()
        self.ren_win = self.ren_widget.GetRenderWindow()

        self.ren_win.GetInteractor().SetInteractorStyle(vtk.vtkInteractorStyleTrackballCamera())

        self.ren_win.AddRenderer(self.ren)

        self.addAxes()
        self.setGridOpacity(30, render=False)
        self.setBackgroundColor('white', render=False)

    def createNavTree(self):
        self.nav_tree = QTreeWidget()
        self.prop_root = QTreeWidgetItem(['Properties',])
        self.nav_tree.addTopLevelItem(self.prop_root)
        self.nav_tree.setHeaderHidden(True)
        self.nav_dock = QDockWidget('Navigation Tree')
        self.nav_dock.setWidget(self.nav_tree)
        self.nav_dock.setFeatures(QDockWidget.NoDockWidgetFeatures)
        self.addDockWidget(Qt.LeftDockWidgetArea, self.nav_dock)

    def addActions(self):
        self.toolbars = {}
        self.actions = {}
        self.toolbars['View'] = QToolBar("View")
        self.addToolBar(self.toolbars['View'])
        act = QAction('Zoom', self)
        act.triggered.connect(lambda *arg: self.resetView())
        self.actions[f'zoom'] = act
        self.toolbars['View'].addAction(act)
        for v in ['xy', 'yz', 'xz']:
            act = QAction(v, self)
            act.triggered.connect(lambda *args, v=v: self.setView(v))
            self.actions[f'{v}_view'] = act
            self.toolbars['View'].addAction(act)

        act = QAction('2D', self)
        act.setCheckable(True)
        act.toggled.connect(lambda val: self.set3DView(not val))
        self.actions[f'2D'] = act
        self.toolbars['View'].addAction(act)

        act = QAction('PP', self)
        act.setCheckable(True)
        act.toggled.connect(lambda val: self.setParallelProjection(val))
        self.actions[f'PP'] = act
        self.toolbars['View'].addAction(act)

    def start(self):
        self.ren_widget.Initialize()
        self.ren_widget.Start()
        self.resetView()

    def render(self, force=False):
        self.ren_win.GetInteractor().Render()

    def setView(self, view, render=True):
        if view == 'xy':   d = 2
        elif view == 'yz': d = 0
        elif view == 'xz': d = 1
        else: d = view
        cam = self.ren.GetActiveCamera()
        self.ren.ResetCamera()
        cam.SetViewUp(0.5,0.5,0.5)
        fp = list(cam.GetFocalPoint())
        fp[d]+=1
        cam.SetPosition(fp)
        cam.SetRoll(0)
        if render:
            self.resetView()

    def _getCurrView(self):
        cam = self.ren.GetActiveCamera()
        pos = cam.GetPosition()
        fp  = cam.GetFocalPoint()
        v = [pos[n]-fp[n] for n in range(3)]
        return np.argmax(np.abs(v))

    def set3DView(self, val):
        self.actions[f'PP'].setEnabled(val)

        if val: #3D
            self.restoreCamera(render=False)
        else: # 2D
            self.saveCamera()
            self.setView(self._getCurrView(), render=True)
            self.setParallelProjection(True, render=False)
        self.set2DInteractionStyle(not val, render=True)

    def setParallelProjection(self, val, render=True):
        cam = self.ren.GetActiveCamera()
        cam.SetParallelProjection(val)
        cam.Modified()
        if render:
            self.render()

    def set2DInteractionStyle(self, val, render=True):
        if val:
            self.ren_win.GetInteractor().SetInteractorStyle(vtk.vtkInteractorStyleRubberBand2D())
        else:
            self.ren_win.GetInteractor().SetInteractorStyle(vtk.vtkInteractorStyleTrackballCamera())

        if render:
            self.render()

    def saveCamera(self):
        self.cam_data = {}
        cam = self.ren.GetActiveCamera()
        self.cam_data['pos'] = cam.GetPosition()
        self.cam_data['focal_pnt'] = cam.GetFocalPoint()
        self.cam_data['view_up'] = cam.GetViewUp()
        self.cam_data['view_ang'] = cam.GetViewAngle()
        self.cam_data['parallel_proj'] = cam.GetParallelProjection()

    def restoreCamera(self, render=True):
        if self.cam_data is None:
            return
        cam = self.ren.GetActiveCamera()
        cam.SetPosition(self.cam_data['pos'])
        cam.SetFocalPoint(self.cam_data['focal_pnt'])
        cam.SetViewUp(self.cam_data['view_up'])
        cam.SetViewAngle(self.cam_data['view_ang'])
        cam.SetParallelProjection(self.cam_data['parallel_proj'])
        cam.Modified()
        if render:
            self.render()

    def resetView(self, render=True):
        self.ren.ResetCamera()
        if render:
            self.render()

    def addAxes(self):
        self.axes = vtk.vtkAxesActor()
        self.axes.SetTotalLength(4,4,4)
        self.axes_marker = vtk.vtkOrientationMarkerWidget()
        assembly = vtk.vtkPropAssembly()
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
        if grid.GetMeshType()==CSRectGrid.CoordinateSystem.CARTESIAN:
            self.rect_grid = vtk.vtkRectilinearGrid()
            self.rect_grid.SetDimensions(Ng)
            self.rect_grid.SetXCoordinates( array2vtkFloat(grid.GetLines(0, do_sort=True)) )
            self.rect_grid.SetYCoordinates( array2vtkFloat(grid.GetLines(1, do_sort=True)) )
            self.rect_grid.SetZCoordinates( array2vtkFloat(grid.GetLines(2, do_sort=True)) )
        elif grid.GetMeshType()==CSRectGrid.CYLINDRICAL:
            self.rect_grid = vtk.vtkStructuredGrid()
            self.rect_grid.SetDimensions(Ng)
            points = vtk.vtkPoints()
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
            if grid.GetMeshType()==CSRectGrid.CoordinateSystem.CARTESIAN:
                grid_plane_flt = vtk.vtkRectilinearGridGeometryFilter()
            elif grid.GetMeshType()==CSRectGrid.CoordinateSystem.CYLINDRICAL:
                grid_plane_flt = vtk.vtkStructuredGridGeometryFilter()
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
            self.grid_plane_actor[ny] = vtk.vtkActor()
            grid_mapper = vtk.vtkPolyDataMapper()
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

    main = Viewer()
    main.show()

    main.renderCSX(CSX)
    main.resetView()

    main.start()

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
