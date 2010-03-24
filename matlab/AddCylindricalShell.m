function CSX = AddCylindricalShell(CSX, propName, prio, start, stop, rad, shell_width)
%function CSX = AddCylindricalShell(CSX, propName, prio, start, stop, rad, shell_width)
%
% CSXCAD matlab interface
% -----------------------
% author: Thorsten Liebig

cylinder.ATTRIBUTE.Priority = prio;
cylinder.ATTRIBUTE.Radius = rad; 
cylinder.ATTRIBUTE.ShellWidth = shell_width; 

cylinder.P0.ATTRIBUTE.X=start(1);
cylinder.P0.ATTRIBUTE.Y=start(2);
cylinder.P0.ATTRIBUTE.Z=start(3);

cylinder.P1.ATTRIBUTE.X=stop(1);
cylinder.P1.ATTRIBUTE.Y=stop(2);
cylinder.P1.ATTRIBUTE.Z=stop(3);

CSX = Add2Property(CSX,propName, cylinder, 'CylindricalShell');