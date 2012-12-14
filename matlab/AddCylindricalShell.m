function CSX = AddCylindricalShell(CSX, propName, prio, start, stop, rad, shell_width, varargin)
%function CSX = AddCylindricalShell(CSX, propName, prio, start, stop, rad, shell_width, varargin)
%
% Add a cylinder shell to CSX and assign to a property with name <propName>.
%
%  start:       cylinder axis start coordinates
%  stop :       cylinder axis box stop  coordinates
%  rad  :       cylinder radius
%  shell_width: cylinder shell width
%  prio :       primitive priority
%
%  Note:
%       the inner radius of this shell is rad-shell_width/2
%       the outer radius of this shell is rad+shell_width/2
%
%   example:
%       CSX = AddMetal(CSX,'metal'); %create PEC with propName 'metal'
%       CSX = AddCylindricalShell(CSX,'metal',10,[0 0 0],[0 0 200],50,10);
%
% See also AddBox, AddCylinder, AddSphere, AddSphericalShell,
% AddCurve, AddWire, AddMetal
%
% CSXCAD matlab interface
% -----------------------
% author: Thorsten Liebig

cylinder.ATTRIBUTE.Priority = prio;
cylinder.ATTRIBUTE.Radius = rad; 
cylinder.ATTRIBUTE.ShellWidth = shell_width; 

cylinder.P1.ATTRIBUTE.X=start(1);
cylinder.P1.ATTRIBUTE.Y=start(2);
cylinder.P1.ATTRIBUTE.Z=start(3);

cylinder.P2.ATTRIBUTE.X=stop(1);
cylinder.P2.ATTRIBUTE.Y=stop(2);
cylinder.P2.ATTRIBUTE.Z=stop(3);

cylinder = AddPrimitiveArgs(cylinder,varargin{:});

CSX = Add2Property(CSX,propName, cylinder, 'CylindricalShell');
