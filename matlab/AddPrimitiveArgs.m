function primitive = AddPrimitiveArgs(primitive, varargin)
% prim = AddPrimitiveArgs(primitive, varargin)
%
% meant for internal use only
%
% CSXCAD matlab interface
% -----------------------
% author: Thorsten Liebig

transform = [];

for n=1:2:(nargin-2)
    if (strcmp(varargin{n},'Transform'))
        transform = varargin([n:n+1]);
        varargin([n:n+1]) = [];
        break
    end
end

for n=1:2:numel(varargin)
    primitive.ATTRIBUTE.(varargin{n}) = varargin{n+1};
end

if ~isempty(transform)
    for n=1:2:numel(transform{2})
        primitive.Transformation.(transform{2}{n}).ATTRIBUTE.Argument=transform{2}{n+1};
    end
end
