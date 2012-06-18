function export_povray( CSX, filename, options )
% export_povray( CSX, filename, options )
%
% Exports the geometry defined in CSX to filename as a povray file.
%
% CSX: CSX-object created by InitCSX()
% filename: export filename (e.g. '/tmp/export.pov')
% options (optional): struct
%   .camera: (string) use this as the camera definition line
%            1 camera on positive x-axis
%            2 camera on positive y-axis
%            3 camera on positive z-axis
%   .light:  (string) use this as the light definition line
%            1 point light at camera position
%   .header: (string) add this to the header of the file
%   .ignore: (cell array) ignore these CSX-properties
%   .obj_modifier: struct
%      .<propname>: (string) povray object modifier for corresponding primitives
%                   example: options.obj_modifier.copper = 'pigment { color rgbt <0.8,0.5,0,0> }';
%
% See also InitCSX 
% CSXCAD matlab interface
% -----------------------
% author: Sebastian Held <sebastian.held@gmx.de>
% 12. Jun 2010: initial version

if nargin < 3
	options = [];
end

fid = fopen( filename, 'w' );

% write header
fprintf( fid, '%s\n', '// povray-file exported by openEMS' );
fprintf( fid, '%s\n', '#include "colors.inc"' );
fprintf( fid, '%s\n', '#include "metals.inc"' );
fprintf( fid, '%s\n', '#include "textures.inc"' );
fprintf( fid, '%s\n', '#include "transforms.inc"' );
fprintf( fid, '%s\n', 'background { color rgb<1.000000,1.000000,1.000000> }' );
if isfield(options,'header')
	fprintf( fid, '%s\n', header );
end

if isfield(CSX.Properties,'Material')
	% process material
	Material = CSX.Properties.Material;
	process_primitives( fid, Material, 'pigment { color rgbt <0.000, 0.533, 0.800,0.0> } finish { diffuse 0.6 }', options );
end
if isfield(CSX.Properties,'Metal')
	% process PEC
	Metal = CSX.Properties.Metal;
	process_primitives( fid, Metal, 'texture { Copper_Metal }', options );
end

% create coordinate system vectors (for debugging)
debug_coords = 0;
if debug_coords
    fprintf( fid, [...
    '#macro Axis_( AxisLen, RedTexture,WhiteTexture) \n' ...
    'union{\n' ...
    '    cylinder {<0,-AxisLen,0>,<0,AxisLen,0>,0.5\n' ...
    '              texture{checker texture{RedTexture} \n' ...
    '                              texture{WhiteTexture}\n' ...
    '                      translate<0.1,0,0.1>}}\n' ...
    '    cone{<0,AxisLen,0>,2,<0,AxisLen+0.7,0>,0\n' ...
    '         texture{RedTexture}}\n' ...
    '     } // end of union                   \n' ...
    '#end // of macro "Axis( )"\n' ...
    '\n' ...
    '#macro AxisXYZ( AxisLenX, AxisLenY, AxisLenZ, TexRed, TexWhite)\n' ...
    'union{\n' ...
    '   object{Axis_(AxisLenX, TexRed, TexWhite)   rotate< 0,0,-90>}   // x axis\n' ...
    '   object{Axis_(AxisLenY, TexRed, TexWhite)   rotate< 0,0,  0>}   // y axis \n' ...
    '   object{Axis_(AxisLenZ, TexRed, TexWhite)   rotate<90,0,  0>}   // z axis\n' ...
    '   text{ ttf"timrom.ttf",  "x",  0.15,  0  texture{TexRed} \n' ...
    '         scale 10 translate <AxisLenX+0.05,0.4,-0.10>}\n' ...
    '   text{ ttf"timrom.ttf",  "y",  0.15,  0  texture{TexRed} \n' ...
    '         scale 10 translate <-0.75,AxisLenY+0.50,-0.00>}\n' ...
    '   text{ ttf"timrom.ttf",  "z",  0.15,  0  texture{TexRed} \n' ...
    '         scale 10 translate <-0.75,0.2,AxisLenZ+0.50>}\n' ...
    '   scale <1000,1000,1000>\n' ...
    '} // end of union\n' ...
    '#end// of macro\n' ...
    '\n' ...
    'object{AxisXYZ( 10, 10, 10, texture{ pigment{rgb<1,0,0>} finish{ phong 1}}, texture{ pigment{rgb<1,1,1>} finish{ phong 1}} )}\n' ...
    ''] );
end

% create camera and light
xmin = min(CSX.RectilinearGrid.XLines);
xmax = max(CSX.RectilinearGrid.XLines);
ymin = min(CSX.RectilinearGrid.YLines);
ymax = max(CSX.RectilinearGrid.YLines);
zmin = min(CSX.RectilinearGrid.ZLines);
zmax = max(CSX.RectilinearGrid.ZLines);
center = [(xmin+xmax)/2 (ymin+ymax)/2 (zmin+zmax)/2];
% default camera
camera_pos = [center(1), ymin, zmax+sqrt((xmax-xmin)^2+(ymax-ymin)^2)];
camera = ['camera {location ' pov_vect(camera_pos) ' right <-1.33,0,0> look_at ' pov_vect(center) ' angle 40}']; % right-handed coordinate system
if isfield(options,'camera')
	if ischar(options.camera)
		camera = options.camera;
	elseif options.camera == 1
		% looking from positive x-axis
		camera_pos = [xmax+sqrt((ymax-ymin)^2+(zmax-zmin)^2), 0, 0];
		camera = ['camera {location ' pov_vect(camera_pos) ' right <-1.33,0,0> look_at ' pov_vect(center) ' angle 40}'];
	elseif options.camera == 2
		% looking from positive y-axis
		camera_pos = [0, ymax+sqrt((xmax-xmin)^2+(zmax-zmin)^2), 0];
		camera = ['camera {location ' pov_vect(camera_pos) ' right <-1.33,0,0> look_at ' pov_vect(center) ' angle 40}'];
	elseif options.camera == 3
		% looking from positive z-axis
		camera_pos = [0, 0, zmax+sqrt((xmax-xmin)^2+(ymax-ymin)^2)];
		camera = ['camera {location ' pov_vect(camera_pos) ' right <-1.33,0,0> look_at ' pov_vect(center) ' angle 40}'];
	end
end
fprintf( fid, '%s\n', camera );
% default light
light = 'light_source { <3500,-3500,10000> White area_light <10000, 0, 0>, <0, 10000, 0>, 2, 2 adaptive 1 }';
if isfield(options,'light')
	if ischar(options.light)
		light = options.light;
	elseif options.light == 1
		% point light at position of camera
		light = ['light_source { ' pov_vect(camera_pos) ', rgb <1,1,1> }'];
	end
end
fprintf( fid, '%s\n', light );

fclose( fid );

% -----------------------------------------------------------------------------
function str = pov_vect( vec )
if numel(vec) == 3
	str = ['<' num2str(vec(1)) ',' num2str(vec(2)) ',' num2str(vec(3)) '>'];
else
	str = ['<' num2str(vec(1)) ',' num2str(vec(2)) '>'];
end

% -----------------------------------------------------------------------------
function str = primitive_box( CSX_box, options )
start = [CSX_box.P1.ATTRIBUTE.X CSX_box.P1.ATTRIBUTE.Y CSX_box.P1.ATTRIBUTE.Z];
stop  = [CSX_box.P2.ATTRIBUTE.X CSX_box.P2.ATTRIBUTE.Y CSX_box.P2.ATTRIBUTE.Z];
if any( start == stop )
	% 2D box
	% povray supports 2D polygons, but has no priority concept, therefore use the box primitive
	epsilon = 1; % FIXME this should be small compared to any other linear dimension of any object in the scene
	epsilon = (start == stop) * epsilon;  % select identical components
	start = start - epsilon;
	stop  = stop + epsilon;
end
str = ['box { ' pov_vect(start) ', ' pov_vect(stop) ' ' options '}'];

% -----------------------------------------------------------------------------
function str = primitive_cylinder( CSX_cylinder, options )
start  = [CSX_cylinder.P1.ATTRIBUTE.X CSX_cylinder.P0.ATTRIBUTE.Y CSX_cylinder.P1.ATTRIBUTE.Z];
stop   = [CSX_cylinder.P2.ATTRIBUTE.X CSX_cylinder.P1.ATTRIBUTE.Y CSX_cylinder.P2.ATTRIBUTE.Z];
radius = CSX_cylinder.ATTRIBUTE.Radius;
str = ['cylinder { ' pov_vect(start) ', ' pov_vect(stop) ', ' num2str(radius) ' ' options '}'];

% -----------------------------------------------------------------------------
function str = primitive_wire( CSX_wire, options )
radius = CSX_wire.ATTRIBUTE.WireRadius;
str = ['sphere_sweep { linear_spline, ' num2str(numel(CSX_wire.Vertex))];
for a=1:numel(CSX_wire.Vertex)
	% iterate over all vertices
	v = [CSX_wire.Vertex{a}.ATTRIBUTE.X CSX_wire.Vertex{a}.ATTRIBUTE.Y CSX_wire.Vertex{a}.ATTRIBUTE.Z];
	str = [str ', ' pov_vect(v) ', ' num2str(radius)];
end
str = [str ' ' options '}'];

% -----------------------------------------------------------------------------
function str = primitive_polygon( CSX_polygon, options )
Elevation = -CSX_polygon.ATTRIBUTE.Elevation;
% NormDir = CSX_polygon.ATTRIBUTE.NormDir;
epsilon = 1; % FIXME this should be small compared to any other linear dimension of any object in the scene
str = ['prism { linear_spline linear_sweep ' num2str(Elevation - epsilon) ', ' num2str(Elevation + epsilon) ', ' num2str(numel(CSX_polygon.Vertex)+1)];
for a=1:numel(CSX_polygon.Vertex)
	% iterate over all vertices
	v = [CSX_polygon.Vertex{a}.ATTRIBUTE.X1 CSX_polygon.Vertex{a}.ATTRIBUTE.X2];
	str = [str ', ' pov_vect(v)];
end
v = [CSX_polygon.Vertex{1}.ATTRIBUTE.X1 CSX_polygon.Vertex{1}.ATTRIBUTE.X2]; % close prism
str = [str ', ' pov_vect(v)];
% str = [str ' ' options ' Point_At_Trans(' pov_vect(NormDir) ')}']; % needs transforms.inc
str = [str ' ' options ' rotate<-90,0,0> }'];

% -----------------------------------------------------------------------------
function process_primitives( fid, prop, default_obj_modifier, options )
ignore = {};
if isfield(options,'ignore'), ignore = options.ignore; end
for num=1:numel(prop)
	Name = prop{num}.ATTRIBUTE.Name;
	if any( strcmp(Name,ignore) )
		disp( ['omitting   ' Name '...'] );
		continue
	end
	obj_modifier = default_obj_modifier;
	if isfield(options,'obj_modifier') && isfield(options.obj_modifier,Name)
		obj_modifier = options.obj_modifier.(Name);
	end
	disp( ['processing ' prop{num}.ATTRIBUTE.Name '...'] );
	fprintf( fid, '%s\n', ['// ' Name] );
	if isfield(prop{num}.Primitives,'Box')
		for a=1:numel(prop{num}.Primitives.Box)
			% iterate over all boxes
			Box = prop{num}.Primitives.Box{a};
			str = primitive_box( Box, obj_modifier );
			fprintf( fid, '%s\n', str );
		end
	end
	if isfield(prop{num}.Primitives,'Cylinder')
		for a=1:numel(prop{num}.Primitives.Cylinder)
			% iterate over all cylinders
			Cylinder = prop{num}.Primitives.Cylinder{a};
			str = primitive_cylinder( Cylinder, obj_modifier );
			fprintf( fid, '%s\n', str );
		end
	end
	if isfield(prop{num}.Primitives,'Wire')
		for a=1:numel(prop{num}.Primitives.Wire)
			% iterate over all wires
			Wire = prop{num}.Primitives.Wire{a};
			str = primitive_wire( Wire, obj_modifier );
			fprintf( fid, '%s\n', str );
		end
	end
	if isfield(prop{num}.Primitives,'Polygon')
		for a=1:numel(prop{num}.Primitives.Polygon)
			% iterate over all polygons
			Polygon = prop{num}.Primitives.Polygon{a};
			str = primitive_polygon( Polygon, obj_modifier );
			fprintf( fid, '%s\n', str );
		end
	end
end

