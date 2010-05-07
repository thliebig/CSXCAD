function CSX = AddMaterial(CSX, name, varargin)
% function CSX = AddMaterial(CSX, name, varargin)
%
% e.g. CSX = AddMaterial(CSX, 'mat_name')
%
% CSXCAD matlab interface
% -----------------------
% author: Thorsten Liebig

if ~ischar(name)
    error('CSXCAD::AddMaterial: name must be a string');
end

if isfield(CSX.Properties,'Material')
    CSX.Properties.Material{end+1}.ATTRIBUTE.Name=name;    
else
	CSX.Properties.Material = {}; % create cell array
    CSX.Properties.Material{1}.ATTRIBUTE.Name=name;
end

CSX.Properties.Material{end}.Property.ATTRIBUTE = [];

for n=1:(nargin-2)/2
    CSX.Properties.Material{end}.ATTRIBUTE.(varargin{2*n-1})=varargin{2*n};
end
