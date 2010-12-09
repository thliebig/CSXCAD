function CSX = InitCSX(varargin)
% function CSX = InitCSX()
%
% Inititalize the CSX data-structure.
% 
% example:
%     CSX = InitCSX(); %for a default cartesian mesh
% or
%     CSX = InitCSX('CoordSystem','1'); % for a cylindrical mesh definition
% 
% See also DefineRectGrid, SmoothMeshLines, SmoothMeshLines2
% 
% CSXCAD matlab interface
% -----------------------
% author: Thorsten Liebig

CSX.Properties = [];

if (nargin>0)
    for n=1:nargin/2
        CSX.ATTRIBUTE.(varargin{2*n-1}) = (varargin{2*n});
    end
end