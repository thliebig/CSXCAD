function CSX = ImportPLY(CSX, propName, prio, filename, varargin)
% function CSX = ImportPLY(CSX, propName, prio, filename, varargin)
%
% example:
%   CSX = AddMetal( CSX, 'cad_model' ); % create a perfect electric conductor (PEC)
%   CSX = ImportPLY(CSX, 'cad_model',10, 'sphere.ply','Transform',{'Scale',1/unit});
%
%   Note: make sure the file 'sphere.ply' is in the working directory
%
% See also AddBox, AddCylinder, AddCylindricalShell, AddSphere, AddSphericalShell,
% AddCurve, AddWire, AddMetal, ImportSTL
%
% CSXCAD matlab interface
% -----------------------
% author: Thorsten Liebig

plyfile.ATTRIBUTE.Priority = prio;
plyfile.ATTRIBUTE.FileName = filename;
plyfile.ATTRIBUTE.FileType = 'PLY';

plyfile = AddPrimitiveArgs(plyfile,varargin{:});

CSX = Add2Property(CSX,propName, plyfile, 'PolyhedronReader');
