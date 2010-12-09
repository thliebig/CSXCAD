function CSX = AddWire(CSX, propName, prio, points, wire_rad, varargin)
% function CSX = AddWire(CSX, propName, prio, points, wire_rad, varargin)
%
% Add a wire to CSX and assign to a property with name <propName>.
% 
% Warning: This is a 1D object, not all properties may be compatible with a
% 1D object, e.g. a material property.
%
%  points:      curve coordinates array
%  prio :       primitive priority
%  wire_rad:    wire radius
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
%       % create a metal wire with finite radius...
%       CSX = AddMetal(CSX,'metal'); %create PEC with propName 'metal'
%       CSX = AddCurve(CSX,'metal',10, points, 2);
%
% See also AddBox, AddCylindricalShell, AddCylinder, AddSphere,
% AddSphericalShell, AddCurve, AddMetal
%
% CSXCAD matlab interface
% -----------------------
% author: Thorsten Liebig

wire.ATTRIBUTE.Priority = prio;
wire.ATTRIBUTE.WireRadius = wire_rad; 

wire.Vertex={};
for s=1:size(points,2)
    wire.Vertex{end+1}.ATTRIBUTE.X = points(1,s);
    wire.Vertex{end}.ATTRIBUTE.Y = points(2,s);
    wire.Vertex{end}.ATTRIBUTE.Z = points(3,s);
end

wire = AddPrimitiveArgs(wire,varargin{:});

CSX = Add2Property(CSX,propName, wire, 'Wire');