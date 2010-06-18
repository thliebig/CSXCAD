function export_empire( CSX, filename, options )
% export_empire( CSX, filename, options )
%
% Exports the geometry defined in CSX to filename as a empire file.
%
% CSX: CSX-object created by InitCSX()
% filename: export filename (e.g. '/tmp/export.gym')
% options (optional): struct
%
% See also InitCSX 
% CSXCAD matlab interface
% -----------------------
% author: Sebastian Held <sebastian.held@gmx.de>
% 17. Jun 2010: initial version

if nargin < 3
	options = [];
end

fid = fopen( filename, 'w' );

% write header
fprintf( fid, '%s\n\n', 'GANYMEDE  4' );
fprintf( fid, '%s\n', 'ORIGINS 0' );
fprintf( fid, '%s\n', 'list [mat.mat(3,1, 0.0 ,0.0 ,0.0)]' );
fprintf( fid, '%s\n\n', 'END ORIGINS' );
export_empire_meshlines(fid,CSX);
fprintf( fid, '%s\n', 'LAYER 1' );
fprintf( fid, '%s\n', ' texts     [stack_text(text=''name @@initial'',st=STACK), stack_text(text=''conductor'',st=STACK)]' );
fprintf( fid, '%s\n', ' color     stack_color(''black'')' );
fprintf( fid, '%s\n', ' fill      stack_color(''black'')' );
fprintf( fid, '%s\n', ' pattern   ''No Fill''' );
fprintf( fid, '%s\n', ' opaque    0.0' );
fprintf( fid, '%s\n', ' width     1' );
fprintf( fid, '%s\n', ' acadcolor 61' );
fprintf( fid, '%s\n', ' vis       1' );
fprintf( fid, '%s\n', ' autodisc  1' );
fprintf( fid, '%s\n', ' lock      0' );
fprintf( fid, '%s\n', ' arrow     stack_arrow([0.0, 0.0, 0.0, 0.0, 0.0, 1000.0])' );
fprintf( fid, '%s\n\n', 'END LAYER' );
export_empire_layer(fid,2,'empty','conductor')
fprintf( fid, '\n' );
fprintf( fid, '%s\n\n', 'CURRENT_LAYER 2' );


global layer_num
layer_num = 2;

if isfield(CSX.Properties,'Material')
	% process material
	Material = CSX.Properties.Material;
	options.Property = 'Material';
	process_primitives( fid, Material, options );
end
if isfield(CSX.Properties,'Metal')
	% process PEC
	Metal = CSX.Properties.Metal;
	options.Property = 'Metal';
	process_primitives( fid, Metal, options );
end


fclose( fid );

% -----------------------------------------------------------------------------
function str = gym_vect( start,stop )
str = ['[' num2str(start(1)) ',' num2str(stop(1)) ',' num2str(start(2)) ',' num2str(stop(2)) ',' num2str(start(3)) ',' num2str(stop(3)) ']'];
function str = gym_vector( v )
str = ['[' num2str(v(1))];
for a=2:numel(v), str = [str ', ' num2str(v(a))]; end
str = [str ']'];

% -----------------------------------------------------------------------------
function str = primitive_box( fid, CSX_box, layernr )
start = [CSX_box.P1.ATTRIBUTE.X CSX_box.P1.ATTRIBUTE.Y CSX_box.P1.ATTRIBUTE.Z];
stop  = [CSX_box.P2.ATTRIBUTE.X CSX_box.P2.ATTRIBUTE.Y CSX_box.P2.ATTRIBUTE.Z];
fprintf( fid, '%s\n', 'BOX 1' );
fprintf( fid, '%s\n', ' texts	[]' );
fprintf( fid, '%s\n', [' layer_	' num2str(layernr)] );
fprintf( fid, '%s\n', [' points	' gym_vect(start,stop)] );
fprintf( fid, '%s\n\n', 'END BOX' );


% -----------------------------------------------------------------------------
function str = primitive_cylinder( fid, CSX_cylinder, layernr )
start  = [CSX_cylinder.P0.ATTRIBUTE.X CSX_cylinder.P0.ATTRIBUTE.Y CSX_cylinder.P0.ATTRIBUTE.Z];
stop   = [CSX_cylinder.P1.ATTRIBUTE.X CSX_cylinder.P1.ATTRIBUTE.Y CSX_cylinder.P1.ATTRIBUTE.Z];

radius = CSX_cylinder.ATTRIBUTE.Radius;
fprintf( fid, '%s\n', 'POLY 1' );
fprintf( fid, '%s\n', ' texts	[]' );
fprintf( fid, '%s\n', [' layer_	' num2str(layernr)] );
fprintf( fid, '%s\n', [' objects [stack_point([' num2str(start(1)) ',' num2str(start(2)) ',' num2str(start(3)) '],radius=' num2str(radius) ')]'] );
fprintf( fid, '%s\n', [' arrow	stack_arrow(' gym_vect(start,stop) ')'] );
fprintf( fid, '%s\n', ' rad  	1' );
fprintf( fid, '%s\n', 'END POLY' );

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
function primitive_polygon( fid, CSX_polygon, layernr )
Elevation = CSX_polygon.ATTRIBUTE.Elevation;
NormDir = [CSX_polygon.NormDir.ATTRIBUTE.X CSX_polygon.NormDir.ATTRIBUTE.Y CSX_polygon.NormDir.ATTRIBUTE.Z];
for a=1:numel(CSX_polygon.Vertex)
	% iterate over all vertices
	if (NormDir(1) == 0) && (NormDir(2) == 0), v = [CSX_polygon.Vertex{a}.ATTRIBUTE.X1 CSX_polygon.Vertex{a}.ATTRIBUTE.X2 Elevation]; end
	if (NormDir(2) == 0) && (NormDir(3) == 0), v = [Elevation CSX_polygon.Vertex{a}.ATTRIBUTE.X1 CSX_polygon.Vertex{a}.ATTRIBUTE.X2]; end
	if (NormDir(1) == 0) && (NormDir(3) == 0), v = [CSX_polygon.Vertex{a}.ATTRIBUTE.X1 Elevation CSX_polygon.Vertex{a}.ATTRIBUTE.X2]; end
	if a == 1
		str = [num2str(v(1)) ',' num2str(v(2)) ',' num2str(v(3))];
	else
		str = [str ', ' num2str(v(1)) ',' num2str(v(2)) ',' num2str(v(3))];
	end
end
fprintf( fid, '%s\n', 'POLY 1' );
fprintf( fid, '%s\n', ' texts	[]' );
fprintf( fid, '%s\n', [' layer_	' num2str(layernr)] );
fprintf( fid, '%s\n', [' objects	(' str ')'] );
fprintf( fid, '%s\n', [' arrow	stack_arrow(' gym_vect(NormDir*Elevation,NormDir*Elevation) ')'] );
fprintf( fid, '%s\n', ' rad  	100.0' );
fprintf( fid, '%s\n\n', 'END POLY' );

% -----------------------------------------------------------------------------
function process_primitives( fid, prop, options )
global layer_num
ignore = {};
if isfield(options,'ignore'), ignore = options.ignore; end
for num=1:numel(prop)
	Name = prop{num}.ATTRIBUTE.Name;
	if any( strcmp(Name,ignore) )
		disp( ['omitting   ' Name '...'] );
		continue
	end
	disp( ['processing ' prop{num}.ATTRIBUTE.Name '...'] );
	layer_num = layer_num + 1;
	if strcmp(options.Property,'Metal')
		prio = 200;
		properties = ['conductor name Conductor sigma infinite sides auto rough 0 prio ' num2str(prio) ' automodel auto'];
	else
		prio = 100;
		epsr = 1;
		if isfield(prop{num}.Property.ATTRIBUTE,'Epsilon'), epsr = prop{num}.Property.ATTRIBUTE.Epsilon; end
		properties = ['dielectric name Dielectric epsr ' num2str(epsr) ' sigma 0 prio ' num2str(prio) ' tand 0 density 0'];
	end
	export_empire_layer(fid,layer_num,prop{num}.ATTRIBUTE.Name,properties)
	if isfield(prop{num}.Primitives,'Box')
		for a=1:numel(prop{num}.Primitives.Box)
			% iterate over all boxes
			Box = prop{num}.Primitives.Box{a};
			primitive_box( fid, Box, layer_num );
		end
	end
	if isfield(prop{num}.Primitives,'Cylinder')
		for a=1:numel(prop{num}.Primitives.Cylinder)
			% iterate over all cylinders
			Cylinder = prop{num}.Primitives.Cylinder{a};
			primitive_cylinder( fid, Cylinder, layer_num );
		end
	end
	if isfield(prop{num}.Primitives,'Wire')
		for a=1:numel(prop{num}.Primitives.Wire)
			% iterate over all wires
			Wire = prop{num}.Primitives.Wire{a};
%			str = primitive_wire( Wire, obj_modifier );
%			fprintf( fid, '%s\n', str );
		end
	end
	if isfield(prop{num}.Primitives,'Polygon')
		for a=1:numel(prop{num}.Primitives.Polygon)
			% iterate over all polygons
			Polygon = prop{num}.Primitives.Polygon{a};
			primitive_polygon( fid, Polygon, layer_num );
		end
	end
end




function export_empire_layer(fid,nr,name,options)
fprintf( fid, '%s\n', ['LAYER ' num2str(nr)] );
fprintf( fid, '%s\n', [' texts     [stack_text(text=''name ' name ''',st=STACK), stack_text(text=''' options ''',st=STACK)]'] );
fprintf( fid, '%s\n', ' color     stack_color(''#08C'')' );
fprintf( fid, '%s\n', ' fill      stack_color(''#08C'')' );
fprintf( fid, '%s\n', ' pattern   ''No Fill''' );
fprintf( fid, '%s\n', ' opaque    0.0' );
fprintf( fid, '%s\n', ' width     1' );
fprintf( fid, '%s\n', ' acadcolor 61' );
fprintf( fid, '%s\n', ' vis       1' );
fprintf( fid, '%s\n', ' autodisc  0' );
fprintf( fid, '%s\n', ' lock      0' );
fprintf( fid, '%s\n', ' arrow     stack_arrow([0.0, 0.0, 0.0, 0.0, 0.0, 1000.0])' );
fprintf( fid, '%s\n\n', 'END LAYER' );

function export_empire_meshlines(fid,CSX)
fprintf( fid, '%s\n', 'DISC 0' );
fprintf( fid, '%s\n', ['   lines ' gym_vector(CSX.RectilinearGrid.XLines)] );
fprintf( fid, '%s\n', '  fixed []' );
fprintf( fid, '%s\n', '  auto  []' );
fprintf( fid, '%s\n', 'END DISC' );
fprintf( fid, '%s\n', 'DISC 1' );
fprintf( fid, '%s\n', ['   lines ' gym_vector(CSX.RectilinearGrid.YLines)] );
fprintf( fid, '%s\n', '  fixed []' );
fprintf( fid, '%s\n', '  auto  []' );
fprintf( fid, '%s\n', 'END DISC' );
fprintf( fid, '%s\n', 'DISC 2' );
fprintf( fid, '%s\n', ['   lines ' gym_vector(CSX.RectilinearGrid.ZLines)] );
fprintf( fid, '%s\n', '  fixed []' );
fprintf( fid, '%s\n', '  auto  []' );
fprintf( fid, '%s\n\n', 'END DISC' );


