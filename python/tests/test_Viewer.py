import contextlib
import io
import json
import math
import os
import shutil
import tempfile
import unittest

import numpy as np

from CSXCAD.CSXCAD import ContinuousStructure
from CSXCAD import CSPrimitives

try:
    import vtk  # noqa: F401
    HAS_VTK = True
except ImportError:
    HAS_VTK = False


def _HasGL():
    if not HAS_VTK:
        return False
    from CSXCAD.Viewer.Core import OffscreenBackend
    try:
        OffscreenBackend()
        return True
    except RuntimeError:
        return False


def _Volume(pd):
    from vtkmodules.vtkFiltersCore import vtkMassProperties, vtkTriangleFilter
    tri = vtkTriangleFilter()
    tri.SetInputData(pd)
    mp = vtkMassProperties()
    mp.SetInputConnection(tri.GetOutputPort())
    mp.Update()
    return mp.GetVolume()


def _Structure():
    """ A small structure with a substrate, metal, port and a hidden dump. """
    csx = ContinuousStructure()
    grid = csx.GetGrid()
    grid.SetDeltaUnit(1e-3)
    grid.SetLines('x', np.linspace(-30, 30, 13))
    grid.SetLines('y', np.linspace(-30, 30, 13))
    grid.SetLines('z', [-5, 0, 0.5, 1, 1.5, 4, 10])
    sub = csx.AddMaterial('substrate', epsilon=3.38)
    sub.SetColor('#2e8b57', 120)
    sub.AddBox([-25, -25, 0], [25, 25, 1.5])
    patch = csx.AddMetal('patch')
    patch.SetColor('#ff0000')
    patch.AddBox([-10, -10, 1.5], [10, 10, 1.5])
    patch.AddCylinder([0, 0, 0], [0, 0, 1.5], 1)
    port = csx.AddExcitation('port', 0, [0, 0, 1])
    port.AddBox([-5, 0, 0], [-5, 0, 1.5])
    dump = csx.AddDump('Et')
    dump.AddBox([-30, -30, 0.7], [30, 30, 0.7])
    dump2 = csx.AddDump('Ht')
    dump2.AddBox([-30, -30, 0.7], [30, 30, 0.7])
    return csx


def _Interactive(v, size):
    """ Attach `v` to an offscreen window with an interactor, like a GUI
    does; returns the interactor, to send it events. """
    from vtkmodules.vtkRenderingUI import vtkGenericRenderWindowInteractor
    from CSXCAD.Viewer.Core import NewOffscreenWindow
    rw = NewOffscreenWindow()
    iren = vtkGenericRenderWindowInteractor()
    iren.SetRenderWindow(rw)
    v.Attach(rw, iren, onscreen=True)
    rw.SetSize(*size)
    iren.Initialize()
    return iren


def _Figure():
    """ S11 and S21 curves (and one without a label) next to a map with a
    colour bar, and a suptitle. """
    from matplotlib.figure import Figure
    fig = Figure()
    ax1, ax2 = fig.subplots(1, 2)
    ax1.plot([0, 1], [0, 1], label='S11')
    ax1.plot([0, 1], [1, 0], label='S21')
    ax1.plot([0, 1], [0.5, 0.5])          # no label: not a curve
    ax2.set_title('SAR')
    m = ax2.pcolormesh(np.ones((3, 4)))
    fig.colorbar(m, ax=ax2)                # its axes are no curve
    fig.suptitle('S-parameters')
    return fig


@unittest.skipUnless(HAS_VTK, 'vtk not available')
class TestViewerGeometry(unittest.TestCase):

    def setUp(self):
        self.csx = ContinuousStructure()
        self.m = self.csx.AddMetal('m')

    def _pd(self, prim, **kw):
        from CSXCAD.Viewer.Geometry import PrimitivePolyData
        prim.Update()
        return PrimitivePolyData(prim, arc_resolution=kw.pop('arc_resolution', 64), **kw)

    def test_primitives_bounds_and_volume(self):
        m = self.m
        cases = [
            (m.AddBox([0, 0, 0], [1, 2, 3]), 6),
            (m.AddSphere([1, 1, 1], 2), 4 / 3 * math.pi * 8),
            (m.AddSphericalShell([0, 0, 0], 2, 1), 4 / 3 * math.pi * (2.5 ** 3 - 1.5 ** 3)),
            (m.AddCylinder([0, 0, 0], [1, 1, 1], 0.5), math.pi * 0.25 * math.sqrt(3)),
            (m.AddCylindricalShell([0, 0, 0], [0, 0, 2], 1, 0.4), math.pi * (1.2 ** 2 - 0.8 ** 2) * 2),
            (m.AddLinPoly([[0, 2, 2, 1, 1, 0], [0, 0, 1, 1, 2, 2]], 'z', 0, 3), 9),
            (m.AddMultiBox(boxes=[([0, 0, 0], [1, 1, 1]), ([2, 2, 2], [3, 4, 5])]), 7),
            (m.AddPolygon([[0, 2, 2, 1, 1, 0], [0, 0, 1, 1, 2, 2]], 'x', 1), None),
            (m.AddCurve([[0, 1, 2], [0, 2, 0], [0, 3, 1]]), None),
            (m.AddWire([[0, 1], [0, 2], [0, 3]], 0.1), None),
            (m.AddPoint([1, 2, 3]), None),
        ]
        for prim, volume in cases:
            pd = self._pd(prim)
            name = prim.GetTypeName()
            bb = np.asarray(prim.GetBoundBox()).T.ravel()
            if prim.GetType() != CSPrimitives.PrimitiveType.WIRE:
                np.testing.assert_allclose(pd.GetBounds(), bb, atol=2e-3 * np.ptp(bb), err_msg=name)
            if volume is not None:
                self.assertAlmostEqual(_Volume(pd) / volume, 1, delta=0.01, msg=name)

    def test_closed_solids_point_outward(self):
        # every face of a closed solid faces outward, as an STL export needs it:
        # each edge passed once in each direction, and a positive signed volume
        from vtkmodules.util.numpy_support import vtk_to_numpy
        m = self.m
        cbox = m.AddBox([0.5, 0.2, -1], [2, 2.5, 1])
        cbox.SetCoordinateSystem(1)
        cases = [
            m.AddBox([0, 0, 0], [1, 2, 3]),
            m.AddBox([1, 2, 3], [0, 0, 0]),
            cbox,
            m.AddSphere([1, 1, 1], 2),
            m.AddSphericalShell([0, 0, 0], 2, 1),
            m.AddCylinder([0, 0, 0], [1, 1, 1], 0.5),
            m.AddCylindricalShell([0, 0, 0], [0, 0, 2], 1, 0.4),
            m.AddLinPoly([[0, 2, 2, 1, 1, 0], [0, 0, 1, 1, 2, 2]], 'z', 0, 3),
            m.AddLinPoly([[0, 0, 1, 1, 2, 2], [0, 2, 2, 1, 1, 0]], 'z', 0, 3),      # clockwise
            m.AddLinPoly([[0, 50, 50, 0], [10, 27, -27, -10]], 'y', -1, 2),        # the horn's walls
            m.AddLinPoly([[0, 2, 2, 0], [0, 0, 1, 1]], 'x', 0, -2),                # extruded backwards
            m.AddRotPoly([[1, 2, 2, 1.5], [0.2, 0, 1, 1.4]], 'x', 0, 'z', [0, math.pi / 2]),
            m.AddRotPoly([[1, 1.5, 2, 2], [0.2, 1.4, 1, 0]], 'z', 0, 'y', [0, 2 * math.pi]),
            m.AddMultiBox(boxes=[([0, 0, 0], [1, 1, 1]), ([2, 2, 2], [3, 4, 5])]),
        ]
        for prim in cases:
            with self.subTest(prim.GetTypeName()):
                pd = self._pd(prim)
                pts = vtk_to_numpy(pd.GetPoints().GetData())
                conn = vtk_to_numpy(pd.GetPolys().GetConnectivityArray())
                off = vtk_to_numpy(pd.GetPolys().GetOffsetsArray())
                key = {}
                ids = [key.setdefault(tuple(np.round(p, 9)), len(key)) for p in pts]
                passes, vol = {}, 0
                for n in range(len(off) - 1):
                    f = conn[off[n]:off[n + 1]]
                    for i in range(1, len(f) - 1):
                        vol += np.dot(pts[f[0]], np.cross(pts[f[i]], pts[f[i + 1]])) / 6
                    f = [ids[i] for i in f]
                    for a, b in zip(f, f[1:] + f[:1]):
                        if a != b:
                            e = (min(a, b), max(a, b))
                            passes[e] = passes.get(e, 0) + (1 if a < b else -1)
                self.assertEqual(sum(v != 0 for v in passes.values()), 0)
                self.assertGreater(vol, 0)

    def test_transform_and_cylindrical_box(self):
        box = self.m.AddBox([0, 0, 0], [1, 1, 1])
        box.AddTransform('Translate', [5, 0, 0])
        box.AddTransform('Scale', 2)
        pd = self._pd(box)
        np.testing.assert_allclose(pd.GetBounds(), (10, 12, 0, 2, 0, 2))
        self.assertAlmostEqual(_Volume(pd), 8)

        cbox = self.m.AddBox([0.5, 0.2, -1], [2, 2.5, 1])
        cbox.SetCoordinateSystem(1)
        pd = self._pd(cbox, arc_step_deg=1)
        self.assertAlmostEqual(_Volume(pd) / (0.5 * (4 - 0.25) * 2.3 * 2), 1, delta=1e-3)

    def test_rotpoly_matches_isinside(self):
        from vtkmodules.vtkFiltersModeling import vtkSelectEnclosedPoints
        from CSXCAD.Viewer.Geometry import PolyDataFromArrays
        rng = np.random.RandomState(1)
        for nd, ax, ang in (('x', 'z', [0, math.pi / 2]), ('z', 'y', [0, 2 * math.pi]), ('z', 'x', [0.5, 2.5])):
            rp = self.m.AddRotPoly([[1, 2, 2, 1.5], [0.2, 0, 1, 1.4]], nd, 0, ax, ang)
            pd = self._pd(rp, arc_resolution=128)
            b = np.array(pd.GetBounds()).reshape(3, 2)
            P = b[:, 0] + (b[:, 1] - b[:, 0]) * rng.random_sample((2000, 3))
            sel = vtkSelectEnclosedPoints()
            sel.SetInputData(PolyDataFromArrays(P))
            sel.SetSurfaceData(pd)
            sel.Update()
            vtk_in = np.array([sel.IsInside(i) for i in range(len(P))], bool)
            csx_in = np.array([rp.IsInside(p) for p in P], bool)
            self.assertLess(np.mean(vtk_in != csx_in), 0.005, msg=(nd, ax, ang))
            self.assertGreater(csx_in.mean(), 0.1)

    def test_primitive_id_and_report(self):
        from vtkmodules.util.numpy_support import vtk_to_numpy
        from CSXCAD.Viewer.Geometry import PropertyPolyData
        prims = [self.m.AddBox([0, 0, 0], [1, 1, 1]), self.m.AddSphere([5, 0, 0], 1),
                 self.m.AddCylinder([0, 0, 0], [0, 0, 0], 1)]
        pd, report = PropertyPolyData(self.m)
        ids = vtk_to_numpy(pd.GetCellData().GetArray('primitive_id'))
        self.assertEqual(set(ids), {prims[0].GetID(), prims[1].GetID()})
        # the cells of the sphere are around its centre
        from CSXCAD.Viewer.Scene import ExtractCells
        sphere = ExtractCells(pd, np.nonzero(ids == prims[1].GetID())[0])
        np.testing.assert_allclose(sphere.GetBounds(), (4, 6, -1, 1, -1, 1), atol=1e-6)
        self.assertEqual(report, [dict(prop='m', primitive_id=prims[2].GetID(), type='Cylinder',
                                       reason='zero length')])


@unittest.skipUnless(HAS_VTK, 'vtk not available')
class TestViewerState(unittest.TestCase):

    def test_state_roundtrip(self):
        from CSXCAD.Viewer import Viewer
        csx = _Structure()
        v = Viewer(csx, size=(640, 480), render_mode='solid+edges', transparency=0.5,
                   grid=dict(planes={'z': dict(position=1.4)}, outline=True), clip=dict(normal='x', position=1),
                   highlight=['patch'], visible={'results/fields': False})
        v.SetView('+y')
        st = v.GetState()
        self.assertEqual(st['grid']['planes']['z']['index'], 4)
        v2 = Viewer(csx, **json.loads(json.dumps(st)))
        self.assertEqual(v2.GetState(), st)
        # visibility of items that are added later
        item = v2.AddItem('arrow', None, group='results/fields')
        self.assertFalse(item.GetVisible())
        with self.assertRaises(ValueError):
            Viewer(csx, colour='red')
        with self.assertRaises(ValueError):
            v.SetState(grid=dict(plane={}))

    def test_camera_steps(self):
        # view, azimuth, elevation and focus as view state keys: applied in
        # that order, kept as the camera
        from CSXCAD.Viewer import Viewer

        def direction(v):
            c = v.GetCamera()
            d = np.subtract(c['position'], c['focal_point'])
            return d / np.linalg.norm(d)
        # iso looks from (1, 1, 1): 45 degree around, 35.26 degree up; the
        # turn is about the vertical (z up), the tilt raises the camera
        d = direction(Viewer(_Structure(), view='iso', azimuth=30))
        self.assertAlmostEqual(abs(np.degrees(np.arctan2(d[1], d[0])) - 45), 30, places=6)
        self.assertAlmostEqual(np.degrees(np.arcsin(d[2])), np.degrees(np.arcsin(1 / np.sqrt(3))), places=6)
        v = Viewer(_Structure(), view='iso', azimuth=30, elevation=20)
        self.assertAlmostEqual(np.degrees(np.arcsin(direction(v)[2])), 35.264 + 20, places=2)
        st = v.GetState()
        self.assertEqual((st['view'], st['azimuth'], st['elevation'], st['focus']), (None, None, None, None))
        self.assertEqual(Viewer(_Structure(), **st).GetCamera(), v.GetCamera())
        # focus: on the patch's via (its primitive 1, a JSON list), seen from +z
        whole = Viewer(_Structure(), view='+z').GetCamera()
        cam = Viewer(_Structure(), view='+z', focus=['patch', 1]).GetCamera()
        np.testing.assert_allclose(cam['focal_point'][:2], (0, 0), atol=1e-6)
        self.assertLess(cam['position'][2], whole['position'][2])
        # a framed view needs its picture fitted: in view of all, not cut by the window
        v = Viewer(_Structure(), view='iso', elevation=40)
        self.assertTrue(v._auto_fit)
        # a small target keeps a tenth of the structure (50) around it
        v = Viewer(_Structure(), view='+z', focus='port')
        c = v.GetCamera()
        half = np.linalg.norm(np.subtract(c['position'], c['focal_point'])) * np.tan(np.radians(c['view_angle']) / 2)
        self.assertGreater(half, 2.5)
        with self.assertRaisesRegex(KeyError, 'patch" has no primitive 2; it has 2 \\(index 0..1\\)'):
            Viewer(_Structure(), focus=['patch', 2])

    def test_fit_with_clip(self):
        # the fit frames what the section plane leaves: x <= 0
        from CSXCAD.Viewer import Viewer
        v = Viewer(_Structure(), view='+z', clip=dict(normal='x', position=0))
        self.assertAlmostEqual(v.GetCamera()['focal_point'][0], -12.5, delta=0.1)
        v.SetClip(None)
        v.Fit()
        self.assertAlmostEqual(v.GetCamera()['focal_point'][0], 0, delta=0.1)

    def test_highlight_checks(self):
        # a primitive by its number in the property, as a JSON list too, the
        # same in every session (primitive IDs are not); a wrong one says
        # which exist
        from CSXCAD.Viewer import Viewer
        v = Viewer(_Structure(), highlight=['patch', 1])
        self.assertEqual(v.GetHighlight(), [dict(prop='patch', primitive=1)])
        np.testing.assert_allclose(v._scene.highlight_actor.GetBounds()[:4], (-1, 1, -1, 1), atol=0.05)   # the via
        st = v.GetState()
        _Structure()   # more primitives in this process: other IDs
        self.assertEqual(Viewer(_Structure(), **st).GetHighlight(), [dict(prop='patch', primitive=1)])
        with self.assertRaisesRegex(KeyError, 'patch" has no primitive 2; it has 2 \\(index 0..1\\)'):
            v.Highlight(['patch', 2])
        with self.assertRaisesRegex(KeyError, 'the properties: substrate, patch, port, Et, Ht'):
            v.Highlight('nothing')

    def test_write_through_and_groups(self):
        from CSXCAD.Viewer import Viewer
        csx = _Structure()
        v = Viewer(csx)
        v.SetPropertyColor('patch', fill='#00ff00', opacity=0.5)
        self.assertEqual(csx.GetPropertiesByName('patch')[0].GetFillColor(), (0, 255, 0, 128))
        v.SetPropertyRenderMode('substrate', 'edges')
        self.assertEqual(csx.GetPropertiesByName('substrate')[0].GetRenderMode(), 'edges')
        # edges alone carry the fill colour, next to faces the EdgeColor
        v.SetPropertyColor('substrate', edge='black')
        edges = lambda name: tuple(v._scene.Entry(name).edge_actor.GetProperty().GetColor())
        self.assertEqual(edges('substrate'), tuple(c / 255 for c in (0x2e, 0x8b, 0x57)))
        v.SetPropertyRenderMode('substrate', 'solid+edges')
        self.assertEqual(edges('substrate'), (0, 0, 0))
        v.SetPropertyRenderMode('substrate', 'edges')
        v.SetVisible('properties/dumps', True)
        self.assertTrue(all(p.GetVisibility() for p in csx.GetPropertiesByName('Et') + csx.GetPropertiesByName('Ht')))
        self.assertTrue(v.GetVisible('properties'))
        v.SetVisible('properties/dumps', False)
        self.assertFalse(csx.GetPropertiesByName('Et')[0].GetVisibility())
        self.assertIsNone(v.GetVisible('properties'))
        snippet = v.GetStateAsPython()
        self.assertIn("SetRenderMode('edges')", snippet)
        self.assertIn("SetFillColor('#00ff00', 128)", snippet)
        self.assertNotIn("('patch')[0].SetEdgeColor", snippet)

    def test_all_actions(self):
        # every toolbar button and key binding runs, and toggles toggle
        from CSXCAD.Viewer import Viewer, Actions
        v = Viewer(_Structure())
        v.AddItem('anim', None).AddAnimation(3, lambda step: None)
        cwd = os.getcwd()
        tmp = tempfile.mkdtemp()
        try:
            os.chdir(tmp)
            with contextlib.redirect_stdout(io.StringIO()):
                for a in Actions.ACTIONS:
                    if (a.name == 'screenshot' and not _HasGL()) or (a.enabled and not a.enabled(v)):
                        continue
                    before = a.get(v) if a.checkable else None
                    a.Trigger(v)
                    # (without an event loop, play stops after one frame)
                    if a.checkable and not a.radio and a.name != 'play':
                        self.assertNotEqual(a.get(v), before, a.name)
                    if a.radio:
                        self.assertTrue(a.get(v), a.name)
        finally:
            os.chdir(cwd)
            shutil.rmtree(tmp)

    def test_added_actions(self):
        # an application's actions get key bindings and may replace built-in ones
        from CSXCAD.Viewer import Viewer, Actions
        v = Viewer(_Structure())
        calls = []
        v.AddAction(Actions.Action('run', 'Run simulation', lambda v: calls.append('run'), shortcut='K',
                                   group='Simulation'), toolbar='Simulation')
        v.AddAction(Actions.Action('fit', 'Own fit', lambda v: calls.append('fit'), shortcut='R'))
        self.assertTrue(Actions.HandleKey(v, 'k'))
        self.assertTrue(Actions.HandleKey(v, 'r'))
        self.assertEqual(calls, ['run', 'fit'])
        self.assertEqual(len(v.GetActions()), len(Actions.ACTIONS) + 1)
        self.assertEqual(v.GetActionToolbar('run'), 'Simulation')
        self.assertIn('Run simulation', Actions.HelpText(v))

    def test_property_visible(self):
        # by name, written to the property, like ticking its tree row
        from CSXCAD.Viewer import Viewer
        csx = _Structure()
        v = Viewer(csx, isolate='patch')
        v.SetPropertyVisible('Et', True)
        self.assertTrue(csx.GetPropertiesByName('Et')[0].GetVisibility())
        self.assertEqual(v.GetIsolate(), ['patch', 'Et'])
        v.SetPropertyVisible('patch', False)
        self.assertFalse(csx.GetPropertiesByName('patch')[0].GetVisibility())
        self.assertFalse(v.GetVisible('properties/materials/patch'))
        with self.assertRaises(KeyError):
            v.SetPropertyVisible('nothing', True)

    def test_remove_group(self):
        from CSXCAD.Viewer import Viewer, ViewItem
        v = Viewer(_Structure())
        removed = []

        class Item(ViewItem):
            def OnRemoved(self):
                removed.append(self.GetName())

        a = v.AddItem(Item('a'), group='results/fields')
        b = v.AddItem(Item('b'), group='results/fields/deep')
        c = v.AddItem(Item('c'), group='results/other')
        v.RemoveGroup('results/fields')
        self.assertEqual(sorted(removed), ['a', 'b'])
        self.assertEqual((a.GetViewer(), b.GetViewer()), (None, None))
        self.assertEqual(list(v.GetItems()), ['results/other/c'])
        self.assertFalse(any(p.startswith('results/fields') for p in v._scene.tree.nodes))
        self.assertIs(c.GetViewer(), v)
        for path in ('properties', 'properties/materials', 'items', 'results/other/c', 'grid'):
            with self.assertRaises(ValueError, msg=path):
                v.RemoveGroup(path)

    def test_show_while_isolated(self):
        # ticking a property in the tree while others are isolated shows it
        from CSXCAD.Viewer import Viewer
        v = Viewer(_Structure(), isolate='patch')
        sub = v._scene.Entry('substrate').path
        self.assertFalse(v.GetVisible(sub))
        v.SetVisible(sub, True)
        self.assertTrue(v.GetVisible(sub))
        self.assertEqual(v.GetIsolate(), ['patch', 'substrate'])
        v.SetVisible(sub, False)
        self.assertFalse(v.GetVisible(sub))

    def test_relative_size(self):
        from CSXCAD.Viewer import Viewer
        v = Viewer(_Structure())
        self.assertEqual(v.GetSize(), (800, 600))   # the default, on the reference screen
        v.SetSize((0.5, 2.0))
        self.assertEqual(v.GetState()['size'], [0.5, 2.0])
        from CSXCAD.Viewer.Core import ViewerHost

        class Host4K(ViewerHost):   # as a GUI on a 4K screen
            def ScreenSize(self):
                return (3840, 2160)
        v.SetHost(Host4K())
        v.SetSize((0.5, 2.0))
        self.assertEqual(v.GetSize(), (1970, 985))
        self.assertEqual(v.GetState()['size'], [0.5, 2.0])
        v.SetSize((640, 480))   # pixels stay pixels
        self.assertEqual(v.GetSize(), (640, 480))

    def test_item_settings(self):
        # item settings travel in the view state, also to items added later
        from CSXCAD.Viewer import Viewer, ViewItem
        from vtkmodules.vtkRenderingCore import vtkActor

        class Item(ViewItem):
            def __init__(self, name):
                super().__init__(name, vtkActor())
                self.planes, self.scale = [[0, 1.0]], 1.0

            def GetParameters(self):
                return [dict(name='planes', type='str', value=self.planes, form=False),
                        dict(name='pos', type='slider', value=self.planes[0][1], values=[0, 1, 2], state=False),
                        dict(name='scale', type='float', value=self.scale, default=1.0)]

            def SetParameter(self, name, value):
                if name == 'pos':
                    self.planes = [[0, value]]
                elif name in ('planes', 'scale'):
                    setattr(self, name, value)
                else:
                    super().SetParameter(name, value)

        v = Viewer(_Structure(), items={'results/a': dict(scale=2.0, opacity=0.5)})
        a = v.AddItem(Item('a'), group='results')
        self.assertEqual((a.scale, a.GetOpacity()), (2.0, 0.5))
        self.assertEqual(a.GetProps()[0].GetProperty().GetOpacity(), 0.5)
        v.SetItemParameter('results/a', 'pos', 1.7)   # snapped
        self.assertEqual(v.GetState()['items'], {'results/a': dict(planes=[[0, 2.0]], scale=2.0, opacity=0.5)})
        a.scale = 1.0
        self.assertIn("items={'results/a': {'planes': ((0, 2.0),), 'opacity': 0.5}}", v.GetStateAsPython())
        with self.assertWarns(UserWarning):
            v.SetState(items={'results/a': dict(colour=1)})
        # the summary lists the settings the view state takes (not the form-only pos)
        settings = v.Describe()['items']['results/a']['settings']
        self.assertEqual(list(settings), ['planes', 'scale', 'opacity'])
        self.assertEqual(settings['scale'], dict(value=1.0, type='float', default=1.0))
        self.assertEqual(v.AddItem(Item('a'), group='results').GetPath(), 'results/a (2)')

    def test_structure_bounds(self):
        # the shown properties only: neither hidden dumps nor items count
        from CSXCAD.Viewer import Viewer, ViewItem
        from vtkmodules.vtkFiltersSources import vtkSphereSource
        from vtkmodules.vtkRenderingCore import vtkActor, vtkPolyDataMapper
        v = Viewer(_Structure())
        src = vtkSphereSource()
        src.SetRadius(100)
        m = vtkPolyDataMapper()
        m.SetInputConnection(src.GetOutputPort())
        a = vtkActor()
        a.SetMapper(m)
        v.AddItem(ViewItem('ball', a, include_in_bounds=True))
        self.assertEqual(v.GetStructureBounds(), (-25, 25, -25, 25, 0, 1.5))   # the substrate
        v.Isolate('patch')
        self.assertEqual(v.GetStructureBounds(), (-10, 10, -10, 10, 0, 1.5))   # patch and its via
        v.Isolate(None)
        v.SetVisible('properties', False)   # none shown: all of them, the dumps too
        self.assertEqual(v.GetStructureBounds(), (-30, 30, -30, 30, 0, 1.5))

    def test_item_parts(self):
        # parts of an item are tree rows of their own, shown and hidden on
        # their own, also through the view state of a later viewer
        from CSXCAD.Viewer import Viewer, ViewItem

        class Cuts(ViewItem):
            def __init__(self, name, n):
                super().__init__(name)
                self.n, self.hidden = n, set()

            def GetParts(self):
                return [('cut {}'.format(i), 'cut {}: z = {}'.format(i, 2 * i)) for i in range(self.n)]

            def SetPartVisible(self, part, val):
                (self.hidden.discard if val else self.hidden.add)(part)

            def GetPartVisible(self, part):
                return part not in self.hidden

            def GetParameters(self):
                return [dict(name='n', type='int', value=self.n)]

            def SetParameter(self, name, value):
                self.n = int(value)

        v = Viewer(_Structure())
        cuts = v.AddItem(Cuts('cuts', 2), group='results')
        rows = [(r['kind'], r['path'], r['label']) for r in v.GetTree('results/cuts')['children']]
        self.assertEqual(rows, [('part', 'results/cuts/cut 0', 'cut 0: z = 0'),
                                ('part', 'results/cuts/cut 1', 'cut 1: z = 2')])
        v.SetVisible('results/cuts/cut 1', False)
        self.assertEqual(cuts.hidden, {'cut 1'})
        self.assertEqual((v.GetVisible('results/cuts/cut 1'), v.GetVisible('results/cuts')), (False, True))
        v.SetVisible('results', False)   # the item, not its parts
        v.SetVisible('results', True)
        self.assertEqual(cuts.hidden, {'cut 1'})
        visible = v.GetState()['visible']
        self.assertEqual((visible['results/cuts/cut 0'], visible['results/cuts/cut 1']), (True, False))
        v.SetItemParameter(cuts, 'n', 1)   # the rows follow the parts
        self.assertEqual([r['path'] for r in v.GetTree('results/cuts')['children']], ['results/cuts/cut 0'])
        cuts.n = 3
        cuts.PartsChanged()                # changed other than through a setting
        self.assertEqual(len(v.GetTree('results/cuts')['children']), 3)
        v.RemoveItem(cuts)
        self.assertFalse(any(p.startswith('results/cuts') for p in v._scene.tree.nodes))

        v2 = Viewer(_Structure(), visible=visible)   # before the item exists
        cuts2 = v2.AddItem(Cuts('cuts', 2), group='results')
        self.assertEqual(cuts2.hidden, {'cut 1'})
        self.assertEqual(v2.GetState()['visible'], visible)

    def test_file_name(self):
        # a structure read from a file knows it (GUI: Reload, Save XML)
        from CSXCAD.Viewer import Viewer
        with tempfile.TemporaryDirectory() as tmp:
            fn = os.path.join(tmp, 'struct.xml')
            _Structure().Write2XML(fn)
            v = Viewer(fn)
            self.assertEqual(v.GetFileName(), fn)
            v.SetCSX(_Structure())
            self.assertIsNone(v.GetFileName())
            v.SetFileName(fn)
            self.assertEqual(v.GetFileName(), fn)

    def test_describe_failing_item(self):
        # a failing item hook does not break the summary (nor the output mode)
        from CSXCAD.Viewer import Viewer, ViewItem

        class Broken(ViewItem):
            def GetInfo(self):
                raise RuntimeError('no data')

        v = Viewer(_Structure())
        v.AddItem(Broken('broken'))
        with self.assertWarns(UserWarning):
            d = v.Describe()
        self.assertEqual(d['items']['items/broken']['info'], None)

    def test_animation_clock(self):
        from CSXCAD.Viewer import Viewer, ViewItem

        class Item(ViewItem):
            def __init__(self, name):
                super().__init__(name)
                self.log = []

            def OnPrepareAnimationStep(self, step):
                self.log.append(('prepare', step))

            def OnAnimationStep(self, step):
                self.log.append(('step', step))

        v = Viewer(_Structure())
        a3 = v.AddItem(Item('a3'))
        a6 = v.AddItem(Item('a6'), group='results')
        v.AddAnimation(a3, 3)
        a6.AddAnimation(6)
        v.Play()   # without an event loop: prepares, then shows tick 0
        self.assertEqual(a3.log, [('prepare', 0), ('prepare', 1), ('prepare', 2), ('step', 0)])
        self.assertEqual(a6.log[-1], ('step', 0))
        for _ in range(7):
            v.StepAnimation(1)
        steps = lambda it: [s for kind, s in it.log if kind == 'step']
        self.assertEqual(steps(a3), [0, 1, 2, 0])
        self.assertEqual(steps(a6), [0, 1, 2, 3, 4, 5, 0, 1])
        v2 = Viewer(_Structure(), animation=v.GetAnimation())
        b6 = v2.AddItem(Item('a6'))
        b6.AddAnimation(6)
        self.assertEqual(v2.GetAnimation()['step'], 1)


@unittest.skipUnless(_HasGL(), 'no offscreen OpenGL')
class TestViewerOutput(unittest.TestCase):

    def setUp(self):
        self.dir = tempfile.mkdtemp()

    def tearDown(self):
        shutil.rmtree(self.dir)
        os.environ.pop('CSXCAD_VIEWER_OUTPUT', None)

    def test_screenshot(self):
        from CSXCAD.Viewer import Viewer
        v = Viewer(_Structure(), offscreen=True, size=(200, 150))
        img = v.Screenshot()
        self.assertEqual(img.shape, (150, 200, 3))
        self.assertGreater(len(np.unique(img.reshape(-1, 3), axis=0)), 10)
        v.SetMode2D(True)
        img = v.Screenshot(size=(100, 80), scale=2, transparent_background=True)
        self.assertEqual(img.shape, (160, 200, 4))
        self.assertEqual(img[0, 0, 3], 0)

    def test_render_false(self):
        # render=False renders nothing, a Batch() renders once at its end
        from CSXCAD.Viewer import Viewer
        v = Viewer(_Structure(), size=(200, 150))
        _Interactive(v, (200, 150))
        renders = []
        v.GetRenderer().AddObserver('StartEvent', lambda o, e: renders.append(1))
        v.SetVisible('properties/dumps', True, render=False)
        v.SetVisible('grid', False, render=False)
        v.SetState(render_mode='edges', grid=dict(outline=True), render=False)
        v.SetRenderMode('solid', render=False)
        self.assertEqual(renders, [])
        with v.Batch():
            v.SetVisible('grid', True)
            v.SetState(render_mode='edges')
        self.assertEqual(len(renders), 1)
        with v.Batch():
            with v.Batch(render=False):   # an inner one leaves it to the outer one
                v.SetVisible('grid', False)
            self.assertEqual(len(renders), 1)
        self.assertEqual(len(renders), 2)

    def test_drag_legend(self):
        from vtkmodules.vtkRenderingAnnotation import vtkScalarBarActor
        from vtkmodules.vtkCommonCore import vtkLookupTable
        from CSXCAD.Viewer import Viewer, ViewItem
        v = Viewer(_Structure(), size=(400, 300))
        iren = _Interactive(v, (400, 300))
        item = v.AddItem(ViewItem('field'), group='results')
        bar = vtkScalarBarActor()
        bar.SetLookupTable(vtkLookupTable())
        v.AddLegend(item, bar)
        v.Render()

        def event(name, x, y, repeat=0):
            iren.SetEventInformation(x, y, 0, 0, '\0', repeat)
            iren.InvokeEvent(name)
        x0, y0 = bar.GetPosition()
        cx = int(400 * (x0 + bar.GetWidth() / 2))
        cy = int(300 * (y0 + bar.GetHeight() / 2))
        cam = v.GetCamera()
        event('LeftButtonPressEvent', cx, cy)
        event('MouseMoveEvent', cx - 100, cy - 60)
        event('LeftButtonReleaseEvent', cx - 100, cy - 60)
        self.assertEqual(v.GetCamera(), cam)   # dragged, not rotated
        np.testing.assert_allclose(bar.GetPosition(), (x0 - 0.25, y0 - 0.2), atol=1e-9)
        st = v.GetState()['legends']
        np.testing.assert_allclose(st['results/field'], (x0 - 0.25, y0 - 0.2), atol=1e-9)
        v2 = Viewer(_Structure(), offscreen=True, size=(400, 300), legends=st)
        v2.AddLegend(v2.AddItem(ViewItem('field'), group='results'), bar)
        v2.Capture()
        np.testing.assert_allclose(bar.GetPosition(), (x0 - 0.25, y0 - 0.2), atol=1e-9)
        v.Render()
        # a double click puts it back
        event('LeftButtonPressEvent', cx - 100, cy - 60, repeat=1)
        event('LeftButtonReleaseEvent', cx - 100, cy - 60)
        self.assertEqual(v.GetLegendPositions(), {})
        self.assertAlmostEqual(bar.GetPosition()[0], x0)
        # elsewhere the left button still rotates
        event('LeftButtonPressEvent', 100, 150)
        event('MouseMoveEvent', 160, 150)
        event('LeftButtonReleaseEvent', 160, 150)
        self.assertNotEqual(v.GetCamera()['position'], cam['position'])

    def test_export_animation(self):
        # every available format writes one frame per tick
        from CSXCAD.Viewer import Viewer
        from CSXCAD.Viewer.Animation import ExportFormats
        v = Viewer(_Structure(), offscreen=True, size=(160, 120))
        v.AddItem('anim', None).AddAnimation(3, lambda step: v.SetBackground(('white', 'gray', 'black')[step]))
        v.ExportAnimation(os.path.join(self.dir, 'frames'), 'png')
        self.assertEqual(len(os.listdir(os.path.join(self.dir, 'frames'))), 3)
        for fmt in set(ExportFormats()) - {'png'}:
            fn = os.path.join(self.dir, 'anim.' + fmt)
            v.ExportAnimation(fn, fmt, loops=2)
            self.assertGreater(os.path.getsize(fn), 0, fmt)
        if 'gif' in ExportFormats():
            from PIL import Image
            with Image.open(os.path.join(self.dir, 'anim.gif')) as im:
                self.assertEqual(im.n_frames, 6)

    def test_export_item(self):
        # exporting items renders them elsewhere; the view stays intact
        from vtkmodules.vtkFiltersSources import vtkSphereSource
        from vtkmodules.vtkRenderingCore import vtkActor, vtkPolyDataMapper
        from CSXCAD.Viewer import Viewer, ViewItem
        v = Viewer(_Structure(), offscreen=True, size=(200, 150), view='+z')
        src = vtkSphereSource()
        src.SetRadius(5)
        m = vtkPolyDataMapper()
        m.SetInputConnection(src.GetOutputPort())
        a = vtkActor()
        a.SetMapper(m)
        v.AddItem(ViewItem('ball', a))
        before = v.Capture()
        fn = os.path.join(self.dir, 'scene.glb')
        v.Export('glb', fn)
        np.testing.assert_array_equal(v.Capture(), before)
        with open(fn, 'rb') as f:
            self.assertIn(b'"ball"', f.read())

    def _EdgeFile(self, name, unit=None):
        """ An x edge and a y edge as openEMS's PEC dump writes them (metres,
        cell array direction), optionally with a LengthUnit. """
        from vtkmodules.vtkCommonCore import vtkPoints, vtkUnsignedCharArray, vtkDoubleArray
        from vtkmodules.vtkCommonDataModel import vtkCellArray, vtkPolyData
        from vtkmodules.vtkIOXML import vtkXMLPolyDataWriter
        s = 1 if unit is None else 1 / unit
        pts = vtkPoints()
        for p in ((-0.02, 0, 0.0015), (0.02, 0, 0.0015), (0, -0.02, 0.0015), (0, 0.02, 0.0015)):
            pts.InsertNextPoint(*(c * s for c in p))
        lines = vtkCellArray()
        direction = vtkUnsignedCharArray()
        direction.SetName('direction')
        for i in (0, 2):
            lines.InsertNextCell(2)
            lines.InsertCellPoint(i)
            lines.InsertCellPoint(i + 1)
            direction.InsertNextValue(i // 2)
        pd = vtkPolyData()
        pd.SetPoints(pts)
        pd.SetLines(lines)
        pd.GetCellData().AddArray(direction)
        if unit is not None:
            u = vtkDoubleArray()
            u.SetName('LengthUnit')
            u.InsertNextValue(unit)
            pd.GetFieldData().AddArray(u)
        fn = os.path.join(self.dir, name)
        w = vtkXMLPolyDataWriter()
        w.SetFileName(fn)
        w.SetInputData(pd)
        w.Write()
        return fn

    def test_add_file(self):
        # a VTK file in drawing units: metres by default, else its LengthUnit
        from CSXCAD.Viewer import Viewer
        v = Viewer(_Structure(), offscreen=True, size=(200, 200), view='+z')
        item = v.AddFile(self._EdgeFile('edges.vtp'), line_width=6)
        self.assertEqual(item.GetPath(), 'files/edges.vtp')
        np.testing.assert_allclose(item.GetBounds(), (-20, 20, -20, 20, 1.5, 1.5), atol=1e-6)
        mm = v.AddFile(self._EdgeFile('mm.vtp', unit=1e-3))
        np.testing.assert_allclose(mm.GetBounds(), item.GetBounds(), atol=1e-6)
        raw = v.AddFile(self._EdgeFile('raw.vtp'), unit='drawing')
        np.testing.assert_allclose(raw.GetBounds(), (-0.02, 0.02, -0.02, 0.02, 0.0015, 0.0015), atol=1e-9)
        v.RemoveItem(mm)
        v.RemoveItem(raw)
        # visible on the sheet they lie on, coloured by direction like the axes
        v.Isolate('type:metal')
        self.assertEqual(v.GetIsolate(), ['patch'])
        v.SetItemParameter('files/edges.vtp', 'color', 'blue')
        v.SetVisible('files/edges.vtp', False)
        img = v.Capture().astype(int)
        rows, cols = np.nonzero((img[..., 0] > 150) & (img[..., 1] < 80) & (img[..., 2] < 80))
        v.SetVisible('files/edges.vtp', True)
        img = v.Capture().astype(int)
        blue = (img[..., 2] > 150) & (img[..., 0] < 80) & (img[..., 1] < 80)
        on_patch = blue[rows.min() + 5:rows.max() - 5, cols.min() + 5:cols.max() - 5]
        self.assertGreater(on_patch.sum(), 100)
        v.SetItemParameter('files/edges.vtp', 'color_by', 'direction')
        img = v.Capture().astype(int)
        red = (img[..., 0] > 150) & (img[..., 1] < 80) & (img[..., 2] < 80)
        green = (img[..., 1] > 150) & (img[..., 0] < 80) & (img[..., 2] < 80)
        rows, cols = np.nonzero(green)
        self.assertGreater(green.sum(), 100)        # the y edge: a vertical line
        self.assertLess(np.ptp(cols), 15)
        self.assertGreater(np.ptp(rows), 60)
        self.assertGreater(red.sum(), 100)          # the x edge
        # cut by the section plane like the structure (results are not)
        v.SetClip(normal='x', position=-5)
        img = v.Capture().astype(int)
        red_cut = (img[..., 0] > 150) & (img[..., 1] < 80) & (img[..., 2] < 80)
        self.assertGreater(red_cut.sum(), 20)
        self.assertLess(np.nonzero(red_cut)[1].max(), np.nonzero(red)[1].min() + 0.4 * np.ptp(np.nonzero(red)[1]))
        green_cut = (img[..., 1] > 150) & (img[..., 0] < 80) & (img[..., 2] < 80)
        self.assertEqual(green_cut.sum(), 0)        # the y edge at x=0 is cut away
        v.SetClip(None)
        self.assertEqual(item.GetValueText(None, None, 1), 'direction y')
        with self.assertRaisesRegex(ValueError, 'one of solid, direction'):
            item.SetParameter('color_by', 'speed')
        d = v.Describe()['items']['files/edges.vtp']
        self.assertEqual(d['settings']['color_by']['choices'], ['solid', 'direction'])
        self.assertIn('files/edges.vtp (shown): 2 lines', v.Describe('text'))
        # mistakes say what is valid
        with self.assertRaisesRegex(ValueError, 'the extensions: .vtp, .vtu'):
            v.AddFile(os.path.join(self.dir, 'edges.stl'))
        with self.assertRaises(FileNotFoundError):
            v.AddFile(os.path.join(self.dir, 'none.vtp'))
        with self.assertRaises(KeyError) as cm:
            v.Isolate('type:Wire')
        self.assertEqual(cm.exception.args[0],
                         'no property of type "Wire"; the types: DumpBox, Excitation, Material, Metal')

    def test_vtp_round_trip(self):
        # an exported property, added back as a file, covers the property
        from CSXCAD.Viewer import Viewer
        v = Viewer(_Structure(), offscreen=True)
        v.Export('vtp', os.path.join(self.dir, 'vtp'))
        item = v.AddFile(os.path.join(self.dir, 'vtp', 'patch.vtp'))
        np.testing.assert_allclose(item.GetBounds(), v._scene.Entry('patch').GetBounds(), atol=1e-6)
        item = v.AddFile(os.path.join(self.dir, 'vtp', 'patch.vtp'), unit='drawing')
        np.testing.assert_allclose(item.GetBounds()[1], 10e-3, rtol=1e-6)   # the file is in metres

    def test_tight_fit(self):
        # the fit follows the geometry, not its bounding box, and keeps the
        # legend column free
        from vtkmodules.vtkRenderingAnnotation import vtkScalarBarActor
        from vtkmodules.vtkCommonCore import vtkLookupTable
        from CSXCAD.Viewer import Viewer, ViewItem
        csx = ContinuousStructure()
        csx.AddMetal('ball').AddSphere([0, 0, 0], 10)
        v = Viewer(csx, offscreen=True, size=(400, 300), axes=False, grid=dict(visible=False))
        ren = v.GetRenderer()

        def extent():
            pts = v._FitPoints()[::7]
            xy = []
            for p in pts:
                ren.SetWorldPoint(*p, 1.0)
                ren.WorldToDisplay()
                xy.append(ren.GetDisplayPoint()[:2])
            xy = np.array(xy) / (400, 300)
            return xy.min(axis=0), xy.max(axis=0)
        v.SetView('iso', fit=True)
        lo, hi = extent()
        self.assertGreater(hi[1] - lo[1], 0.9)
        self.assertTrue((lo > 0).all() and (hi < 1).all())
        bar = vtkScalarBarActor()
        bar.SetLookupTable(vtkLookupTable())
        v.AddLegend(v.AddItem(ViewItem('field')), bar)
        v.SetView('iso', fit=True)
        lo, hi = extent()
        self.assertLess(hi[0], 1 - v._LegendShare(400) + 0.01)

    def test_clipping_follows_items(self):
        # an item added after the camera was set is not cut by the near plane
        from vtkmodules.vtkFiltersSources import vtkSphereSource
        from vtkmodules.vtkRenderingCore import vtkActor, vtkPolyDataMapper
        from CSXCAD.Viewer import Viewer, ViewItem
        v = Viewer(_Structure(), size=(400, 300), view='iso')
        _Interactive(v, (400, 300))
        v.Render()
        cam = v.GetRenderer().GetActiveCamera()
        pos = np.array(cam.GetPosition())
        src = vtkSphereSource()
        src.SetCenter(*(0.85 * pos))   # close to the camera
        src.SetRadius(0.05 * np.linalg.norm(pos))
        m = vtkPolyDataMapper()
        m.SetInputConnection(src.GetOutputPort())
        a = vtkActor()
        a.SetMapper(m)
        v.SetCamera(v.GetCamera())   # a fixed view: no refit
        v.AddItem(ViewItem('ball', a))
        v.Render()
        near = cam.GetClippingRange()[0]
        self.assertLess(near, 0.1 * np.linalg.norm(pos))

    def test_rubber_band_2d(self):
        from CSXCAD.Viewer import Viewer
        v = Viewer(_Structure(), size=(400, 300))
        iren = _Interactive(v, (400, 300))
        v.SetMode2D(True)

        def event(name, x, y):
            iren.SetEventInformation(x, y, 0, 0)
            iren.InvokeEvent(name)
        scale = v.GetCamera()['parallel_scale']
        event('LeftButtonPressEvent', 50, 60)
        event('MouseMoveEvent', 150, 210)
        band = v._band.GetMapper().GetInput()
        self.assertEqual([band.GetPoint(i)[:2] for i in range(4)],
                         [(50, 60), (150, 60), (150, 210), (50, 210)])
        event('LeftButtonReleaseEvent', 150, 210)
        self.assertIsNone(v._band)
        self.assertAlmostEqual(v.GetCamera()['parallel_scale'], scale * 150 / 300)

    def test_focus_at_cursor(self):
        from CSXCAD.Viewer import Viewer
        v = Viewer(_Structure(), size=(400, 300), view='+z')
        iren = _Interactive(v, (400, 300))
        v.Fit()
        ren = v._renderer
        ren.SetWorldPoint(5, 5, 1.5, 1)
        ren.WorldToDisplay()
        x, y, _ = ren.GetDisplayPoint()

        def press_f(x, y):
            iren.SetEventInformation(int(round(x)), int(round(y)), 0, 0, 'f', 0, 'f')
            iren.InvokeEvent('CharEvent')
        # nothing under the mouse: the camera stays
        cam = v.GetCamera()
        press_f(1, 1)
        self.assertEqual(v.GetCamera(), cam)
        # 3D: the camera turns towards the point and moves closer
        press_f(x, y)
        new = v.GetCamera()
        np.testing.assert_allclose(new['focal_point'], (5, 5, 1.5), atol=0.3)
        self.assertLess(np.linalg.norm(np.subtract(new['position'], new['focal_point'])),
                        np.linalg.norm(np.subtract(cam['position'], cam['focal_point'])))
        # 2D: the view slides within the plane, still looking along -z
        v.SetMode2D(True)
        ren.SetWorldPoint(-5, -5, 1.5, 1)
        ren.WorldToDisplay()
        press_f(*ren.GetDisplayPoint()[:2])
        new = v.GetCamera()
        np.testing.assert_allclose(new['focal_point'][:2], (-5, -5), atol=0.3)
        d = np.subtract(new['focal_point'], new['position'])
        np.testing.assert_allclose(d / np.linalg.norm(d), (0, 0, -1), atol=1e-9)

    def test_click(self):
        from CSXCAD.Viewer import Viewer
        v = Viewer(_Structure(), size=(400, 300))
        iren = _Interactive(v, (400, 300))
        from CSXCAD.Viewer.Core import ViewerHost
        clicks = []

        class Host(ViewerHost):
            def OnClick(self, x, y):
                clicks.append((x, y))
        v.SetHost(Host())

        def drag(x0, x1):
            for name, x in (('LeftButtonPressEvent', x0), ('MouseMoveEvent', x1), ('LeftButtonReleaseEvent', x1)):
                iren.SetEventInformation(x, 150, 0, 0)
                iren.InvokeEvent(name)
        # in 3D the trackball style grabs the mouse, so the release is not seen directly
        for mode_2d in (False, True):
            v.SetMode2D(mode_2d)
            del clicks[:]
            drag(200, 202)
            drag(200, 260)
            self.assertEqual(clicks, [(202, 150)])
        # switching picking off also clears the selection
        from CSXCAD.Viewer.Actions import GetAction
        v.Highlight('patch')
        GetAction('picking').Trigger(v)
        self.assertFalse(v.GetPicking())
        self.assertIsNone(v.GetHighlight())
        drag(200, 200)
        self.assertEqual(clicks, [(202, 150)])
        # without primitive selection a highlighted primitive becomes its property
        v.SetPrimitiveSelection(True)
        v.Highlight(('patch', 0))
        GetAction('select_primitives').Trigger(v)
        self.assertEqual(v.GetHighlight(), ['patch'])

    def test_scaled_capture_of_window(self):
        # a capture at another size renders in an own window and comes back
        from CSXCAD.Viewer import Viewer
        v = Viewer(_Structure(), size=(200, 150))
        _Interactive(v, (200, 150))
        one = v.Capture().astype(float)
        two = v.Capture(scale=2).astype(float)
        self.assertEqual(two.shape[:2], (300, 400))
        small = two.reshape(150, 2, 200, 2, -1).mean(axis=(1, 3))
        self.assertLess(np.abs(small - one).mean(), 8)
        np.testing.assert_allclose(v.Capture().astype(float), one, atol=1)

    def test_legends_and_hover(self):
        from CSXCAD.Viewer import Viewer, ViewItem
        from vtkmodules.vtkFiltersSources import vtkPlaneSource
        from vtkmodules.vtkRenderingCore import vtkActor, vtkPolyDataMapper
        from vtkmodules.vtkRenderingAnnotation import vtkScalarBarActor
        from vtkmodules.vtkCommonCore import vtkLookupTable

        class Plane(ViewItem):
            def __init__(self, name, z):
                src = vtkPlaneSource()
                src.SetOrigin(-5, -5, z)
                src.SetPoint1(25, -5, z)
                src.SetPoint2(-5, 25, z)
                src.Update()
                m = vtkPolyDataMapper()
                m.SetInputData(src.GetOutput())
                a = vtkActor()
                a.SetMapper(m)
                super().__init__(name, a, include_in_bounds=True)

            def GetValueText(self, pos, prop, cell):
                return '{} z={:.1f}'.format(self.GetName(), pos[2])

        v = Viewer(_Structure(), offscreen=True, size=(1200, 600))
        cam = v.GetCamera()
        items = [v.AddItem(Plane(n, z), group='results') for n, z in (('a', 3), ('b', 4), ('c', 5))]
        # a view that was only fitted is fitted again to include new items
        self.assertNotEqual(v.GetCamera()['position'], cam['position'])
        v.SetCamera(cam)
        v.AddItem(Plane('d', 40), group='results').SetVisible(False)
        self.assertEqual(v.GetCamera(), cam)
        bars = [vtkScalarBarActor() for _ in items]
        for bar in bars:
            bar.SetLookupTable(vtkLookupTable())
        for it, bar in zip(items, bars):
            v.AddLegend(it, bar)
        v.Capture()
        x = [b.GetPosition()[0] for b in bars]
        self.assertTrue(x[0] < x[1] < x[2] < 1)
        v.SetVisible('results/b', False)
        v.Capture()
        self.assertFalse(bars[1].GetVisibility())
        self.assertAlmostEqual(bars[0].GetPosition()[0], x[1])
        v.SetView('+z')
        v.Capture()
        self.assertEqual(v.GetItemValueText(600, 300), 'c z=5.0')
        self.assertEqual(v.Pick(600, 300)['item'], 'results/c')   # the nearest one
        v.SetSize((400, 300))
        v.Capture()
        self.assertEqual(bars[0].GetPosition()[0], bars[2].GetPosition()[0])   # stacked
        self.assertIsNone(v.GetItemValueText(2, 2))
        v.RemoveItem(items[2])
        self.assertEqual(len(v.GetLegends()), 2)

    def test_highlight_curved_hidden(self):
        from CSXCAD.Viewer import Viewer
        csx = _Structure()
        ball = csx.AddMetal('ball')
        ball.AddSphere([0, 0, 5], 3)
        ball.SetVisibility(False)
        v = Viewer(csx, offscreen=True, size=(400, 300))
        v.Highlight('ball')
        # a sphere has no feature edges: the outline is its silhouette, and a
        # tint shows the shape of the hidden property
        sc = v._scene
        lines = sc.highlight_actor.GetMapper().GetInputAlgorithm()
        lines.Update()
        self.assertGreater(lines.GetOutput().GetNumberOfLines(), 0)
        self.assertFalse(sc.Entry('ball').surf_actor.GetVisibility())
        np.testing.assert_allclose(sc.highlight_surf.GetBounds(), (-3, 3, -3, 3, 2, 8), atol=0.05)

    def test_labels(self):
        from CSXCAD.Viewer import Viewer
        v = Viewer(_Structure(), offscreen=True, size=(400, 300), labels=True, view='+z')
        v.Screenshot()
        ren = v._renderer
        # the substrate label attaches to its top, upper-right corner
        ren.SetWorldPoint(25, 25, 1.5, 1)
        ren.WorldToDisplay()
        # (labels and leaders are placed relative to the view, for tiled screenshots)
        w, h = ren.GetSize()
        anchors = v._scene._leaders.GetMapper().GetInput().GetPoints()
        anchors = [anchors.GetPoint(i)[:2] for i in range(0, anchors.GetNumberOfPoints(), 2)]
        self.assertIn(tuple(round(c) for c in ren.GetDisplayPoint()[:2]),
                      [(round(a[0] * w), round(a[1] * h)) for a in anchors])
        # and no two labels overlap
        boxes = []
        for e in v._scene.entries:
            if e.label is not None and e.label.GetVisibility():
                bb = [0.0] * 4
                e.label.GetBoundingBox(ren, bb)
                x, y = e.label.GetPosition()
                boxes.append((x * w, y * h, bb[1] - bb[0], bb[3] - bb[2]))
        self.assertGreaterEqual(len(boxes), 3)
        for i, a in enumerate(boxes):
            for b in boxes[i + 1:]:
                self.assertFalse(a[0] < b[0] + b[2] and b[0] < a[0] + a[2] and
                                 a[1] < b[1] + b[3] and b[1] < a[1] + a[3])

    def test_export(self):
        from vtkmodules.vtkIOGeometry import vtkSTLReader
        from CSXCAD.Viewer.Export import ExportCSX
        csx = _Structure()
        res = ExportCSX(csx, 'stl', self.dir)
        names = sorted(os.path.basename(f) for f in res['files'])
        self.assertEqual(names, ['patch.stl', 'substrate.stl'])   # dumps hidden, port is a line
        rd = vtkSTLReader()
        rd.SetFileName(os.path.join(self.dir, 'substrate.stl'))
        rd.Update()
        np.testing.assert_allclose(rd.GetOutput().GetBounds(), (-0.025, 0.025, -0.025, 0.025, 0, 0.0015), atol=1e-7)
        res = ExportCSX(csx, 'vtp', self.dir, include_hidden=True)
        self.assertEqual(len(res['files']), 5)
        fn = os.path.join(self.dir, 'scene.glb')
        ExportCSX(csx, 'glb', fn)
        with open(fn, 'rb') as f:
            self.assertEqual(f.read(4), b'glTF')

    def test_cli_and_output_mode(self):
        from CSXCAD.Viewer import CLI, Viewer
        fn = os.path.join(self.dir, 'struct.xml')
        _Structure().Write2XML(fn)
        out = io.StringIO()
        with contextlib.redirect_stdout(out), contextlib.redirect_stderr(io.StringIO()):
            self.assertEqual(CLI.Main([fn, '--describe', 'json', '--export', 'stl', os.path.join(self.dir, 'stl')]), 0)
        d = json.loads(out.getvalue())
        self.assertEqual([p['name'] for p in d['properties']], ['substrate', 'patch', 'port', 'Et', 'Ht'])
        self.assertEqual(d['properties'][1]['primitive_types'], ['Box', 'Cylinder'])
        self.assertTrue(os.path.isfile(os.path.join(self.dir, 'stl', 'patch.stl')))

        def run(*args):
            out, err = io.StringIO(), io.StringIO()
            with contextlib.redirect_stdout(out), contextlib.redirect_stderr(err):
                code = CLI.Main([fn] + list(args))
            return code, out.getvalue(), err.getvalue()
        # the view from single keys: JSON values, plain words as text
        code, out, _ = run('--print-view', '--set', 'view=+z', '--set', 'elevation=-30',
                           '--set', 'clip={"normal": "y", "position": 2}', '--set', 'labels=true')
        st = json.loads(out)
        self.assertEqual((code, st['clip'], st['labels']), (0, dict(normal='y', position=2.0, invert=False), True))
        ref = Viewer(fn, view='+z', elevation=-30, clip=dict(normal='y', position=2), labels=True)
        np.testing.assert_allclose(st['camera']['position'], ref.GetCamera()['position'])
        # mistakes say what is valid
        self.assertEqual(run('--set', 'render_mode=edgez', '--print-view')[:2], (1, ''))
        self.assertIn('use one of', run('--set', 'render_mode=edgez', '--print-view')[2])
        self.assertIn('valid keys:', run('--set', 'colour=red', '--print-view')[2])
        self.assertEqual(run('--set', 'isolate=nothing', '--print-view')[2].strip(),
                         'csxcad-view: unknown property "nothing"; the properties: substrate, patch, port, Et, Ht')
        code, _, err = run('--set', 'visible={"properties/dump": false}', '--print-view')
        self.assertEqual(code, 1)
        self.assertIn('no tree path properties/dump; the paths:', err)
        self.assertIn('properties/dumps', err)
        # VTK files next to the structure
        edges = self._EdgeFile('edges.vtp')
        self.assertIn('files/edges.vtp (shown): 2 lines', run('--file', edges, '--describe')[1])
        code, _, err = run('--file', os.path.join(self.dir, 'none.vtp'), '--describe')
        self.assertEqual((code, err.strip()), (1, 'csxcad-view: {}: no such file'.format(
            os.path.join(self.dir, 'none.vtp'))))
        # the text summary has the tree paths and primitive ids for the view keys
        text = run('--describe')[1]
        self.assertIn('patch [Metal] properties/materials/patch: 2 prim (0 Box, 1 Cylinder)', text)
        self.assertIn('tree paths: grid, grid/x', text)
        # every key is explained
        from CSXCAD.Viewer.ViewState import DEFAULTS, KEY_HELP
        self.assertEqual(set(KEY_HELP), set(DEFAULTS))
        keys = CLI.KeyList()
        self.assertTrue(all('  {} '.format(k) in keys for k in DEFAULTS if k != 'version'))
        # python -m CSXCAD.Viewer is csxcad-view
        import subprocess
        import sys
        out = subprocess.run([sys.executable, '-m', 'CSXCAD.Viewer', '--list-view-keys'], stdout=subprocess.PIPE, stderr=subprocess.PIPE,
                             universal_newlines=True)
        self.assertEqual(out.stdout.strip(), keys.strip(), out.stderr)

        os.environ['CSXCAD_VIEWER_OUTPUT'] = os.path.join(self.dir, 'out')
        v = Viewer(_Structure(), name='demo')
        v.Show()
        v.SetView('+z')
        cam = v.GetCamera()
        v.AddItem('nothing', None)
        v.Show()
        files = sorted(os.listdir(os.path.join(self.dir, 'out')))
        self.assertEqual(files, ['demo_2_describe.json', 'demo_2_view.png', 'demo_2_views.png',
                                 'demo_describe.json', 'demo_view.png', 'demo_views.png'])
        self.assertEqual(v.GetCamera(), cam)
        with open(os.path.join(self.dir, 'out', 'demo_2_describe.json')) as f:
            self.assertIn('items/nothing', json.load(f)['items'])


class TestViewerFigures(unittest.TestCase):

    def setUp(self):
        if not HAS_VTK:
            self.skipTest('vtk not available')
        try:
            import matplotlib
        except ImportError:
            self.skipTest('matplotlib not available')
        matplotlib.use('Agg')

    def _Figure(self):
        return _Figure()

    def test_tree_and_visibility(self):
        from CSXCAD.Viewer import Viewer
        v = Viewer(_Structure(), offscreen=True)
        fig = self._Figure()
        vf = v.AddFigure(fig)
        self.assertEqual(vf.GetName(), 'S-parameters')
        self.assertEqual(vf.GetCurves(), ['S11', 'S21', 'SAR'])
        node = v.GetTree('figures')
        self.assertEqual([c['kind'] for c in node['children']], ['figure'])
        self.assertEqual([c['label'] for c in node['children'][0]['children']], ['S11', 'S21', 'SAR'])
        self.assertTrue(v.GetVisible('figures/S-parameters'))
        v.SetVisible('figures/S-parameters/S21', False)
        self.assertFalse(fig.axes[0].get_lines()[1].get_visible())
        self.assertFalse(v.GetVisible('figures/S-parameters/S21'))
        v.SetVisible('figures', False)         # the group hides the figures, not the curves
        self.assertFalse(vf.IsShown())
        self.assertTrue(v.GetVisible('figures/S-parameters/S11'))
        v.RaiseFigure('S-parameters')          # raising shows it again
        self.assertTrue(vf.IsShown())
        s11, s21 = fig.axes[0].get_lines()[:2]
        lw = s11.get_linewidth()
        sar = vf.GetCurveArtists('SAR')[0]
        edges = np.array(sar.get_linewidth(), copy=True)
        v.HighlightCurves(['figures/S-parameters/S11', 'figures/S-parameters/SAR'])
        self.assertGreater(s11.get_linewidth(), lw)    # thicker
        np.testing.assert_array_equal(sar.get_linewidth(), edges)   # the map keeps its edges
        self.assertEqual(vf.GetHighlight(), ['S11', 'SAR'])
        v.HighlightCurves(['figures/S-parameters/S21'])
        self.assertEqual((s11.get_linewidth(), vf.GetHighlight()), (lw, ['S21']))
        v.HighlightCurves([])
        self.assertEqual(s21.get_linewidth(), lw)
        with self.assertRaises(ValueError):
            v.HighlightCurves(['figures/S-parameters'])
        d = v.Describe()['figures']['figures/S-parameters']
        self.assertEqual((d['shown'], d['curves']), (True, dict(S11=True, S21=False, SAR=True)))
        self.assertIn('figures/S-parameters (shown): S11, S21 (hidden), SAR', v.Describe('text'))
        vf2 = v.AddFigure(self._Figure(), curves={'both': fig.axes[0].get_lines()[:2]})
        self.assertEqual(vf2.GetName(), 'S-parameters (2)')
        self.assertEqual(vf2.GetCurves(), ['both'])
        self.assertEqual(list(v.GetFigures()), ['S-parameters', 'S-parameters (2)'])
        with self.assertRaises(ValueError):
            v.AddFigure(vf)
        v.RemoveFigure(vf)
        v.RemoveFigure('S-parameters (2)')
        self.assertNotIn('figures', [n['path'] for n in v.GetTree()])

    def test_remove_figures_group(self):
        # the figures group takes its figures along (in the GUI: their tabs)
        from CSXCAD.Viewer import Viewer
        from CSXCAD.Viewer.Core import ViewerHost
        removed = []

        class Host(ViewerHost):
            def FigureRemoved(self, figure):
                removed.append(figure.GetName())

        v = Viewer(_Structure(), offscreen=True)
        v.SetHost(Host())
        vf = v.AddFigure(self._Figure())
        v.AddFigure(self._Figure(), name='other')
        v.RemoveGroup('figures')
        self.assertEqual((v.GetFigures(), removed, vf.GetPath()), ({}, ['S-parameters', 'other'], None))
        self.assertNotIn('figures', v._scene.tree.nodes)
        v.AddFigure(vf)   # free to be added again

    def test_view_before_figures(self):
        # a view applied before the figures exist (e.g. openems-view) shows or
        # hides them and their curves, and the state reports them
        from CSXCAD.Viewer import Viewer
        v = Viewer(_Structure(), offscreen=True,
                   visible={'figures/S-parameters/S21': False, 'figures/other': False})
        vf = v.AddFigure(self._Figure())
        other = v.AddFigure(self._Figure(), name='other')
        self.assertEqual((vf.IsShown(), vf.GetCurveVisible('S21'), vf.GetCurveVisible('S11')), (True, False, True))
        self.assertFalse(other.IsShown())
        st = v.GetState()['visible']
        self.assertEqual((st['figures/S-parameters/S21'], st['figures/S-parameters/S11'], st['figures/other']),
                         (False, True, False))

    def test_plain_label(self):
        # mathtext labels become plain text for the tree rows
        from CSXCAD.Viewer import Viewer
        from CSXCAD.Viewer.Figures import PlainLabel
        self.assertEqual(PlainLabel(r'$\Re\{Z_{in}\}$'), 'Re(Zᵢₙ)')
        self.assertEqual(PlainLabel(r'arg $S_{11}$ / $\pi a^2$'), 'arg S₁₁ / πa²')
        self.assertEqual(PlainLabel(r'$\beta_{MSL}$, $Z_{L, 0}$'), 'β_MSL, Z_(L, 0)')
        self.assertEqual(PlainLabel(r'cost \$5'), 'cost $5')   # an escaped dollar, no math
        fig = self._Figure()
        s11, s21 = fig.axes[0].get_lines()[:2]
        # two names with the same plain text
        vf = Viewer(_Structure(), offscreen=True).AddFigure(fig, curves={'$S_{11}$': s11, 'S₁₁': s21})
        self.assertEqual(vf.GetCurves(), ['S₁₁', 'S₁₁ (2)'])
        self.assertEqual(vf.GetCurveArtists('S₁₁ (2)'), [s21])
        s21.set_label('$S11$')   # the same for the labels found in the figure
        self.assertEqual(Viewer(_Structure(), offscreen=True).AddFigure(fig).GetCurves(),
                         ['S11', 'S11 (2)', 'SAR'])

    def test_output_mode(self):
        from CSXCAD.Viewer import Core, Viewer
        v = Viewer(_Structure(), offscreen=True, name='fig_test')
        v.AddFigure(self._Figure())
        hidden = v.AddFigure(self._Figure(), name='hidden', shown=False)
        self.assertFalse(hidden.IsShown())
        with tempfile.TemporaryDirectory() as tmp:
            v._WriteOutput(tmp)
            self.assertTrue(os.path.exists(os.path.join(tmp, 'fig_test_S-parameters.png')))
            self.assertFalse(os.path.exists(os.path.join(tmp, 'fig_test_hidden.png')))
        Core._OUTPUT['counts'].pop('fig_test', None)

    def test_gui(self):
        # in a subprocess, to use Qt's offscreen platform and a scratch config
        try:
            import PySide6  # noqa: F401
        except ImportError:
            self.skipTest('PySide6 not available')
        import subprocess
        import sys
        code = """
import sys
sys.path.insert(0, {tests!r})
from test_Viewer import _Structure, _Figure
import matplotlib
matplotlib.use('QtAgg')
from PySide6.QtWidgets import QApplication, QWidget
app = QApplication([])
from CSXCAD.Viewer import Viewer
from CSXCAD.Viewer.GUI.MainWindow import MainWindow
v = Viewer(_Structure(), offscreen=True)
early = v.AddFigure(_Figure(), name='early', shown=False)   # before the window
w = MainWindow(v, render_widget=QWidget())
w.show()
w.Start()
assert early.GetPath() in w._figure_subs and w._figure_subs[early.GetPath()][0].isHidden()
v.RaiseFigure('early')
assert w.mdi.currentSubWindow() is w._figure_subs[early.GetPath()][0]
v.RaiseFigure(None)
v.RemoveFigure(early)
vf = v.AddFigure(_Figure())
app.processEvents()
subs = w.mdi.subWindowList()
assert [s.windowTitle() for s in subs] == ['3D view', 'S-parameters'], [s.windowTitle() for s in subs]
from PySide6.QtCore import Qt
assert all(s.testAttribute(Qt.WidgetAttribute.WA_NativeWindow) for s in subs)   # see _AddSubWindow
sub, tb = w._figure_subs[vf.GetPath()]
assert tb.iconSize() == w.Toolbar('View').iconSize()   # same height as the viewer's toolbars
assert w.mdi.currentSubWindow() is w._view_sub
assert not tb.isVisible()                          # the 3D view is in front
w.tree._items['figures/S-parameters/S21'].setSelected(True)
app.processEvents()
assert w.mdi.currentSubWindow() is sub and tb.isVisible()
s21 = vf.GetCurveArtists('S21')[0]
assert vf.GetHighlight() == ['S21'] and s21.get_linewidth() > vf.GetCurveArtists('S11')[0].get_linewidth()
assert w.vtk_widget.isHidden()                     # a native window would paint over the tab
w.tree.clearSelection()
assert vf.GetHighlight() == []                     # deselected: back to its width
w.tree._items['properties/materials/patch'].setSelected(True)
app.processEvents()
assert w.mdi.currentSubWindow() is w._view_sub and not tb.isVisible()
assert not w.vtk_widget.isHidden()
row = w.tree._items['figures/S-parameters/S11']
row.setCheckState(0, Qt.CheckState.Unchecked)     # the tree hides a curve
assert not vf.GetCurveVisible('S11')
w._view_sub.close()                                # the 3D view cannot be closed
v.SetWindowMode('windows')
v.RaiseFigure(vf)
assert not w.vtk_widget.isHidden()                 # side by side, the 3D view stays
w._view_sub.close()
assert not w._view_sub.isHidden() and len(w.mdi.subWindowList()) == 2
assert not (w._view_sub.windowFlags() & Qt.WindowType.WindowCloseButtonHint)
sub.close()                                        # a figure closes as hidden
assert sub.isHidden() and not vf.IsShown()
v.SetWindowMode('tabs')
from PySide6.QtWidgets import QTabBar
tabs = lambda: [w.mdi.findChild(QTabBar).tabText(i) for i in range(w.mdi.findChild(QTabBar).count())]
assert tabs() == ['3D view'], tabs()               # a hidden figure has no tab (a mode switch rebuilds the bar)
assert w.mdi.currentSubWindow() is w._view_sub
v.SetWindowMode('windows')
assert w.tree._items['figures/S-parameters'].checkState(0) == Qt.CheckState.Unchecked
v.RaiseFigure(vf)                                  # and comes back
assert not sub.isHidden() and w.mdi.currentSubWindow() is sub
v.SetWindowMode('tabs')
assert tabs() == ['3D view', 'S-parameters'], tabs()
v.SetVisible(vf.GetPath(), False)                  # hiding the current tab: the 3D view comes to the front
assert w.mdi.currentSubWindow() is w._view_sub and tabs() == ['3D view'] and not w.vtk_widget.isHidden()
v.RaiseFigure(vf)
second = v.AddFigure(_Figure(), name='second')
v.SetVisible(vf.GetPath(), False)
assert tabs() == ['3D view', 'second'], tabs()
w.mdi.setActiveSubWindow(w._figure_subs[second.GetPath()][0])   # a click on another tab
assert tabs() == ['3D view', 'second'], tabs()
v.SetVisible(vf.GetPath(), True)                   # back at its place, not at the end
assert tabs() == ['3D view', 'S-parameters', 'second'], tabs()
assert [s.windowTitle() for s in w.mdi.subWindowList()] == tabs()   # the area follows the tab order
assert w.mdi.currentSubWindow() is w._figure_subs[vf.GetPath()][0]
v.RemoveFigure(second)
assert w._figure_subs[vf.GetPath()][0].isMaximized()
v.RemoveFigure(vf)
assert [s.windowTitle() for s in w.mdi.subWindowList()] == ['3D view']
w.close()
assert w.final_state is not None and w.final_state['window_mode'] == 'tabs'
w2 = MainWindow(Viewer(_Structure(), offscreen=True, window_mode='windows'), render_widget=QWidget())
assert w2.WindowMode() == 'windows' and w2._mode_actions['windows'].isChecked()   # from the view, not saved
w2.close()
print('ok')
""".format(tests=os.path.dirname(os.path.abspath(__file__)))
        with tempfile.TemporaryDirectory() as tmp:
            env = dict(os.environ, QT_QPA_PLATFORM='offscreen', XDG_CONFIG_HOME=tmp)
            # (a timeout: a modal dialog would wait forever)
            out = subprocess.run([sys.executable, '-c', code], stdout=subprocess.PIPE, stderr=subprocess.PIPE, universal_newlines=True, env=env, timeout=300)
        self.assertEqual(out.stdout.strip(), 'ok', out.stderr)


class TestViewerImport(unittest.TestCase):

    def test_lazy_imports(self):
        import subprocess
        import sys
        code = ('import sys, CSXCAD, CSXCAD.Viewer; '
                'print(any(m.startswith(("vtk", "PySide6")) for m in sys.modules))')
        out = subprocess.run([sys.executable, '-c', code], stdout=subprocess.PIPE, stderr=subprocess.PIPE, universal_newlines=True)
        self.assertEqual(out.stdout.strip(), 'False', out.stderr)

    def test_items_render_without_viewer(self):
        # an item built before any Viewer exists (only ViewItem imported) gets
        # OpenGL actors; generic ones are not drawn
        if not HAS_VTK:
            self.skipTest('vtk not available')
        import subprocess
        import sys
        code = ('from CSXCAD.Viewer import ViewItem; '
                'from vtkmodules.vtkRenderingCore import vtkActor; '
                'from vtkmodules.vtkRenderingAnnotation import vtkScalarBarActor; '
                'print(type(vtkActor()).__name__, type(vtkScalarBarActor()).__name__)')
        out = subprocess.run([sys.executable, '-c', code], stdout=subprocess.PIPE, stderr=subprocess.PIPE, universal_newlines=True)
        self.assertEqual(out.stdout.split()[0], 'vtkOpenGLActor', out.stderr)

    def test_gui_extensions(self):
        # an application adds actions, a dock and editable item settings;
        # in a subprocess, to use Qt's offscreen platform and a scratch config
        try:
            import PySide6  # noqa: F401
        except ImportError:
            self.skipTest('PySide6 not available')
        if not HAS_VTK:
            self.skipTest('vtk not available')
        import subprocess
        import sys
        code = """
import sys
sys.path.insert(0, {tests!r})
from test_Viewer import _Structure
from PySide6.QtWidgets import QApplication, QWidget, QLabel, QSpinBox
app = QApplication([])
from CSXCAD.Viewer import Viewer, ViewItem
from CSXCAD.Viewer.Actions import Action
from CSXCAD.Viewer.GUI.MainWindow import MainWindow

class Win(MainWindow):
    TITLE = 'App'
    SETTINGS = ('CSXCAD-test', 'app')
    def Customize(self):
        self.AddDock('sim', 'Simulation', QLabel('idle'))

class Field(ViewItem):
    n = 3
    def GetParameters(self):
        return [dict(name='n', type='int', value=self.n, min=1, max=10, help='how many')] + [
            dict(name='pos{{}}'.format(i), type='slider', value=0, values=[0, 1, 2], section='Planes')
            for i in range(self.n - 3)]
    def SetParameter(self, name, value):
        self.n = value
    def GetDescription(self):
        return 'n={{}}'.format(self.n)
    def GetActions(self):
        return [('Reset', lambda: setattr(self, 'n', 3))]

v = Viewer(_Structure(), offscreen=True)
runs = []
v.AddAction(Action('run', 'Run', lambda v: runs.append(1), group='Simulation'), toolbar='Simulation')
w = Win(v, render_widget=QWidget())
item = v.AddItem(Field('field'), group='results')
v.AddAction(Action('stop', 'Stop', lambda v: None, group='Simulation'), toolbar='Simulation')
assert w.windowTitle() == 'App', w.windowTitle()
w.show()
w.Start()
v.SetWindowPosition((0.1, 0.2))
a = w.screen().availableGeometry()
assert (w.frameGeometry().x() - a.x(), w.frameGeometry().y() - a.y()) == \
    (round(0.1 * a.width()), round(0.2 * a.height())), (w.frameGeometry(), a)
assert v.GetState()['window_position'] == [0.1, 0.2], v.GetState()['window_position']
assert v.GetState()['size'][0] <= 1   # relative on a screen
assert w.dock_sim.widget().text() == 'idle'
assert [a.text() for a in w.Toolbar('Simulation').actions()] == ['Run', 'Stop']
assert [a.text() for a in w.Menu('Simulation').actions()] == ['Run', 'Stop']
w.Action('run').trigger()
assert runs == [1]
assert not any(p.startswith('grid') for p in w.tree._items)   # the grid has its own dock
assert all(it.treeWidget() is w.tree for it in w.tree._items.values())   # all rows shown
w2 = Win(Viewer(_Structure(), offscreen=True, isolate='patch'), render_widget=QWidget())
w2.Start()
expanded = {{p: it.isExpanded() for p, it in w2.tree._items.items() if p.count('/') == 1 and it.childCount()}}
assert expanded['properties/materials'] and not any(e for p, e in expanded.items() if p != 'properties/materials'), expanded
from PySide6.QtWidgets import QTreeWidgetItem
row = w.tree._items['properties/materials/patch']
v.SetPrimitiveSelection(True)
w.tree.Refresh()
w.tree._OnExpanded(row)
assert row.childCount() > 0
v.SetPrimitiveSelection(False)
w.tree.Refresh()
assert row.childCount() == 0
assert row.childIndicatorPolicy() == QTreeWidgetItem.ChildIndicatorPolicy.DontShowIndicator
row.setSelected(True)
assert v.GetHighlight() == ['patch'], v.GetHighlight()
w.tree.clearSelection()   # e.g. a click on empty space in the tree
assert v.GetHighlight() is None, v.GetHighlight()
w.info.ShowItem(item)
spin = w.item_editor.findChild(QSpinBox)
assert spin.toolTip() == 'how many', spin.toolTip()
spin.setValue(5)
assert item.n == 5, item.n
from CSXCAD.Viewer.GUI.Widgets import SliderField
assert len(w.item_editor.findChildren(SliderField)) == 2   # rebuilt
w.tree.Refresh()
assert w.tree._items['results/field'].text(1) == 'n=5'
w.info.clear()
assert w.item_editor.item is None and w.dock_settings.widget() is w.item_editor
import os, tempfile
fn = os.path.join(tempfile.mkdtemp(), 'struct.xml')
_Structure().Write2XML(fn)
w3 = Win(Viewer(fn, offscreen=True), render_widget=QWidget())
assert w3.windowTitle() == 'App - struct.xml', w3.windowTitle()
os.remove(fn)
w3.SaveXML()                      # to its file, no "Save as" dialog
assert os.path.isfile(fn)
other = _Structure()
other.AddMetal('extra').AddBox([0, 0, 0], [1, 1, 1])
other.Write2XML(fn)               # changed on disk
w3.Reload()                       # reads the file again
assert 'extra' in w3.viewer.GetPropertyNames(), w3.viewer.GetPropertyNames()
print('ok')
""".format(tests=os.path.dirname(os.path.abspath(__file__)))
        with tempfile.TemporaryDirectory() as tmp:
            env = dict(os.environ, QT_QPA_PLATFORM='offscreen', XDG_CONFIG_HOME=tmp)
            # (a timeout: a modal dialog would wait forever)
            out = subprocess.run([sys.executable, '-c', code], stdout=subprocess.PIPE, stderr=subprocess.PIPE, universal_newlines=True, env=env, timeout=300)
        self.assertEqual(out.stdout.strip(), 'ok', out.stderr)

    def test_import_prepares_qt(self):
        # imported before matplotlib starts Qt: X11 (Wayland as fallback) and PySide6
        try:
            import PySide6  # noqa: F401
        except ImportError:
            self.skipTest('PySide6 not available')
        import subprocess
        import sys
        env = {k: v for k, v in os.environ.items() if k not in ('QT_QPA_PLATFORM', 'QT_API')}
        env['DISPLAY'] = ':0'
        code = "import os, CSXCAD.Viewer; print(os.environ.get('QT_API'), os.environ.get('QT_QPA_PLATFORM'))"
        out = subprocess.run([sys.executable, '-c', code], stdout=subprocess.PIPE, stderr=subprocess.PIPE, universal_newlines=True, env=env)
        expect = 'pyside6 ' + ('xcb;wayland' if sys.platform.startswith('linux') else 'None')
        self.assertEqual(out.stdout.strip(), expect, out.stderr)


if __name__ == '__main__':
    unittest.main()
