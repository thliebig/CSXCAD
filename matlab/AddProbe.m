function CSX = AddProbe(CSX, name, type, weight, FD_samples)
% function CSX = AddProbe(CSX, name, type, weight)
%
% name:     name of the property and probe file 
%
% type:     0 for voltage probing
%           1 for current probing
%
% all following parameter are optional:
%
% weight:   weighting factor for this integral parameter
%           (default is 1)
%
% FD_samples: dump in the frequency domain at the given samples (in Hz)
%
% examples:
%       CSX = AddProbe(CSX,'ut1',0); %voltate probe
%       CSX = AddProbe(CSX,'it1',1); %current probe
%
% See also ReadUI in the openEMS matlab interface
%
% CSXCAD matlab interface
% -----------------------
% author: Thorsten Liebig

if (nargin<4)
    weight=1;
end

if ~ischar(name)
    error('CSXCAD::AddProbe: name must be a string');
end

if isfield(CSX.Properties,'ProbeBox')
    CSX.Properties.ProbeBox{end+1}.ATTRIBUTE.Name=name;    
else
	CSX.Properties.ProbeBox = {}; % create cell array
    CSX.Properties.ProbeBox{1}.ATTRIBUTE.Name=name;
end

CSX.Properties.ProbeBox{end}.ATTRIBUTE.Type=type;
CSX.Properties.ProbeBox{end}.ATTRIBUTE.Weight=weight;

if (nargin>4)
    CSX.Properties.ProbeBox{end}.FD_Samples=FD_samples;
end

