function CSX = AddProbe(CSX, name, type, weight, FD_samples, varargin)
% function CSX = AddProbe(CSX, name, type, weight)
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
% all following parameter are optional:
%
% weight:   weighting factor (default is 1)
%
% FD_samples: dump in the frequency domain at the given samples (in Hz)
%
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

if (nargin<4)
    weight=1;
end

if (nargin<5)
    FD_samples = [];
end

if ~ischar(name)
    error('CSXCAD::AddProbe: name must be a string');
end

[CSX pos] = AddProperty(CSX, 'ProbeBox', name, 'Type', type, 'Weight', weight);

if (nargin>4)
    if (numel(FD_samples)>0)
        CSX.Properties.ProbeBox{pos}.FD_Samples=FD_samples;
    end
end

for n=1:(nargin-5)/2
    if ( (ischar(varargin{2*n})) || isnumeric(varargin{2*n}))
        CSX.Properties.ProbeBox{pos}.Attributes.ATTRIBUTE.(varargin{2*n-1}) = varargin{2*n};
    else 
        value = varargin{2*n};
        CSX.Properties.ProbeBox{pos}.Attributes.ATTRIBUTE.([varargin{2*n-1} 'X']) = value{1};
        CSX.Properties.ProbeBox{pos}.Attributes.ATTRIBUTE.([varargin{2*n-1} 'Y']) = value{2};
        CSX.Properties.ProbeBox{pos}.Attributes.ATTRIBUTE.([varargin{2*n-1} 'Z']) = value{3};
    end
end

