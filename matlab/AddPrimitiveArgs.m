function primitive = AddPrimitiveArgs(primitive, varargin)
% prim = AddPrimitiveArgs(primitive, varargin)
%
% meant for internal use only
%
% CSXCAD matlab interface
% -----------------------
% author: Thorsten Liebig

for n=1:(nargin-1)/2
    primitive.ATTRIBUTE.(varargin{2*n-1}) = varargin{2*n};
end