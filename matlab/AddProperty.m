function [CSX pos] = AddProperty(CSX, type, name, varargin)
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
    pos = numel(CSX.Properties.(type))+1;
    for n=1:numel(CSX.Properties.(type))
       if  strcmp(CSX.Properties.(type){n}.ATTRIBUTE.Name, name)
           pos=n;
       end
    end
    CSX.Properties.(type){pos}.ATTRIBUTE.Name=name;    
else
	CSX.Properties.(type) = {}; % create cell array
    CSX.Properties.(type){1}.ATTRIBUTE.Name=name;
    pos = 1;
end

for n=1:(nargin-3)/2
    CSX.Properties.(type){end}.ATTRIBUTE.(varargin{2*n-1})=varargin{2*n};
end
