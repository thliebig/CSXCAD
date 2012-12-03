function CSX = AddDump(CSX, name, varargin)
% function CSX = AddDump(CSX, name, varargin)
%
% Add a dump property to CSX with the given name.
% 
% possible arguments for useage with openEMS:
%   DumpType:   0 for E-field time-domain dump (default)
%               1 for H-field time-domain dump
%               2 for electric current time-domain dump
%               3 for total current density (rot(H)) time-domain dump
%
%              10 for E-field frequency-domain dump
%              11 for H-field frequency-domain dump
%              12 for electric current frequency-domain dump
%              13 for total current density (rot(H)) frequency-domain dump
%
%              20 local SAR frequency-domain dump
%              21  1g averaging SAR frequency-domain dump
%              22 10g averaging SAR frequency-domain dump
%
%              29 raw data needed for SAR calculations (electric field FD,
%                 cell volume, conductivity and density)
% 
%   DumpMode:   0 no-interpolation
%               1 node-interpolation (default, see warning below)
%               2 cell-interpolation (see warning below)
%
%   FileType:   0 vtk-file dump      (default)
%               1 hdf5-file dump
%
%   SubSampling:   field domain sub-sampling, e.g. '2,2,4'
%   OptResolution: field domain dump resolution, e.g. '10' or '10,20,5'
% 
%   Warning:
%       FDTD Interpolation abnormalities:
%           - no-interpolation: fields are located in the mesh by the
%           Yee-scheme, the mesh only specifies E- or H-Yee-nodes 
%             --> use node- or cell-interpolation or be aware of the offset
%           - E-field dump & node-interpolation: normal electric fields on
%           boundaries will have false amplitude due to forward/backward
%           interpolation --> use no- or cell-interpolation
%           - H-field dump & cell-interpolation: normal magnetic fields on
%           boundaries will have false amplitude due to forward/backward
%           interpolation --> use no- or node-interpolation
%
% e.g. AddDump(CSX,'Et');
%      CSX = AddBox(CSX,'Et',10,[0 0 0],[100 100 200]); %assign box
% 
% or   AddDump(CSX,'Ef',DumpType, 10, 'Frequency',[1e9 2e9]);
%      CSX = AddBox(CSX,'Ef',10,[0 0 0],[100 100 200]); %assign box
% 
% or   AddDump(CSX,'Ht','SubSampling','2,2,4','DumpType',1);
%      CSX = AddBox(CSX,'Ht',10,[0 0 0],[100 100 200]); %assign box
% 
% See also AddMaterial, AddExcitation, AddProbe, AddMetal, AddBox
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

% make Node-Interpolation the default
CSX.Properties.DumpBox{end}.ATTRIBUTE.DumpMode = 1;

for n=1:(nargin-2)/2
    if ~ischar(varargin{2*n-1})
        error(['CSXCAD::AddDump: not an attribute: ' varargin{2*n-1}]);
    end
    if strcmp(varargin{2*n-1},'Frequency')
        CSX.Properties.DumpBox{end}.FD_Samples=varargin{2*n};
    else
        CSX.Properties.DumpBox{end}.ATTRIBUTE = setfield(CSX.Properties.DumpBox{end}.ATTRIBUTE, varargin{2*n-1},varargin{2*n});
    end
end
