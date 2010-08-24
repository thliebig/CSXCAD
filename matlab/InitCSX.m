function CSX = InitCSX(varargin)
% function CSX = InitCSX()
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