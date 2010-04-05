function CSX = AddDump(CSX, name, type, mode, fileType)
% function CSX = AddDump(CSX, name, type, mode, fileType)
%
% type      (field type)    is optional
% mode      (dump mode)     is optional
% fileType  (file type)     is optional
%
% e.g. AddDump(CSX,'Et',0,2,1);
% or   AddDump(CSX,'Ht',1);
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

if (nargin>2)
    CSX.Properties.DumpBox{end}.ATTRIBUTE.DumpType=type;
end
if (nargin>3)
    CSX.Properties.DumpBox{end}.ATTRIBUTE.DumpMode=mode;
end
if (nargin>4)
    CSX.Properties.DumpBox{end}.ATTRIBUTE.FileType=fileType;
end
