function CSX = AddLorentzMaterial(CSX, name, varargin)
% function CSX = AddLorentzMaterial(CSX, name, varargin)
%
% Add a Drude/Lorentz type dispersive material model.
% Note: openEMS currently only supports a drude material type.
%
% The drude type frequency dependent material:
% eps_r(w) = eps_r* ( 1 - w_eps_plasma^2/(w*(w-j/t_eps_r)) )
% mue_r(w) = mue_r* ( 1 - w_mue_plasma^2/(w*(w-j/t_mue_r)) )
% with
% w_eps_plasma: the respective electric angular plasma frequency
% w_mue_plasma: the respective magnetic angular plasma frequency
% t_eps_r:      the respective electric relaxation time
% t_mue_r:      the respective magnetic relaxation time
%
% Use SetMaterialProperty to define the material constants:
%   'EpsilonPlasmaFrequency':  electric plasma frequency (f_eps_plasma)
%   'MuePlasmaFrequency':      magnetic plasma frequency (f_mue_plasma)
%   'EpsilonRelaxTime':        electric plasma relaxation time (losses)
%   'MueRelaxTime':            magnetic plasma relaxation time (losses)
%
% Note: all properties must be positive values
%
% Higher order Drude type:
%   'EpsilonPlasmaFrequency_<n>':  n-th order electric plasma frequency (f_eps_plasma)
%   'MuePlasmaFrequency_<n>':      n-th order magnetic plasma frequency (f_mue_plasma)
%   'EpsilonRelaxTime_<n>':        n-th order electric plasma relaxation time (losses)
%   'MueRelaxTime_<n>':            n-th order magnetic plasma relaxation time (losses)
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
