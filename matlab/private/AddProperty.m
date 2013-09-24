function [CSX pos] = AddProperty(CSX, type, name, varargin)
% function [CSX pos] = AddProperty(CSX, type, name, varargin)
%
% internal function to add a property to CSX.Properties
%
% CSXCAD matlab interface
% -----------------------
% author: Thorsten Liebig

% check if this property already exists
[type_found pos] = FindProperty(CSX, name);
if (pos>0)
    error('CSXCAD:AddProperty',['property with name "' name '" with type "' type_found '" already exists! Choose a different name!']);
end

if isfield(CSX.Properties,type)
    pos = numel(CSX.Properties.(type))+1;
else
	CSX.Properties.(type) = {}; % create cell array
    pos = 1;
end

CSX.Properties.(type){pos}.ATTRIBUTE.Name=name;
for n=1:numel(varargin)/2
    if ~ischar(varargin{2*n-1})
        error(['CSXCAD::AddProperty: not an attribute: ' varargin{2*n-1}]);
    end
    CSX.Properties.(type){pos}.ATTRIBUTE.(varargin{2*n-1})=varargin{2*n};
end
