# -*- coding: utf-8 -*-
"""
Created on Fri Dec  4 16:24:49 2015

@author: thorsten
"""

import os, sys
sys.path.append(os.path.join(os.path.dirname(__file__), '..'))

import numpy as np

from pyCSXCAD import CSXCAD
from pyCSXCAD import CSProperties
from pyCSXCAD import CSPrimitives

csx = CSXCAD.ContinuousStructure()
pset = csx.GetParameterSet()
grid = csx.GetGrid()

##### Test Metal Prop
metal = CSProperties.CSPropMetal(pset)
assert metal.GetQtyPrimitives() == 0
assert metal.GetTypeString()=='Metal'

metal.SetName('metal')
assert metal.GetName()=='metal'

ans = csx.AddProperty(metal)
metal2 = csx.AddMetal('test')
assert metal2.GetName()=='test'

##### Test Excitation Prop
exc_val = np.array([0,1,0])
exc = csx.AddExcitation('excite', 0, exc_val)
assert exc.GetName()=='excite'
assert exc.GetExcitType() == 0
assert (exc.GetExcitation()==exc_val).all()


csx.Write2XML('test_CSXCAD.xml')

del metal

print("all done")