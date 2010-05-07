function CSX = AddMetal(CSX, name)
%function CSX = AddMetal(CSX, name)
%
% CSXCAD matlab interface
% -----------------------
% author: Thorsten Liebig

if ~ischar(name)
    error('CSXCAD::AddMetal: name must be a string');
end

if isfield(CSX.Properties,'Metal')
    CSX.Properties.Metal{end+1}.ATTRIBUTE.Name=name;    
else
    CSX.Properties.Metal{1}.ATTRIBUTE.Name=name;
end
