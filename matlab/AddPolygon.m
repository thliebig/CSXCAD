function CSX = AddPolygon( CSX, materialname, prio, normVector, elevation, points, varargin)
% CSX = AddPolygon( CSX, materialname, prio, normVector, elevation, points, varargin)
%
% CSX: CSX-object created by InitCSX()
% materialname: created by AddMetal() or AddMaterial()
% prio: priority
% normVector: normal vector of the polygon
% elevation: position of the polygon plane
% points: two-dimensional coordinates
%
% example:
% p(1,1) = 0;  % x-coord point 1
% p(2,1) = 0;  % y-coord point 1
% p(1,2) = 10; % x-coord point 2
% p(2,2) = 20; % y-coord point 2
% CSX = AddPolygon( CSX, 'PEC', 1, [0 0 1], 254, p )
%
%
% Sebastian Held <sebastian.held@gmx.de>
% Jun 1 2010
%
% See also InitCSX AddMetal AddMaterial

polygon.ATTRIBUTE.Priority = prio;
polygon.ATTRIBUTE.Elevation = elevation;
polygon.NormDir.ATTRIBUTE.X = normVector(1);
polygon.NormDir.ATTRIBUTE.Y = normVector(2);
polygon.NormDir.ATTRIBUTE.Z = normVector(3);

polygon.Vertex = {};
for s=1:size(points,2)
    polygon.Vertex{end+1}.ATTRIBUTE.X1 = points(1,s);
    polygon.Vertex{end}.ATTRIBUTE.X2   = points(2,s);
end

polygon = AddPrimitiveArgs(polygon,varargin{:});

CSX = Add2Property( CSX, materialname, polygon, 'Polygon');
