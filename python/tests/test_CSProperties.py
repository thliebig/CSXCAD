from CSXCAD import ParameterObjects
from CSXCAD import CSProperties, CSPrimitives

import numpy as np
import unittest

def testCsvGen(fileName):
    Xm, Ym = np.meshgrid(np.array([0, 0.1, 0.2, 0.3]), np.array([0, 0.4, 0.8, 1.2]))
    Xc = 0.15
    Yc = 0.6
    Fx = np.cos(np.arctan2(Ym - Yc, Xm - Xc)) * np.sqrt((Xm - Xc) ** 2.0 + (Ym - Yc) ** 2.0) * 3
    Fy = np.sin(np.arctan2(Ym - Yc, Xm - Xc)) * np.sqrt((Xm - Xc) ** 2.0 + (Ym - Yc) ** 2.0) * 3
    modeMat = np.concatenate((Xm.reshape(-1, 1), Ym.reshape(-1, 1), Fx.reshape(-1, 1), Fy.reshape(-1, 1)), axis=1)
    np.savetxt(fileName, modeMat, delimiter=',')
    return Xm, Ym, Fx, Fy

def interp2d(x_vec, y_vec, values, xi, yi, interp_method = 'linear'):
    
    # Find index of closest coordinate
    j = np.abs(x_vec - xi).argmin()
    i = np.abs(y_vec - yi).argmin()
    
    if interp_method == 'nearest':
        return values[i, j]
    elif interp_method == 'linear':
        # Clip indices to stay within array bounds
        j = np.clip(j, 0, len(x_vec) - 2)
        i = np.clip(i, 0, len(y_vec) - 2)
        
        # 2. Get the coordinates and values of the 4 corners
        x1, x2 = x_vec[j], x_vec[j+1]
        y1, y2 = y_vec[i], y_vec[i+1]
        
        # Values at corners: Q11(bottom-left), Q21(bottom-right), etc.
        # Note: values is indexed [row, col] -> [y, x]
        f11 = values[i, j]
        f21 = values[i, j+1]
        f12 = values[i+1, j]
        f22 = values[i+1, j+1]
        
        # 3. Calculate weights (normalized distances)
        dx = (xi - x1) / (x2 - x1)
        dy = (yi - y1) / (y2 - y1)
        
        # 4. Interpolate
        # First along X
        f_y1 = f11 * (1 - dx) + f21 * dx
        f_y2 = f12 * (1 - dx) + f22 * dx
        
        # Then along Y
        return f_y1 * (1 - dy) + f_y2 * dy
        
    else:
        return 0

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
        
        prop3 = prop.copy()
        modeFileName = 'test_mode.csv' 
        Xm, Ym, Fx, Fy = testCsvGen(modeFileName)
        prop3.SetModeFileName(modeFileName)
        self.assertEqual(prop3.GetModeFileName(), modeFileName)
        
        self.assertTrue(prop3.ParseModeFile())
        
        dx = (Xm[0,1] - Xm[0,0])/10
        dy = (Ym[1,0] - Ym[0,0])/10
        
        for test_x in np.arange(Xm[0,0], Xm[0,-1] + dx, dx):
            for test_y in np.arange(Ym[0,0], Ym[-1,0] + dy, dy):
                
                self.assertTrue(np.abs(prop3.GetModeLinInterp2(test_x, test_y, 0) - interp2d(Xm[0, :], Ym[:, 0], Fx, test_x, test_y)) < 1e-4)
                self.assertTrue(np.abs(prop3.GetModeLinInterp2(test_x, test_y, 1) - interp2d(Xm[0, :], Ym[:, 0], Fy, test_x, test_y)) < 1e-4)
                self.assertTrue(np.abs(prop3.GetModeNearestNeighbor(test_x, test_y, 0) - interp2d(Xm[0, :], Ym[:, 0], Fx, test_x, test_y, 'nearest')) < 1e-4)
                self.assertTrue(np.abs(prop3.GetModeNearestNeighbor(test_x, test_y, 1) - interp2d(Xm[0, :], Ym[:, 0], Fy, test_x, test_y, 'nearest')) < 1e-4)
                
        prop3.ClearModeFile()
        
        __import__('os').remove(modeFileName)
                
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
            self.assertRaises(prop.SetDispersiveMaterialProperty(order=3, eps_plasma=1.2e9))

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
            self.assertRaises(prop.SetDispersiveMaterialProperty(order=1, eps_delta=1.2))

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

if __name__ == '__main__':
    unittest.main()
