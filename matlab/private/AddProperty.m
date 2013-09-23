function [CSX pos] = AddProperty(CSX, type, name, varargin)
% function CSX = AddProperty(CSX, type, name, varargin)
%
% CSXCAD matlab interface
% -----------------------
% author: Thorsten Liebig

% check if this property (for this type) already exists
pos = GetPropertyPosition(CSX, type, name);
if (pos>0)
    error('CSXCAD:AddProperty',['property "' name '" of type "' type '" already exists! Just use it!']);
end

if isfield(CSX.Properties,type)
    pos = numel(CSX.Properties.(type))+1;
else
	CSX.Properties.(type) = {}; % create cell array
    pos = 1;
end

CSX.Properties.(type){pos}.ATTRIBUTE.Name=name;
for n=1:numel(varargin)/2
    CSX.Properties.(type){pos}.ATTRIBUTE.(varargin{2*n-1})=varargin{2*n};
end
