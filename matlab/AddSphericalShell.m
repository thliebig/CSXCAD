function CSX = AddSphericalShell(CSX, propName, prio, center, rad, shell_width, varargin)
% function CSX = AddSphericalShell(CSX, propName, prio, center, rad, shell_width, varargin)
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

CSX = Add2Property(CSX,propName, sphere, 'Sphere');