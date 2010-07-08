function CSX = AddLinPoly( CSX, materialname, prio, normVector, elevation, points, Length)
% CSX = AddLinPoly( CSX, materialname, prio, normVector, elevation, points, Length)
%
% CSX: CSX-object created by InitCSX()
% materialname: created by AddMetal() or AddMaterial()
% prio: priority
% normVector: normal vector of the polygon
% elevation: position of the polygon plane
% points: two-dimensional coordinates
% length: linear extrution in normal direction, starting at elevation
%
% example:
% p(1,1) = 0;  % x-coord point 1
% p(2,1) = 0;  % y-coord point 1
% p(1,2) = 10; % x-coord point 2
% p(2,2) = 20; % y-coord point 2
% CSX = AddLinPoly( CSX, 'PEC', 1, [0 0 1], 254, p , 10)
%
% Thorsten Liebig <thorsten.liebig@gmx.de>
% July 6 2010
%
% See also InitCSX AddMetal AddMaterial AddPolygon

polygon.ATTRIBUTE.Priority = prio;
polygon.ATTRIBUTE.Elevation = elevation;
polygon.ATTRIBUTE.Length = Length;
polygon.NormDir.ATTRIBUTE.X = normVector(1);
polygon.NormDir.ATTRIBUTE.Y = normVector(2);
polygon.NormDir.ATTRIBUTE.Z = normVector(3);

polygon.Vertex = {};
for s=1:size(points,2)
    polygon.Vertex{end+1}.ATTRIBUTE.X1 = points(1,s);
    polygon.Vertex{end}.ATTRIBUTE.X2   = points(2,s);
end

CSX = Add2Property( CSX, materialname, polygon, 'LinPoly');
