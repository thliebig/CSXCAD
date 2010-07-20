function CSX = AddPoint(CSX, propName, prio, pos)
% function CSX = AddBox(CSX, propName, prio, pos)
%
% CSXCAD matlab interface

point.ATTRIBUTE.Priority = prio;

point.ATTRIBUTE.X=pos(1);
point.ATTRIBUTE.Y=pos(2);
point.ATTRIBUTE.Z=pos(3);

CSX = Add2Property(CSX,propName, point, 'Point');
