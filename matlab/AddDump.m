function CSX = AddDump(CSX, name, varargin)
% function CSX = AddDump(CSX, name, varargin)
%
% possible arguments for useage with openEMS:
%   DumpType:   0 for E-field time-domain dump
%               1 for H-field time-domain dump
%
%   DumpMode:   0 no-interpolation
%               1 node-interpolation
%               2 cell-interpolation
%
%   FileType:   0 vtk-file dump
%               1 hdf5-file dump
%
%   SubSampling: field domain sub-sampling, e.g. '2,2,4'
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
	CSX.Properties.DumpBox = {}; % create cell array
    CSX.Properties.DumpBox{1}.ATTRIBUTE.Name=name;
end

for n=1:(nargin-2)/2
    if ~ischar(varargin{2*n-1})
        error(['CSXCAD::AddDump: not an attribute: ' varargin{2*n-1}]);
    end
    CSX.Properties.DumpBox{end}.ATTRIBUTE = setfield(CSX.Properties.DumpBox{end}.ATTRIBUTE, varargin{2*n-1},varargin{2*n});
end
