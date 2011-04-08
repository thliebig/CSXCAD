function CSX = AddLumpedElement(CSX, name, direction, varargin)
% function CSX = AddLumpedElement(CSX, name direction, varargin)
%
% Add a lumped element property to CSX with the given name.
% Remember to add at least one or more box primitive to any
% property.
% 
% arguments:
%   direction:  0,1,2 for x-,y- or z-orientation of the lumped element
% 
% optional arguments:
%   'R', 'C', 'L':  definition of the lumped element properties
%   'Caps':         0 or 1 to (de)activate lumped element caps (1=default)
%
% examples:
% lumped element capacitor in y-direction with 1pF
% CSX = AddLumpedElement( CSX, 'Capacitor', 1, 'Caps', 1, 'C', 1e-12 );
% CSX = AddBox( CSX, 'RO3010', 0, [0 0 0], [10 10 10] );
%
% See also AddMaterial, AddMetal, AddExcitation, AddProbe, AddDump, AddBox
%
% CSXCAD matlab interface
% -----------------------
% author: Thorsten Liebig

CSX = AddProperty(CSX, 'LumpedElement', name, 'Direction', direction, varargin{:});
