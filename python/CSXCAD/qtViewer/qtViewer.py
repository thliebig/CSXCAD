import os, sys
import numpy as np

if os.name == 'posix':
    # should not hurt on any other system either
    os.environ['QT_QPA_PLATFORM'] = 'xcb'

from CSXCAD.qtViewer.qtSettings import Qt, QApplication, QMainWindow
from CSXCAD.qtViewer.qtSettings import createApp

from CSXCAD.qtViewer.qtSettings import QToolBar, QAction

from CSXCAD.qtViewer.qtSettings import QTreeWidget, QTreeWidgetItem, QDockWidget
from CSXCAD.qtViewer.qtSettings import QIcon, QColor

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

ICON_PATH = os.path.join(os.path.dirname(__file__), 'icons')

def getIcon(icon):
    fn = os.path.join(ICON_PATH, icon + '.png')
    if not os.path.exists(fn):
        return None
    return QIcon(fn)

class TreeRenderItem(QTreeWidgetItem):
    def __init__(self, viewer, **kw):
        self.viewer = viewer
        self.actor = None
        QTreeWidgetItem.__init__(self, **kw)
        self.setFlags(Qt.ItemIsUserCheckable | Qt.ItemIsEnabled)

    def addActor(self):
        if self.actor is not None:
            self.viewer.addActor(self.actor)

    def removeActor(self):
        if self.actor is not None:
            self.viewer.removeActor(self.actor)
        del actor
        self.actor = None

    def setVisible(self, val, render=False):
        if self.actor is not None:
            self.actor.SetVisibility(val)
            if render:
                self.viewer.render()

    # def data(self, col, role):
    #     return QTreeWidgetItem.data(self, col, role)

class PropertyRootItem(TreeRenderItem):
    def __init__(self, viewer, **kw):
        self.prop_items = {}
        TreeRenderItem.__init__(self, viewer, **kw)
        self.setText(0, 'Properties')
        self.setFlags(Qt.ItemIsUserCheckable | Qt.ItemIsAutoTristate | Qt.ItemIsEnabled)

    def clear(self):
        for prop in list(self.prop_items):
            self.removeProperty(prop)

    def removeProperty(self, prop, render=False):
         if prop not in self.prop_items:
             return
         item = self.prop_items[prop]
         item.removeActor()
         del self.prop_items[prop]
         self.removeChild(item)
         if render:
            self.viewer.render()

    def updateCSX(self, force=False):
        default_CS = self.viewer.CSX.GetCoordInputType()
        for prop in self.viewer.CSX.GetAllProperties():
            if force:
                self.removeProperty(prop)
            item = PropertyItem(prop, self.viewer)
            self.prop_items[prop] = item
            self.addChild(item)

    def setData(self, col, role, val):
        do_block_ren = False
        if col==0:
            if role == Qt.CheckStateRole:
                do_block_ren = True
                vis = (val == 2) # 2 == Qt.Checked
                for prop in self.prop_items:
                    prop.SetVisibility(vis)
        if do_block_ren:
            self.viewer.blockRender(True)
        TreeRenderItem.setData(self, col, role, val)
        if do_block_ren:
            self.viewer.blockRender(False)
            self.viewer.render()

class PropertyItem(TreeRenderItem):
    def __init__(self, prop, viewer, **kw):
        self.prop  = prop
        TreeRenderItem.__init__(self, viewer, **kw)
        self.setText(0, '{} ({})'.format(prop.GetName(), prop.GetTypeString()))
        default_CS = viewer.CSX.GetCoordInputType()
        arc_resolution = viewer.arc_resolution
        self.actor = renderPrimitives(prop, default_CS=default_CS, arc_resolution=arc_resolution)
        self.setVisible(prop.GetVisibility())
        self.addActor()

    def data(self, col, role):
        if col==0:
            if role == Qt.DecorationRole:
                return QColor(*self.prop.GetFillColor())
            elif role == Qt.CheckStateRole:
                return Qt.Checked if self.prop.GetVisibility() else Qt.Unchecked
        return TreeRenderItem.data(self, col, role)

    def setData(self, col, role, val):
        if col==0:
            if role == Qt.CheckStateRole:
                vis = (val == 2) # 2 == Qt.Checked
                if self.prop:
                    self.prop.SetVisibility(vis)
                self.setVisible(vis, render=True)
        return TreeRenderItem.setData(self, col, role, val)

class RectGridRootItem(TreeRenderItem):
    def __init__(self, viewer, **kw):
        self.grid_items = [None, None, None]
        self.rect_grid = None
        TreeRenderItem.__init__(self, viewer, **kw)
        self.setText(0, 'Grid')
        self.setFlags(Qt.ItemIsUserCheckable | Qt.ItemIsAutoTristate | Qt.ItemIsEnabled)
        #self.setCheckState(0, Qt.Checked)

    def setData(self, col, role, val):
        do_block_ren = False
        if col==0:
            if role == Qt.CheckStateRole:
                do_block_ren = True
                vis = (val == 2) # 2 == Qt.Checked
                for item in self.grid_items:
                    if item is None:
                        continue
                    item.setVisible(val, render=False)
        if do_block_ren:
            self.viewer.blockRender(True)
        TreeRenderItem.setData(self, col, role, val)
        if do_block_ren:
            self.viewer.blockRender(False)
            self.viewer.render()

    def clear(self):
        for n in range(3):
            if self.grid_items[n] is not None:
                self.grid_items[n].clear()
        self.grid_items = [None, None, None]

        if self.rect_grid is not None:
            del self.rect_grid
            self.rect_grid = None

    def getGrid(self):
        return self.rect_grid

    def renderGrid(self):
        self.clear()
        if self.viewer.CSX is None:
            return
        grid = self.viewer.CSX.GetGrid()
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
                pos[2] = z
                for a in grid.GetLines(1, do_sort=True):
                    cosj = np.cos(a)
                    sinj = np.sin(a)
                    for r in grid.GetLines(0, do_sort=True):
                        pos[0] = r * cosj
                        pos[1] = r * sinj
                        points.SetPoint(n, pos)
                        n += 1
            self.rect_grid.SetPoints(points)

        for n in range(3):
            self.grid_items[n] = GridDirItem(n, self.rect_grid, self.viewer)
            self.addChild(self.grid_items[n])
        self.setGridOpacity(30)


    def setGridOpacity(self, val, render=False):
        for n in range(3):
            if self.grid_items[n] is not None:
                self.grid_items[n].setGridOpacity(val, render=False)
        if render:
            self.viewer.render()

class GridDirItem(TreeRenderItem):
    def __init__(self, ny, rect_grid, viewer, **kw):
        self.ny  = ny
        self.rect_grid = rect_grid
        self.grid_plane_filter = None
        TreeRenderItem.__init__(self, viewer, **kw)
        self.setText(0, '{}-Plane'.format(['yz', 'xz', 'xy'][ny]))
        self.renderGridDir(0)
        self.viewer.blockRender(True)
        self.setCheckState(0, Qt.Checked)
        self.viewer.blockRender(False)

    def setData(self, col, role, val):
        if col==0:
            if role == Qt.CheckStateRole:
                vis = (val == 2) # 2 == Qt.Checked
                self.setVisible(val, render=True)
        return TreeRenderItem.setData(self, col, role, val)

    def clear(self):
        self.rect_grid = None
        self.removeActor()
        if self.grid_plane_filter is None:
            return
        del self.grid_plane_filter

    def renderGridDir(self, plane_pos):
        if self.viewer.CSX is None or self.rect_grid is None:
            return

        ny = self.ny
        grid = self.viewer.CSX.GetGrid()
        Ng = [grid.GetQtyLines(n) for n in range(3)]

        if plane_pos<0:
            plane_pos = 0
        elif plane_pos>=Ng[ny]:
            plane_pos = Ng[ny]-1

        if self.grid_plane_filter is None:
            if grid.GetMeshType()==CSRectGrid.CoordinateSystem.CARTESIAN:
                grid_plane_flt = vtk.vtkRectilinearGridGeometryFilter()
            elif grid.GetMeshType()==CSRectGrid.CoordinateSystem.CYLINDRICAL:
                grid_plane_flt = vtk.vtkStructuredGridGeometryFilter()
            grid_plane_flt.SetInputData(self.rect_grid)
            self.grid_plane_filter = grid_plane_flt
        else:
            grid_plane_flt = self.grid_plane_filter

        if ny == 2:
            grid_plane_flt.SetExtent(0, Ng[0]-1, 0, Ng[1]-1, plane_pos, plane_pos)
        elif ny == 1:
            grid_plane_flt.SetExtent(0, Ng[0]-1, plane_pos, plane_pos, 0, Ng[2]-1)
        elif ny == 0:
            grid_plane_flt.SetExtent(plane_pos, plane_pos, 0, Ng[1]-1, 0, Ng[2]-1)

        if self.actor is None:
            self.actor = vtk.vtkActor()
            grid_mapper = vtk.vtkPolyDataMapper()
            grid_mapper.SetInputConnection(grid_plane_flt.GetOutputPort())
            self.actor.SetMapper(grid_mapper)
            self.actor.GetProperty().SetColor(0,0,0)
            self.actor.GetProperty().SetDiffuse(0)
            self.actor.GetProperty().SetAmbient(1)
            self.actor.GetProperty().SetRepresentationToWireframe()
            self.addActor()

    def setGridOpacity(self, val, render=False):
        self.grid_opacity = float(val)/255
        if self.actor:
            self.actor.GetProperty().SetOpacity(self.grid_opacity)
        if render:
            self.viewer.render()

class Viewer(QMainWindow):
    arc_resolution = 32
    def __init__(self, **kw):
        QMainWindow.__init__(self, **kw)
        self.setWindowTitle('Simple CSXCAD Viewer')
        self.resize(800, 600)

        self.CSX = None
        self.cam_data = None
        self.rect_grid = None
        self.grid_plane_actor = [None]*3
        self.grid_plane_filter = [None]*3

        self.block_render = False

        self.setCentralWidget(self.createCentralWidget())

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

    def createCentralWidget(self):
        self.ren_widget = QVTKRenderWindowInteractor(self)
        return self.ren_widget


    def createNavTree(self):
        self.nav_tree = QTreeWidget()
        self.nav_tree.setStyleSheet(
            "QTreeView::indicator:unchecked { image: url(" + ICON_PATH + "/bulb_off.png); }" + \
            "QTreeView::indicator:checked   { image: url(" + ICON_PATH + "/bulb.png); }"  + \
            "QTreeView::indicator:indeterminate { image: url(" + ICON_PATH + "/bulb.png); }"
            )

        self.prop_root = PropertyRootItem(self)
        self.grid_root = RectGridRootItem(self)

        self.nav_tree.addTopLevelItem(self.prop_root)
        self.nav_tree.addTopLevelItem(self.grid_root)
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
        act = QAction(getIcon('viewmagfit'), 'Zoom', self)
        act.triggered.connect(lambda *arg: self.resetView())
        self.actions[f'zoom'] = act
        self.toolbars['View'].addAction(act)
        for v in ['xy', 'yz', 'xz']:
            icon = getIcon('Arrows' + v.upper())
            act = QAction(icon, v, self)
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

    def blockRender(self, val):
        self.block_render = val

    def render(self, force=False):
        if self.block_render:
            return
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

    def addActor(self, actor):
        self.ren.AddActor(actor)

    def clear(self):
        self.prop_root.clear()
        self.grid_root.clear()

    def renderCSX(self, CSX):
        self.clear()
        self.CSX = CSX
        self.prop_root.updateCSX()
        self.grid_root.renderGrid()

    def setGridOpacity(self, val, render=True):
        self.grid_root.setGridOpacity(val, render=False)
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
