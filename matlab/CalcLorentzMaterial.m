function eps_lorentz = CalcLorentzMaterial(f, eps_r, kappa, plasmaFreq, LorPoleFreq, t_relax)
% eps_lorentz = CalcLorentzMaterial(f, eps_r, kappa, plasmaFreq, LorPoleFreq, t_relax)
%
% Calculate the Lorentz type dispersive material constant
%
% arguments:
%   f:           frequeny range of interest
%   eps_r:       eps_r infinity
%   kappa:       conductivity (losses)
%   plasmaFreq:  (vector) plasma frequencies (Drude model)
%   LorPoleFreq: (vector) Lorentz pole frequencies (zero for pure Drude model)
%   t_relax:     (vector) relaxation time (losses)
%
% return:
%   eps_lorentz: the complex relative permitivity
%
% Example:
% % silver (AG) at optical frequencies (Drude model) [1, p. 201]
% f = linspace(300e12, 1100e12, 201);
% eps_model = CalcLorentzMaterial(f, 3.942, 7.97e3, 7e15/2/pi, 0, 1/2.3e13);
%
% figure
% plot(f,real(eps_model))
% hold on;
% grid on;
% plot(f,imag(eps_model),'r--')
%
% % silver (AG) at optical frequencies (Drude+Lorentz model) [1, p. 201]
% f = linspace(300e12, 1100e12, 201);
% eps_model = CalcLorentzMaterial(f, 1.138, 4.04e3, [13e15 9.61e15]/2/pi, [0 7.5e15]/2/pi,[1/2.59e13 1/3e14]);
%
% figure
% plot(f,real(eps_model))
% hold on;
% grid on;
% plot(f,imag(eps_model),'r--')
%
% See also: CalcDebyeMaterial
%
% [1] Rennings, Andre: "Elektromagnetische Zeitbereichssimulationen
%     innovativer Antennen auf Basis von Metamaterialien."
%     PhD Thesis, University of Duisburg-Essen, September 2008
%
% CSXCAD matlab interface
% -----------------------
% author: Thorsten Liebig (2013)

EPS0 = 8.85418781762e-12;
eps_lorentz = ones(size(f))*eps_r - 1j*kappa./(2*pi*f)/EPS0;

w = 2*pi*f;
for n=1:numel(plasmaFreq)
    if (t_relax(n)>0)
        w_r = 1/t_relax(n);
    else
        w_r = 0;
    end
    eps_lorentz = eps_lorentz - eps_r*(2*pi*plasmaFreq(n))^2./(w.^2 - (2*pi*LorPoleFreq(n))^2 - 2j*pi*f*w_r);
end

end
