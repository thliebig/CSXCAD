function CSX = SetExcitationWeightFile(CSX, name, fileName, propDir, origin)
% function CSX = SetExcitationWeightFile(CSX, name, fileName, propDir)
% function CSX = SetExcitationWeightFile(CSX, name, fileName, propDir, origin)
%
% Set the HDF5 weight file and propagation direction for an excitation.
%
% Required arguments:
%   CSX      - CSX structure
%   name     - name of the excitation property
%   fileName - path to the HDF5 weight file
%   propDir  - propagation direction vector, e.g. [0 0 1] for z-propagation
%
% Optional:
%   origin   - [x,y,z] coordinate origin for weight file evaluation
%              (in drawing units). If given, coordinates are shifted by this
%              offset before interpolation into the mode file.
%
% See also AddExcitation, SetExcitationWeight, InitCSX, DefineRectGrid
%
% CSXCAD matlab interface
% -----------------------
% Author: (2023-2025) Gadi Lahav - gadi@rfwithcare.com

if nargin < 5
    origin = [];
end

if ~isfield(CSX,'Properties')
    error('CSXCAD::SetExcitationWeightFile: no properties found');
end
if ~isfield(CSX.Properties,'Excitation')
    error('CSXCAD::SetExcitationWeightFile: no excitation properties found');
end

pos = 0;
for n = 1:numel(CSX.Properties.Excitation)
    if strcmp(CSX.Properties.Excitation{n}.ATTRIBUTE.Name, name)
        pos = n;
        break;
    end
end

if pos == 0
    error('CSXCAD::SetExcitationWeightFile: excitation "%s" not found', name);
end

CSX.Properties.Excitation{pos}.ATTRIBUTE.WeightFile = fileName;
CSX.Properties.Excitation{pos}.ATTRIBUTE.PropDir = propDir;

if ~isempty(origin)
    CSX.Properties.Excitation{pos}.WeightOrigin.ATTRIBUTE.X = num2str(origin(1), 15);
    CSX.Properties.Excitation{pos}.WeightOrigin.ATTRIBUTE.Y = num2str(origin(2), 15);
    CSX.Properties.Excitation{pos}.WeightOrigin.ATTRIBUTE.Z = num2str(origin(3), 15);
end
