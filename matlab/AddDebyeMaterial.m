function CSX = AddDebyeMaterial(CSX, name, varargin)
% function CSX = AddDebyeMaterial(CSX, name, varargin)
%
% Add a Debye type dispersive material model.
%
% The Debye type frequency dependent material:
% eps_r(w) = eps_r + sum_p ( eps_r_delta,p / (1+jw*t_relex,p) ) - j*kappa/w
%
% with
% eps_r_delta,p: the delta electric relative permitivity
% t_relex,p:     the electric relaxation time
%
% Use SetMaterialProperty to define the material constants:
%   'EpsilonDelta_p':      p-th eps_r_delta,p
%   'EpsilonRelaxTime_p':  p-th t_relex,p
%
% example:
%     CSX = AddDebyeMaterial(CSX,'debye');
%     CSX = SetMaterialProperty(CSX,'debye','Epsilon',5,'EpsilonDelta_1',0.1,'EpsilonRelaxTime_1',1e-9);
%     [..]
%     CSX = AddBox(CSX,'debye', 10 ,start,stop);
%
% See also AddBox, AddMaterial, SetMaterialProperty, AddLorentzMaterial
%
% CSXCAD matlab interface
% -----------------------
% author: Thorsten Liebig (2013)

CSX = AddProperty(CSX, 'DebyeMaterial', name, varargin{:});
