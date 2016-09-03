# -*- coding: utf-8 -*-
"""
Created on Sat Dec 12 22:04:58 2015

@author: thorsten
"""

import numpy as np

from CSXCAD import CSRectGrid

grid = CSRectGrid.CSRectGrid(CoordSystem=0)

assert grid.GetMeshType()==0

grid.SetMeshType(1)
assert grid.GetMeshType()==1

grid.SetLines('x', [0, 1, 2])
grid.SetLines('y', [-2,0, 1])
assert grid.GetQtyLines('y')==3
grid.AddLine('y',4)
assert grid.GetQtyLines('y')==4
grid.AddLine('y',[4, 2, 5])
assert grid.GetQtyLines('y')==7
assert (grid.GetLines('y')==np.array([-2.,  0.,  1.,  4.,  4., 2.,  5.])).all()  # check unsorted lines
grid.Sort('y')
assert (grid.GetLines('y')==np.array([-2.,  0.,  1.,  2.,  4.,  5.])).all()
assert grid.GetQtyLines('y')==6

grid.SetLines('z', [10, 11, 12])

assert grid.GetLine('y', 1) == 0.0

assert grid.GetQtyLines('x')==3

grid.SetDeltaUnit(1e-3)
assert grid.GetDeltaUnit()==1e-3

assert grid.IsValid()

# check grid snapping
assert grid.Snap2LineNumber('y', 1)==(2,True)
assert grid.Snap2LineNumber('y', 1.1)==(2,True)
assert grid.Snap2LineNumber('y', 1.5)==(3,True)
assert grid.Snap2LineNumber('y', 1.6)==(3,True)
assert grid.Snap2LineNumber('y', 5.0)==(5,True)
assert grid.Snap2LineNumber('y', 5.01)==(5,False)

assert grid.Snap2LineNumber('y', -2.0)==(0,True)
assert grid.Snap2LineNumber('y', -2.01)==(0,False)

print(grid.GetSimArea())

grid.ClearLines('x')
assert grid.GetQtyLines('x')==0
assert grid.GetQtyLines('y')==6
assert grid.GetQtyLines('z')==3

assert grid.IsValid()==False

grid.Clear()
assert grid.GetQtyLines('y')==0

assert grid.IsValid()==False


print("all ok")