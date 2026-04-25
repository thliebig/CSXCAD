# -*- coding: utf-8 -*-
import math
import os
import tempfile
import unittest

import numpy as np


from CSXCAD.CSXCAD import ContinuousStructure
from CSXCAD import CSRectGrid
from CSXCAD.CSProperties import CSPropLorentzMaterial, CSPropDebyeMaterial, CSPropDiscMaterial


def _decode(s):
    return s.decode('UTF-8') if isinstance(s, bytes) else s


class TestXMLRoundTrip(unittest.TestCase):
    """XML write/read round-trip tests.

    Every test sets only non-default attribute values so that a silent
    re-import failure (wrong value or missing tag) produces an assertion
    error rather than a false pass.
    """

    def setUp(self):
        self.csx = ContinuousStructure()
        self.fn = os.path.join(tempfile.gettempdir(), 'test_xml_roundtrip.xml')

    def tearDown(self):
        if os.path.exists(self.fn):
            os.remove(self.fn)

    # ------------------------------------------------------------------
    def _roundtrip(self):
        self.assertTrue(self.csx.Write2XML(self.fn))
        csx2 = ContinuousStructure()
        self.assertEqual(csx2.ReadFromXML(self.fn), '')
        return csx2

    def _prop(self, csx, name):
        props = csx.GetPropertiesByName(name)
        self.assertEqual(len(props), 1,
                         'Expected exactly one property named "{}", got {}'.format(name, len(props)))
        return props[0]

    # ==================================================================
    # Base CSProperties attributes (shared by every property type)
    # ==================================================================

    def test_base_fill_color(self):
        p = self.csx.AddMetal('fill_color')
        p.SetFillColor((100, 150, 200), alpha=128)
        p2 = self._prop(self._roundtrip(), 'fill_color')
        self.assertEqual(p2.GetFillColor(), (100, 150, 200, 128))

    def test_base_edge_color(self):
        p = self.csx.AddMetal('edge_color')
        p.SetEdgeColor((50, 75, 100), alpha=200)
        p2 = self._prop(self._roundtrip(), 'edge_color')
        self.assertEqual(p2.GetEdgeColor(), (50, 75, 100, 200))

    def test_base_visibility(self):
        p = self.csx.AddMetal('visibility')
        p.SetVisibility(False)          # default is True
        p2 = self._prop(self._roundtrip(), 'visibility')
        self.assertFalse(p2.GetVisibility())

    def test_base_custom_attribute(self):
        p = self.csx.AddMetal('custom_attr')
        p.SetAttributeValue('mykey', 'myvalue')
        p2 = self._prop(self._roundtrip(), 'custom_attr')
        self.assertTrue(p2.ExistAttribute('mykey'))
        self.assertEqual(p2.GetAttributeValue('mykey'), 'myvalue')

    # ==================================================================
    # CSPropMaterial
    # ==================================================================

    def test_material_isotropic(self):
        m = self.csx.AddMaterial('mat_iso')
        m.SetMaterialProperty(epsilon=2.5, mue=1.5, kappa=0.1, sigma=0.2, density=1200.0)
        m.SetMaterialWeight(epsilon='2*x+1', kappa='y*z', density='rho')

        m2 = self._prop(self._roundtrip(), 'mat_iso')
        self.assertAlmostEqual(m2.GetMaterialProperty('epsilon'), 2.5)
        self.assertAlmostEqual(m2.GetMaterialProperty('mue'),     1.5)
        self.assertAlmostEqual(m2.GetMaterialProperty('kappa'),   0.1)
        self.assertAlmostEqual(m2.GetMaterialProperty('sigma'),   0.2)
        self.assertAlmostEqual(m2.GetMaterialProperty('density'), 1200.0)
        self.assertEqual(m2.GetMaterialWeight('epsilon'), '2*x+1')
        self.assertEqual(m2.GetMaterialWeight('kappa'),   'y*z')
        self.assertEqual(m2.GetMaterialWeight('density'), 'rho')

    def test_material_anisotropic(self):
        m = self.csx.AddMaterial('mat_aniso')
        m.SetIsotropy(False)            # default is True
        m.SetMaterialProperty(epsilon=[2.0, 3.0, 4.0], mue=[1.1, 1.2, 1.3])
        m.SetMaterialWeight(epsilon=['x', 'y', 'z'])

        m2 = self._prop(self._roundtrip(), 'mat_aniso')
        self.assertFalse(m2.GetIsotropy())
        np.testing.assert_array_almost_equal(m2.GetMaterialProperty('epsilon'), [2.0, 3.0, 4.0])
        np.testing.assert_array_almost_equal(m2.GetMaterialProperty('mue'),     [1.1, 1.2, 1.3])
        self.assertEqual(m2.GetMaterialWeight('epsilon'), ['x', 'y', 'z'])

    # ==================================================================
    # CSPropConductingSheet
    # ==================================================================

    def test_conducting_sheet(self):
        self.csx.AddConductingSheet('cs', conductivity=56e6, thickness=18e-6)
        p2 = self._prop(self._roundtrip(), 'cs')
        self.assertEqual(p2.GetTypeString(), 'ConductingSheet')
        self.assertAlmostEqual(p2.GetConductivity(), 56e6)
        self.assertAlmostEqual(p2.GetThickness(),    18e-6)

    # ==================================================================
    # CSPropLumpedElement
    # ==================================================================

    def test_lumped_element(self):
        # LEtype=1 is LE_SERIES (non-default; default is LE_PARALLEL=0)
        self.csx.AddLumpedElement('le', ny=1, caps=True, R=50.0, C=1e-12, L=1e-9, LEtype=1)
        p2 = self._prop(self._roundtrip(), 'le')
        self.assertEqual(p2.GetTypeString(), 'LumpedElement')
        self.assertEqual(p2.GetDirection(), 1)
        self.assertTrue(p2.GetCaps())
        self.assertAlmostEqual(p2.GetResistance(), 50.0)
        self.assertAlmostEqual(p2.GetCapacity(),   1e-12)
        self.assertAlmostEqual(p2.GetInductance(), 1e-9)
        self.assertEqual(p2.GetLEtype(), 1)

    # ==================================================================
    # CSPropAbsorbingBC
    # ==================================================================

    def test_absorbing_bc(self):
        # AbsorbingBoundaryType 1 = MUR_1ST (non-default UNDEFINED=0)
        # NormalSignPositive=False is non-default (default True)
        abc = self.csx.AddAbsorbingBC('abc',
                                       NormalSignPositive=False,
                                       PhaseVelocity=3e8,
                                       AbsorbingBoundaryType=1)
        abc.AddBox([0, 0, 0], [1, 1, 1])

        p2 = self._prop(self._roundtrip(), 'abc')
        self.assertEqual(p2.GetTypeString(), 'AbsorbingBC')
        self.assertFalse(p2.GetNormalSignPositive())
        self.assertAlmostEqual(p2.GetPhaseVelocity(), 3e8)
        self.assertEqual(p2.GetAbsorbingBoundaryType(), 1)

    # ==================================================================
    # CSPropExcitation
    # ==================================================================

    def test_excitation(self):
        exc = self.csx.AddExcitation('exc', 1, [1.0, 0.0, 0.0], delay=1e-9)
        exc.SetPropagationDir([0.0, 0.0, 1.0])
        exc.SetWeightFunction(['sin(x)', 'cos(y)', 'z'])
        exc.SetFrequency(2.4e9)
        exc.SetEnabled(False)           # default is True

        p2 = self._prop(self._roundtrip(), 'exc')
        self.assertEqual(p2.GetTypeString(), 'Excitation')
        self.assertEqual(p2.GetExcitType(), 1)
        np.testing.assert_array_almost_equal(p2.GetExcitation(),      [1.0, 0.0, 0.0])
        np.testing.assert_array_almost_equal(p2.GetPropagationDir(),  [0.0, 0.0, 1.0])
        self.assertEqual(p2.GetWeightFunction(), ['sin(x)', 'cos(y)', 'z'])
        self.assertAlmostEqual(p2.GetFrequency(), 2.4e9)
        self.assertAlmostEqual(p2.GetDelay(),     1e-9)
        self.assertFalse(p2.GetEnabled())

    # ==================================================================
    # CSPropProbeBox
    # ==================================================================

    def test_probe_box(self):
        probe = self.csx.AddProbe('probe', 2, weight=2.5, norm_dir=1)
        probe.AddFrequency([1e9, 2e9, 3e9])
        probe.SetModeFunction(['sin(x)', 'cos(y)', '1'])

        p2 = self._prop(self._roundtrip(), 'probe')
        self.assertEqual(p2.GetTypeString(), 'ProbeBox')
        self.assertEqual(p2.GetProbeType(),  2)
        self.assertAlmostEqual(p2.GetWeighting(), 2.5)
        self.assertEqual(p2.GetNormalDir(), 1)
        np.testing.assert_array_almost_equal(p2.GetFrequency(), [1e9, 2e9, 3e9])
        self.assertEqual(p2.GetAttributeValue('ModeFunctionX'), 'sin(x)')
        self.assertEqual(p2.GetAttributeValue('ModeFunctionY'), 'cos(y)')
        self.assertEqual(p2.GetAttributeValue('ModeFunctionZ'), '1')

    # ==================================================================
    # CSPropDumpBox
    # ==================================================================

    def test_dump_box(self):
        dump = self.csx.AddDump('dump', dump_type=10, dump_mode=2, file_type=1)
        dump.SetOptResolution([0.1, 0.2, 0.3])
        dump.SetSubSampling([2, 3, 4])
        dump.AddFrequency([1e9, 2e9])

        p2 = self._prop(self._roundtrip(), 'dump')
        self.assertEqual(p2.GetTypeString(), 'DumpBox')
        self.assertEqual(p2.GetDumpType(), 10)
        self.assertEqual(p2.GetDumpMode(),  2)
        self.assertEqual(p2.GetFileType(),  1)
        np.testing.assert_array_almost_equal(p2.GetOptResolution(), [0.1, 0.2, 0.3])
        np.testing.assert_array_almost_equal(p2.GetSubSampling(),   [2,   3,   4  ])
        np.testing.assert_array_almost_equal(p2.GetFrequency(),     [1e9, 2e9])

    # ==================================================================
    # CSPropLorentzMaterial
    # ==================================================================

    def test_lorentz_material(self):
        mat = CSPropLorentzMaterial(self.csx.GetParameterSet(), epsilon=2.0, order=1)
        self.csx.AddProperty(mat)
        mat.SetName('lorentz')
        mat.SetDispersiveMaterialProperty(0,
            eps_plasma=1e9, eps_pole_freq=2e9, eps_relax=1e-9,
            mue_plasma=3e9, mue_pole_freq=4e9, mue_relax=2e-9)
        mat.SetDispersiveMaterialWeight(0, eps_plasma='x+1', mue_plasma='y*2')

        p2 = self._prop(self._roundtrip(), 'lorentz')
        self.assertEqual(p2.GetTypeString(), 'LorentzMaterial')
        self.assertEqual(p2.GetDispersionOrder(), 1)
        self.assertAlmostEqual(p2.GetMaterialProperty('epsilon'), 2.0)
        self.assertAlmostEqual(p2.GetDispersiveMaterialProperty('eps_plasma',   0), 1e9)
        self.assertAlmostEqual(p2.GetDispersiveMaterialProperty('eps_pole_freq',0), 2e9)
        self.assertAlmostEqual(p2.GetDispersiveMaterialProperty('eps_relax',    0), 1e-9)
        self.assertAlmostEqual(p2.GetDispersiveMaterialProperty('mue_plasma',   0), 3e9)
        self.assertAlmostEqual(p2.GetDispersiveMaterialProperty('mue_pole_freq',0), 4e9)
        self.assertAlmostEqual(p2.GetDispersiveMaterialProperty('mue_relax',    0), 2e-9)
        self.assertEqual(p2.GetDispersiveMaterialWeight('eps_plasma', 0), 'x+1')
        self.assertEqual(p2.GetDispersiveMaterialWeight('mue_plasma', 0), 'y*2')

    # ==================================================================
    # CSPropDebyeMaterial
    # ==================================================================

    def test_debye_material(self):
        mat = CSPropDebyeMaterial(self.csx.GetParameterSet(), epsilon=1.5, order=1)
        self.csx.AddProperty(mat)
        mat.SetName('debye')
        mat.SetDispersiveMaterialProperty(0, eps_delta=0.5, eps_relax=1e-12)
        mat.SetDispersiveMaterialWeight(0, eps_delta='y*2', eps_relax='z+1')

        p2 = self._prop(self._roundtrip(), 'debye')
        self.assertEqual(p2.GetTypeString(), 'DebyeMaterial')
        self.assertEqual(p2.GetDispersionOrder(), 1)
        self.assertAlmostEqual(p2.GetMaterialProperty('epsilon'), 1.5)
        self.assertAlmostEqual(p2.GetDispersiveMaterialProperty('eps_delta', 0), 0.5)
        self.assertAlmostEqual(p2.GetDispersiveMaterialProperty('eps_relax', 0), 1e-12)
        self.assertEqual(p2.GetDispersiveMaterialWeight('eps_delta', 0), 'y*2')
        self.assertEqual(p2.GetDispersiveMaterialWeight('eps_relax', 0), 'z+1')

    # ==================================================================
    # Primitives — geometry and priority
    # ==================================================================

    def test_prim_point(self):
        self.csx.AddMetal('pt').AddPoint([1.5, 2.5, 3.5])
        pt = self._prop(self._roundtrip(), 'pt').GetPrimitive(0)
        self.assertEqual(pt.GetTypeName(), 'Point')
        np.testing.assert_array_almost_equal(pt.GetCoord(), [1.5, 2.5, 3.5])

    def test_prim_box(self):
        self.csx.AddMetal('box').AddBox([1, 2, 3], [4, 5, 6], priority=3)
        b = self._prop(self._roundtrip(), 'box').GetPrimitive(0)
        self.assertEqual(b.GetTypeName(), 'Box')
        self.assertEqual(b.GetPriority(), 3)        # non-default (0)
        np.testing.assert_array_almost_equal(b.GetStart(), [1, 2, 3])
        np.testing.assert_array_almost_equal(b.GetStop(),  [4, 5, 6])

    def test_prim_cylinder(self):
        self.csx.AddMetal('cyl').AddCylinder([1, 2, 3], [4, 5, 6], 1.5)
        c = self._prop(self._roundtrip(), 'cyl').GetPrimitive(0)
        self.assertEqual(c.GetTypeName(), 'Cylinder')
        np.testing.assert_array_almost_equal(c.GetStart(), [1, 2, 3])
        np.testing.assert_array_almost_equal(c.GetStop(),  [4, 5, 6])
        self.assertAlmostEqual(c.GetRadius(), 1.5)

    def test_prim_cylindrical_shell(self):
        self.csx.AddMetal('cylsh').AddCylindricalShell([0, 0, 0], [0, 0, 5], 2.0, 0.3)
        c = self._prop(self._roundtrip(), 'cylsh').GetPrimitive(0)
        self.assertEqual(c.GetTypeName(), 'CylindricalShell')
        self.assertAlmostEqual(c.GetRadius(),     2.0)
        self.assertAlmostEqual(c.GetShellWidth(), 0.3)

    def test_prim_sphere(self):
        self.csx.AddMetal('sph').AddSphere([1, 2, 3], 0.5)
        s = self._prop(self._roundtrip(), 'sph').GetPrimitive(0)
        self.assertEqual(s.GetTypeName(), 'Sphere')
        np.testing.assert_array_almost_equal(s.GetCenter(), [1, 2, 3])
        self.assertAlmostEqual(s.GetRadius(), 0.5)

    def test_prim_spherical_shell(self):
        self.csx.AddMetal('sphsh').AddSphericalShell([1, 2, 3], 2.0, 0.2)
        s = self._prop(self._roundtrip(), 'sphsh').GetPrimitive(0)
        self.assertEqual(s.GetTypeName(), 'SphericalShell')
        np.testing.assert_array_almost_equal(s.GetCenter(), [1, 2, 3])
        self.assertAlmostEqual(s.GetRadius(),     2.0)
        self.assertAlmostEqual(s.GetShellWidth(), 0.2)

    def test_prim_polygon(self):
        x0 = [0.0, 1.0, 0.5]
        x1 = [0.0, 0.0, 1.0]
        self.csx.AddMetal('poly').AddPolygon([x0, x1], norm_dir=2, elevation=1.5)
        pg = self._prop(self._roundtrip(), 'poly').GetPrimitive(0)
        self.assertEqual(pg.GetTypeName(), 'Polygon')
        self.assertEqual(pg.GetNormDir(), 2)        # non-default (0)
        self.assertAlmostEqual(pg.GetElevation(), 1.5)
        rx0, rx1 = pg.GetCoords()
        np.testing.assert_array_almost_equal(rx0, x0)
        np.testing.assert_array_almost_equal(rx1, x1)

    def test_prim_linpoly(self):
        x0, x1 = [0.0, 1.0, 0.5], [0.0, 0.0, 1.0]
        self.csx.AddMetal('lp').AddLinPoly([x0, x1], norm_dir=1, elevation=0.5, length=2.0)
        lp = self._prop(self._roundtrip(), 'lp').GetPrimitive(0)
        self.assertEqual(lp.GetTypeName(), 'LinPoly')
        self.assertEqual(lp.GetNormDir(), 1)
        self.assertAlmostEqual(lp.GetElevation(), 0.5)
        self.assertAlmostEqual(lp.GetLength(),    2.0)

    def test_prim_rotpoly(self):
        x0, x1 = [0.0, 1.0, 0.5], [0.0, 0.0, 1.0]
        self.csx.AddMetal('rp').AddRotPoly([x0, x1], norm_dir=2, elevation=0.0,
                                            rot_axis=0, angle=[0.1, math.pi])
        rp = self._prop(self._roundtrip(), 'rp').GetPrimitive(0)
        self.assertEqual(rp.GetTypeName(), 'RotPoly')
        self.assertEqual(rp.GetRotAxisDir(), 0)
        a0, a1 = rp.GetAngle()
        self.assertAlmostEqual(a0, 0.1, places=5)
        self.assertAlmostEqual(a1, math.pi, places=5)

    def test_prim_curve(self):
        self.csx.AddMetal('crv').AddCurve([[0, 1, 2, 3], [4, 5, 6, 7], [8, 9, 10, 11]])
        c = self._prop(self._roundtrip(), 'crv').GetPrimitive(0)
        self.assertEqual(c.GetTypeName(), 'Curve')
        self.assertEqual(c.GetNumberOfPoints(), 4)
        np.testing.assert_array_almost_equal(c.GetPoint(0), [0, 4,  8])
        np.testing.assert_array_almost_equal(c.GetPoint(3), [3, 7, 11])

    def test_prim_wire(self):
        self.csx.AddMetal('wire').AddWire([[0, 1, 2], [0, 0, 0], [0, 0, 0]], radius=0.25)
        w = self._prop(self._roundtrip(), 'wire').GetPrimitive(0)
        self.assertEqual(w.GetTypeName(), 'Wire')
        self.assertAlmostEqual(w.GetWireRadius(), 0.25)

    def test_prim_polyhedron(self):
        ph = self.csx.AddMetal('ph').AddPolyhedron()
        ph.AddVertex(0, 0, 0); ph.AddVertex(1, 0, 0)
        ph.AddVertex(0, 1, 0); ph.AddVertex(0, 0, 1)
        ph.AddFace([0, 1, 2]); ph.AddFace([0, 1, 3])
        ph.AddFace([0, 2, 3]); ph.AddFace([1, 2, 3])

        ph2 = self._prop(self._roundtrip(), 'ph').GetPrimitive(0)
        self.assertEqual(ph2.GetTypeName(), 'Polyhedron')
        self.assertEqual(ph2.GetNumVertices(), 4)
        self.assertEqual(ph2.GetNumFaces(),    4)
        np.testing.assert_array_almost_equal(ph2.GetVertex(0), [0, 0, 0])
        np.testing.assert_array_almost_equal(ph2.GetVertex(1), [1, 0, 0])
        np.testing.assert_array_equal(ph2.GetFace(0), [0, 1, 2])

    def test_prim_polyhedron_reader(self):
        stl_fn = os.path.join(tempfile.gettempdir(), 'test_phr.stl')
        with open(stl_fn, 'w') as f:
            f.write('solid test\n'
                    '  facet normal 0 0 1\n'
                    '    outer loop\n'
                    '      vertex 0 0 0\n'
                    '      vertex 1 0 0\n'
                    '      vertex 0 1 0\n'
                    '    endloop\n'
                    '  endfacet\n'
                    'endsolid test\n')
        try:
            self.csx.AddMetal('phr').AddPolyhedronReader(stl_fn)
            ph2 = self._prop(self._roundtrip(), 'phr').GetPrimitive(0)
            self.assertEqual(ph2.GetTypeName(), 'PolyhedronReader')
            self.assertEqual(_decode(ph2.GetFilename()), stl_fn)
            self.assertEqual(ph2.GetFileType(), 1)  # auto-set from .stl extension
        finally:
            if os.path.exists(stl_fn):
                os.remove(stl_fn)

    # ==================================================================
    # Primitive transform
    # ==================================================================

    def test_prim_transform_translate(self):
        box = self.csx.AddMetal('tr').AddBox([0, 0, 0], [1, 1, 1])
        box.AddTransform('Translate', [1.5, 2.5, 3.5])

        b2 = self._prop(self._roundtrip(), 'tr').GetPrimitive(0)
        self.assertTrue(b2.HasTransform())
        m = b2.GetTransform().GetMatrix()
        self.assertAlmostEqual(m[0, 3], 1.5)
        self.assertAlmostEqual(m[1, 3], 2.5)
        self.assertAlmostEqual(m[2, 3], 3.5)

    def test_prim_transform_scale(self):
        box = self.csx.AddMetal('sc').AddBox([0, 0, 0], [1, 1, 1])
        box.AddTransform('Scale', 3.0)

        b2 = self._prop(self._roundtrip(), 'sc').GetPrimitive(0)
        self.assertTrue(b2.HasTransform())
        m = b2.GetTransform().GetMatrix()
        self.assertAlmostEqual(m[0, 0], 3.0)
        self.assertAlmostEqual(m[1, 1], 3.0)
        self.assertAlmostEqual(m[2, 2], 3.0)

    # ==================================================================
    # Primitive coordinate system
    # ==================================================================

    def test_prim_coordinate_system(self):
        box = self.csx.AddMetal('cs').AddBox([0, 0, 0], [1, 1, 1])
        box.SetCoordinateSystem(CSRectGrid.CoordinateSystem.CYLINDRICAL)  # non-default

        b2 = self._prop(self._roundtrip(), 'cs').GetPrimitive(0)
        self.assertEqual(b2.GetCoordinateSystem(), CSRectGrid.CoordinateSystem.CYLINDRICAL)


    # ==================================================================
    # Grid disc-lines
    # ==================================================================

    def test_grid_roundtrip(self):
        grid = self.csx.GetGrid()
        grid.SetDeltaUnit(1e-3)
        grid.AddLine('x', -10.0)
        grid.AddLine('x',   0.0)
        grid.AddLine('x',  10.0)
        grid.AddLine('y',  -5.0)
        grid.AddLine('y',   5.0)
        grid.AddLine('z',   0.0)
        grid.AddLine('z',   2.0)

        csx2 = self._roundtrip()
        g2 = csx2.GetGrid()
        self.assertAlmostEqual(g2.GetDeltaUnit(), 1e-3)
        self.assertEqual(g2.GetQtyLines('x'), 3)
        self.assertEqual(g2.GetQtyLines('y'), 2)
        self.assertEqual(g2.GetQtyLines('z'), 2)
        np.testing.assert_array_almost_equal(g2.GetLines('x'), [-10.0, 0.0, 10.0])
        np.testing.assert_array_almost_equal(g2.GetLines('y'), [-5.0, 5.0])
        np.testing.assert_array_almost_equal(g2.GetLines('z'), [0.0, 2.0])

    # ==================================================================
    # Primitive transform — RotateAxis
    # ==================================================================

    def test_prim_transform_rotate_axis(self):
        box = self.csx.AddMetal('rot').AddBox([0, 0, 0], [1, 1, 1])
        box.AddTransform('RotateAxis', 'z', 45.0)  # degrees

        b2 = self._prop(self._roundtrip(), 'rot').GetPrimitive(0)
        self.assertTrue(b2.HasTransform())
        m = b2.GetTransform().GetMatrix()
        c = math.cos(math.radians(45.0))
        s = math.sin(math.radians(45.0))
        self.assertAlmostEqual(m[0, 0],  c, places=5)
        self.assertAlmostEqual(m[0, 1], -s, places=5)
        self.assertAlmostEqual(m[1, 0],  s, places=5)
        self.assertAlmostEqual(m[1, 1],  c, places=5)

    # ==================================================================
    # DumpBox NormalDir
    # ==================================================================

    def test_dump_normal_dir(self):
        dump = self.csx.AddDump('dump_nd', dump_type=2)
        dump.SetNormalDir(2)            # non-default (-1)

        p2 = self._prop(self._roundtrip(), 'dump_nd')
        self.assertEqual(p2.GetNormalDir(), 2)


class TestBackgroundMaterial(unittest.TestCase):
    def setUp(self):
        self.csx = ContinuousStructure()
        self.fn  = os.path.join(tempfile.gettempdir(), 'test_bgmat.xml')

    def tearDown(self):
        if os.path.exists(self.fn):
            os.remove(self.fn)

    def test_defaults(self):
        bg = self.csx.GetBackgroundMaterial()
        self.assertAlmostEqual(bg.GetEpsilon(), 1.0)
        self.assertAlmostEqual(bg.GetMue(),     1.0)
        self.assertAlmostEqual(bg.GetKappa(),   0.0)
        self.assertAlmostEqual(bg.GetSigma(),   0.0)

    def test_roundtrip(self):
        bg = self.csx.GetBackgroundMaterial()
        bg.SetEpsilon(2.5)
        bg.SetMue(1.5)
        bg.SetKappa(0.01)
        bg.SetSigma(0.02)

        self.assertTrue(self.csx.Write2XML(self.fn))
        csx2 = ContinuousStructure()
        self.assertEqual(csx2.ReadFromXML(self.fn), '')
        bg2 = csx2.GetBackgroundMaterial()
        self.assertAlmostEqual(bg2.GetEpsilon(), 2.5)
        self.assertAlmostEqual(bg2.GetMue(),     1.5)
        self.assertAlmostEqual(bg2.GetKappa(),   0.01)
        self.assertAlmostEqual(bg2.GetSigma(),   0.02)

    def test_reset(self):
        bg = self.csx.GetBackgroundMaterial()
        bg.SetEpsilon(3.0)
        bg.Reset()
        self.assertAlmostEqual(bg.GetEpsilon(), 1.0)

    def test_direct_instantiation_raises(self):
        from CSXCAD.CSXCAD import CSBackgroundMaterial
        self.assertRaises(TypeError, CSBackgroundMaterial)


class TestDiscMaterial(unittest.TestCase):
    def setUp(self):
        self.csx = ContinuousStructure()
        self.fn  = os.path.join(tempfile.gettempdir(), 'test_disc.xml')

    def tearDown(self):
        if os.path.exists(self.fn):
            os.remove(self.fn)

    def test_roundtrip_attributes(self):
        disc = self.csx.AddDiscMaterial('disc',
                                        filename='materials.h5',
                                        scale=1e-3,
                                        use_db_background=False)
        self.assertEqual(disc.GetTypeString(), 'DiscMaterial')
        self.assertEqual(disc.GetFilename(),   'materials.h5')
        self.assertAlmostEqual(disc.GetScale(), 1e-3)
        self.assertFalse(disc.GetUseDataBaseForBackground())

        self.assertTrue(self.csx.Write2XML(self.fn))
        csx2 = ContinuousStructure()
        # ReadFromXML will warn that materials.h5 is missing — that is expected
        csx2.ReadFromXML(self.fn)
        props = csx2.GetPropertiesByName('disc')
        self.assertEqual(len(props), 1)
        d2 = props[0]
        self.assertEqual(d2.GetTypeString(), 'DiscMaterial')
        self.assertEqual(d2.GetFilename(),   'materials.h5')
        self.assertAlmostEqual(d2.GetScale(), 1e-3)
        self.assertFalse(d2.GetUseDataBaseForBackground())


class TestMultiBox(unittest.TestCase):
    def setUp(self):
        self.csx = ContinuousStructure()
        self.fn  = os.path.join(tempfile.gettempdir(), 'test_multibox.xml')

    def tearDown(self):
        if os.path.exists(self.fn):
            os.remove(self.fn)

    def _roundtrip(self):
        self.assertTrue(self.csx.Write2XML(self.fn))
        csx2 = ContinuousStructure()
        self.assertEqual(csx2.ReadFromXML(self.fn), '')
        return csx2

    def test_add_and_get(self):
        m  = self.csx.AddMetal('mb')
        mb = m.AddMultiBox(boxes=[([0, 0, 0], [1, 1, 1]),
                                  ([2, 2, 2], [3, 3, 3])])
        self.assertEqual(mb.GetTypeName(),  'MultiBox')
        self.assertEqual(mb.GetQtyBoxes(), 2)
        s0, e0 = mb.GetBox(0)
        np.testing.assert_array_almost_equal(s0, [0, 0, 0])
        np.testing.assert_array_almost_equal(e0, [1, 1, 1])
        s1, e1 = mb.GetBox(1)
        np.testing.assert_array_almost_equal(s1, [2, 2, 2])
        np.testing.assert_array_almost_equal(e1, [3, 3, 3])

    def test_roundtrip(self):
        m  = self.csx.AddMetal('mb')
        m.AddMultiBox(boxes=[([0, 0, 0], [1, 1, 1]),
                             ([2, 2, 2], [3, 3, 3])], priority=2)
        csx2 = self._roundtrip()
        mb2  = csx2.GetPropertiesByName('mb')[0].GetPrimitive(0)
        self.assertEqual(mb2.GetTypeName(),  'MultiBox')
        self.assertEqual(mb2.GetQtyBoxes(), 2)
        self.assertEqual(mb2.GetPriority(), 2)
        s0, e0 = mb2.GetBox(0)
        np.testing.assert_array_almost_equal(s0, [0, 0, 0])
        np.testing.assert_array_almost_equal(e0, [1, 1, 1])

    def test_delete_box(self):
        m  = self.csx.AddMetal('mb2')
        mb = m.AddMultiBox(boxes=[([0, 0, 0], [1, 1, 1]),
                                  ([2, 2, 2], [3, 3, 3]),
                                  ([4, 4, 4], [5, 5, 5])])
        self.assertEqual(mb.GetQtyBoxes(), 3)
        mb.DeleteBox(1)
        self.assertEqual(mb.GetQtyBoxes(), 2)


if __name__ == '__main__':
    unittest.main()
