function CSX = AddSphericalShell(CSX, propName, prio, center, rad, shell_width, varargin)
% function CSX = AddSphericalShell(CSX, propName, prio, center, rad, shell_width, varargin)
%
% Add a sphere shell to CSX and assign to a property with name <propName>.
%
%  center:      sphere center coordinates
%  rad  :       sphere radius
%  shell_width: sphere shell width
%  prio :       primitive priority
%
%  Note:
%       the inner radius of this shell is rad-shell_width/2
%       the outer radius of this shell is rad+shell_width/2
%
%   example:
%       CSX = AddMetal(CSX,'metal'); %create PEC with propName 'metal'
%       CSX = AddSphericalShell(CSX,'metal',10,[0 0 0],50,10);
%
% See also AddBox, AddCylindricalShell, AddCylinder, AddSphere,
% AddCurve, AddWire, AddMetal
%
% CSXCAD matlab interface
% -----------------------
% author: Thorsten Liebig

sphere.ATTRIBUTE.Priority = prio;
sphere.ATTRIBUTE.Radius = rad;
sphere.ATTRIBUTE.ShellWidth = shell_width; 

sphere.Center.ATTRIBUTE.X=center(1);
sphere.Center.ATTRIBUTE.Y=center(2);
sphere.Center.ATTRIBUTE.Z=center(3);

sphere = AddPrimitiveArgs(sphere,varargin{:});

CSX = Add2Property(CSX,propName, sphere, 'SphericalShell');