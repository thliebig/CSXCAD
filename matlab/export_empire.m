function export_empire( CSX, FDTD, filename, options )
% export_empire( CSX, FDTD, filename, options )
%
% Exports the geometry defined in CSX to filename as an Empire python file.
%
% CSX: CSX-object created by InitCSX()
% FDTD: FDTD-object created by InitFDTD()
% filename: export filename (e.g. '/tmp/export.py')
% options (optional): struct
%   options.ignore  : cell array with property names to ignore
%
% CSXCAD matlab interface
% -----------------------
% author: Sebastian Held <sebastian.held@gmx.de>
% 17. Jun 2010: initial version
% 28. Sep 2010: rewritten using Empire python scripting
% See also InitCSX InitFDTD

if nargin < 3
	options = [];
end

fid = fopen( filename, 'w' );

% write header
fprintf( fid, ['# Empire python script\n\n' ...
               '# start Empire and select File/"Run Script" and select this file\n\n'] );
fprintf( fid, 'from scripting import *\n\n' );
export_empire_meshlines( fid, CSX );
export_empire_settings( fid, CSX, FDTD );

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
function primitive_box( fid, CSX_box, layertype )
%primitive_box( fid, CSX_box, layertype )
% layertype may be: 'dielectric Dielectric' or 'conductor Conductor'
global current_layer_prio
properties = '';
if current_layer_prio ~= CSX_box.ATTRIBUTE.Priority
    properties = [properties ' prio ' num2str(CSX_box.ATTRIBUTE.Priority)];
end
if ~isempty(properties)
    fprintf( fid, 'g.objecttext("%s")\n', [layertype properties] );
end
start = [CSX_box.P1.ATTRIBUTE.X CSX_box.P1.ATTRIBUTE.Y CSX_box.P1.ATTRIBUTE.Z];
stop  = [CSX_box.P2.ATTRIBUTE.X CSX_box.P2.ATTRIBUTE.Y CSX_box.P2.ATTRIBUTE.Z];
fprintf( fid, 'g.layerextrude("z",%g,%g)\n', start(3), stop(3) );
fprintf( fid, 'g.box(%g,%g,%g,%g)\n', start(1), start(2), stop(1), stop(2) );

% -----------------------------------------------------------------------------
function primitive_cylinder( fid, CSX_cylinder, layertype )
%primitive_cylinder( fid, CSX_cylinder, layertype )
% layertype may be: 'dielectric Dielectric' or 'conductor Conductor'
global current_layer_prio
properties = '';
if current_layer_prio ~= CSX_cylinder.ATTRIBUTE.Priority
    properties = [properties ' prio ' num2str(CSX_cylinder.ATTRIBUTE.Priority)];
end
if ~isempty(properties)
    fprintf( fid, 'g.objecttext("%s")\n', [layertype properties] );
end

start  = [CSX_cylinder.P1.ATTRIBUTE.X CSX_cylinder.P1.ATTRIBUTE.Y CSX_cylinder.P1.ATTRIBUTE.Z];
stop   = [CSX_cylinder.P2.ATTRIBUTE.X CSX_cylinder.P2.ATTRIBUTE.Y CSX_cylinder.P2.ATTRIBUTE.Z];

radius = CSX_cylinder.ATTRIBUTE.Radius;
if start([1 2]) == stop([1 2])
    fprintf( fid, 'g.layerextrude("z",%g,%g)\n', start(3), stop(3) );
    fprintf( fid, 'g.circle(%g,%g,%g)\n', start(1), start(2), radius );
elseif start([1 3]) == stop([1 3])
    fprintf( fid, 'g.layerextrude("y",%g,%g)\n', start(2), stop(2) );
    fprintf( fid, 'g.circle(%g,%g,%g)\n', start(3), start(1), radius );
elseif start([2 3]) == stop([2 3])
    fprintf( fid, 'g.layerextrude("x",%g,%g)\n', start(1), stop(1) );
    fprintf( fid, 'g.circle(%g,%g,%g)\n', start(2), start(3), radius );
else
    disp( ['cylinder coordinates: (' num2str(start) ') -> (' num2str(stop) ')'] );
    error 'cylinder orientation not supported'
end

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
function primitive_polygon( fid, CSX_polygon, layertype )
%primitive_polygon( fid, CSX_polygon, layertype )
% layertype may be: 'dielectric Dielectric' or 'conductor Conductor'
global current_layer_prio
properties = '';
if current_layer_prio ~= CSX_polygon.ATTRIBUTE.Priority
    properties = [properties ' prio ' num2str(CSX_polygon.ATTRIBUTE.Priority)];
end
if ~isempty(properties)
    fprintf( fid, 'g.objecttext("%s")\n', [layertype properties] );
end
Elevation = CSX_polygon.ATTRIBUTE.Elevation;
if (CSX_polygon.NormDir.ATTRIBUTE.X ~= 0)
    NormDir = 'x';
elseif (CSX_polygon.NormDir.ATTRIBUTE.Y ~= 0)
    NormDir = 'y';
elseif (CSX_polygon.NormDir.ATTRIBUTE.Z ~= 0)
    NormDir = 'z';
end
coords = [];
for a=1:numel(CSX_polygon.Vertex)
	% iterate over all vertices
	coords = [coords CSX_polygon.Vertex{a}.ATTRIBUTE.X1 CSX_polygon.Vertex{a}.ATTRIBUTE.X2];
end
fprintf( fid, 'g.layerextrude("%s",%g,%g)\n', NormDir, Elevation, Elevation+1 );
fprintf( fid, 'g.poly(' );
for n=1:numel(coords)
    if n > 1
        fprintf( fid, ', %g', coords(n) );
    else
        fprintf( fid, '%g', coords(n) );
    end
end
fprintf( fid, ')\n' );




% -----------------------------------------------------------------------------
function process_primitives( fid, prop, options )
global current_layer_prio
ignore = {};
if isfield(options,'ignore'), ignore = options.ignore; end

% iterate over all properties and extract the priority
prio = [];
for num=1:numel(prop)
    if isfield(prop{num}.Primitives,'Box')
        for a=1:numel(prop{num}.Primitives.Box)
            % iterate over all boxes
            prio(end+1) = prop{num}.Primitives.Box{a}.ATTRIBUTE.Priority;
        end
    end
    if isfield(prop{num}.Primitives,'Cylinder')
        for a=1:numel(prop{num}.Primitives.Cylinder)
            % iterate over all cylinders
            prio(end+1) = prop{num}.Primitives.Cylinder{a}.ATTRIBUTE.Priority;
        end
    end
    if isfield(prop{num}.Primitives,'Polygon')
        for a=1:numel(prop{num}.Primitives.Polygon)
            % iterate over all polygons
            prio(end+1) = prop{num}.Primitives.Polygon{a}.ATTRIBUTE.Priority;
        end
    end
end
% Now prio is a vector full of priorities. Extract the priority, which is
% most often used.
u_prios = unique(prio);
count = histc(prio,u_prios);
[~,idx] = max(count);
current_layer_prio = u_prios(idx);

% now export all properties/primitives
for num=1:numel(prop)
	Name = prop{num}.ATTRIBUTE.Name;
    if any( strcmp(Name,ignore) )
		disp( ['omitting   ' Name '...'] );
		continue
    end
    
    disp( ['processing ' prop{num}.ATTRIBUTE.Name '...'] );

    if strcmp(options.Property,'Metal')
        layertype = 'conductor';
		properties = [layertype ' sigma infinite sides auto rough 0 prio ' num2str(current_layer_prio) ' automodel auto'];
	else
        layertype = 'dielectric';
		epsr = 1;
        sigma = 0;
        if isfield(prop{num},'PropertyX')
            if isfield(prop{num}.PropertyX.ATTRIBUTE,'Epsilon'), epsr = prop{num}.PropertyX.ATTRIBUTE.Epsilon; end
            if isfield(prop{num}.PropertyX.ATTRIBUTE,'Kappa'), sigma = prop{num}.PropertyX.ATTRIBUTE.Kappa; end
        end
		properties = [layertype ' epsr ' num2str(epsr) ' sigma ' num2str(sigma) ' prio ' num2str(current_layer_prio) ' tand 0 density 0'];
    end
    
    export_empire_layer( fid, Name, properties )
	
    if isfield(prop{num}.Primitives,'Box')
        for a=1:numel(prop{num}.Primitives.Box)
            % iterate over all boxes
            Box = prop{num}.Primitives.Box{a};
            primitive_box( fid, Box, layertype );
        end
    end
    if isfield(prop{num}.Primitives,'Cylinder')
        for a=1:numel(prop{num}.Primitives.Cylinder)
            % iterate over all cylinders
            Cylinder = prop{num}.Primitives.Cylinder{a};
            primitive_cylinder( fid, Cylinder, layertype );
        end
    end
    if isfield(prop{num}.Primitives,'Wire')
        for a=1:numel(prop{num}.Primitives.Wire)
            % iterate over all wires
            Wire = prop{num}.Primitives.Wire{a};
%			str = primitive_wire( Wire, obj_modifier );
%			fprintf( fid, '%s\n', str );
        end
        warning( 'CSXCAD:export_empire', 'primitive wire currently unsupported' );
    end
    if isfield(prop{num}.Primitives,'Polygon')
        for a=1:numel(prop{num}.Primitives.Polygon)
            % iterate over all polygons
            Polygon = prop{num}.Primitives.Polygon{a};
            primitive_polygon( fid, Polygon, layertype );
        end
    end
    
    fprintf( fid, 'g.load()\n\n' );
end




function export_empire_layer( fid, name, layertext )
fprintf( fid, '# create a new layer\n' );
fprintf( fid, 'g = gmf()\n' );
fprintf( fid, 'g.layer("%s")\n', name );
fprintf( fid, 'g.layerextrude("z",0,0)\n' );
fprintf( fid, 'g.layertext("%s")\n', layertext );

function export_empire_meshlines(fid,CSX)
fprintf( fid, '# create the mesh\n' );
fprintf( fid, 'empty_disc()\n' );
fprintf( fid, 'g = gmf()\n' );
fprintf( fid, 'g.raw_write("DISC X %g")\n', CSX.RectilinearGrid.XLines );
fprintf( fid, 'g.raw_write("DISC Y %g")\n', CSX.RectilinearGrid.YLines );
fprintf( fid, 'g.raw_write("DISC Z %g")\n', CSX.RectilinearGrid.ZLines );
fprintf( fid, 'g.load()\n\n' );

function empire_BC = convert_BC( openEMS_BC )
if ischar(openEMS_BC)
    if strcmp(openEMS_BC,'PEC')
        empire_BC = 'electric';
    elseif strcmp(openEMS_BC,'PMC')
        empire_BC = 'magnetic';
    elseif strncmp(openEMS_BC,'MUR',3) || strncmp(openEMS_BC,'PML',3)
        empire_BC = 'pml 6';
    else
        empire_BC = 'UNKNOWN';
    end
else
    if openEMS_BC == 0
        empire_BC = 'electric';
    elseif openEMS_BC == 1
        empire_BC = 'magnetic';
    elseif openEMS_BC == 2 || openEMS_BC == 3
        empire_BC = 'pml 6';
    else
        empire_BC = 'UNKNOWN';
    end    
end

function export_empire_settings(fid,CSX,FDTD)
fprintf( fid, '# settings\n' );
fprintf( fid, 'set_fs_parameter( ''dx'', %g )\n', CSX.RectilinearGrid.ATTRIBUTE.DeltaUnit );
fprintf( fid, 'set_fs_parameter( ''xmin'', ''%s'' )\n', convert_BC(FDTD.BoundaryCond.ATTRIBUTE.xmin) );
fprintf( fid, 'set_fs_parameter( ''xmax'', ''%s'' )\n', convert_BC(FDTD.BoundaryCond.ATTRIBUTE.xmax) );
fprintf( fid, 'set_fs_parameter( ''ymin'', ''%s'' )\n', convert_BC(FDTD.BoundaryCond.ATTRIBUTE.ymin) );
fprintf( fid, 'set_fs_parameter( ''ymax'', ''%s'' )\n', convert_BC(FDTD.BoundaryCond.ATTRIBUTE.ymax) );
fprintf( fid, 'set_fs_parameter( ''zmin'', ''%s'' )\n', convert_BC(FDTD.BoundaryCond.ATTRIBUTE.zmin) );
fprintf( fid, 'set_fs_parameter( ''zmax'', ''%s'' )\n', convert_BC(FDTD.BoundaryCond.ATTRIBUTE.zmax) );
fprintf( fid, 'set_fs_parameter( ''f_0'', %g )\n', FDTD.Excitation.ATTRIBUTE.f0 );
fprintf( fid, 'set_fs_parameter( ''f_c'', %g )\n', FDTD.Excitation.ATTRIBUTE.fc );
f_start = max(0,FDTD.Excitation.ATTRIBUTE.f0 - FDTD.Excitation.ATTRIBUTE.fc);
f_end   = max(0,FDTD.Excitation.ATTRIBUTE.f0 + FDTD.Excitation.ATTRIBUTE.fc);
fprintf( fid, 'set_fs_parameter( ''F_START'', %g )\n', f_start );
fprintf( fid, 'set_fs_parameter( ''F_END'', %g )\n', f_end );
fprintf( fid, 'set_fs_parameter( ''F_MID'', %g )\n', (f_start+f_end)/2 );
