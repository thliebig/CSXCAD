function str = vector2str(vec, acc)
% function str = vector2str(vec, acc)
%
% internal function for CSXCAD
% 
% (c) Thorsten Liebig 2012

str = [];

if (numel(vec)==0)
    return
end

if (nargin<2)
    acc = 9;
end

for n=1:numel(vec)
    str = [str num2str(vec(n),acc) ','];
end

str = str(1:end-1);