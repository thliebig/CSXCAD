function CSX = SetBackgroundMaterial(CSX, varargin)
% function CSX = SetBackgroundMaterial(CSX, varargin))
%
% Set the background material properties
%
% variable arguments:
%   'Epsilon'       : background rel. electric permittivity (default 1)
%   'Kappa'         : background electric conductivity (default 0)
%   'Mue'           : background rel. magnetic permeability (default 1) 
%
% example:
%     CSX = InitCSX();
%     CSX = SetBackgroundMaterial(CSX, 'Epsilon', 4)
% 
% See also InitCSX
% 
% CSXCAD matlab interface
% -----------------------
% author: Thorsten Liebig (c) 2013

for n=1:2:numel(varargin)
    CSX.BackgroundMaterial.ATTRIBUTE.(varargin{n}) = (varargin{n+1});
end
