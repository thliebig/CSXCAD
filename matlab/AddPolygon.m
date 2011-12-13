function CSX = AddPolygon( CSX, materialname, prio, normDir, elevation, points, varargin)
% CSX = AddPolygon( CSX, materialname, prio, normDir, elevation, points, varargin)
%
% CSX:          CSX-object created by InitCSX()
% materialname: created by AddMetal() or AddMaterial()
% prio:         priority
% normDir:      normal direction of the polygon (0->x, 1->y, 2->z)
% elevation:    position of the polygon plane
% points:       two-dimensional coordinates
%
% Warning: Polygon has to be defined using Cartesian Coords
%          for use with cylindrical mesh, set 'CoordSystem',0
%
% example:
% p(1,1) = 0;  % x-coord point 1
% p(2,1) = 0;  % y-coord point 1
% p(1,2) = 10; % x-coord point 2
% p(2,2) = 20; % y-coord point 2
% CSX = AddPolygon( CSX, 'PEC', 1, 2, 254, p, 'CoordSystem',0)
%
%
% (c) 2011 Thorsten Liebig <thorsten.liebig@gmx.de>
% (c) 2010 Sebastian Held <sebastian.held@gmx.de>
%
% See also InitCSX AddMetal AddMaterial AddLinPoly AddRotPoly

polygon.ATTRIBUTE.Priority = prio;
polygon.ATTRIBUTE.Elevation = elevation;
polygon.ATTRIBUTE.NormDir = normDir;

polygon.Vertex = {};
for s=1:size(points,2)
    polygon.Vertex{end+1}.ATTRIBUTE.X1 = points(1,s);
    polygon.Vertex{end}.ATTRIBUTE.X2   = points(2,s);
end

polygon = AddPrimitiveArgs(polygon,varargin{:});

CSX = Add2Property( CSX, materialname, polygon, 'Polygon');
