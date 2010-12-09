function CSX = AddSphere(CSX, propName, prio, center, rad, varargin)
% function CSX = AddSphere(CSX, propName, prio, center, rad, varargin)
%
% Add a sphere to CSX and assign to a property with name <propName>.
%
%  center:  sphere center coordinates
%  rad  :   sphere radius
%  prio :   primitive priority
%
%   example:
%       CSX = AddMetal(CSX,'metal'); %create PEC with propName 'metal'
%       CSX = AddSphere(CSX,'metal',10,[0 0 0],50);
%
% See also AddBox, AddCylindricalShell, AddCylinder, AddSphericalShell,
% AddCurve, AddWire, AddMetal
%
% CSXCAD matlab interface
% -----------------------
% author: Thorsten Liebig

sphere.ATTRIBUTE.Priority = prio;
sphere.ATTRIBUTE.Radius = rad;

sphere.Center.ATTRIBUTE.X=center(1);
sphere.Center.ATTRIBUTE.Y=center(2);
sphere.Center.ATTRIBUTE.Z=center(3);

sphere = AddPrimitiveArgs(sphere,varargin{:});

CSX = Add2Property(CSX,propName, sphere, 'Sphere');