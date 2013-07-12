function CSX = AddRotPoly( CSX, materialname, prio, normDir, points, RotAxisDir, angle, varargin)
% function CSX = AddRotPoly( CSX, materialname, prio, normDir, points, RotAxisDir, angle, varargin)
%
% CSX:          CSX-object created by InitCSX()
% materialname: created by AddMetal() or AddMaterial()
% prio:         priority
% normDir:      normal direction of the polygon,
%               e.g. 'x', 'y' or 'z', or numeric 0..2
% points:       two-dimensional coordinates
% RotAxisDir:   direction of the rotational axis
%               e.g. 'x', 'y' or 'z', or numeric 0..2
% angle (optional): rotational start/stop angle, default is [0 2pi]
%
% Warning: Polygon has to be defined using Cartesian Coords
%          for use with cylindrical mesh, set 'CoordSystem',0
%
% example:
% p(1,1) = 0;  % x-coord point 1
% p(2,1) = 0;  % y-coord point 1
% p(1,2) = 10; % x-coord point 2
% p(2,2) = 20; % y-coord point 2
% % normal direction: z
% CSX = AddRotPoly( CSX, 'PEC', 1, 'z', p , 'y');
%
% 2011, Thorsten Liebig <thorsten.liebig@gmx.de>
%
% See also InitCSX AddMetal AddMaterial AddPolygon

if (nargin<7)
    angle = [0 2*pi];
end

polygon.ATTRIBUTE.Priority = prio;
polygon.ATTRIBUTE.NormDir = DirChar2Int(normDir);
polygon.ATTRIBUTE.RotAxisDir = DirChar2Int(RotAxisDir);

polygon.Angles.ATTRIBUTE.Start = angle(1);
polygon.Angles.ATTRIBUTE.Stop = angle(2);

polygon.Vertex = {};
for s=1:size(points,2)
    polygon.Vertex{end+1}.ATTRIBUTE.X1 = points(1,s);
    polygon.Vertex{end}.ATTRIBUTE.X2   = points(2,s);
end

polygon = AddPrimitiveArgs(polygon,varargin{:});

CSX = Add2Property( CSX, materialname, polygon, 'RotPoly');
