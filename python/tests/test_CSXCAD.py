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


if __name__ == '__main__':
    unittest.main()
