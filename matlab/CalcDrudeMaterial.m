function eps_drude = CalcDrudeMaterial(f, eps_r, kappa, plasmaFreq, t_relax)
% eps_drude = CalcDrudeMaterial(f, eps_r, kappa, plasmaFreq, t_relax)
%
% Calculate the Drude type dispersive material constant
%
% arguments:
%   f:           frequeny range of interest
%   eps_r:       eps_r infinity
%   kappa:       conductivity (losses)
%   plasmaFreq:  (vector) plasma frequencies
%   t_relax:     (vector) relaxation time (losses)
%
% return:
%   eps_drude: the complex relative permitivity
%
% Example:
% % silver (AG) at optical frequencies (Drude model) [1, p. 201]
% f = linspace(300e12, 1100e12, 201);
% eps_model = CalcDrudeMaterial(f, 3.942, 7.97e3, 7e15/2/pi, 0, 1/2.3e13);
%
% figure
% plot(f,real(eps_model))
% hold on;
% grid on;
% plot(f,imag(eps_model),'r--')
%
% See also: CalcDebyeMaterial, CalcLorentzMaterial
%
% [1] Rennings, Andre: "Elektromagnetische Zeitbereichssimulationen
%     innovativer Antennen auf Basis von Metamaterialien."
%     PhD Thesis, University of Duisburg-Essen, September 2008
%
% CSXCAD matlab interface
% -----------------------
% author: Thorsten Liebig (2013)

eps_drude = CalcLorentzMaterial(f, eps_r, kappa, plasmaFreq, plasmaFreq*0, t_relax);
