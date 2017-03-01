# -*- coding: utf-8 -*-
"""
Created on Fri Dec  4 16:24:49 2015

@author: thorsten
"""

import numpy as np

from CSXCAD.CSXCAD import ContinuousStructure
from CSXCAD import CSProperties

from CSXCAD.CSProperties import CSPropMetal
from CSXCAD import CSPrimitives
csx = ContinuousStructure()
pset = csx.GetParameterSet()
grid = csx.GetGrid()

##### Test Metal Prop
metal = CSPropMetal(pset)
assert metal.GetQtyPrimitives() == 0
assert metal.GetTypeString()=='Metal'

metal.SetName('metal')
assert metal.GetName()=='metal'


print(csx.GetQtyPrimitives(0))
metal.AddBox([0,0,0], [1,1,1])

ans = csx.AddProperty(metal)

print(csx.GetQtyPrimitives(CSProperties.ANY))
print(csx.GetQtyPrimitives(CSProperties.METAL))
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

print("all ok")