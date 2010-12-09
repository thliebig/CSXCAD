function CSX = AddMaterial(CSX, name, varargin)
% function CSX = AddMaterial(CSX, name, varargin)
%
% Add a material property to CSX with the given name.
% Remember to add at least one or more geometrical primitives to any
% property.
%
% examples:
% CSX = AddMaterial( CSX, 'RO3010' );
% CSX = SetMaterialProperty( CSX, 'RO3010', 'Epsilon', 10.2, 'Mue', 1 );
% CSX = AddBox( CSX, 'RO3010', 0, [0 0 0], [100 1000 1000] );
%
% See also SetMaterialProperty, SetMaterialWeight, AddMetal, AddExcitation,
% AddProbe, AddDump, AddBox
%
% CSXCAD matlab interface
% -----------------------
% author: Thorsten Liebig

CSX = AddProperty(CSX, 'Material', name, varargin{:});
