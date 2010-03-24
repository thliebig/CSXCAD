function CSX = AddMaterial(CSX, name)
% function CSX = AddMaterial(CSX, name)
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
    CSX.Properties.Material{1}.ATTRIBUTE.Name=name;
end

CSX.Properties.Material{end}.Property.ATTRIBUTE = [];
