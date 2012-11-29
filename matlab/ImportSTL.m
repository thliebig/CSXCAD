function CSX = ImportSTL(CSX, propName, prio, filename, varargin)
% function CSX = ImportSTL(CSX, propName, prio, filename, varargin)
%
%
% See also AddBox, AddCylinder, AddCylindricalShell, AddSphere, AddSphericalShell,
% AddCurve, AddWire, AddMetal
%
% CSXCAD matlab interface
% -----------------------
% author: Thorsten Liebig

stlfile.ATTRIBUTE.Priority = prio;
stlfile.ATTRIBUTE.FileName = filename;
stlfile.ATTRIBUTE.FileType = 'STL';

stlfile = AddPrimitiveArgs(stlfile,varargin{:});

CSX = Add2Property(CSX,propName, stlfile, 'PolyhedronReader');
