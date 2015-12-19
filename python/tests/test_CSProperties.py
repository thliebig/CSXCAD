# -*- coding: utf-8 -*-
"""
Created on Thu Dec 10 21:47:29 2015

@author: thorsten
"""

import os, sys
sys.path.append(os.path.join(os.path.dirname(__file__), '..'))

import numpy as np

from pyCSXCAD import CSXCAD
from pyCSXCAD import CSProperties

CSX = CSXCAD.ContinuousStructure()

# Test Material
mat = CSX.AddMaterial('mat1', Epsilon = 5.0)
assert mat.GetMaterialProperty('Epsilon')==5.0
assert mat.GetMaterialProperty('Mue')==1.0

mat.SetMaterialProperty(Epsilon=1.0, Mue=2.0)
assert mat.GetMaterialProperty('Epsilon')==1.0
assert mat.GetMaterialProperty('Mue')==2.0

mat.SetMaterialWeight(Epsilon='sin(x)', Mue='cos(y)', Density='z*z')
assert mat.GetMaterialWeight('Epsilon')=='sin(x)'
assert mat.GetMaterialWeight('Mue')=='cos(y)'
assert mat.GetMaterialWeight('Density')=='z*z'

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

CSX.Write2XML('prop_test.xml')

print("all ok!")