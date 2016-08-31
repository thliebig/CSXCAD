# -*- coding: utf-8 -*-
"""
Created on Thu Dec 10 21:47:29 2015

@author: thorsten
"""

import numpy as np

from CSXCAD import CSXCAD
from CSXCAD import CSProperties

CSX = CSXCAD.ContinuousStructure()

# Test Material
mat = CSX.AddMaterial('mat1', epsilon = 5.0)
assert mat.GetMaterialProperty('epsilon')==5.0
assert mat.GetMaterialProperty('mue')==1.0

mat.SetMaterialProperty(epsilon=1.0, mue=2.0)
assert mat.GetMaterialProperty('epsilon')==1.0
assert mat.GetMaterialProperty('mue')==2.0

mat.SetMaterialWeight(epsilon='sin(x)', mue='cos(y)', density='z*z')
assert mat.GetMaterialWeight('epsilon')=='sin(x)'
assert mat.GetMaterialWeight('mue')=='cos(y)'
assert mat.GetMaterialWeight('density')=='z*z'

# Test LumpedElement
LE = CSX.AddLumpedElement('LE', R = 50, C=1e-12, caps=True, ny='x')
assert LE.GetResistance()==50
LE.SetResistance(55.0)
LE.SetDirection('x')
assert LE.GetResistance()==55
assert LE.GetCapacity()==1e-12
assert LE.GetCaps()==True
LE.SetCaps(False)
assert LE.GetCaps()==False
assert LE.GetDirection()==0

# Test ConductingSheet
CS = CSX.AddConductingSheet('sheet', conductivity=56e6, thickness=35e-6)
assert CS.GetConductivity()==56e6
assert CS.GetThickness()==35e-6

# Test CSPropExcitation
Exc = CSX.AddExcitation('excite', exc_type=1, exc_val=[-1.0, 0, 1.0], delay=1e-9)
assert Exc.GetExcitType()==1
assert (Exc.GetExcitation()==[-1.0, 0, 1.0]).all()
assert Exc.GetDelay()==1e-9

CSX.Write2XML('prop_test.xml')

print("all ok!")