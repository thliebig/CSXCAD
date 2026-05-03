function CSX = AddLumpedElement(CSX, name, direction, varargin)
% function CSX = AddLumpedElement(CSX, name, direction, varargin)
%
% Add a lumped element property to CSX with the given name.
% Remember to add at least one or more box primitive to any
% property.
%
% arguments:
% - direction:  0,1,2 or 'x','y','z' for the orientation of the lumped element
%
% optional arguments:
% - 'R', 'C', 'L':  definition of the lumped element properties
% - 'Caps':         0 or 1 to (de)activate lumped element caps (1=default)
%                   If Caps are enable, a small PEC plate is added to each
%                   end of the lumped element to ensure contact to e.g
%                   a microstrip line
% - 'LEtype':       0 = parallel (default), 1 = series
%                   Parallel: R, C and L are all in parallel (standard
%                   lumped capacitor/resistor/LC tank).
%                   Series: R, L and C are in series (use this for a
%                   series inductor, e.g. to tune antenna length).
%
% examples:
%
%     %lumped element capacitor in y-direction with 1pF
%     CSX = AddLumpedElement( CSX, 'Capacitor', 1, 'Caps', 1, 'C', 1e-12 );
%     CSX = AddBox( CSX, 'Capacitor', 0, [0 0 0], [10 10 10] );
%
%     %series inductor in x-direction with 10nH (e.g. to tune antenna length)
%     CSX = AddLumpedElement( CSX, 'Inductor', 0, 'Caps', 1, 'L', 10e-9, 'LEtype', 1 );
%     CSX = AddBox( CSX, 'Inductor', 0, [0 0 0], [10 10 10] );
%
% See also AddMaterial, AddMetal, AddExcitation, AddProbe, AddDump, AddBox
%
% CSXCAD matlab interface
% -----------------------
% author: Thorsten Liebig

CSX = AddProperty(CSX, 'LumpedElement', name, 'Direction', DirChar2Int(direction), varargin{:});
