function CSX = AddDump(CSX, name, type, mode)
% function CSX = AddDump(CSX, name, type, mode)
%
% CSXCAD matlab interface
% -----------------------
% author: Thorsten Liebig

if ~ischar(name)
    error('CSXCAD::AddDump: name must be a string');
end

if isfield(CSX.Properties,'DumpBox')
    CSX.Properties.DumpBox{end+1}.ATTRIBUTE.Name=name;    
else
    CSX.Properties.DumpBox{1}.ATTRIBUTE.Name=name;
end

CSX.Properties.DumpBox{end}.ATTRIBUTE.DumpType=type;
CSX.Properties.DumpBox{end}.ATTRIBUTE.DumpMode=mode;
