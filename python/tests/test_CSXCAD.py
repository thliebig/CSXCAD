import numpy as np

from CSXCAD.CSXCAD import ContinuousStructure
from CSXCAD import CSProperties

from CSXCAD.CSProperties import CSPropMetal
from CSXCAD import CSPrimitives

import os, tempfile
import unittest

class Test_CSXCAD(unittest.TestCase):
    def setUp(self):
        self.csx  = ContinuousStructure()

    def test_add_prop(self):

        pset = self.csx.GetParameterSet()

        ##### Test Metal Prop
        metal = CSPropMetal(pset)
        self.assertEqual(metal.GetQtyPrimitives(), 0)
        self.assertEqual(metal.GetTypeString(), 'Metal')

        metal.SetName('metal')
        self.assertEqual(metal.GetName(), 'metal')
        box = metal.AddBox([0,0,0], [1,1,1])

        # still no primitives as metal is not added to csx yet!
        self.assertEqual(self.csx.GetQtyPrimitives(CSProperties.PropertyType.ANY), 0)

        self.csx.AddProperty(metal)

        self.assertEqual(self.csx.GetQtyPrimitives(CSProperties.PropertyType.ANY), 1)
        self.assertEqual(self.csx.GetQtyPrimitives(CSProperties.PropertyType.METAL), 1)

        self.assertEqual(self.csx.GetQtyProperties(), 1)
        self.assertEqual(self.csx.GetProperty(0), metal)
        self.assertEqual(len(self.csx.GetPropertiesByName('metal')), 1)
        self.assertEqual(self.csx.GetPropertiesByName('metal')[0], metal)

        self.assertEqual(len(self.csx.GetPropertyByType(CSProperties.PropertyType.METAL)), 1)
        self.assertEqual(self.csx.GetPropertyByType(CSProperties.PropertyType.METAL)[0], metal)

        self.assertEqual(len(self.csx.GetPropertyByType(CSProperties.PropertyType.ANY)), 1)
        self.assertEqual(self.csx.GetPropertyByType(CSProperties.PropertyType.ANY)[0], metal)

        self.assertEqual(len(self.csx.GetPropertyByType(CSProperties.PropertyType.MATERIAL)), 0)

        self.assertEqual(self.csx.GetPropertyByCoordPriority([0.5, 0.5, 0.5]), metal)
        self.assertEqual(self.csx.GetPropertyByCoordPriority([1.5, 0.5, 0.5]), None)

        self.assertFalse(box.GetPrimitiveUsed())
        self.assertEqual(self.csx.GetPropertyByCoordPriority([0.5, 0.5, 0.5], markFoundAsUsed=True), metal)
        self.assertTrue(box.GetPrimitiveUsed())  # now it needs to be marked as used/found


        self.csx.Reset()
        self.assertEqual(self.csx.GetQtyPrimitives(CSProperties.PropertyType.ANY), 0)

    def test_write_xml(self):
        fn = os.path.join(tempfile.gettempdir(), 'test_CSXCAD.xml')
        self.assertTrue(self.csx.Write2XML(fn))

        # check for error return message
        self.assertEqual(self.csx.ReadFromXML(fn), '')


    def test_grid(self):
        grid = self.csx.GetGrid()
        self.assertIsNotNone(grid)
        self.assertIs(self.csx.grid, grid)

        self.assertEqual(self.csx.GetCoordInputType(), 0)
        self.csx.SetMeshType(1)
        self.assertEqual(self.csx.GetCoordInputType(), 1)
        self.assertEqual(grid.GetMeshType(), 1)
        self.csx.SetMeshType(0)

        mesh = {'x': [-10, 0, 10], 'y': [-5, 0, 5], 'z': [0, 1]}
        returned_grid = self.csx.DefineGrid(mesh, 1e-3)
        self.assertIs(returned_grid, grid)
        self.assertEqual(grid.GetQtyLines('x'), 3)
        self.assertEqual(grid.GetQtyLines('y'), 3)
        self.assertEqual(grid.GetQtyLines('z'), 2)
        self.assertEqual(grid.GetDeltaUnit(), 1e-3)

    def test_grid_define_with_smoothing(self):
        grid = self.csx.GetGrid()
        mesh = {'x': [-10, 0, 10], 'y': [-5, 5], 'z': [0, 1]}
        self.csx.DefineGrid(mesh, 1e-3, smooth_mesh_res=2)
        self.assertGreater(grid.GetQtyLines('y'), 2)

    def test_property_management(self):
        metal1 = self.csx.AddMetal('m1')
        metal2 = self.csx.AddMetal('m2')
        self.assertEqual(self.csx.GetQtyProperties(), 2)

        props = self.csx.GetAllProperties()
        self.assertEqual(len(props), 2)
        self.assertIn(metal1, props)
        self.assertIn(metal2, props)

        self.csx.RemoveProperty(metal1)
        self.assertEqual(self.csx.GetQtyProperties(), 1)
        self.assertEqual(self.csx.GetAllProperties()[0], metal2)

        self.csx.DeleteProperty(metal2)
        self.assertEqual(self.csx.GetQtyProperties(), 0)

    def test_get_all_primitives(self):
        metal = self.csx.AddMetal('metal')
        mat   = self.csx.AddMaterial('mat')
        metal.AddBox([0, 0, 0], [1, 1, 1])
        metal.AddSphere([2, 2, 2], 0.5)
        mat.AddBox([5, 5, 5], [6, 6, 6])

        all_prims = self.csx.GetAllPrimitives()
        self.assertEqual(len(all_prims), 3)

        metal_prims = self.csx.GetAllPrimitives(prop_type=CSProperties.PropertyType.METAL)
        self.assertEqual(len(metal_prims), 2)

        mat_prims = self.csx.GetAllPrimitives(prop_type=CSProperties.PropertyType.MATERIAL)
        self.assertEqual(len(mat_prims), 1)


if __name__ == '__main__':
    unittest.main()
