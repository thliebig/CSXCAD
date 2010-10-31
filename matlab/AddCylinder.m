function CSX = AddCylinder(CSX, propName, prio, start, stop, rad, varargin)
% function CSX = AddCylinder(CSX, propName, prio, start, stop, rad, varargin)
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