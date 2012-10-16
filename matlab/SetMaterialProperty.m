function CSX = SetMaterialProperty(CSX, name, varargin)
% function CSX = SetMaterialProperty(CSX, name, varargin)
%
% Use this function to define the material constants:
%   'Epsilon':  relative electric permittivity: [Epsilon] = 1
%   'Mue':      relative magnetic permeability: [Mue} = 1
%   'Kappa':    electric conductivity: [Kappa] = S/m
%   'Sigma':    magnetic conductivity (non-physical property): [Sigma] = Ohm/m
%   'Density':  material mass density: [Density] = kg/m^3, e.g. water: 1000
%                   necessary for SAR calculations
%
% examples:
% CSX = AddMaterial( CSX, 'RO3010' );
% CSX = SetMaterialProperty( CSX, 'RO3010', 'Epsilon', 10.2, 'Mue', 1 );
% CSX = AddBox( CSX, 'RO3010', 0, [0 0 0], [100 1000 1000] );
%
% % anisotropic material
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

