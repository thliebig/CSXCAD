function CSX = AddPolyhedron(CSX, propName, prio, vertices, faces, varargin)
% CSX = AddPolyhedron(CSX, propName, prio, vertices, faces, varargin)
%
% Add a polyhedron to CSX and assign to a property with name <propName>.
%
%  prio:       primitive priority
%  vertices:   cell array of all vertices
%  faces:      cell array of all faces
%
%  Note: - The polyhedron must be a closed surface for 3D discretisation
%        - All faces must contain the vertices in a right-handed order with
%          the normal direction for each face pointing out of the solid
%
% optional:
%   Transformation: perform a transformation on a primitive by adding
%      e.g.: 'Transform', {'Scale','1,1,2','Rotate_X',pi/4,'Translate','0,0,100'}
%      Note: This will only affect the 3D material/metal discretisation
%
% example:
%   % example tetrahedron
%   vertices{1}=[0 0 0];
%   vertices{2}=[1 0 0];
%   vertices{3}=[0 1 0];
%   vertices{4}=[0 0 1];
%   faces{1}=[0 2 1];
%   faces{2}=[0 1 3];
%   faces{3}=[0 3 2];
%   faces{4}=[1 2 3];
%   CSX = AddMetal( CSX, 'metal' );
%   CSX = AddPolyhedron(CSX, 'metal', 0, vertices, faces);
%
%
% See also AddBox, AddCylinder, AddCylindricalShell, AddSphere, AddSphericalShell,
% AddCurve, AddWire, AddMetal
%
% CSXCAD matlab interface
% -----------------------
% author: Thorsten Liebig

polyhedron.ATTRIBUTE.Priority = prio;

for n=1:numel(vertices)
    polyhedron.Vertex{n}=vector2str(vertices{n});
end
for n=1:numel(faces)
    polyhedron.Face{n}=vector2str(faces{n});
end

polyhedron = AddPrimitiveArgs(polyhedron,varargin{:});

CSX = Add2Property(CSX,propName, polyhedron, 'Polyhedron');
