function CSX = SetMaterialProperty(CSX, name, varargin)
% function CSX = SetMaterialProperty(CSX, name, varargin)
%
% Use this function to define the material constants:
%   'Epsilon':  relative electric permitivity
%   'Mue':      relative magnetic permeability
%   'Kappa':    electric conductivity
%   'Sigma':    magnetc conductivity (non-physical property)
%
% examples:
% CSX = AddMaterial( CSX, 'RO3010' );
% CSX = SetMaterialProperty( CSX, 'RO3010', 'Epsilon', 10.2, 'Mue', 1 );
% CSX = AddBox( CSX, 'RO3010', 0, [0 0 0], [100 1000 1000] );
%
% %unisotropic material
% CSX = AddMaterial( CSX, 'sheet','Isotropy',0);
% CSX = SetMaterialProperty(CSX, 'sheet', 'Kappa', [0 0 kappa]);
% CSX = AddBox( CSX, 'sheet', 0, [0 0 0], [10 1000 1000] );
%
% See also AddMaterial, SetMaterialWeight
% 
% CSXCAD matlab interface
% -----------------------
% author: Thorsten Liebig

CSX = SetPropertyArgs(CSX, GetPropertyType(CSX,name), name, 'Property', varargin{:});

