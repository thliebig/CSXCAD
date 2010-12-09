function CSX = AddCylinder(CSX, propName, prio, start, stop, rad, varargin)
% function CSX = AddCylinder(CSX, propName, prio, start, stop, rad, varargin)
%
% Add a cylinder to CSX and assign to a property with name <propName>.
%
%  start:   cylinder axis start coordinates
%  stop :   cylinder axis box stop  coordinates
%  rad  :   cylinder radius
%  prio :   primitive priority
%
%   example:
%       CSX = AddMetal(CSX,'metal'); %create PEC with propName 'metal'
%       CSX = AddCylinder(CSX,'metal',10,[0 0 0],[0 0 200],50);
%
% See also AddBox, AddCylindricalShell, AddSphere, AddSphericalShell,
% AddCurve, AddWire, AddMetal
%
% CSXCAD matlab interface
% -----------------------
% author: Thorsten Liebig

cylinder.ATTRIBUTE.Priority = prio;
cylinder.ATTRIBUTE.Radius = rad; 

cylinder.P1.ATTRIBUTE.X=start(1);
cylinder.P1.ATTRIBUTE.Y=start(2);
cylinder.P1.ATTRIBUTE.Z=start(3);

cylinder.P2.ATTRIBUTE.X=stop(1);
cylinder.P2.ATTRIBUTE.Y=stop(2);
cylinder.P2.ATTRIBUTE.Z=stop(3);

cylinder = AddPrimitiveArgs(cylinder,varargin{:});

CSX = Add2Property(CSX,propName, cylinder, 'Cylinder');