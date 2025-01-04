# -*- coding: utf-8 -*-
"""
Created on Sun Dec 13 14:49:46 2015

@author: thorsten
"""

import numpy as np

from CSXCAD import ParameterObjects
from CSXCAD import CSProperties
from CSXCAD import CSPrimitives

import unittest

import os

DATA_PATH = os.path.dirname(__file__)

class Test_CSPrimMethods(unittest.TestCase):
    def setUp(self):
        self.pset  = ParameterObjects.ParameterSet()
        self.metal = CSProperties.CSPropMetal(self.pset)

    def test_general(self):
        box = CSPrimitives.CSPrimBox(self.pset, self.metal)

        self.assertEqual(box.GetProperty(), self.metal)
        self.assertEqual(box.GetParameterSet(), self.pset)

        self.assertEqual(self.metal.GetPrimitive(0), box)
        self.assertEqual(self.metal.GetAllPrimitives(), [box,])

        self.assertEqual(box.GetType(), 1)
        self.assertEqual(box.GetTypeName(), 'Box')
        self.assertEqual(box.GetPriority(), 0)

        box.SetPriority(10)
        self.assertEqual(box.GetPriority(), 10)

        box.SetPriority(-10)
        self.assertEqual(box.GetPriority(), -10)

        box.SetStart([0,0,0])
        box.SetStop([1,2,3])
        self.assertEqual(box.GetDimension(), 3)

        box.SetStop([0,0,3])
        self.assertEqual(box.GetDimension(), 1)

        box.SetStop([1,2,3])
        self.assertTrue(box.Update())

        self.assertTrue( (box.GetBoundBox() == np.array([[0,0,0],[1,2,3]])).all())
        self.assertTrue( (box.GetStart()    == np.array([0,0,0])).all())
        self.assertTrue( (box.GetStop()     == np.array([1,2,3])).all())

        self.assertEqual(box.GetDimension(), 3)
        self.assertTrue(box.IsInside([0,0,0]))
        self.assertTrue(box.IsInside([0.5,0.5,0.5]))
        self.assertFalse(box.IsInside([-0.5,0.5,0.5]))

        tr = box.GetTransform()
        self.assertFalse(tr.HasTransform())
        self.assertFalse(box.HasTransform())

        box.AddTransform('Translate', [0, 1, 0])
        self.assertTrue(tr.HasTransform())
        self.assertTrue(box.HasTransform())

        self.assertEqual(box.GetTransform(), tr) # make sure a repeated get transform will yield always the same python object
        self.assertEqual(box.GetCoordinateSystem(), None)

        box.SetCoordinateSystem(0)
        self.assertEqual(box.GetCoordinateSystem(), 0)

        with self.assertRaises(Exception):
            # must raise an exception for the unknown/undefined keyword
            CSPrimitives.CSPrimBox(self.pset, self.metal, unknown_key = -1)


    def test_point(self):
        coord = [0,0.1,0.2]
        point = CSPrimitives.CSPrimPoint(self.pset, self.metal, coord=coord)
        self.assertTrue( (point.GetCoord() == coord).all())

        coord = np.array(coord)*2
        point.SetCoord(coord)
        self.assertTrue( (point.GetCoord() == coord).all())

        point2 = point.copy()
        self.assertFalse(point == point2)
        self.assertTrue( (point.GetCoord() == coord).all())

        self.assertTrue(point.GetID() != point2.GetID())

    def test_box(self):
        box = CSPrimitives.CSPrimBox(self.pset, self.metal, priority=10)
        self.assertEqual(box.GetPriority(), 10)
        tr = box.GetTransform()
        self.assertFalse(tr.HasTransform())

        box2 = box.copy()
        tr = box2.GetTransform()
        self.assertFalse(tr.HasTransform())
        self.assertTrue(box.GetID() != box2.GetID())

    def test_cylinder(self):
        # TEST Cylinder
        cyl = CSPrimitives.CSPrimCylinder(self.pset, self.metal)
        start = [1,2,3]
        cyl.SetStart(start)
        self.assertTrue( (cyl.GetStart()==start).all() )

        stop = np.array([5,6,7])
        cyl.SetStop(stop)
        self.assertTrue( (cyl.GetStop()==stop).all() )

        self.assertTrue( cyl.GetType()==5 )
        self.assertTrue( cyl.GetTypeName()=='Cylinder' )

        self.assertTrue( cyl.GetPriority()==0 )

        cyl.SetRadius(5)
        self.assertTrue( cyl.GetRadius()==5.0 )

        cyl2 = cyl.copy()
        self.assertTrue( cyl2.GetRadius()==5.0 )
        cyl.SetRadius(1)
        self.assertTrue( cyl2.GetRadius()==5.0 )
        self.assertTrue( (cyl2.GetStart()==start).all() )
        self.assertTrue( (cyl2.GetStop()==stop).all() )
        self.assertTrue(cyl.GetID() != cyl2.GetID())

    def test_cylinder_shell(self):
        # TEST Cylinder-Shell
        cyl_shell = CSPrimitives.CSPrimCylindricalShell(self.pset, self.metal)
        start = [1,2,3]
        cyl_shell.SetStart(start)
        self.assertTrue( (cyl_shell.GetStart()==start).all() )

        stop = np.array([5,6,7])
        cyl_shell.SetStop(stop)
        self.assertTrue( (cyl_shell.GetStop()==stop).all() )

        self.assertTrue( cyl_shell.GetType()==6 )
        self.assertTrue( cyl_shell.GetTypeName()=='CylindricalShell' )

        self.assertTrue( cyl_shell.GetPriority()==0 )

        cyl_shell.SetRadius(5)
        self.assertTrue( cyl_shell.GetRadius()==5.0 )

        cyl_shell.SetShellWidth(1.5)
        self.assertTrue( cyl_shell.GetShellWidth()==1.5 )

        cyl_shell2 = cyl_shell.copy()
        self.assertTrue( (cyl_shell2.GetStart()==start).all() )
        self.assertTrue( (cyl_shell2.GetStop()==stop).all() )
        self.assertTrue( cyl_shell2.GetRadius()==5.0 )
        self.assertTrue( cyl_shell2.GetShellWidth()==1.5 )
        self.assertTrue(cyl_shell.GetID() != cyl_shell2.GetID())

    def test_sphere(self):
        # TEST Sphere
        sphere = CSPrimitives.CSPrimSphere(self.pset, self.metal)
        center = np.array([1.1,2.2,3.3])
        sphere.SetCenter(center)
        self.assertTrue( (sphere.GetCenter()==center).all() )

        self.assertTrue( sphere.GetType()==3 )
        self.assertTrue( sphere.GetTypeName()=='Sphere' )

        self.assertTrue( sphere.GetPriority()==0 )

        sphere.SetRadius(5)
        self.assertTrue( sphere.GetRadius()==5.0 )

        sphere2 = sphere.copy()
        self.assertTrue( (sphere2.GetCenter()==center).all() )
        self.assertTrue( sphere2.GetRadius()==5.0 )
        self.assertTrue(sphere.GetID() != sphere2.GetID())

    def test_sphere_shell(self):
        # TEST Sphere-Shell
        sphere_shell = CSPrimitives.CSPrimSphericalShell(self.pset, self.metal)
        center = np.array([1.1,2.2,3.3])
        sphere_shell.SetCenter(center)
        self.assertTrue( (sphere_shell.GetCenter()==center).all() )

        self.assertTrue( sphere_shell.GetType()==4 )
        self.assertTrue( sphere_shell.GetTypeName()=='SphericalShell' )

        self.assertTrue( sphere_shell.GetPriority()==0 )

        sphere_shell.SetRadius(5)
        self.assertTrue( sphere_shell.GetRadius()==5.0 )

        sphere_shell.SetShellWidth(1.5)
        self.assertTrue( sphere_shell.GetShellWidth()==1.5 )

        sphere_shell2 = sphere_shell.copy()
        self.assertFalse(sphere_shell2==sphere_shell)
        self.assertFalse(sphere_shell2==sphere_shell)
        self.assertTrue( (sphere_shell2.GetCenter()==center).all() )
        self.assertTrue( sphere_shell2.GetRadius()==5.0 )
        self.assertTrue( sphere_shell2.GetShellWidth()==1.5 )
        self.assertTrue(sphere_shell.GetID() != sphere_shell2.GetID())

    def test_polygon(self):
        # Test polygon
        poly = CSPrimitives.CSPrimPolygon(self.pset, self.metal)
        x0 = np.array([0, 0, 1, 1])
        x1 = np.array([0, 1, 1, 0])
        poly.SetCoords(x0, x1)
        o0, o1 = poly.GetCoords()
        self.assertTrue( (o0==x0).all() )
        self.assertTrue( (o1==x1).all() )

        self.assertTrue( poly.GetQtyCoords()==4 )
        self.assertTrue( poly.GetNormDir()==0 )
        poly.SetNormDir('y')
        self.assertTrue( poly.GetNormDir()==1 )
        poly.SetNormDir(2)
        self.assertTrue( poly.GetNormDir()==2 )

        poly.SetElevation(0.123)
        self.assertTrue( poly.GetElevation()==0.123 )

        poly2 = poly.copy()
        self.assertTrue( poly2.GetNormDir()==2 )
        self.assertTrue( poly2.GetElevation()==0.123 )
        self.assertTrue(poly.GetID() != poly2.GetID())

        o0, o1 = poly2.GetCoords()
        self.assertTrue( (o0==x0).all() )
        self.assertTrue( (o1==x1).all() )


    def test_lin_poly(self):
        # Test Lin-Polygon
        linpoly = CSPrimitives.CSPrimLinPoly(self.pset, self.metal)
        x0 = np.array([0, 0, 1, 1])
        x1 = np.array([0, 1, 1, 0])
        linpoly.SetCoords(x0, x1)
        o0, o1 = linpoly.GetCoords()
        self.assertTrue( (o0==x0).all() )
        self.assertTrue( (o1==x1).all() )

        self.assertTrue( linpoly.GetQtyCoords()==4 )
        self.assertTrue( linpoly.GetNormDir()==0 )
        linpoly.SetNormDir('y')
        self.assertTrue( linpoly.GetNormDir()==1 )
        linpoly.SetNormDir(2)
        self.assertTrue( linpoly.GetNormDir()==2 )

        linpoly.SetElevation(0.123)
        self.assertTrue( linpoly.GetElevation()==0.123 )

        linpoly.SetLength(11.23)
        self.assertTrue( linpoly.GetLength()==11.23 )

        linpoly2 = linpoly.copy()
        self.assertTrue( linpoly2.GetNormDir()==2 )
        self.assertTrue( linpoly2.GetElevation()==0.123 )
        self.assertTrue( linpoly2.GetLength()==11.23 )
        o0, o1 = linpoly2.GetCoords()
        self.assertTrue( (o0==x0).all() )
        self.assertTrue( (o1==x1).all() )

    def test_rot_poly(self):
        # Test Rot-Polygon
        rotpoly = CSPrimitives.CSPrimRotPoly(self.pset, self.metal)
        x0 = np.array([0, 0, 1, 1]) + 1.5
        x1 = np.array([0, 1, 1, 0]) + 2.5
        rotpoly.SetCoords(x0, x1)
        o0, o1 = rotpoly.GetCoords()
        self.assertTrue( (o0==x0).all() )
        self.assertTrue( (o1==x1).all() )

        self.assertTrue( rotpoly.GetQtyCoords()==4 )
        self.assertTrue( rotpoly.GetNormDir()==0 )
        rotpoly.SetNormDir('y')
        self.assertTrue( rotpoly.GetNormDir()==1 )
        rotpoly.SetNormDir(0)
        self.assertTrue( rotpoly.GetNormDir()==0 )

        rotpoly.SetRotAxisDir('z')
        self.assertTrue( rotpoly.GetRotAxisDir()==2 )

        rotpoly.SetAngle(0, 2*np.pi)
        self.assertTrue( (rotpoly.GetAngle() == np.array([0, 2*np.pi])).all() )

        rotpoly2 = rotpoly.copy()
        self.assertTrue( rotpoly2.GetQtyCoords()==4 )
        self.assertTrue( rotpoly2.GetNormDir()==0 )
        self.assertTrue( rotpoly2.GetRotAxisDir()==2 )
        self.assertTrue( (rotpoly2.GetAngle() == np.array([0, 2*np.pi])).all() )
        o0, o1 = rotpoly2.GetCoords()
        self.assertTrue( (o0==x0).all() )
        self.assertTrue( (o1==x1).all() )

    def test_curve(self):
        # Test Curve
        x = np.array([0, 0, 1, 1]) + 1.5
        y = np.array([0, 1, 1, 0]) + 2.5
        z = np.array([0, 1, 3, 4])
        curve = CSPrimitives.CSPrimCurve(self.pset, self.metal)#, points=[x,y,z])
        curve.SetPoints(x, y, z)

        self.assertTrue( (curve.GetPoint(0)==np.array([1.5,2.5,0])).all() )
        self.assertTrue( curve.GetNumberOfPoints()==4 )

        curve.AddPoint([10, 10, 10.5])
        self.assertTrue( curve.GetNumberOfPoints()==5 )

        curve2 = curve.copy()
        self.assertTrue( (curve2.GetPoint(0)==np.array([1.5,2.5,0])).all() )
        self.assertTrue( curve2.GetNumberOfPoints()==5 )

        curve.ClearPoints()
        self.assertTrue( curve.GetNumberOfPoints()==0 )
        self.assertTrue( curve2.GetNumberOfPoints()==5 )

    def test_wire(self):
        # Test Wire
        wire = CSPrimitives.CSPrimWire(self.pset, self.metal)
        x = np.array([0, 0, 1, 1]) - 1.5
        y = np.array([0, 1, 1, 0]) - 2.5
        z = np.array([0, 1, 3, 4])
        wire.SetPoints(x, y, z)

        self.assertTrue( (wire.GetPoint(0)==np.array([-1.5,-2.5,0])).all() )
        self.assertTrue( wire.GetNumberOfPoints()==4 )

        wire.AddPoint([10, -10, 10.5])
        self.assertTrue( wire.GetNumberOfPoints()==5 )

        wire.SetWireRadius(1.5)
        self.assertTrue( wire.GetWireRadius()==1.5 )

        wire2 = wire.copy()
        self.assertTrue( (wire2.GetPoint(0)==np.array([-1.5,-2.5,0])).all() )
        self.assertTrue( wire2.GetNumberOfPoints()==5 )
        self.assertTrue( wire2.GetWireRadius()==1.5 )

    def test_polyhedron(self):
        # Test CSPrimPolyhedron
        ph = CSPrimitives.CSPrimPolyhedron(self.pset, self.metal)
        self.assertTrue( ph.GetNumVertices()==0 )
        self.assertTrue( ph.GetNumFaces()==0 )

        ph.AddVertex(0,0,0)
        ph.AddVertex(0,1,0)
        ph.AddVertex(1,0,0)
        self.assertTrue( ph.GetNumVertices()==3 )
        self.assertTrue( (ph.GetVertex(0)==np.array([0,0,0])).all() )

        ph.AddFace([0,1,2])
        self.assertTrue( ph.GetNumFaces()==1 )
        self.assertTrue( (ph.GetFace(0)==np.array([0,1,2])).all() )

        ph2 = ph.copy()
        self.assertTrue( ph2.GetNumVertices()==3 )
        self.assertTrue( (ph2.GetVertex(0)==np.array([0,0,0])).all() )
        self.assertTrue( ph2.GetNumFaces()==1 )
        self.assertTrue( (ph2.GetFace(0)==np.array([0,1,2])).all() )

    def test_polyhedron2(self):
        ## Test CSPrimPolyhedron
        ph = CSPrimitives.CSPrimPolyhedron(self.pset, self.metal)

        # create pyramid
        x0 = 0
        y0 = 0
        z0 = 0
        width  = 50
        height = 150
        ph.AddVertex(x0        , y0        , z0)  #0
        ph.AddVertex(x0+width  , y0        , z0)  #1
        ph.AddVertex(x0+width  , y0+width  , z0)  #2
        ph.AddVertex(x0        , y0+width  , z0)  #3
        ph.AddVertex(x0+width/2, y0+width/2, z0+height)  #4 (tip)

        ph.AddFace([0,1,2])
        ph.AddFace([0,2,3])
        ph.AddFace([1,0,4])
        ph.AddFace([0,3,4])
        ph.AddFace([3,2,4])
        ph.AddFace([2,1,4])

        self.assertTrue((ph.GetFace(1)==np.array([0,2,3])).all())
        self.assertEqual(ph.GetNumVertices(), 5)
        self.assertEqual(ph.GetNumFaces()   , 6)

        self.assertTrue (ph.IsInside([x0+width/2, y0+width/2, z0+height/2]))
        self.assertFalse(ph.IsInside([x0        , y0        , z0+height/2]))

        ph2 = ph.copy()
        self.assertTrue((ph2.GetFace(1)==np.array([0,2,3])).all())
        self.assertEqual(ph2.GetNumVertices(), 5)
        self.assertEqual(ph2.GetNumFaces()   , 6)

        self.assertTrue (ph2.IsInside([x0+width/2, y0+width/2, z0+height/2]))
        self.assertFalse(ph2.IsInside([x0        , y0        , z0+height/2]))

    def test_polyhedron_reader(self):
        ## Test CSPrimPolyhedronReader
        phr = CSPrimitives.CSPrimPolyhedronReader(self.pset, self.metal)
        phr.SetFilename(os.path.join(DATA_PATH, 'sphere.stl'))
        self.assertTrue( phr.ReadFile() )
        self.assertEqual(phr.GetNumVertices(), 50)
        self.assertEqual(phr.GetNumFaces(), 96)

        phr2 = phr.copy()
        self.assertTrue( phr2.ReadFile() )
        self.assertEqual(phr2.GetNumVertices(), 50)
        self.assertEqual(phr2.GetNumFaces(), 96)

if __name__ == '__main__':
    unittest.main()
