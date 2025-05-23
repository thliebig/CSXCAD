function CSX = AddLinPoly( CSX, materialname, prio, normDir, elevation, points, Length, varargin)
% CSX = AddLinPoly( CSX, materialname, prio, normDir, elevation, points, Length, varargin)
%
% - CSX:          CSX-object created by InitCSX()
% - materialname: created by AddMetal() or AddMaterial()
% - prio:         priority
% - normDir:      normal direction of the polygon,
%                 e.g. 'x', 'y' or 'z', or numeric 0..2
% - elevation:    position of the polygon plane
% - points:       two-dimensional coordinates
% - length:       linear extrution in normal direction, starting at elevation
%
% Warning: Polygon has to be defined using Cartesian Coords
%          for use with cylindrical mesh, set 'CoordSystem',0
%
% example:
%
%     p(1,1) = 0;  % x-coord point 1
%     p(2,1) = 0;  % y-coord point 1
%     p(1,2) = 10; % x-coord point 2
%     p(2,2) = 20; % y-coord point 2
%     % normal direction: z (2)
%     CSX = AddLinPoly( CSX, 'PEC', 1, 2, 254, p , 10, 'CoordSystem',0)
%
% 2011, Thorsten Liebig <thorsten.liebig@gmx.de>
%
% See also InitCSX AddMetal AddMaterial AddPolygon AddRotPoly

polygon.ATTRIBUTE.Priority = prio;
polygon.ATTRIBUTE.Elevation = elevation;
polygon.ATTRIBUTE.Length = Length;
polygon.ATTRIBUTE.NormDir = DirChar2Int(normDir);

polygon.Vertex = {};
for s=1:size(points,2)
    polygon.Vertex{end+1}.ATTRIBUTE.X1 = points(1,s);
    polygon.Vertex{end}.ATTRIBUTE.X2   = points(2,s);
end

polygon = AddPrimitiveArgs(polygon,varargin{:});

CSX = Add2Property( CSX, materialname, polygon, 'LinPoly');
