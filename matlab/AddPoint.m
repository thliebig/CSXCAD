function CSX = AddPoint(CSX, propName, prio, pos, varargin)
% CSX = AddPoint(CSX, propName, prio, pos, varargin)
%
% CSXCAD matlab interface

point.ATTRIBUTE.Priority = prio;

point.ATTRIBUTE.X=pos(1);
point.ATTRIBUTE.Y=pos(2);
point.ATTRIBUTE.Z=pos(3);

point = AddPrimitiveArgs(point,varargin{:});

CSX = Add2Property(CSX,propName, point, 'Point');
