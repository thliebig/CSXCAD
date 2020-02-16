function export_gerber( CSX, filename, options )
% export_gerber( CSX, filename, options )
%
% Exports the geometry defined in CSX to filename as a gerber RS-274X file.
% The xy-plane is exported.
%
% CSX: CSX-object created by InitCSX()
% filename: export filename (e.g. '/tmp/export.gbr')
% options (optional): struct
%   .header: (string) add this to the header of the file
%   .ignore: (cell array) ignore these CSX-properties
%
% See also InitCSX 
% CSXCAD matlab interface
% -----------------------
% author: Sebastian Held <sebastian.held@gmx.de>
% 13. Jun 2010: initial version

%TODO:
%   .include: (cell array) include only these CSX-properties
%      if neither .ignore or .include is specified, process all properties

if nargin < 3
	options = [];
end

fid = fopen( filename, 'w' );

% write header
fprintf( fid, '%s\n', 'G04 gerber RS274X-file exported by openEMS*' );
fprintf( fid, '%s\n', '%FSLAX66Y66*%' ); % leading zeros omitted; absolute coordinate values; 123456.123456
fprintf( fid, '%s\n', '%MOMM*%' ); % set units to mm
fprintf( fid, '%s\n', '%INopenEMS export*%' ); % image name
fprintf( fid, '%s\n', '%ADD10C,0.00100*%' ); % aperture description
if isfield(options,'header')
	fprintf( fid, '%s\n', header );
end

global drawingunit
drawingunit = CSX.RectilinearGrid.ATTRIBUTE.DeltaUnit;
global CoordSystem
CoordSystem = CSX.ATTRIBUTE.CoordSystem;

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
if isfield(CSX.Properties,'ConductingSheet')
	% process conductor
	Metal = CSX.Properties.ConductingSheet;
	options.Property = 'Metal';
	process_primitives( fid, Metal, options );
end

fprintf( fid, '%s\n', 'M02*' ); % end of program
fclose( fid );


% -----------------------------------------------------------------------------
function str = gerber_coord(v, CoordSystem)
global drawingunit

if (CoordSystem==1)
    r = v(1);
    a = v(2);
    v(1) = r*cos(a) * drawingunit;
    v(2) = r*sin(a) * drawingunit;
else
    v(1) = v(1) * drawingunit;
    v(2) = v(2) * drawingunit;
end
x = sprintf( '%+013i', round(v(1)*1e9) ); % mm
y = sprintf( '%+013i', round(v(2)*1e9) ); % mm
if (numel(x) ~= 13) || (numel(y) ~= 13)
	error( ['gerber_coord(): coordinate transformation failed: x=' x ' y=' y]  );
end
str = ['X' x 'Y' y];

% -----------------------------------------------------------------------------
function primitive_box( fid, CSX_box, CoordSystem )
start = [CSX_box.P1.ATTRIBUTE.X CSX_box.P1.ATTRIBUTE.Y];
stop  = [CSX_box.P2.ATTRIBUTE.X CSX_box.P2.ATTRIBUTE.Y];

if (CoordSystem==0)
    fprintf( fid, '%s\n', 'G36*' );
    fprintf( fid, '%s\n', [gerber_coord(start, CoordSystem) 'D02*'] );
    fprintf( fid, '%s\n', [gerber_coord([stop(1) start(2)], CoordSystem) 'D01*'] );
    fprintf( fid, '%s\n', [gerber_coord([stop], CoordSystem) 'D01*'] );
    fprintf( fid, '%s\n', [gerber_coord([start(1) stop(2)], CoordSystem) 'D01*'] );
    fprintf( fid, '%s\n', [gerber_coord(start, CoordSystem) 'D01*'] );
    fprintf( fid, '%s\n', 'G37*' );
elseif (CoordSystem==1)
    r = sort([start(1) stop(1)]);
    a = sort([start(2) stop(2)]);
    max_arc = 0.5*pi/180; 
    a = linspace(a(1),a(2),ceil((a(2)-a(1))/max_arc));
    
    fprintf( fid, '%s\n', 'G36*' );
    fprintf( fid, '%s\n', [gerber_coord([r(1) a(1)], CoordSystem) 'D02*'] );

    for ang = a(2:end)
        fprintf( fid, '%s\n', [gerber_coord([r(1) ang], CoordSystem) 'D01*'] );        
    end
    
    for ang = fliplr(a)
        fprintf( fid, '%s\n', [gerber_coord([r(2) ang], CoordSystem) 'D01*'] );        
    end
    
    fprintf( fid, '%s\n', [gerber_coord([r(1) a(1)], CoordSystem) 'D01*'] );
    fprintf( fid, '%s\n', 'G37*' );
else
    error 'unknown coordinate system'
end

% -----------------------------------------------------------------------------
function primitive_cylinder( fid, CSX_cylinder, CoordSystem )
global drawingunit
start  = [CSX_cylinder.P1.ATTRIBUTE.X CSX_cylinder.P1.ATTRIBUTE.Y];
stop   = [CSX_cylinder.P2.ATTRIBUTE.X CSX_cylinder.P2.ATTRIBUTE.Y];
radius = CSX_cylinder.ATTRIBUTE.Radius * drawingunit;
if start(1:2) == stop(1:2)
	% via => draw circle
	fprintf( fid, '%%ADD10C,%f*%%\n', radius*2*1e3 ); % aperture definition (mm)
	fprintf( fid, '%s\n', 'G54D10*' ); % select aperture D10
	fprintf( fid, '%s\n', [gerber_coord(start, CoordSystem) 'D03*'] ); % flash
else
	disp( ['omitting   primitive cylinder, because the projection onto the z-plane is not a circle'] );	
end


% -----------------------------------------------------------------------------
function primitive_polygon( fid, CSX_polygon, CoordSystem )
if CSX_polygon.ATTRIBUTE.NormDir ~= 2
	disp( ['omitting primitive polygon, because the normal direction is not 2'] );
end
fprintf( fid, '%s\n', 'G36*' );
v = [CSX_polygon.Vertex{1}.ATTRIBUTE.X1 CSX_polygon.Vertex{1}.ATTRIBUTE.X2];
fprintf( fid, '%s\n', [gerber_coord(v, CoordSystem) 'D02*'] );
for a=2:numel(CSX_polygon.Vertex)
	% iterate over all vertices
	v = [CSX_polygon.Vertex{a}.ATTRIBUTE.X1 CSX_polygon.Vertex{a}.ATTRIBUTE.X2];
	fprintf( fid, '%s\n', [gerber_coord(v, CoordSystem) 'D01*'] );
end
fprintf( fid, '%s\n', 'G37*' );

% -----------------------------------------------------------------------------
function primCoordSystem = GetCoordSystem(prim)
global CoordSystem
primCoordSystem = CoordSystem;
if (isfield(prim.ATTRIBUTE,'CoordSystem'))
    primCoordSystem = prim.ATTRIBUTE.CoordSystem;
end

% -----------------------------------------------------------------------------
function process_primitives( fid, prop, options )
ignore = {};
if isfield(options,'ignore'), ignore = options.ignore; end
for num=1:numel(prop)
	Name = prop{num}.ATTRIBUTE.Name;
	if any( strcmp(Name,ignore) )
		disp( ['omitting   ' Name '...'] );
		continue
	end
	if strcmp(options.Property,'Material') && ~isfield(prop{num}.Property.ATTRIBUTE,'Kappa')
		disp( ['omitting   ' Name ' (no Kappa-value)...'] );
		continue
    end
    
	disp( ['processing ' prop{num}.ATTRIBUTE.Name '...'] );
	fprintf( fid, '%s\n', ['%LN' Name '*%'] );
    if ~isfield(prop{num},'Primitives')
        continue
    end
	if isfield(prop{num}.Primitives,'Box')
		for a=1:numel(prop{num}.Primitives.Box)
			% iterate over all boxes
			Box = prop{num}.Primitives.Box{a};
			primitive_box( fid, Box, GetCoordSystem(Box) );
		end
	end
	if isfield(prop{num}.Primitives,'Cylinder')
		for a=1:numel(prop{num}.Primitives.Cylinder)
			% iterate over all cylinders
			Cylinder = prop{num}.Primitives.Cylinder{a};
        	primitive_cylinder( fid, Cylinder, GetCoordSystem(Cylinder) );
		end
	end
	if isfield(prop{num}.Primitives,'Polygon')
		for a=1:numel(prop{num}.Primitives.Polygon)
			% iterate over all polygons
			Polygon = prop{num}.Primitives.Polygon{a};
        	primitive_polygon( fid, Polygon, GetCoordSystem(Polygon) );
		end
	end
end

