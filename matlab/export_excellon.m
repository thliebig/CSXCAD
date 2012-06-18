function export_excellon( CSX, filename, options )
% export_excellon( CSX, filename, options )
%
% Exports the geometry defined in CSX to filename as an excellon drill file.
% Only cylinders will be considered for export.
% The xy-plane is exported.
%
% CSX: CSX-object created by InitCSX()
% filename: export filename (e.g. '/tmp/export.gbr')
% options (optional): struct
%   .header: (string) add this to the header of the file
%      comment line must have ';' as the very first character
%   .exclude: (cell array) ignore these CSX-properties
%   .include: (cell array) include only these CSX-properties
%      if neither .exclude or .include is specified, process all properties
%
% See also InitCSX 
% CSXCAD matlab interface
% -----------------------
% author: Sebastian Held <sebastian.held@gmx.de>
% 26. Aug 2010: initial version


if nargin < 3
	options = [];
end

fid = fopen( filename, 'w' );

% create header
header = [];
header = [header 'M48\n'];
header = [header '; EXCELLON file written by CSXCAD\n' ];
header = [header '; ' datestr(now) '\n' ];
header = [header 'METRIC,LZ\n'];
header = [header 'VER,1\n'];
header = [header 'FMAT,1\n'];

if isfield(options,'header')
	header = [header options.header];
end

% determine the drawing unit
options.drawingunit = CSX.RectilinearGrid.ATTRIBUTE.DeltaUnit;

if isfield(CSX.Properties,'Material')
	% process material
	Material = CSX.Properties.Material;
	options.Property = 'Material';
	[tools,drill] = process_primitives( Material, options );
end
if isfield(CSX.Properties,'Metal')
	% process PEC
	Metal = CSX.Properties.Metal;
	options.Property = 'Metal';
	[tools_,drill_] = process_primitives( Metal, options );
    tools = unique( [tools tools_] );
    drill = [drill drill_];
end

% check the generated tools
[~,m] = unique( cellfun( @(x) x(1:3), tools, 'UniformOutput', 0 ) );
if length(m) ~= numel(tools)
    disp( 'the tool list is not unique:' );
    disp( tools );
    error( 'the tool list is not unique!' );
end
a = unique( cellfun( @(x) x(4), tools, 'UniformOutput', 0 ) );
if (length(a) > 1) || (a ~= 'C')
    disp( 'the tool list has errors:' );
    disp( tools );
    error( 'the tool list has errors!' );
end
    
% convert cell array of strings to string
tools = char(cellfun( @(x)[x '\n'], tools ));
drill = char(cellfun( @(x)[x '\n'], drill ));

% save the file
fprintf( fid, header );
fprintf( fid, tools );
fprintf( fid, '%s\n', 'M95' ); % end of program header
fprintf( fid, drill );
fprintf( fid, '%s\n', 'M00' ); % end
fclose( fid );


% -----------------------------------------------------------------------------
function str = coord(v)
% str = coord(v)
% 2D-vector v takes coordinates in unit m
x = sprintf( '%.3f', v(1)*1e3 ); % mm
y = sprintf( '%.3f', v(2)*1e3 ); % mm
str = ['X' x 'Y' y];

% -----------------------------------------------------------------------------
function [tool,drill] = primitive_cylinder( CSX_cylinder, drawingunit )
start  = [CSX_cylinder.P1.ATTRIBUTE.X CSX_cylinder.P1.ATTRIBUTE.Y] * drawingunit;
stop   = [CSX_cylinder.P2.ATTRIBUTE.X CSX_cylinder.P2.ATTRIBUTE.Y] * drawingunit;
radius = CSX_cylinder.ATTRIBUTE.Radius * drawingunit;
if start(1:2) == stop(1:2)
	% via in z-plane
    dia_mm = radius * 2 * 1e3; % mm
    tool = sprintf( 'T%02iC1.3f', round(dia_mm*10), dia_mm );
    drill = sprintf( 'T%02i\n%s', round(dia_mm*10), coord(start(1:2)) );
else
	disp( 'omitting   primitive cylinder, because the projection onto the z-plane is not a circle' );	
end


% -----------------------------------------------------------------------------
function [tools,drill] = process_primitives( prop, options )
exclude = {};
if isfield(options,'exclude'), exclude = options.exclude; end
for num=1:numel(prop)
	Name = prop{num}.ATTRIBUTE.Name;
    if any( strcmp(Name,exclude) )
		disp( ['omitting   ' Name '...'] );
		continue
    end
    if isfield(options,'include')
        include = options.include;
        if ~any( strcmp(Name,include) )
            disp( ['omitting   ' Name '...'] );
            continue
        end
    end
    
    tools = {};
    drill = {};
    
    disp( ['processing ' prop{num}.ATTRIBUTE.Name '...'] );
	fprintf( fid, '%s\n', ['%LN' Name '*%'] );
	if isfield(prop{num}.Primitives,'Cylinder')
		for a=1:numel(prop{num}.Primitives.Cylinder)
			% iterate over all cylinders
			Cylinder = prop{num}.Primitives.Cylinder{a};
			[tool,drill_] = primitive_cylinder( fid, Cylinder );
            tools = [tools tool];
            drill = [drill drill_];
		end
	end
end

