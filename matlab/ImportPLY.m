function CSX = ImportPLY(CSX, propName, prio, filename, varargin)
% function CSX = ImportPLY(CSX, propName, prio, filename, varargin)
%
%
% See also AddBox, AddCylinder, AddCylindricalShell, AddSphere, AddSphericalShell,
% AddCurve, AddWire, AddMetal
%
% CSXCAD matlab interface
% -----------------------
% author: Thorsten Liebig

plyfile.ATTRIBUTE.Priority = prio;
plyfile.ATTRIBUTE.FileName = filename;
plyfile.ATTRIBUTE.FileType = 'PLY';

plyfile = AddPrimitiveArgs(plyfile,varargin{:});

CSX = Add2Property(CSX,propName, plyfile, 'PolyhedronReader');
