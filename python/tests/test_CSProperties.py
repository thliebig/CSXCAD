import os
import tempfile
import unittest

import numpy as np

from CSXCAD import ParameterObjects
from CSXCAD import CSProperties, CSPrimitives, CSRectGrid
from CSXCAD.CSProperties import CSPropDiscMaterial
from CSXCAD.CSXCAD import ContinuousStructure

try:
    import h5py
    HAS_H5PY = True
except ImportError:
    HAS_H5PY = False

try:
    from scipy.interpolate import RegularGridInterpolator
    HAS_SCIPY = True
except ImportError:
    HAS_SCIPY = False


def write_mode_hdf5(path, x, y, Vx, Vy):
    """Write a mode HDF5 file. Vx/Vy must have shape (nx, ny)."""
    with h5py.File(path, 'w') as f:
        f.attrs['Version'] = 1.0
        f.create_dataset('x', data=x)
        f.create_dataset('y', data=y)
        f.create_dataset('Vx', data=Vx)
        f.create_dataset('Vy', data=Vy)


class Test_CSPrimMethods(unittest.TestCase):
    def setUp(self):
        self.pset  = ParameterObjects.ParameterSet()

    def test_add_primitives(self):
        prop = CSProperties.CSPropMetal(self.pset)
        box = prop.AddBox(start= [0,0,0] , stop=[1,1,1])

        self.assertEqual( prop.GetQtyPrimitives(), 1)
        self.assertEqual( id(prop.GetPrimitive(0)), id(box))

        self.assertEqual( prop.GetPrimitive(0).GetType(), CSPrimitives.PrimitiveType.BOX)
        self.assertEqual( len(prop.GetAllPrimitives()), 1)
        self.assertEqual( prop.GetAllPrimitives(), [box,])

        prim = prop.AddCylinder(start= [0,0,0] , stop=[1,1,1], radius=0.5)
        self.assertEqual( prop.GetQtyPrimitives(), 2)
        self.assertEqual( prop.GetPrimitive(1).GetType(), CSPrimitives.PrimitiveType.CYLINDER)

        prim = prop.AddCylindricalShell(start= [0,0,0] , stop=[1,1,1], radius=0.5, shell_width=0.1)
        self.assertEqual( prop.GetQtyPrimitives(), 3)
        self.assertEqual( prop.GetPrimitive(2).GetType(), CSPrimitives.PrimitiveType.CYLINDRICALSHELL)

        prim = prop.AddSphere(center=[1,1,1], radius=0.5)
        self.assertEqual( prop.GetQtyPrimitives(), 4)
        self.assertEqual( prop.GetPrimitive(3).GetType(), CSPrimitives.PrimitiveType.SPHERE)

        prim = prop.AddSphericalShell(center=[1,1,1], radius=0.5, shell_width=0.1)
        self.assertEqual( prop.GetQtyPrimitives(), 5)
        self.assertEqual( prop.GetPrimitive(4).GetType(), CSPrimitives.PrimitiveType.SPHERICALSHELL)

        self.assertEqual( len(prop.GetAllPrimitives()), 5)

        self.assertEqual(prop.GetPrimitive(-1), prop.GetPrimitive(prop.GetQtyPrimitives()-1))

        self.assertEqual(prop.GetPrimitive(10), None)

        prop2 = prop.copy()
        self.assertEqual(prop2.GetQtyPrimitives(), 0)

        prop3 = prop.copy(True) # copy prim too
        self.assertEqual(prop3.GetQtyPrimitives(), 5)
        self.assertEqual(prop3.GetQtyPrimitives(), prop.GetQtyPrimitives())
        for n in range(5):
            prim1 = prop.GetPrimitive(n)
            prim2 = prop3.GetPrimitive(n)
            self.assertNotEqual(prim1, prim2)
            self.assertEqual(prim1.GetType(), prim2.GetType())
        
        prim3 = prop3.GetPrimitive(4)
        self.assertEqual(prim3.GetType(), CSPrimitives.PrimitiveType.SPHERICALSHELL)
        self.assertEqual(prim3.GetRadius(), 0.5)

    def test_attributes(self):
        prop = CSProperties.CSPropMaterial(self.pset, name='material_1')

        self.assertEqual(prop.GetName(), 'material_1')
        prop.SetName('new_name')
        self.assertEqual(prop.GetName(), 'new_name')

        prop.SetAttributeValue('my_attr1', 'value1')
        self.assertTrue(prop.ExistAttribute('my_attr1'))
        self.assertFalse(prop.ExistAttribute('my_attr2'))
        self.assertEqual(prop.GetAttributeValue('my_attr1'), 'value1')

        prop.SetAttributeValue('my_attr2', 'value2')
        prop.SetAttributeValue('my_attr3', 'value3')
        prop.SetAttributeValue('my_attr4', 'value4')

        prop.SetAttributeValue('my_attr1', 'value_new')
        self.assertEqual(prop.GetAttributeValue('my_attr1'), 'value_new')

        self.assertEqual(prop.GetAttributeValue('my_attr3'), 'value3')
        prop.RemoveAttribute('my_attr3')
        self.assertFalse(prop.ExistAttribute('my_attr3'))
        self.assertEqual(prop.GetAttributeValue('my_attr3'), '')

        names = prop.GetAttributeNames()
        self.assertEqual(names, ('my_attr1', 'my_attr2', 'my_attr4'))
        attrs = prop.GetAttributes()
        self.assertEqual(attrs, {'my_attr1': 'value_new', 'my_attr2': 'value2', 'my_attr4': 'value4'})

        prop.AddAttribute('test_add', 'added_value')
        self.assertTrue(prop.ExistAttribute('test_add'))
        self.assertEqual(prop.GetAttributeValue('test_add'), 'added_value')


        # test that attributes are copied!
        prop2 = prop.copy()
        self.assertEqual(prop2.GetName(), 'new_name')
        self.assertEqual(prop2.GetAttributeValue('my_attr1'), 'value_new')
        self.assertEqual(prop.GetAttributeNames(), prop2.GetAttributeNames())
        self.assertEqual(prop.GetAttributes(), prop2.GetAttributes())

        prop.SetAttributeValue('my_attr1', 'value_new_again')
        self.assertEqual(prop2.GetAttributeValue('my_attr1'), 'value_new')
        self.assertNotEqual(prop.GetAttributes(), prop2.GetAttributes())

    def test_colors(self):
        prop = CSProperties.CSPropMaterial(self.pset)
        prop.SetColor('red')
        self.assertEqual(prop.GetFillColor(), (255,0,0,255))
        self.assertEqual(prop.GetEdgeColor(), (255,0,0,255))
        prop.SetColor('red', 128) # 50% transparency
        self.assertEqual(prop.GetFillColor(), (255,0,0,128))
        self.assertEqual(prop.GetEdgeColor(), (255,0,0,128))
        prop.SetColor('#00ff00') # green
        self.assertEqual(prop.GetFillColor(), (0,255,0,255))
        self.assertEqual(prop.GetEdgeColor(), (0,255,0,255))
        prop.SetColor('#00ff00', 128) # green + 50% transparency
        self.assertEqual(prop.GetFillColor(), (0,255,0,128))
        self.assertEqual(prop.GetEdgeColor(), (0,255,0,128))

        prop.SetColor((128, 128, 128)) # grey
        self.assertEqual(prop.GetFillColor(), (128,128,128,255))
        self.assertEqual(prop.GetEdgeColor(), (128,128,128,255))
        prop.SetColor((128, 128, 128), 111) # grey
        self.assertEqual(prop.GetFillColor(), (128,128,128,111))
        self.assertEqual(prop.GetEdgeColor(), (128,128,128,111))

        prop.SetFillColor([100, 150, 200]) # grey
        self.assertEqual(prop.GetFillColor(), (100,150,200,255))
        self.assertEqual(prop.GetEdgeColor(), (128,128,128,111))
        prop.SetEdgeColor([200, 210, 220], 50) # grey
        self.assertEqual(prop.GetFillColor(), (100,150,200,255))
        self.assertEqual(prop.GetEdgeColor(), (200,210,220, 50))

        prop2 = prop.copy()
        self.assertFalse(prop2 == prop)
        self.assertEqual(prop.GetVisibility(), prop2.GetVisibility())
        prop2.SetVisibility(False)
        self.assertNotEqual(prop.GetVisibility(), prop2.GetVisibility())
        self.assertEqual(prop2.GetFillColor(), (100,150,200,255))
        self.assertEqual(prop2.GetEdgeColor(), (200,210,220, 50))

        prop.SetColor((128, 128, 128)) # change prop color --> should not affect prop2
        self.assertEqual(prop2.GetFillColor(), (100,150,200,255))
        self.assertEqual(prop2.GetEdgeColor(), (200,210,220, 50))

        del prop
        self.assertEqual(prop2.GetFillColor(), (100,150,200,255))
        self.assertEqual(prop2.GetEdgeColor(), (200,210,220, 50))

    def test_visibility(self):
        prop = CSProperties.CSPropMetal(self.pset)
        self.assertTrue(prop.GetVisibility())
        prop.SetVisibility(False)
        self.assertFalse(prop.GetVisibility())
        prop.SetVisibility(True)
        self.assertTrue(prop.GetVisibility())

        prop2 = prop.copy()
        self.assertEqual(prop.GetVisibility(), prop2.GetVisibility())
        prop.SetVisibility(False)
        self.assertNotEqual(prop.GetVisibility(), prop2.GetVisibility())

    def test_metal(self):
        prop = CSProperties.CSPropMetal(self.pset)

        self.assertTrue(prop.GetMaterial())
        self.assertEqual( prop.GetType(), CSProperties.PropertyType.METAL)
        self.assertEqual( prop.GetTypeString(), 'Metal')

        prop2 = prop.copy()
        self.assertEqual( prop2.GetType(), CSProperties.PropertyType.METAL)
        self.assertEqual( prop2.GetTypeString(), 'Metal')

    def test_material(self):
        prop = CSProperties.CSPropMaterial(self.pset, epsilon = 5.0)

        self.assertTrue(prop.GetMaterial())
        self.assertEqual( prop.GetType(), CSProperties.PropertyType.MATERIAL)
        self.assertEqual( prop.GetTypeString(), 'Material')

        self.assertEqual( prop.GetMaterialProperty('epsilon'), 5.0)
        self.assertEqual( prop.GetMaterialProperty('mue'), 1.0)

#        print(CSX.GetQtyProperties())

        prop.SetMaterialProperty(epsilon=[1.0, 2.0, 3.0], mue=2.0)
        self.assertEqual( prop.GetMaterialProperty('epsilon'), 1.0)
        self.assertEqual( prop.GetMaterialProperty('mue'), 2.0)

        prop.SetMaterialWeight(epsilon=['sin(x)', 'y', 'z'],  mue='cos(y)', density='z*z')
        self.assertEqual( prop.GetMaterialWeight('epsilon'), 'sin(x)')
        self.assertEqual( prop.GetMaterialWeight('mue'), 'cos(y)')
        self.assertEqual( prop.GetMaterialWeight('density'), 'z*z')

        prop.SetIsotropy(False)
        self.assertFalse( prop.GetIsotropy(),False)
        self.assertTrue( (prop.GetMaterialProperty('epsilon')==[1.0, 2.0, 3.0]).all())

        prop.SetMaterialProperty(epsilon=[5.0, 2.0, 3.0], mue=2.0)

        prop2 = prop.copy()
        self.assertFalse( prop.GetIsotropy(),False)
        self.assertTrue( (prop2.GetMaterialProperty('epsilon')==[5.0, 2.0, 3.0]).all())
        prop2.SetIsotropy(True)
        self.assertEqual( prop2.GetMaterialProperty('mue'), 2)
        self.assertEqual( prop2.GetMaterialWeight('mue'), 'cos(y)')
        self.assertEqual( prop2.GetMaterialWeight('density'), 'z*z')

    def test_lumped_elem(self):
        prop = CSProperties.CSPropLumpedElement(self.pset, R = 50, C=1e-12, caps=True, ny='x')

        self.assertFalse(prop.GetMaterial())
        self.assertEqual( prop.GetType(), CSProperties.PropertyType.LUMPED_ELEMENT)
        self.assertEqual( prop.GetTypeString(), 'LumpedElement')

        self.assertEqual( prop.GetResistance(), 50)
        prop.SetResistance(55.0)
        prop.SetDirection('x')
        self.assertEqual( prop.GetResistance(), 55)
        self.assertEqual( prop.GetCapacity(), 1e-12)
        self.assertTrue( prop.GetCaps())
        prop.SetCaps(False)
        self.assertFalse( prop.GetCaps())
        self.assertEqual( prop.GetDirection(), 0)

        prop2 = prop.copy()
        self.assertFalse(prop2.GetMaterial())
        self.assertEqual( prop2.GetType(), CSProperties.PropertyType.LUMPED_ELEMENT)
        self.assertEqual( prop2.GetTypeString(), 'LumpedElement')

        self.assertEqual( prop2.GetResistance(), 55)
        self.assertEqual( prop2.GetCapacity(), 1e-12)
        self.assertFalse( prop2.GetCaps())
        self.assertEqual( prop2.GetDirection(), 0)  

    def test_cond_sheet(self):
        prop = CSProperties.CSPropConductingSheet(self.pset, conductivity=56e6, thickness=35e-6)

        self.assertTrue(prop.GetMaterial())
        self.assertEqual( prop.GetType(), CSProperties.PropertyType.CONDUCTINGSHEET + CSProperties.PropertyType.METAL)
        self.assertEqual( prop.GetTypeString(), 'ConductingSheet')

        self.assertEqual( prop.GetConductivity(), 56e6)
        self.assertEqual( prop.GetThickness(), 35e-6)

        prop2 = prop.copy()
        self.assertTrue(prop2.GetMaterial())
        self.assertEqual( prop2.GetType(), CSProperties.PropertyType.CONDUCTINGSHEET + CSProperties.PropertyType.METAL)
        self.assertEqual( prop2.GetTypeString(), 'ConductingSheet')

        self.assertEqual( prop2.GetConductivity(), 56e6)
        self.assertEqual( prop2.GetThickness(), 35e-6)

    def test_excitation(self):
        prop = CSProperties.CSPropExcitation(self.pset, exc_type=1, exc_val=[-1.0, 0, 1.0], delay=1e-9)

        self.assertFalse(prop.GetMaterial())
        self.assertEqual( prop.GetType(), CSProperties.PropertyType.EXCITATION)
        self.assertEqual( prop.GetTypeString(), 'Excitation')

        self.assertEqual( prop.GetExcitType(), 1)
        self.assertTrue( (prop.GetExcitation()==[-1.0, 0, 1.0]).all() )
        self.assertEqual( prop.GetDelay(),1e-9)

        prop.SetWeightFunction(['y','x','z'])
        self.assertEqual(prop.GetWeightFunction(), ['y','x','z'])

        prop2 = prop.copy()
        self.assertEqual( prop2.GetType(), CSProperties.PropertyType.EXCITATION)
        self.assertEqual( prop2.GetTypeString(), 'Excitation')

        self.assertEqual( prop2.GetExcitType(), 1)
        self.assertTrue( (prop2.GetExcitation()==[-1.0, 0, 1.0]).all() )
        self.assertEqual( prop2.GetDelay(),1e-9)

        self.assertEqual(prop2.GetWeightFunction(), ['y','x','z'])

    def test_probe(self):
        prop = CSProperties.CSPropProbeBox(self.pset, frequency=[1e9, 2.4e9])

        self.assertFalse(prop.GetMaterial())
        self.assertEqual( prop.GetType(), CSProperties.PropertyType.PROBEBOX)
        self.assertEqual( prop.GetTypeString(), 'ProbeBox')

        self.assertEqual(prop.GetFrequencyCount(), 2)
        prop.AddFrequency(5e9)
        self.assertEqual(prop.GetFrequencyCount(), 3)
        self.assertTrue((prop.GetFrequency() == [1e9, 2.4e9, 5e9]).all())

        prop.ClearFrequency()
        self.assertEqual(prop.GetFrequencyCount(), 0)

        prop.SetFrequency(np.linspace(1e9,2e9,11))
        self.assertEqual(prop.GetFrequencyCount(), 11)

        prop2 = prop.copy()
        self.assertEqual( prop2.GetType(), CSProperties.PropertyType.PROBEBOX)
        self.assertEqual( prop2.GetTypeString(), 'ProbeBox')

        self.assertEqual(prop2.GetFrequencyCount(), 11)
        prop2.AddFrequency(5e9)
        self.assertEqual(prop2.GetFrequencyCount(), 12)
        self.assertEqual(prop.GetFrequencyCount(), 11)

    @unittest.skipUnless(HAS_H5PY, 'h5py not available')
    def test_excitation_weight_file(self):
        # Grid axes
        x = np.array([0.0, 0.1, 0.2, 0.3])
        y = np.array([0.0, 0.4, 0.8, 1.2])
        xv, yv = np.meshgrid(x, y, indexing='ij')  # shape (nx, ny)

        # Linear fields so bilinear interpolation is exact
        Vx = xv.copy()   # Vx[i,j] = x[i]
        Vy = yv.copy()   # Vy[i,j] = y[j]

        fd, path = tempfile.mkstemp(suffix='.h5')
        os.close(fd)
        try:
            write_mode_hdf5(path, x, y, Vx, Vy)

            # Unit excitation amplitudes so GetWeightedExcitation == interpolated field
            prop = CSProperties.CSPropExcitation(self.pset, exc_type=0,
                                                 exc_val=[1.0, 1.0, 1.0])
            prop.SetPropagationDir([0, 0, 1])   # z-propagation: transverse plane is x-y
            prop.SetWeightFile(path)
            self.assertEqual(prop.GetWeightFile(), path)

            for xi in np.linspace(x[0], x[-1], 7):
                for yi in np.linspace(y[0], y[-1], 7):
                    coords = [xi, yi, 0.0]
                    # ny=0 (x-component): should equal Vx(xi,yi) = xi
                    self.assertAlmostEqual(prop.GetWeightedExcitation(0, coords), xi, places=10)
                    # ny=1 (y-component): should equal Vy(xi,yi) = yi
                    self.assertAlmostEqual(prop.GetWeightedExcitation(1, coords), yi, places=10)
                    # ny=2 (propagation direction): always zero
                    self.assertAlmostEqual(prop.GetWeightedExcitation(2, coords), 0.0, places=10)
        finally:
            os.remove(path)

    @unittest.skipUnless(HAS_H5PY and HAS_SCIPY, 'h5py and scipy required')
    def test_excitation_weight_file_te10(self):
        # TE10 mode of a WR-90 rectangular waveguide (a=22.86mm, b=10.16mm),
        # propagating in z.  Transverse E-field: Vx=0, Vy=sin(pi*x/a).
        # The sinusoidal variation is non-linear, so this is a genuine test
        # of bilinear interpolation accuracy verified against scipy.
        a = 22.86e-3   # width  (x-direction)
        b = 10.16e-3   # height (y-direction)

        # Coarse mesh — intentionally sparse so interpolation error is visible
        x = np.linspace(0, a, 7)
        y = np.linspace(0, b, 5)
        xv, yv = np.meshgrid(x, y, indexing='ij')   # shape (nx, ny)
        Vx = np.zeros_like(xv)
        Vy = np.sin(np.pi * xv / a)

        fd, path = tempfile.mkstemp(suffix='.h5')
        os.close(fd)
        try:
            write_mode_hdf5(path, x, y, Vx, Vy)

            prop = CSProperties.CSPropExcitation(self.pset, exc_type=0,
                                                 exc_val=[1.0, 1.0, 1.0])
            prop.SetPropagationDir([0, 0, 1])
            prop.SetWeightFile(path)

            # scipy reference — same linear method, independent implementation
            ref_Vx = RegularGridInterpolator((x, y), Vx, method='linear')
            ref_Vy = RegularGridInterpolator((x, y), Vy, method='linear')

            # Fine test mesh offset from coarse grid so points fall between nodes
            x_test = np.linspace(x[0] + 0.3*(x[1]-x[0]), x[-1] - 0.3*(x[-1]-x[-2]), 17)
            y_test = np.linspace(y[0] + 0.3*(y[1]-y[0]), y[-1] - 0.3*(y[-1]-y[-2]), 11)

            for xi in x_test:
                for yi in y_test:
                    coords = [xi, yi, 0.0]
                    self.assertAlmostEqual(
                        prop.GetWeightedExcitation(0, coords),
                        ref_Vx([[xi, yi]]).item(), places=10)
                    self.assertAlmostEqual(
                        prop.GetWeightedExcitation(1, coords),
                        ref_Vy([[xi, yi]]).item(), places=10)
        finally:
            os.remove(path)

    def test_excitation_cylindrical_analytic(self):
        # With cylindrical input (rho, alpha, z), the analytic weight function
        # receives the converted Cartesian coords and also rho/a/r/theta.
        prop = CSProperties.CSPropExcitation(self.pset, exc_type=0, exc_val=[1.0, 1.0, 1.0])
        prop.SetPropagationDir([0, 0, 1])
        prop.SetCoordInputType(int(CSRectGrid.CoordinateSystem.CYLINDRICAL))
        self.assertEqual(prop.GetCoordInputType(), 1)

        prop.SetWeightFunction(['rho', 'a', '0'])   # rho and alpha (a) in drawing coords

        rho, alpha = 2.0, np.pi / 4
        # ny=0: weight = rho, returned as rho * exc_val[0] = rho
        self.assertAlmostEqual(prop.GetWeightedExcitation(0, [rho, alpha, 0.0]), rho)
        # ny=1: weight = a (alpha), returned as alpha * exc_val[1]
        self.assertAlmostEqual(prop.GetWeightedExcitation(1, [rho, alpha, 0.0]), alpha)

        # x = rho*cos(alpha), y = rho*sin(alpha) — weight 'x' should give that
        prop.SetWeightFunction(['x', 'y', '0'])
        self.assertAlmostEqual(prop.GetWeightedExcitation(0, [rho, alpha, 0.0]),
                               rho * np.cos(alpha))
        self.assertAlmostEqual(prop.GetWeightedExcitation(1, [rho, alpha, 0.0]),
                               rho * np.sin(alpha))

    @unittest.skipUnless(HAS_H5PY, 'h5py not available')
    def test_excitation_cylindrical_weight_file(self):
        # With cylindrical input and an HDF5 weight file the cylindrical coords
        # must be converted to Cartesian before interpolation.
        x = np.linspace(-1.0, 1.0, 9)
        y = np.linspace(-1.0, 1.0, 9)
        xv, yv = np.meshgrid(x, y, indexing='ij')
        Vx = xv.copy()   # Vx(x,y) = x
        Vy = yv.copy()   # Vy(x,y) = y

        fd, path = tempfile.mkstemp(suffix='.h5')
        os.close(fd)
        try:
            write_mode_hdf5(path, x, y, Vx, Vy)

            prop = CSProperties.CSPropExcitation(self.pset, exc_type=0, exc_val=[1.0, 1.0, 1.0])
            prop.SetPropagationDir([0, 0, 1])
            prop.SetCoordInputType(int(CSRectGrid.CoordinateSystem.CYLINDRICAL))
            prop.SetWeightFile(path)

            for rho in [0.3, 0.6]:
                for alpha in [0.0, np.pi/4, np.pi/2]:
                    cx = [rho, alpha, 0.0]   # cylindrical input
                    ex = rho * np.cos(alpha)  # expected Cartesian x
                    ey = rho * np.sin(alpha)  # expected Cartesian y
                    self.assertAlmostEqual(prop.GetWeightedExcitation(0, cx), ex, places=10)
                    self.assertAlmostEqual(prop.GetWeightedExcitation(1, cx), ey, places=10)
        finally:
            os.remove(path)

    @unittest.skipUnless(HAS_H5PY, 'h5py not available')
    def test_excitation_weight_file_copy(self):
        # Verify that copying a CSPropExcitation deep-copies the loaded mode data.
        # After the source file is deleted the copy must still interpolate correctly.
        x = np.array([0.0, 0.1, 0.2, 0.3])
        y = np.array([0.0, 0.4, 0.8, 1.2])
        xv, yv = np.meshgrid(x, y, indexing='ij')
        Vx = xv.copy()   # linear so bilinear interpolation is exact
        Vy = yv.copy()

        fd, path = tempfile.mkstemp(suffix='.h5')
        os.close(fd)
        write_mode_hdf5(path, x, y, Vx, Vy)

        prop = CSProperties.CSPropExcitation(self.pset, exc_type=0, exc_val=[1.0, 1.0, 1.0])
        prop.SetPropagationDir([0, 0, 1])
        prop.SetWeightFile(path)
        # trigger loading of the mode data into memory
        prop.GetWeightedExcitation(0, [0.05, 0.2, 0.0])

        prop2 = prop.copy()
        os.remove(path)   # source file gone — copy must rely on its own in-memory data

        self.assertEqual(prop2.GetWeightFile(), path)
        for xi in np.linspace(x[0], x[-1], 5):
            for yi in np.linspace(y[0], y[-1], 5):
                coords = [xi, yi, 0.0]
                self.assertAlmostEqual(prop2.GetWeightedExcitation(0, coords), xi, places=10)
                self.assertAlmostEqual(prop2.GetWeightedExcitation(1, coords), yi, places=10)
                self.assertAlmostEqual(prop2.GetWeightedExcitation(2, coords), 0.0, places=10)

    def test_dump(self):
        prop = CSProperties.CSPropDumpBox(self.pset, dump_type=10, dump_mode=2, file_type=5, opt_resolution=[10,11.5,12], sub_sampling=[1,2,4])

        self.assertFalse(prop.GetMaterial())
        self.assertEqual( prop.GetType(), CSProperties.PropertyType.DUMPBOX)
        self.assertEqual( prop.GetTypeString(), 'DumpBox')

        self.assertEqual(prop.GetDumpType(), 10)
        self.assertEqual(prop.GetDumpMode(), 2)
        self.assertEqual(prop.GetFileType(), 5)
        self.assertTrue((prop.GetOptResolution() == [10,11.5,12]).all())
        self.assertTrue((prop.GetSubSampling() == [1,2,4]).all())

        prop2 = prop.copy()
        self.assertEqual( prop2.GetType(), CSProperties.PropertyType.DUMPBOX)
        self.assertEqual( prop2.GetTypeString(), 'DumpBox')

        self.assertEqual(prop2.GetDumpType(), 10)
        self.assertEqual(prop2.GetDumpMode(), 2)
        self.assertEqual(prop2.GetFileType(), 5)
        self.assertTrue((prop2.GetOptResolution() == [10,11.5,12]).all())
        self.assertTrue((prop2.GetSubSampling() == [1,2,4]).all())

    def test_lorentz(self):
        prop = CSProperties.CSPropLorentzMaterial(self.pset, order=2)
        self.assertTrue(prop.GetMaterial())
        self.assertEqual(prop.GetType(), CSProperties.PropertyType.LORENTZMATERIAL + CSProperties.PropertyType.DISPERSIVEMATERIAL + CSProperties.PropertyType.MATERIAL)
        self.assertEqual(prop.GetTypeString(), 'LorentzMaterial')

        self.assertEqual(prop.GetDispersionOrder(), 2)
        prop.SetDispersionOrder(3)
        self.assertEqual(prop.GetDispersionOrder(), 3)

        with self.assertRaises(IndexError):
            prop.SetDispersiveMaterialProperty(order=3, eps_plasma=1.2e9)

        prop.SetDispersiveMaterialProperty(order=0, eps_plasma=1.2e9)
        self.assertEqual(prop.GetDispersiveMaterialProperty('eps_plasma', order=0), 1.2e9)
        prop.SetDispersiveMaterialProperty(order=1, eps_pole_freq=2.2e9)
        self.assertEqual(prop.GetDispersiveMaterialProperty('eps_pole_freq', order=1), 2.2e9)
        prop.SetDispersiveMaterialProperty(order=2, eps_relax=2.2e-9)
        self.assertEqual(prop.GetDispersiveMaterialProperty('eps_relax', order=2), 2.2e-9)

        prop.SetDispersiveMaterialProperty(order=0, mue_plasma=1.3e9)
        self.assertEqual(prop.GetDispersiveMaterialProperty('mue_plasma', order=0), 1.3e9)
        prop.SetDispersiveMaterialProperty(order=1, mue_pole_freq=2.3e9)
        self.assertEqual(prop.GetDispersiveMaterialProperty('mue_pole_freq', order=1), 2.3e9)
        prop.SetDispersiveMaterialProperty(order=2, mue_relax=2.3e-9)
        self.assertEqual(prop.GetDispersiveMaterialProperty('mue_relax', order=2), 2.3e-9)

        prop2 = prop.copy()
        self.assertEqual(prop2.GetDispersionOrder(), 3)
        self.assertEqual(prop2.GetDispersiveMaterialProperty('eps_plasma', order=0), 1.2e9)
        self.assertEqual(prop2.GetDispersiveMaterialProperty('eps_pole_freq', order=1), 2.2e9)
        self.assertEqual(prop2.GetDispersiveMaterialProperty('eps_relax', order=2), 2.2e-9)
        self.assertEqual(prop2.GetDispersiveMaterialProperty('mue_plasma', order=0), 1.3e9)
        self.assertEqual(prop2.GetDispersiveMaterialProperty('mue_pole_freq', order=1), 2.3e9)
        self.assertEqual(prop2.GetDispersiveMaterialProperty('mue_relax', order=2), 2.3e-9)

        prop.SetDispersionOrder(0)  # <-- this must not influence the copy
        self.assertEqual(prop2.GetDispersionOrder(), 3)
        self.assertEqual(prop2.GetDispersiveMaterialProperty('eps_plasma', order=0), 1.2e9)
        self.assertEqual(prop2.GetDispersiveMaterialProperty('eps_pole_freq', order=1), 2.2e9)
        self.assertEqual(prop2.GetDispersiveMaterialProperty('eps_relax', order=2), 2.2e-9)
        self.assertEqual(prop2.GetDispersiveMaterialProperty('mue_plasma', order=0), 1.3e9)
        self.assertEqual(prop2.GetDispersiveMaterialProperty('mue_pole_freq', order=1), 2.3e9)
        self.assertEqual(prop2.GetDispersiveMaterialProperty('mue_relax', order=2), 2.3e-9)

        prop.SetDispersionOrder(3) # <-- this should reset everything zo zero
        self.assertEqual(prop.GetDispersionOrder(), 3)
        self.assertEqual(prop.GetDispersiveMaterialProperty('eps_plasma', order=0), 0)
        self.assertEqual(prop.GetDispersiveMaterialProperty('eps_pole_freq', order=1), 0)
        self.assertEqual(prop.GetDispersiveMaterialProperty('eps_relax', order=2), 0)
        self.assertEqual(prop.GetDispersiveMaterialProperty('mue_plasma', order=0), 0)
        self.assertEqual(prop.GetDispersiveMaterialProperty('mue_pole_freq', order=1), 0)
        self.assertEqual(prop.GetDispersiveMaterialProperty('mue_relax', order=2), 0)

    def test_debye(self):
        prop = CSProperties.CSPropDebyeMaterial(self.pset, order=2)
        self.assertTrue(prop.GetMaterial())
        self.assertEqual(prop.GetType(), CSProperties.PropertyType.DEBYEMATERIAL + CSProperties.PropertyType.DISPERSIVEMATERIAL + CSProperties.PropertyType.MATERIAL)
        self.assertEqual(prop.GetTypeString(), 'DebyeMaterial')

        self.assertEqual(prop.GetDispersionOrder(), 2)
        prop.SetDispersionOrder(1)
        self.assertEqual(prop.GetDispersionOrder(), 1)

        with self.assertRaises(IndexError):
            prop.SetDispersiveMaterialProperty(order=1, eps_delta=1.2)

        prop.SetDispersiveMaterialProperty(order=0, eps_delta=1.2)
        self.assertEqual(prop.GetDispersiveMaterialProperty('eps_delta', order=0), 1.2)

        prop.SetDispersiveMaterialWeight(order=0, eps_delta=['sin(x)', 'y', 'z'], eps_relax=['cos(x)', '2*y', '3*z'])
        self.assertEqual(prop.GetDispersiveMaterialWeight('eps_delta', order=0), 'sin(x)')
        self.assertEqual(prop.GetDispersiveMaterialWeight('eps_relax', order=0), 'cos(x)')

        prop2 = prop.copy()
        self.assertEqual(prop2.GetDispersionOrder(), 1)
        self.assertEqual(prop2.GetDispersiveMaterialProperty('eps_delta', order=0), 1.2)
        self.assertEqual(prop2.GetDispersiveMaterialWeight('eps_delta', order=0), 'sin(x)')
        self.assertEqual(prop2.GetDispersiveMaterialWeight('eps_relax', order=0), 'cos(x)')

    def test_property_id(self):
        from CSXCAD.CSXCAD import ContinuousStructure
        csx = ContinuousStructure()
        prop1 = csx.AddMetal('m1')
        prop2 = csx.AddMaterial('m2')
        self.assertIsInstance(prop1.GetID(), int)
        self.assertIsInstance(prop2.GetID(), int)
        self.assertNotEqual(prop1.GetID(), prop2.GetID())

    def test_absorbing_bc(self):
        prop = CSProperties.CSPropAbsorbingBC(self.pset)

        self.assertFalse(prop.GetMaterial())
        self.assertEqual(prop.GetType(), CSProperties.PropertyType.ABSORBING_BC)
        self.assertEqual(prop.GetTypeString(), 'AbsorbingBC')

        prop.SetNormalSignPositive(True)
        self.assertTrue(prop.GetNormalSignPositive())
        prop.SetNormalSignPositive(False)
        self.assertFalse(prop.GetNormalSignPositive())

        prop.SetPhaseVelocity(3e8)
        self.assertAlmostEqual(prop.GetPhaseVelocity(), 3e8)

        prop.SetAbsorbingBoundaryType(CSProperties.ABCtype.MUR_1ST)
        self.assertEqual(prop.GetAbsorbingBoundaryType(), CSProperties.ABCtype.MUR_1ST)
        prop.SetAbsorbingBoundaryType(CSProperties.ABCtype.MUR_1ST_SA)
        self.assertEqual(prop.GetAbsorbingBoundaryType(), CSProperties.ABCtype.MUR_1ST_SA)

        prop2 = prop.copy()
        self.assertEqual(prop2.GetType(), CSProperties.PropertyType.ABSORBING_BC)
        self.assertFalse(prop2.GetNormalSignPositive())
        self.assertAlmostEqual(prop2.GetPhaseVelocity(), 3e8)
        self.assertEqual(prop2.GetAbsorbingBoundaryType(), CSProperties.ABCtype.MUR_1ST_SA)

    def test_absorbing_bc_kwargs(self):
        prop = CSProperties.CSPropAbsorbingBC(
            self.pset,
            NormalSignPositive=True,
            PhaseVelocity=2e8,
            AbsorbingBoundaryType=CSProperties.ABCtype.MUR_1ST,
        )
        self.assertTrue(prop.GetNormalSignPositive())
        self.assertAlmostEqual(prop.GetPhaseVelocity(), 2e8)
        self.assertEqual(prop.GetAbsorbingBoundaryType(), CSProperties.ABCtype.MUR_1ST)

    def test_probe_full(self):
        prop = CSProperties.CSPropProbeBox(self.pset, p_type=2, weight=0.5, norm_dir=1)

        self.assertEqual(prop.GetProbeType(), 2)
        prop.SetProbeType(3)
        self.assertEqual(prop.GetProbeType(), 3)

        self.assertAlmostEqual(prop.GetWeighting(), 0.5)
        prop.SetWeighting(2.0)
        self.assertAlmostEqual(prop.GetWeighting(), 2.0)

        self.assertEqual(prop.GetNormalDir(), 1)
        prop.SetNormalDir(2)
        self.assertEqual(prop.GetNormalDir(), 2)

        prop.SetModeFunction(['sin(x)', 'cos(y)', '0'])
        self.assertEqual(prop.GetModeFunction(), ['sin(x)', 'cos(y)', '0'])

        prop.SetModeFile('/path/to/mode.h5')
        self.assertEqual(prop.GetModeFile(), '/path/to/mode.h5')

        prop.SetModeOrigin([1.0, 2.0, 3.0])
        self.assertEqual(prop.GetModeOrigin(), [1.0, 2.0, 3.0])

        prop2 = prop.copy()
        self.assertEqual(prop2.GetProbeType(), 3)
        self.assertAlmostEqual(prop2.GetWeighting(), 2.0)
        self.assertEqual(prop2.GetNormalDir(), 2)
        self.assertEqual(prop2.GetModeFunction(), ['sin(x)', 'cos(y)', '0'])
        self.assertEqual(prop2.GetModeFile(), '/path/to/mode.h5')
        self.assertEqual(prop2.GetModeOrigin(), [1.0, 2.0, 3.0])

    def test_probe_mode_function_kwarg(self):
        prop = CSProperties.CSPropProbeBox(self.pset, p_type=10,
                                           mode_function=['x', 'y', '0'])
        self.assertEqual(prop.GetModeFunction(), ['x', 'y', '0'])

    def test_excitation_full(self):
        prop = CSProperties.CSPropExcitation(self.pset, exc_type=0, exc_val=[1.0, 0, 0])

        prop.SetEnabled(False)
        self.assertFalse(prop.GetEnabled())
        prop.SetEnabled(True)
        self.assertTrue(prop.GetEnabled())

        prop.SetPropagationDir([0, 0, 1])
        pd = prop.GetPropagationDir()
        self.assertTrue((pd == [0, 0, 1]).all())

        prop.SetFrequency(2.4e9)
        self.assertAlmostEqual(prop.GetFrequency(), 2.4e9)

        prop.SetWeightFile('/mode.h5')
        self.assertEqual(prop.GetWeightFile(), '/mode.h5')

        prop.SetWeightOrigin([1.0, 2.5, 3.0])
        self.assertEqual(prop.GetWeightOrigin(), [1.0, 2.5, 3.0])

        prop2 = prop.copy()
        self.assertTrue(prop2.GetEnabled())
        self.assertTrue((prop2.GetPropagationDir() == [0, 0, 1]).all())
        self.assertAlmostEqual(prop2.GetFrequency(), 2.4e9)
        self.assertEqual(prop2.GetWeightFile(), '/mode.h5')
        self.assertEqual(prop2.GetWeightOrigin(), [1.0, 2.5, 3.0])

    def test_excitation_zero_weight_function_roundtrip(self):
        # '0' is parsed as numeric 0.0 by ParameterScalar; GetWeightFunction()
        # returns '' for numeric components.  SetWeightFunction('') must treat
        # '' as 0.0 so that a get/set round-trip preserves zero weights.
        exc = CSProperties.CSPropExcitation(self.pset, exc_type=0,
                                             exc_val=[1.0, 1.0, 1.0])
        exc.SetPropagationDir([0, 0, 1])
        exc.SetWeightFunction(['cos(x)', '0', '0'])

        funcs = exc.GetWeightFunction()
        self.assertEqual(funcs[1], '')  # '0' stored numeric; GetString() returns ''

        exc2 = CSProperties.CSPropExcitation(self.pset, exc_type=0,
                                              exc_val=[1.0, 1.0, 1.0])
        exc2.SetPropagationDir([0, 0, 1])
        exc2.SetWeightFunction(funcs)   # feeds '' for components 1 and 2

        self.assertAlmostEqual(exc2.GetWeightedExcitation(1, [1.0, 1.0, 0.0]), 0.0)

    def test_excitation_weight_file_kwarg(self):
        prop = CSProperties.CSPropExcitation(self.pset, exc_type=0,
                                             exc_val=[1.0, 0, 0],
                                             weight_file='/kwarg_mode.h5')
        self.assertEqual(prop.GetWeightFile(), '/kwarg_mode.h5')


@unittest.skipUnless(HAS_H5PY, 'h5py not available')
class TestDiscMaterialData(unittest.TestCase):
    """Test CSPropDiscMaterial coordinate-based material queries.

    File layout (3x3x3 mesh points -> 2x2x2 cells, 3 materials):

      Material 0: epsR=1.0  mueR=1.0  kappa=0.00  sigma=0.000  density=   0.0
      Material 1: epsR=4.0  mueR=1.5  kappa=0.10  sigma=0.010  density=1000.0
      Material 2: epsR=9.0  mueR=2.0  kappa=0.50  sigma=0.020  density=2000.0

    Cell assignment indices[iz, iy, ix]:
      (0,0,0)=0  (0,0,1)=1  (0,1,0)=2  (0,1,1)=1
      (1,0,0)=2  (1,0,1)=0  (1,1,0)=1  (1,1,1)=2
    """

    EPS_R   = [1.0, 4.0, 9.0]
    MUE_R   = [1.0, 1.5, 2.0]
    KAPPA   = [0.0, 0.1, 0.5]
    SIGMA   = [0.0, 0.01, 0.02]
    DENSITY = [0.0, 1000.0, 2000.0]

    # cell centres: mesh is [0,10,20] in each axis, centres at 5 and 15
    CELLS = {
        (5,  5,  5 ): 0,
        (15, 5,  5 ): 1,
        (5,  15, 5 ): 2,
        (15, 15, 5 ): 1,
        (5,  5,  15): 2,
        (15, 5,  15): 0,
        (5,  15, 15): 1,
        (15, 15, 15): 2,
    }

    def setUp(self):
        self.csx = ContinuousStructure()
        self.fn  = os.path.join(tempfile.gettempdir(), 'test_discmat_data.h5')
        self._write_hdf5()
        self.mat = CSPropDiscMaterial(self.csx.GetParameterSet(),
                                      filename=self.fn, filetype=0)
        self.csx.AddProperty(self.mat)

    def tearDown(self):
        if os.path.exists(self.fn):
            os.remove(self.fn)

    def _write_hdf5(self):
        with h5py.File(self.fn, 'w') as f:
            f.attrs.create('Version', data=np.float64(2.0))

            mesh = np.array([0.0, 10.0, 20.0], dtype=np.float32)
            f.create_dataset('/mesh/x', data=mesh)
            f.create_dataset('/mesh/y', data=mesh)
            f.create_dataset('/mesh/z', data=mesh)

            # shape (nz-1, ny-1, nx-1); indices[iz, iy, ix]
            indices = np.array([[[0, 1], [2, 1]],
                                 [[2, 0], [1, 2]]], dtype=np.uint8)
            ds = f.create_dataset('/DiscData', data=indices)
            ds.attrs.create('DB_Size',  data=np.int32(3))
            ds.attrs.create('epsR',     data=np.array(self.EPS_R,   dtype=np.float32))
            ds.attrs.create('mueR',     data=np.array(self.MUE_R,   dtype=np.float32))
            ds.attrs.create('kappa',    data=np.array(self.KAPPA,   dtype=np.float32))
            ds.attrs.create('sigma',    data=np.array(self.SIGMA,   dtype=np.float32))
            ds.attrs.create('density',  data=np.array(self.DENSITY, dtype=np.float32))

    def test_read_file_succeeds(self):
        self.assertTrue(self.mat.ReadFile())

    def test_epsilon_at_all_cells(self):
        for (x, y, z), mat_idx in self.CELLS.items():
            with self.subTest(coords=(x, y, z)):
                self.assertAlmostEqual(self.mat.GetEpsilonWeighted(0, [x, y, z]),
                                       self.EPS_R[mat_idx], places=5)

    def test_mue_at_all_cells(self):
        for (x, y, z), mat_idx in self.CELLS.items():
            with self.subTest(coords=(x, y, z)):
                self.assertAlmostEqual(self.mat.GetMueWeighted(0, [x, y, z]),
                                       self.MUE_R[mat_idx], places=5)

    def test_kappa_at_all_cells(self):
        for (x, y, z), mat_idx in self.CELLS.items():
            with self.subTest(coords=(x, y, z)):
                self.assertAlmostEqual(self.mat.GetKappaWeighted(0, [x, y, z]),
                                       self.KAPPA[mat_idx], places=5)

    def test_sigma_at_all_cells(self):
        for (x, y, z), mat_idx in self.CELLS.items():
            with self.subTest(coords=(x, y, z)):
                self.assertAlmostEqual(self.mat.GetSigmaWeighted(0, [x, y, z]),
                                       self.SIGMA[mat_idx], places=5)

    def test_density_at_all_cells(self):
        for (x, y, z), mat_idx in self.CELLS.items():
            with self.subTest(coords=(x, y, z)):
                self.assertAlmostEqual(self.mat.GetDensityWeighted([x, y, z]),
                                       self.DENSITY[mat_idx], places=3)

    def test_outside_mesh_falls_back_to_material_defaults(self):
        self.assertAlmostEqual(self.mat.GetEpsilonWeighted(0, [100, 5, 5]), 1.0)
        self.assertAlmostEqual(self.mat.GetMueWeighted(0,     [5, 100, 5]), 1.0)
        self.assertAlmostEqual(self.mat.GetKappaWeighted(0,   [5, 5, 100]), 0.0)

    def test_index0_as_background_when_db_background_false(self):
        mat2 = CSPropDiscMaterial(self.csx.GetParameterSet(),
                                  filename=self.fn, filetype=0,
                                  use_db_background=False)
        self.csx.AddProperty(mat2)
        # cell (0,0,0) has index 0 -> falls back to CSPropMaterial defaults
        self.assertAlmostEqual(mat2.GetEpsilonWeighted(0, [5, 5, 5]), 1.0)
        # cell (1,0,0) has index 1 -> from database
        self.assertAlmostEqual(mat2.GetEpsilonWeighted(0, [15, 5, 5]), self.EPS_R[1])


class TestDiscMaterialTransform(unittest.TestCase):
    def setUp(self):
        self.csx = ContinuousStructure()
        self.mat = CSPropDiscMaterial(self.csx.GetParameterSet())
        self.csx.AddProperty(self.mat)

    def test_no_transform_by_default(self):
        self.assertIsNone(self.mat.GetTransform())

    def test_set_transform_get_returns_same_wrapper(self):
        from CSXCAD.CSTransform import CSTransform
        t = CSTransform()
        t.Translate([1, 2, 3])
        self.mat.SetTransform(t)
        self.assertIs(self.mat.GetTransform(), t)

    def test_get_transform_repeated_calls_same_wrapper(self):
        from CSXCAD.CSTransform import CSTransform
        t = CSTransform()
        self.mat.SetTransform(t)
        self.assertIs(self.mat.GetTransform(), self.mat.GetTransform())

    def test_set_transform_none_removes_transform(self):
        from CSXCAD.CSTransform import CSTransform
        self.mat.SetTransform(CSTransform())
        self.mat.SetTransform(None)
        self.assertIsNone(self.mat.GetTransform())

    def test_set_transform_replaces_previous(self):
        from CSXCAD.CSTransform import CSTransform
        t1 = CSTransform()
        t1.Translate([1, 0, 0])
        t2 = CSTransform()
        t2.Translate([0, 2, 0])
        self.mat.SetTransform(t1)
        self.mat.SetTransform(t2)
        self.assertIs(self.mat.GetTransform(), t2)

    def test_set_same_transform_twice_is_safe(self):
        from CSXCAD.CSTransform import CSTransform
        t = CSTransform()
        t.Translate([1, 0, 0])
        self.mat.SetTransform(t)
        self.mat.SetTransform(t)          # must not delete-then-use
        self.assertIs(self.mat.GetTransform(), t)

    def test_set_transform_from_get_is_safe(self):
        from CSXCAD.CSTransform import CSTransform
        self.mat.SetTransform(CSTransform())
        self.mat.SetTransform(self.mat.GetTransform())   # SetTransform(GetTransform())
        self.assertIsNotNone(self.mat.GetTransform())

    def test_copy_preserves_transform(self):
        from CSXCAD.CSTransform import CSTransform
        t = CSTransform()
        t.Translate([3, 0, 0])
        self.mat.SetTransform(t)
        mat2 = self.mat.GetCopy()
        tr2 = mat2.GetTransform()
        self.assertIsNotNone(tr2)
        self.assertIsNot(tr2, t)
        self.assertTrue((tr2.GetMatrix() == t.GetMatrix()).all())


if __name__ == '__main__':
    unittest.main()
