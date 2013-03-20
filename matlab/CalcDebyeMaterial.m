function eps_debye = CalcDebyeMaterial(f, eps_r, kappa, eps_Delta, t_relax)
% eps_debye = CalcDebyeMaterial(f, eps_r, kappa, eps_Delta, t_relax)
%
% Calculate the Debye type dispersive material constant
%
% arguments:
%   f:      frequeny range of interest
%   eps_r:  eps_r infinity
%   kappa:  conductivity (losses)
%   eps_Delta: (vector) delta of relative permitivity
%   t_relax:  (vector) relaxation time (losses)
%
% return:
%   eps_debye:    the complex relative permitivity
%
% See also: CalcLorentzMaterial
%
% CSXCAD matlab interface
% -----------------------
% author: Thorsten Liebig (2013)

EPS0 = 8.85418781762e-12;
eps_debye = ones(size(f))*eps_r - 1j*kappa./(2*pi*f)/EPS0;

for n=1:numel(eps_Delta)
    eps_debye = eps_debye + eps_Delta(n)./(1+2j*pi*f*t_relax(n));
end

end
