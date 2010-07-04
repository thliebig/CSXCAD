function CSX = AddMaterial(CSX, name, varargin)
% function CSX = AddMaterial(CSX, name, varargin)
%
% examples:
% CSX = AddMaterial( CSX, 'RO3010' );
% CSX = SetMaterialProperty( CSX, 'RO3010', 'Epsilon', 10.2, 'Mue', 1 );
% CSX = AddBox( CSX, 'RO3010', 0, [0 0 0], [100 1000 1000] );
%
% See also SetMaterialProperty, SetMaterialWeight
%
% CSXCAD matlab interface
% -----------------------
% author: Thorsten Liebig

CSX = AddProperty(CSX, 'Material', name, varargin{:})
