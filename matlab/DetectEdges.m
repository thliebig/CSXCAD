function mesh = DetectEdges(CSX, mesh, varargin)
% mesh = DetectEdges(CSX <, mesh, varargin>)
%
% Returns a mesh with the edges added of certain (see below) primitives
% found in the CSX structure.
%
% optional arguments:
%   mesh:       add edges to an existing (predefined) mesh
%
% optional: 'keyword', value
%   'Debug'         enable debug mode (default is 1)
%   'AddPropertyType'  add a list of additional property types to detect
%                   e.g. 'DumpBox' or {'DumpBox','ProbeBox'}
%   'SetPropertyType'  set the list of property types to detect (override default)
%                   e.g. 'Metal' or {'Metal','ConductingSheet'}
%   'ExcludeProperty'  give a list of property names to exclude from
%                      detection
%   'SetProperty'  give a list of property names to handly exlusively for detection
%
% advanced options: 'keyword', value
%   '2D_Metal_Edge_Res' define a one-third/two-third metal edge resolution
%
% example:
%     CSX = InitCSX();
%     % define all properties and primitives to detect
%     % ...
%     % ...
%     mesh = DetectEdges(CSX);
%     mesh = SmoothMesh(mesh, lambda/20/unit);
%     CSX = DefineRectGrid(CSX, unit, mesh);
%
% Note:
% - Only primitives contained in Metal, Material, Excitation, LumpedElement
%   or ConductingSheet properties are processed
% - Currently this function handles only Box, Polygons, LinPoly and Cylinder.
%
% CSXCAD matlab interface
% -----------------------
% author: Koen De Vleeschauwer (c) 2012
% modified by: Thorsten Liebig (c) 2012, 2013
%
% See also InitCSX, SmoothMesh, DefineRectGrid

supported_properties = {};
supported_properties{end+1}='Metal';
supported_properties{end+1}='Material';
supported_properties{end+1}='Excitation';
supported_properties{end+1}='LumpedElement';
supported_properties{end+1}='ConductingSheet';

exclude_list = {};
prop_list_only = {};

debug = 1;
mesh_2D_metal_edges = 0;

for n=1:2:numel(varargin)
    if (strcmpi(varargin{n},'debug')==1);
        debug = varargin{n+1};
    elseif (strcmpi(varargin{n},'AddPropertyType')==1);
        if iscell(varargin{n+1})
            supported_properties(end+1) = varargin{n+1};
        elseif ischar(varargin{n+1})
            supported_properties{end+1} = varargin{n+1};
        else
           error('CSXCAD:DetectEdges','unknown property definition');
        end
    elseif (strcmpi(varargin{n},'SetPropertyType')==1);
        if iscell(varargin{n+1})
            supported_properties = varargin{n+1};
        elseif ischar(varargin{n+1})
            supported_properties = {varargin{n+1}};
        else
           error('CSXCAD:DetectEdges','unknown property definition');
        end
    elseif (strcmpi(varargin{n},'ExcludeProperty')==1);
        exclude_list = varargin{n+1};
    elseif (strcmpi(varargin{n},'SetProperty')==1);
        prop_list_only = varargin{n+1};
    elseif (strcmpi(varargin{n},'2D_Metal_Edge_Res')==1);
        mesh_2D_metal_edges = varargin{n+1}*[-1 2]/3;
    else
        warning('CSXCAD:DetectEdges',['unknown argument: ' varargin{n}]);
    end
end

if (nargin<2)
    mesh = [];
end

if isempty(mesh)
    if (CSX.ATTRIBUTE.CoordSystem==0)
        mesh.x = [];
        mesh.y = [];
        mesh.z = [];
    elseif (CSX.ATTRIBUTE.CoordSystem==1)
        mesh.r = [];
        mesh.a = [];
        mesh.z = [];
    else
        error('CSXCAD:DetectEdges','unknown coordinate system used');
    end
end

edges = {};
edges.x = [ ];
edges.y = [ ];
edges.z = [ ];

if (~isstruct(CSX))
    error('expected a CSX structure');
end

CoordSystem = CSX.ATTRIBUTE.CoordSystem;

if (isfield(CSX, 'Properties'))
    prop_fn = fieldnames(CSX.Properties);
    for p = 1:numel(prop_fn)
        if (sum(strcmpi(prop_fn{p}, supported_properties))==0)
            continue;
        end
        isMetal = sum(strcmpi(prop_fn{p},{'Metal','ConductingSheet'}));
        property_group = CSX.Properties.(prop_fn{p});
        for m = 1:numel(property_group)
            property=property_group{m};
            if ~isfield(property, 'Primitives')
                continue;
            end
            if (sum(strcmpi(property.ATTRIBUTE.Name,exclude_list)))
                continue;
            end
            if (~isempty(prop_list_only) && (sum(strcmpi(property.ATTRIBUTE.Name,prop_list_only))==0))
                continue;
            end
            primitives = property.Primitives;
            prim_fn = fieldnames(primitives);
            for n_prim = 1:numel(prim_fn)
                if (strcmp(prim_fn{n_prim}, 'Box'))
                    for b = 1:length(primitives.Box)
                        box = primitives.Box{b};
                        x1 = box.P1.ATTRIBUTE.X;
                        y1 = box.P1.ATTRIBUTE.Y;
                        z1 = box.P1.ATTRIBUTE.Z;
                        x2 = box.P2.ATTRIBUTE.X;
                        y2 = box.P2.ATTRIBUTE.Y;
                        z2 = box.P2.ATTRIBUTE.Z;
                        % check dimension
                        dim = (x1~=x2) + (y1~=y2) + (z1~=z2);
                        if ((dim==2) && isMetal)
                            % add to global list of edges, with a given 2D
                            % edge resolution
                            edges = AddEdge (edges, box, x1+sign(x1-x2)*mesh_2D_metal_edges, y1+sign(y1-y2)*mesh_2D_metal_edges, ...
                                z1+sign(z1-z2)*mesh_2D_metal_edges, CoordSystem, debug);
                            edges = AddEdge (edges, box, x2+sign(x2-x1)*mesh_2D_metal_edges, y2+sign(y2-y1)*mesh_2D_metal_edges, ...
                                z2+sign(z2-z1)*mesh_2D_metal_edges, CoordSystem, debug);
                        else
                            % add to global list of edges
                            edges = AddEdge (edges, box, x1, y1, z1, CoordSystem, debug);
                            edges = AddEdge (edges, box, x2, y2, z2, CoordSystem, debug);
                        end
                    end
                elseif (strcmp(prim_fn{n_prim}, 'LinPoly') || strcmp(prim_fn{n_prim}, 'Polygon'))
                    for l = 1:length(primitives.(prim_fn{n_prim}))
                        poly = primitives.(prim_fn{n_prim}){l};
                        dir = poly.ATTRIBUTE.NormDir + 1;
                        dirP = mod(poly.ATTRIBUTE.NormDir+1,3) + 1;
                        dirPP = mod(poly.ATTRIBUTE.NormDir+2,3) + 1;
                        lin_length = 0;
                        if (strcmp(prim_fn{n_prim}, 'LinPoly'))
                            lin_length = poly.ATTRIBUTE.Length;
                        end
                        if (isfield(poly, 'Vertex'))
                            for v = 1:length(poly.Vertex)
                                vertex = poly.Vertex{v};
                                edge(dir) = poly.ATTRIBUTE.Elevation;
                                edge(dirP) = vertex.ATTRIBUTE.X1;
                                edge(dirPP) = vertex.ATTRIBUTE.X2;
                                edges = AddEdge (edges, poly, edge(1), edge(2), edge(3), CoordSystem, debug);
                                if (lin_length~=0)
                                    edge(dir) = edge(dir) + lin_length;
                                    edges = AddEdge (edges, poly, edge(1), edge(2), edge(3), CoordSystem, debug);
                                end
                            end
                        end
                    end
                elseif (strcmp(prim_fn{n_prim}, 'Cylinder'))
                    for c = 1:length(primitives.Cylinder)
                        cylinder = primitives.Cylinder{c};
                        r = cylinder.ATTRIBUTE.Radius;
                        x1 = cylinder.P1.ATTRIBUTE.X;
                        y1 = cylinder.P1.ATTRIBUTE.Y;
                        z1 = cylinder.P1.ATTRIBUTE.Z;
                        x2 = cylinder.P2.ATTRIBUTE.X;
                        y2 = cylinder.P2.ATTRIBUTE.Y;
                        z2 = cylinder.P2.ATTRIBUTE.Z;
                        if ((x1 == x2) && (y1 == y2) && (z1 ~= z2))
                          % cylinder parallel with z axis
                          edges = AddEdge (edges, cylinder, x1 - r, y1 - r, z1, CoordSystem, debug);
                          edges = AddEdge (edges, cylinder, x2 + r, y2 + r, z2, CoordSystem, debug);
                        elseif ((x1 == x2) && (y1 ~= y2) && (z1 == z2))
                          % cylinder parallel with y axis
                          edges = AddEdge (edges, cylinder, x1 - r, y1, z1 - r, CoordSystem, debug);
                          edges = AddEdge (edges, cylinder, x2 + r, y2, z2 + r, CoordSystem, debug);
                        elseif ((x1 ~= x2) && (y1 == y2) && (z1 == z2))
                          % cylinder parallel with x axis
                          edges = AddEdge (edges, cylinder, x1, y1 - r, z1 - r, CoordSystem, debug);
                          edges = AddEdge (edges, cylinder, x2, y2 + r, z2 + r, CoordSystem, debug);
                        elseif (debug > 0)
                          warning('CSXCAD:DetectEdges',['unsupported primitive of type: "' prim_fn{n_prim} '" found, skipping edges']);
                        end
                    end
                else
                    if (debug>0)
                        warning('CSXCAD:DetectEdges',['unsupported primitive of type: "' prim_fn{n_prim} '" found, skipping edges']);
                    end
                end
            end
        end
    end
end

if (CSX.ATTRIBUTE.CoordSystem==0)
    mesh.x = sort(unique([mesh.x edges.x]));
    mesh.y = sort(unique([mesh.y edges.y]));
    mesh.z = sort(unique([mesh.z edges.z]));
elseif (CSX.ATTRIBUTE.CoordSystem==1)
    mesh.r = sort(unique([mesh.r edges.x]));
    mesh.a = sort(unique([mesh.a edges.y]));
    mesh.z = sort(unique([mesh.z edges.z]));
else
    error('CSXCAD:DetectEdges','unknown coordinate system used');
end

end


function edges = AddEdge(edges, csx_prim, x, y, z, CoordSystem, debug)
% Add edges of CSX primitives including some transformations

xt = unique(x);
yt = unique(y);
zt = unique(z);

if isfield(csx_prim.ATTRIBUTE,'CoordSystem')
    if (csx_prim.ATTRIBUTE.CoordSystem~=CoordSystem)
        if (debug>2)
            warning('CSXCAD:DetectEdges','different coordinate systems not supported, skipping edges');
        end
        return
    end
end

if (isfield(csx_prim, 'Transformation'))

    transformation = csx_prim.Transformation;
    trans_fn = fieldnames(transformation);

    for t=1:numel(trans_fn)
        if (strcmp(trans_fn{t}, 'Translate'))
            xt = xt + transformation.Translate.ATTRIBUTE.Argument(1);
            yt = yt + transformation.Translate.ATTRIBUTE.Argument(2);
            zt = zt + transformation.Translate.ATTRIBUTE.Argument(3);
        else
            if (debug>0)
                warning('CSXCAD:DetectEdges','unsupported transformation found in primitive, skipping edges');
            end
            return
        end
    end

end

% add to global list of edges
edges.x = [edges.x xt];
edges.y = [edges.y yt];
edges.z = [edges.z zt];
end

