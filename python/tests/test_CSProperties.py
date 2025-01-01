# -*- coding: utf-8 -*-
"""
Created on Thu Dec 10 21:47:29 2015

@author: thorsten
"""

import numpy as np

from CSXCAD import ParameterObjects
from CSXCAD import CSProperties, CSPrimitives

import unittest

class Test_CSPrimMethods(unittest.TestCase):
    def setUp(self):
        self.pset  = ParameterObjects.ParameterSet()

    def test_add_primitives(self):
        prop = CSProperties.CSPropMetal(self.pset)
        box = prop.AddBox(start= [0,0,0] , stop=[1,1,1])

        self.assertEqual( prop.GetQtyPrimitives(), 1)
        self.assertEqual( id(prop.GetPrimitive(0)), id(box))

        self.assertEqual( prop.GetPrimitive(0).GetType(), CSPrimitives.BOX)
        self.assertEqual( len(prop.GetAllPrimitives()), 1)
        self.assertEqual( prop.GetAllPrimitives(), [box,])

        prim = prop.AddCylinder(start= [0,0,0] , stop=[1,1,1], radius=0.5)
        self.assertEqual( prop.GetQtyPrimitives(), 2)
        self.assertEqual( prop.GetPrimitive(1).GetType(), CSPrimitives.CYLINDER)

        prim = prop.AddCylindricalShell(start= [0,0,0] , stop=[1,1,1], radius=0.5, shell_width=0.1)
        self.assertEqual( prop.GetQtyPrimitives(), 3)
        self.assertEqual( prop.GetPrimitive(2).GetType(), CSPrimitives.CYLINDRICALSHELL)

        prim = prop.AddSphere(center=[1,1,1], radius=0.5)
        self.assertEqual( prop.GetQtyPrimitives(), 4)
        self.assertEqual( prop.GetPrimitive(3).GetType(), CSPrimitives.SPHERE)

        prim = prop.AddSphericalShell(center=[1,1,1], radius=0.5, shell_width=0.1)
        self.assertEqual( prop.GetQtyPrimitives(), 5)
        self.assertEqual( prop.GetPrimitive(4).GetType(), CSPrimitives.SPHERICALSHELL)

        self.assertEqual( len(prop.GetAllPrimitives()), 5)

        self.assertEqual(prop.GetPrimitive(-1), prop.GetPrimitive(prop.GetQtyPrimitives()-1))

        #with self.assertRaises(Exception):
        self.assertEqual(prop.GetPrimitive(10), None)

    def test_attributes(self):
        prop = CSProperties.CSPropMaterial(self.pset)

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

    def test_visibility(self):
        prop = CSProperties.CSPropMetal(self.pset)
        self.assertTrue(prop.GetVisibility())
        prop.SetVisibility(False)
        self.assertFalse(prop.GetVisibility())
        prop.SetVisibility(True)
        self.assertTrue(prop.GetVisibility())

    def test_metal(self):
        prop = CSProperties.CSPropMetal(self.pset)

        self.assertEqual( prop.GetType(), CSProperties.METAL)
        self.assertEqual( prop.GetTypeString(), 'Metal')

    def test_material(self):
        prop = CSProperties.CSPropMaterial(self.pset, epsilon = 5.0)

        self.assertEqual( prop.GetType(), CSProperties.MATERIAL)
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

    def test_lumped_elem(self):
        prop = CSProperties.CSPropLumpedElement(self.pset, R = 50, C=1e-12, caps=True, ny='x')

        self.assertEqual( prop.GetType(), CSProperties.LUMPED_ELEMENT)
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

    def test_cond_sheet(self):
        prop = CSProperties.CSPropConductingSheet(self.pset, conductivity=56e6, thickness=35e-6)

        self.assertEqual( prop.GetType(), CSProperties.CONDUCTINGSHEET + CSProperties.METAL)
        self.assertEqual( prop.GetTypeString(), 'ConductingSheet')

        self.assertEqual( prop.GetConductivity(), 56e6)
        self.assertEqual( prop.GetThickness(), 35e-6)

    def test_excitation(self):
        prop = CSProperties.CSPropExcitation(self.pset,  exc_type=1, exc_val=[-1.0, 0, 1.0], delay=1e-9)

        self.assertEqual( prop.GetType(), CSProperties.EXCITATION)
        self.assertEqual( prop.GetTypeString(), 'Excitation')

        self.assertEqual( prop.GetExcitType(), 1)
        self.assertTrue( (prop.GetExcitation()==[-1.0, 0, 1.0]).all() )
        self.assertEqual( prop.GetDelay(),1e-9)

        prop.SetWeightFunction(['y','x','z'])
        self.assertEqual(prop.GetWeightFunction(), ['y','x','z'])

    def test_probe(self):
        prop = CSProperties.CSPropProbeBox(self.pset, frequency=[1e9, 2.4e9])

        self.assertEqual( prop.GetType(), CSProperties.PROBEBOX)
        self.assertEqual( prop.GetTypeString(), 'ProbeBox')

        self.assertEqual(prop.GetFrequencyCount(), 2)
        prop.AddFrequency(5e9)
        self.assertEqual(prop.GetFrequencyCount(), 3)
        self.assertTrue((prop.GetFrequency() == [1e9, 2.4e9, 5e9]).all())

        prop.ClearFrequency()
        self.assertEqual(prop.GetFrequencyCount(), 0)

        prop.SetFrequency(np.linspace(1e9,2e9,11))
        self.assertEqual(prop.GetFrequencyCount(), 11)

    def test_dump(self):
        prop = CSProperties.CSPropDumpBox(self.pset)

        self.assertEqual( prop.GetType(), CSProperties.DUMPBOX)
        self.assertEqual( prop.GetTypeString(), 'DumpBox')

if __name__ == '__main__':
    unittest.main()
