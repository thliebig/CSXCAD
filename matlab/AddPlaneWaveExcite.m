function CSX = AddPlaneWaveExcite(CSX, name, k_dir, E_dir, f0, varargin)
% function CSX = AddPlaneWaveExcite(CSX, name, k_dir, E_dir, <f0, varargin>)
%
% Creates a plane wave excitation in the sense of a total-field/scattered
% field approach.
%
% Note: A plane wave excitation must not intersect with any kind of
% material. This exctiation type can only be applies in air/vacuum and
% completely surrounding a structure!!!
% 
% Note: Only a single Box can be applied to this property!!
%
% Arguments
% CSX:   CSX-struct created by InitCSX
% name:  property name for the excitation
% k_dir: unit vector of wave progation direction
% E_dir: electric field polarisation vector (must be orthogonal to k_dir)
% f0:    frequency for numerical phase velocity compensation (optional)
% 
% example:
% inc_angle = 0 /180*pi; %incident angle on the x-axis
% k_dir = [cos(inc_angle) sin(inc_angle) 0]; % plane wave direction
% E_dir = [0 0 1]; % plane wave polarization --> E_z
% f0 = 500e6;      % frequency for numerical phase velocity compensation
% 
% CSX = AddPlaneWaveExcite(CSX, 'plane_wave', k_dir, E_dir, f0);
%
% CSXCAD matlab interface
% -----------------------
% author: Thorsten Liebig
%
% See also AddExcitation, AddBox

if (nargin<5)
    f0 = 0;
end

CSX = AddExcitation(CSX, name, 10, E_dir, 'PropDir', k_dir, 'Frequency', f0, varargin{:});
