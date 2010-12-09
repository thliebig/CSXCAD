function CSX = AddCurve(CSX, propName, prio, points, varargin)
% function CSX = AddCurve(CSX, propName, prio, points, varargin)
%
% Add a curve to CSX and assign to a property with name <propName>.
% 
% Warning: This is a 1D object, not all properties may be compatible with a
% 1D object, e.g. a material property.
%
%  points:      curve coordinates array
%  prio :       primitive priority
%
%   example:
%       %first point
%       points(1,1) = 0;
%       points(2,1) = 5;
%       points(3,1) = 10;
%       %second point
%       points(1,2) = 0;
%       points(2,2) = 10;
%       points(3,2) = 10;
%       %third point ...
%       % create a thin metal wire...
%       CSX = AddMetal(CSX,'metal'); %create PEC with propName 'metal'
%       CSX = AddCurve(CSX,'metal',10, points);
%
% See also AddBox, AddCylindricalShell, AddCylinder, AddSphere,
% AddSphericalShell, AddWire, AddMetal
%
% CSXCAD matlab interface
% -----------------------
% author: Thorsten Liebig

curve.ATTRIBUTE.Priority = prio;

curve.Vertex={};
for s=1:size(points,2)
    curve.Vertex{end+1}.ATTRIBUTE.X = points(1,s);
    curve.Vertex{end}.ATTRIBUTE.Y = points(2,s);
    curve.Vertex{end}.ATTRIBUTE.Z = points(3,s);
end

curve = AddPrimitiveArgs(curve,varargin{:});

CSX = Add2Property(CSX,propName, curve, 'Curve');