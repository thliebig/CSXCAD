# -*- coding: utf-8 -*-
"""
Created on Fri Sep  2 21:42:50 2016

@author: thorsten
"""

from CSXCAD import CSTransform
import numpy as np

def compare_coords(c1, c2):
    c1 = np.array(c1)
    c2 = np.array(c2)
    return np.sum(np.abs(c1-c2))<1e-6

tr = CSTransform.CSTransform()

identity = np.identity(4)

assert tr.HasTransform() == False
assert (tr.GetMatrix()==identity).all()

test_pnt = np.array([1,1,1])/np.sqrt(3)


tr.Translate([1,2,1])
translate = identity.copy()
translate[:3,3] = [1,2,1]
assert (tr.GetMatrix()==translate).all()

assert compare_coords(tr.Transform(test_pnt), [ 1.57735027,  2.57735027,  1.57735027])

tr.Reset()
assert tr.HasTransform()==False
assert (tr.GetMatrix()==identity).all()


tr.RotateAxis('x', 45)
assert compare_coords(tr.Transform([0,np.sqrt(2),0]), [0, 1, 1])

tr.Reset()
tr.RotateAxis('x', -45)
assert compare_coords(tr.Transform([0,np.sqrt(2),0]), [0, 1, -1])

tr.Reset()
tr.RotateAxis('z', -90)
assert compare_coords(tr.Transform([1,0,0]), [0, -1, 0])
tr.RotateOrigin([0,0,1], 90)
assert compare_coords(tr.Transform([1,0,0]), [1, 0, 0])

tr.Scale(3)
assert compare_coords(tr.Transform([1,0,0]), [3, 0, 0])

tr.Reset()
tr.Scale([1,2,3])
assert compare_coords(tr.Transform([1,1,1]), [1, 2, 3])

tr.Reset()
tr.RotateOrigin([1,1,1], 90)
mat = tr.GetMatrix()

tr.Reset()
tr.SetMatrix(mat)
assert (tr.GetMatrix()==mat).all()

print('all tests passed')
