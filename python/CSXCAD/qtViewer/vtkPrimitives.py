import os, sys
import numpy as np

import vtk

from CSXCAD.CSXCAD import ContinuousStructure
from CSXCAD import CSProperties
from CSXCAD import CSPrimitives
from CSXCAD import CSRectGrid

def vecLength(v):
    return np.sqrt(sum(np.pow(v,2)))

def vectorAngel(v1, v2):
    v1L=vecLength(v1)
    v2L=vecLength(v2)
    scalarP=np.dot(v1,v2)
    if (v1L*v2L)==0:
        return 0
    angel=scalarP/(v1L*v2L)
    if angel>1:
        return 0.0
    elif angel<-1:
        return 180.0
    else:
        return np.acos(angel)*180.0/np.pi

def sortStartStop(start, stop):
    for n in range(3):
        if start[n]>stop[n]:
            start[n], stop[n] = stop[n], start[n]
    return start, stop

def createLinePoly(coords, radius=None, arc_resolution=8):
    points = vtk.vtkPoints()
    poly = vtk.vtkCellArray()
    profile = vtk.vtkPolyData()
    for n in range(len(coords)):
        points.InsertPoint(n, coords[n])
    for n in range(1, len(coords)):
        poly.InsertNextCell(2)
        poly.InsertCellPoint(n-1)
        poly.InsertCellPoint(n)
    profile.SetPoints(points)
    profile.SetLines(poly)
    if radius:
        tube_flt = vtk.vtkTubeFilter()
        tube_flt.SetNumberOfSides(arc_resolution)
        tube_flt.SetInputData(profile)
        tube_flt.SetRadius(radius)
        tube_flt.Update()
        return tube_flt.GetOutput()
    return profile

def linearExtrude(src_port, ext_vec):
    extrude = vtk.vtkLinearExtrusionFilter()
    extrude.SetInputConnection(src_port)
    extrude.SetExtrusionTypeToVectorExtrusion()
    extrude.SetVector(ext_vec)
    extrude.CappingOn()
    return extrude

def rotationalExtrude(src_port, rot_axis, angle_range, arc_resolution=8):
    extrude = vtk.vtkRotationalExtrusionFilter()
    transform = vtkTransform()
    InvTransform = vtk.vtkTransform()
    transformFilter = vtk.vtkTransformPolyDataFilter()
    InvTransformFilter = vtk.vtkTransformPolyDataFilter()

    start = rot_axis[0,:]
    vector = rot_axis[1,:] - rot_axis[0,:]

    vec_len = vecLength(vector)
    vec_unit = vector/vec_len

    d = np.sqrt(vec_unit[1]*vec_unit[1]+vec_unit[2]*vec_unit[2])
    alpha = np.atan2( vec_unit[1],vec_unit[2])/np.acos(-1)*180
    beta  = np.atan2(-vec_unit[0],d)/np.acos(-1)*180

    InvTransform.Translate(-start)
    InvTransform.RotateX(alpha)
    InvTransform.RotateY(beta)

    InvTransformFilter.SetInputConnection(src_port)
    InvTransformFilter.SetTransform(InvTransform)

    extrude.SetInputConnection(InvTransformFilter.GetOutputPort())
    extrude.SetResolution(arc_resolution)
    extrude.SetAngle(angle_range[1]-angle_range[0])

    transform.RotateZ(-angle_range[0])

    transform.Translate(-start)
    transform.RotateX(alpha)
    transform.RotateY(beta)
    transform.Inverse()

    transform.PostMultiply()

    transformFilter.SetInputConnection(extrude.GetOutputPort())
    transformFilter.SetTransform(transform)
    return transformFilter

def createPolygonPoly(coords):
    points = vtk.vtkPoints()
    poly = vtk.vtkCellArray()
    profile = vtk.vtkPolyData()
    extrude = vtk.vtkLinearExtrusionFilter()
    N = len(coords)
    for i in range(N):
         points.InsertPoint(i, coords[i,:])
    poly.InsertNextCell(N)
    for i in range(N):
        poly.InsertCellPoint(i)
    poly.InsertCellPoint(0)
    profile.SetPoints(points)
    profile.SetPolys(poly)
    tf = vtk.vtkTriangleFilter()
    tf.SetInputData(profile)
    return tf

def createCylinderPoly(start, stop, radius, arc_resolution=8):
    direction = [stop[0]-start[0],stop[1]-start[1],stop[2]-start[2]]
    length = np.sqrt(sum(np.pow(direction,2)))
    if length == 0 and radius <= 0:
        return None
    elif length == 0:
        print('createCylinderPoly: Warning: Zero length cylinder!!!')
        return None

    alpha=0
    beta=0
    src = vtk.vtkCylinderSource()
    src.SetResolution(arc_resolution)
    src.SetRadius(radius)
    src.SetHeight(length)

    transform = vtk.vtkTransform()
    tr_flt = vtk.vtkTransformPolyDataFilter()
    alpha = vectorAngel([direction[0],np.sqrt(direction[1]*direction[1]+direction[2]*direction[2]),0], [0,1,0])
    beta  = vectorAngel([0,direction[1],direction[2]], [0,1,0])
    if direction[0]>0:
        alpha=-alpha
    if direction[2]<0:
        beta=-beta
    transform.Translate(direction[0]/2+start[0],direction[1]/2+start[1],direction[2]/2+start[2])
    transform.RotateWXYZ(beta,1,0,0)
    transform.RotateWXYZ(alpha,0,0,1)

    transform.PostMultiply()
    tr_flt.SetInputConnection(src.GetOutputPort())
    tr_flt.SetTransform(transform)
    tr_flt.Update()
    return tr_flt.GetOutput()

def createCylinderShellPoly(start, stop, radius, width, arc_resolution=8):
    r_i, r_o = radius-width/2, radius+width/2
    direction = [stop[0]-start[0],stop[1]-start[1],stop[2]-start[2]]
    length = np.sqrt(sum(np.pow(direction,2)))
    if length == 0 and radius <= 0:
        return None
    elif length == 0:
        print('createCylinderPoly: Warning: Zero length cylinder!!!')
        return None

    # create a disk in xy plane
    diskSource = vtk.vtkDiskSource()
    diskSource.SetInnerRadius(r_i)
    diskSource.SetOuterRadius(r_o)
    diskSource.SetCircumferentialResolution(arc_resolution)

	# extrude in +z
    linearExtrusionFilter = vtk.vtkLinearExtrusionFilter()
    linearExtrusionFilter.SetInputConnection(diskSource.GetOutputPort())
    linearExtrusionFilter.SetExtrusionTypeToNormalExtrusion()
    linearExtrusionFilter.SetVector(0,0,length)
    linearExtrusionFilter.Update()

    # create transform to correctly orient and move the cylinder shell
    transform = vtk.vtkTransform()
    tr_flt = vtk.vtkTransformPolyDataFilter()

    transform.PostMultiply()
    phi   = np.atan2(direction[1], direction[0])
    theta = np.acos(direction[2] / length)
    transform.RotateY(np.rad2deg(theta))
    transform.RotateZ(np.rad2deg(phi))
    transform.Translate(start)

    tr_flt.SetInputConnection(linearExtrusionFilter.GetOutputPort())
    tr_flt.SetTransform(transform)
    tr_flt.Update()
    return tr_flt.GetOutput()

def transformCylindricalCoords(coord, src_CS=CSRectGrid.CoordinateSystem.CYLINDRICAL):
    if src_CS is not CSRectGrid.CoordinateSystem.CYLINDRICAL:
        return coord
    return [coord[0] * np.cos(coord[1]), coord[0] * np.sin(coord[1]), coord[2]]

def createCylindricalCubePoly(start, stop, arc_resolution=8):
    act_delta = np.deg2rad(3.0)
    src = None
    dr = abs(start[0]-stop[0])
    da = abs(start[1]-stop[1])
    if da>2*np.pi:
        stop[1] = start[1] + 2*np.pi
        da = 2*np.pi
    dz = abs(start[2]-stop[2])
    delta = max(dr, dz)*1e-6
    if dr>delta and dz>delta:# 3D object
        src = vtk.vtkPlaneSource()
        src.SetOrigin(transformCylindricalCoords(start))
        src.SetPoint1(transformCylindricalCoords([start[0], start[1], stop[2]]))
        src.SetPoint2(transformCylindricalCoords([stop[0], start[1], start[2]]))
    elif dr>delta:
        src = vtk.vtkLineSource()
        src.SetPoint1(transformCylindricalCoords(start))
        src.SetPoint2(transformCylindricalCoords([stop[0], start[1], start[2]]))
    elif dz>delta:
        src = vtk.vtkLineSource()
        src.SetPoint1(transformCylindricalCoords(start))
        src.SetPoint2(transformCylindricalCoords([start[0], start[1], stop[2]]))

    if da>np.deg2rad(0.1): # spanning over an angle
        steps = int(abs(start[1]-stop[1])/act_delta)
        if src is None: # this means line in a dir
            src = vtk.vtkLineSource()
            coords = []
            for ang in np.linspace(start[1], stop[1], max(2, steps)):
                coords.append(transformCylindricalCoords([start[0], ang, start[2]]))
            return createLinePoly(coords)

        extrude = vtk.vtkRotationalExtrusionFilter()
        extrude.SetInputConnection(src.GetOutputPort())
        extrude.SetResolution(steps)
        extrude.SetAngle(np.rad2deg(da))
        if da<2*np.pi:
            extrude.CappingOn()
        else:
            extrude.CappingOff()
        extrude.Update()
        return extrude.GetOutput()
    else:
        src.Update()
        return src.GetOutput()

def convertPrim2Polydata(prim, default_CS=CSRectGrid.CoordinateSystem.CARTESIAN, arc_resolution=16):
    pt = prim.GetType()
    prim_cs = prim.GetCoordinateSystem()
    if prim_cs is None:
        prim_cs = default_CS
    dim = prim.GetDimension()
    poly_data = None
    #print(f'convertPrim2Polydata {prim.__class__.__name__}, {pt=}, {dim=}, {prim_cs=}')
    if pt == CSPrimitives.PrimitiveType.POINT:
        pass
    elif pt == CSPrimitives.PrimitiveType.BOX and prim_cs in [None, CSRectGrid.CoordinateSystem.CARTESIAN]:
        start, stop = sortStartStop(prim.GetStart(), prim.GetStop())
        if dim>=2:
            src = vtk.vtkCubeSource()
            src.SetBounds([start[0], stop[0], start[1], stop[1], start[2], stop[2]])
            src.Update()
            poly_data = src.GetOutput()
        elif dim==1:
            poly_data = createLinePoly([start, stop])
    elif pt == CSPrimitives.PrimitiveType.BOX and prim_cs == CSRectGrid.CoordinateSystem.CYLINDRICAL:
        start, stop = sortStartStop(prim.GetStart(), prim.GetStop())
        poly_data = createCylindricalCubePoly(start, stop)
    elif pt == CSPrimitives.PrimitiveType.SPHERE:
        src = vtk.vtkSphereSource()
        src.SetCenter(transformCylindricalCoords(prim.GetCenter(), prim_cs))
        src.SetRadius(prim.GetRadius())
        src.SetPhiResolution(arc_resolution)
        src.SetThetaResolution(arc_resolution)
        src.Update()
        poly_data = src.GetOutput()
    elif pt == CSPrimitives.PrimitiveType.SPHERICALSHELL:
        # This is not really a shell
        # somehow the vtk.vtkBooleanOperationPolyDataFilter does not work anymore as it used too!
        src_o = vtk.vtkSphereSource()
        r = prim.GetRadius()
        w = prim.GetShellWidth()
        center = transformCylindricalCoords(prim.GetCenter(), prim_cs)
        src_o.SetCenter(center)
        src_o.SetRadius(r+w/2)
        src_o.SetPhiResolution(arc_resolution)
        src_o.SetThetaResolution(arc_resolution)
        src_o.Update()
        apd = vtk.vtkAppendPolyData()
        apd.AddInputData(src_o.GetOutput())

        src_i = vtk.vtkSphereSource()
        src_i.SetCenter(center)
        src_i.SetRadius(r-w/2)
        src_i.SetPhiResolution(arc_resolution)
        src_i.SetThetaResolution(arc_resolution)
        src_i.Update()
        apd.AddInputData(src_o.GetOutput())
        apd.Update()
        poly_data = apd.GetOutput()
    elif pt == CSPrimitives.PrimitiveType.CYLINDER:
        start, stop = sortStartStop(prim.GetStart(), prim.GetStop())
        start = transformCylindricalCoords(start, prim_cs)
        stop  = transformCylindricalCoords(stop, prim_cs)
        poly_data = createCylinderPoly(start, stop, prim.GetRadius(), arc_resolution=arc_resolution)
    elif pt == CSPrimitives.PrimitiveType.CYLINDRICALSHELL:
        start, stop = sortStartStop(prim.GetStart(), prim.GetStop())
        start = transformCylindricalCoords(start, prim_cs)
        stop  = transformCylindricalCoords(stop, prim_cs)
        poly_data = createCylinderShellPoly(start, stop, prim.GetRadius(), prim.GetShellWidth(), arc_resolution=arc_resolution)
    elif pt in [CSPrimitives.PrimitiveType.POLYGON, CSPrimitives.PrimitiveType.LINPOLY, CSPrimitives.PrimitiveType.ROTPOLY]:
        if prim_cs == CSRectGrid.CoordinateSystem.CYLINDRICAL:
            print('convertPrim2Polydata:: Error, unable to display polygons in cylindrical coordinates!')
            return None
        x0, x1 = prim.GetCoords()
        if len(x0) <= 2:
            return None
        nd = prim.GetNormDir()
        ndP  = (nd+1)%3
        ndPP = (nd+2)%3
        coords = np.zeros([len(x0), 3])
        coords[:,nd  ] = prim.GetElevation()
        coords[:,ndP ] = x0
        coords[:,ndPP] = x1
        pdf = createPolygonPoly(coords)
        if pdf is None:
            return None
        if pt == CSPrimitives.PrimitiveType.POLYGON:
            pdf.Update()
            poly_data = pdf.GetOutput()
        elif pt == CSPrimitives.PrimitiveType.LINPOLY:
            ext_vec = [0,0,0]
            ext_vec[nd] = prim.GetLength()
            extrude = linearExtrude(pdf.GetOutputPort(),ext_vec)
            extrude.Update()
            poly_data = extrude.GetOutput()
        elif pt == CSPrimitives.PrimitiveType.ROTPOLY:
            rot_axis = np.zeros([2,3])
            rot_axis[1, prim.GetRotAxisDir()]=1
            rot_ext = rotationalExtrude(pdf.GetOutputPort(), rot_axis, np.rad2deg(prim.GetAngle()), arc_resolution=arc_resolution)
            rot_ext.Update()
            poly_data = rot_ext.GetOutput()
    elif pt in [CSPrimitives.PrimitiveType.POLYHEDRON, CSPrimitives.PrimitiveType.POLYHEDRONREADER]:
        if prim_cs == CSRectGrid.CoordinateSystem.CYLINDRICAL:
            print('convertPrim2Polydata:: Error, unable to display polyhedron in cylindrical coordinates!')
            return None
        NV = prim.GetNumVertices()
        NF = prim.GetNumFaces()
        poly = vtk.vtkCellArray()
        poly_data = vtk.vtkPolyData()
        points = vtk.vtkPoints()
        for i in range(NV):
            points.InsertPoint(i, prim.GetVertex(i))
        for i in range(NF):
            if not prim.GetFaceValid(i):
                continue
            vertices=prim.GetFace(i)
            poly.InsertNextCell(len(vertices))
            for p in range(len(vertices)):
                poly.InsertCellPoint(vertices[p])
        poly_data.SetPoints(points)
        poly_data.SetPolys(poly)
    elif pt in [CSPrimitives.PrimitiveType.CURVE, CSPrimitives.PrimitiveType.WIRE]:
        N = prim.GetNumberOfPoints()
        if N<2:
            return None
        coords = [transformCylindricalCoords(prim.GetPoint(n), prim_cs) for n in range(N)]
        radius = None if pt == CSPrimitives.PrimitiveType.CURVE else prim.GetWireRadius()
        poly_data = createLinePoly(coords, radius, arc_resolution)
    if poly_data is None:
        return None

    if not prim.HasTransform():
        return poly_data

    tf = prim.GetTransform()
    tf_matrix = tf.GetMatrix()

    flt = vtk.vtkTransformPolyDataFilter()
    vtrans = vtk.vtkTransform()
    flt.SetInputData(poly_data)
    vtrans.SetMatrix(tf_matrix.ravel())
    flt.SetTransform(vtrans)
    flt.Update()
    return flt.GetOutput()

def renderPrimitives(prop, default_CS=CSRectGrid.CoordinateSystem.CARTESIAN, arc_resolution=8):
    apd = vtk.vtkAppendPolyData()
    has_any = False
    for prim in prop.GetAllPrimitives():
        pd = convertPrim2Polydata(prim, default_CS=default_CS, arc_resolution=arc_resolution)
        if pd is not None:
            apd.AddInputData(pd)
            has_any = True
        else:
            print(f'renderPrimitives::Warning: Unable to render primitive "{prim}" ({prop})')

    if not has_any:
        return
    apd.Update()
    mapper = vtk.vtkPolyDataMapper()
    mapper.SetInputConnection(apd.GetOutputPort())

    actor = vtk.vtkActor()
    actor.SetMapper(mapper)

    # set color and opacity of property
    rgba = prop.GetFillColor()
    actor.GetProperty().SetColor([rgba[n]/255 for n in range(3)])
    actor.GetProperty().SetOpacity(rgba[3]/255)
    return actor
