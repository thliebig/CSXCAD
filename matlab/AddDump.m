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
%               4 for D-field (electric flux density) time-domain dump
%               5 for B-field (magnetic flux density) time-domain dump
%
%              10 for E-field frequency-domain dump
%              11 for H-field frequency-domain dump
%              12 for electric current frequency-domain dump
%              13 for total current density (rot(H)) frequency-domain dump
%              14 for D-field (electric flux density) frequency-domain dump
%              15 for B-field (magnetic flux density) frequency-domain dump
%
%              20 local SAR frequency-domain dump
%              21  1g averaging SAR frequency-domain dump
%              22 10g averaging SAR frequency-domain dump
%
%              29 raw data needed for SAR calculations (electric field FD,
%                 cell volume, conductivity and density)
%
%   Frequency:  specify a frequency vector (required for dump types >=10)
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
%   MultiGridLevel: Request to dump from a multigrid level (default is 0)
%                   Note: This only takes effect if the method supports and
%                   uses multiple grids!
%
%   StartTime/StopTime: Define a start and/or stop time (in seconds) 
%                       for this dump to be active.
% 
%   Warning:
%       FDTD Interpolation abnormalities:
%         - no-interpolation: fields are located in the mesh by the
%           Yee-scheme, the mesh only specifies E- or H-Yee-nodes 
%             --> use node- or cell-interpolation or be aware of the offset
%         - E-field dump & node-interpolation: normal electric fields on
%           boundaries will have false amplitude due to forward/backward
%           interpolation  in case of (strong) changes in material
%           permittivity or on metal surfaces
%             --> use no- or cell-interpolation
%         - H-field dump & cell-interpolation: normal magnetic fields on
%           boundaries will have false amplitude due to forward/backward
%           interpolation in case of (strong) changes in material permeability
%             --> use no- or node-interpolation
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

[CSX pos] = AddProperty(CSX, 'DumpBox', name);

% make Node-Interpolation the default
CSX.Properties.DumpBox{pos}.ATTRIBUTE.DumpMode = 1;

for n=1:numel(varargin)/2
    if ~ischar(varargin{2*n-1})
        error(['CSXCAD::AddDump: not an attribute: ' varargin{2*n-1}]);
    end
    if strcmp(varargin{2*n-1},'Frequency')
        CSX.Properties.DumpBox{pos}.FD_Samples=varargin{2*n};
    else
        CSX.Properties.DumpBox{pos}.ATTRIBUTE.(varargin{2*n-1})=varargin{2*n};
    end
end
