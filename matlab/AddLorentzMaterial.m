function CSX = AddLorentzMaterial(CSX, name, varargin)
% function CSX = AddLorentzMaterial(CSX, name, varargin)
%
% Add a Drude/Lorentz type dispersive material model.
% Note: openEMS currently only supports a drude material type.
%
% The drude type frequency dependent material:
% eps_r(w) = eps_r* ( 1 - w_plasma^2/(w*(w-j/t_r)) )
% mue_r(w) = mue_r* ( 1 - w_plasma^2/(w*(w-j/t_r)) )
% with
% w_plasma: the respective plasma frequency
% t_r:      the respective relaxation time
%
% Use SetMaterialProperty to define the material constants:
%   'EpsilonPlasmaFrequency':  electric plasma frequency
%   'MuePlasmaFrequency':      magnetic plasma frequency
%   'EpsilonRelaxTime':        electric plasma relaxation time (losses)
%   'MueRelaxTime':            magnetic plasma relaxation time (losses)
%
% example:
%     CSX = AddLorentzMaterial(CSX,'drude');
%     CSX = SetMaterialProperty(CSX,'drude','Epsilon',5,'EpsilonPlasmaFrequency',5e9,'EpsilonRelaxTime',1e-9);
%     CSX = SetMaterialProperty(CSX,'drude','Mue',5,'MuePlasmaFrequency',5e9,'MueRelaxTime',1e-9);
%     [..]
%     CSX = AddBox(CSX,'drude', 10 ,start,stop);
%
% See also AddBox, AddMaterial, SetMaterialProperty
%
% CSXCAD matlab interface
% -----------------------
% author: Thorsten Liebig

CSX = AddProperty(CSX, 'LorentzMaterial', name, varargin{:});
