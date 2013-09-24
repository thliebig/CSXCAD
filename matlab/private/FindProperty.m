function [type pos] = FindProperty(CSX, name)
% function [type pos] = FindProperty(CSX, name)
%
% internal function to find a given property
%
% CSXCAD matlab interface
% -----------------------
% author: Thorsten Liebig (c) 2013

pos = 0;
type = GetPropertyType(CSX, name);

if isempty(type)
    return;
end

pos = GetPropertyPosition(CSX, type, name);
