function CSX = AddProperty(CSX, type, name, varargin)
% function CSX = AddProperty(CSX, type, name, varargin)
%
% CSXCAD matlab interface
% -----------------------
% author: Thorsten Liebig

if ~ischar(type)
    error('CSXCAD::AddProperty: type must be a string');
end

if ~ischar(name)
    error('CSXCAD::AddProperty: name must be a string');
end

if isfield(CSX.Properties,type)
    CSX.Properties.(type){end+1}.ATTRIBUTE.Name=name;    
else
	CSX.Properties.(type) = {}; % create cell array
    CSX.Properties.(type){1}.ATTRIBUTE.Name=name;
end

for n=1:(nargin-2)/2
    CSX.Properties.(type){end}.ATTRIBUTE.(varargin{2*n-1})=varargin{2*n};
end
