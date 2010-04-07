function CSX = AddDump(CSX, name, varargin)
% function CSX = AddDump(CSX, name, varargin)
%
% e.g. AddDump(CSX,'Et');
% or   AddDump(CSX,'Ht','SubSampling','2,2,4','DumpType',1);
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

for n=1:(nargin-2)/2
    if ~ischar(varargin{2*n-1})
        error(['CSXCAD::AddDump: not an attribute: ' varargin{2*n-1}]);
    end
    CSX.Properties.DumpBox{end}.ATTRIBUTE = setfield(CSX.Properties.DumpBox{end}.ATTRIBUTE, varargin{2*n-1},varargin{2*n});
end
