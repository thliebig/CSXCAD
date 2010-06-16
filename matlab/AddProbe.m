function CSX = AddProbe(CSX, name, type, weight)
% function CSX = AddProbe(CSX, name, type, weight)
%
% name:     name of the property and probe file 
%
% type:     0 for voltage probing
%           1 for current probing
%
% weight:   weighting factor for this integral parameter
%           (default is 1)
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
