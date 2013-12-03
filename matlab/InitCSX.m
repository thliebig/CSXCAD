function CSX = InitCSX(varargin)
% function CSX = InitCSX()
%
% Inititalize the CSX data-structure.
%
% variable arguments:
%   'CoordSystem'       : define the default coordinate system
%                         0 -> Cartesian
%                         1 -> Cylindircal
%                         2 -> Sphercial (not yet implemented)
% 
% example:
%     CSX = InitCSX(); %for a default cartesian mesh
% or
%     CSX = InitCSX('CoordSystem','1'); % for a cylindrical mesh definition
% 
% See also DefineRectGrid, SmoothMeshLines, SmoothMeshLines2,
%          SetBackgroundMaterial
% 
% CSXCAD matlab interface
% -----------------------
% author: Thorsten Liebig

CSX.Properties = [];

%Cartesian mesh as default coordinate system
CSX.ATTRIBUTE.CoordSystem = 0;

for n=1:2:numel(varargin)
    CSX.ATTRIBUTE.(varargin{n}) = (varargin{n+1});
end
