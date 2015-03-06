function CSX = AddProbe(CSX, name, type, varargin)
% function CSX = AddProbe(CSX, name, type, varargin)
%
% Add a probe property to CSX with the given name.
% Remember to add a geometrical primitive to any property.
%
% name:     name of the property and probe file 
%
% type:     0 for voltage probing
%           1 for current probing
%           2 for E-field probing
%           3 for H-field probing
%
%           10 for waveguide voltage mode matching
%           11 for waveguide current mode matching
%
% all following parameter are optional key/value parameter:
%
% weight:       weighting factor (default is 1)
% frequency:    dump in the frequency domain at the given samples (in Hz)
% ModeFunction: A mode function (used only with type 3/4)
% NormDir:      necessary for current probing box with dimension~=2
% StartTime/StopTime: Define a start and/or stop time (in seconds) 
%                     for this probe to be active.

% examples:
%       CSX = AddProbe(CSX,'ut1',0); %voltate probe
%       CSX = AddProbe(CSX,'it1',1); %current probe
%
% See also ReadUI in the openEMS matlab interface, AddDump,
% AddExcitation, AddMaterial, AddExcitation, AddProbe, AddBox
%
% CSXCAD matlab interface
% -----------------------
% author: Thorsten Liebig

FD_samples = [];
ModeFunction = {};

if ~ischar(name)
    error('CSXCAD::AddProbe: name must be a string');
end

prop_args = {'Type', type};

for n=1:2:numel(varargin)
    if (strcmpi(varargin{n},'weight')==1);
        prop_args{end+1} = 'Weight';
        prop_args{end+1} = varargin{n+1};
    elseif (strcmpi(varargin{n},'Frequency')==1);
        FD_samples = varargin{n+1};
    elseif (strcmpi(varargin{n},'ModeFunction')==1);
        ModeFunction = varargin{n+1};
    elseif (strcmpi(varargin{n},'NormDir')==1);
        prop_args{end+1} = 'NormDir';
        prop_args{end+1} = varargin{n+1};
    elseif (strcmpi(varargin{n},'StartTime')==1);
        prop_args{end+1} = 'StartTime';
        prop_args{end+1} = varargin{n+1};
    elseif (strcmpi(varargin{n},'StopTime')==1);
        prop_args{end+1} = 'StopTime';
        prop_args{end+1} = varargin{n+1};
    else
        warning('CSXCAD:AddProbe',['variable argument key: "' varargin{n+1} '" unknown']);
    end
end

[CSX pos] = AddProperty(CSX, 'ProbeBox', name, prop_args{:});

if (numel(FD_samples)>0)
    CSX.Properties.ProbeBox{pos}.FD_Samples=FD_samples;
end

if (numel(ModeFunction)>0)
    CSX.Properties.ProbeBox{pos}.Attributes.ATTRIBUTE.ModeFunctionX = ModeFunction{1};
    CSX.Properties.ProbeBox{pos}.Attributes.ATTRIBUTE.ModeFunctionY = ModeFunction{2};
    CSX.Properties.ProbeBox{pos}.Attributes.ATTRIBUTE.ModeFunctionZ = ModeFunction{3};
end

