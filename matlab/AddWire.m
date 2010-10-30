function CSX = AddWire(CSX, propName, prio, points, wire_rad, varargin)
% function CSX = AddWire(CSX, propName, prio, points, wire_rad, varargin)
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