function CSX = ImportSTL(CSX, propName, prio, filename, varargin)
% function CSX = ImportSTL(CSX, propName, prio, filename, varargin)
%
% example:
%   CSX = AddMetal( CSX, 'cad_model' ); % create a perfect electric conductor (PEC)
%   CSX = ImportSTL(CSX, 'cad_model',10, 'sphere.stl','Transform',{'Scale',1/unit});
%
%   Note: make sure the file 'sphere.stl' is in the working directory
%
% See also AddBox, AddCylinder, AddCylindricalShell, AddSphere, AddSphericalShell,
% AddCurve, AddWire, AddMetal, ImportPLY
%
% CSXCAD matlab interface
% -----------------------
% author: Thorsten Liebig

stlfile.ATTRIBUTE.Priority = prio;
stlfile.ATTRIBUTE.FileName = filename;
stlfile.ATTRIBUTE.FileType = 'STL';

stlfile = AddPrimitiveArgs(stlfile,varargin{:});

CSX = Add2Property(CSX,propName, stlfile, 'PolyhedronReader');
