function CSX = SetMaterialProperty(CSX, name, varargin)
% function CSX = SetMaterialProperty(CSX, name, varargin)
%
% CSXCAD matlab interface
% -----------------------
% author: Thorsten Liebig

if ~isfield(CSX,'Properties')
    return
end
if ~isfield(CSX.Properties,'Material')
    return
end

pos=0;
for n=1:numel(CSX.Properties.Material)
   if  strcmp(CSX.Properties.Material{n}.ATTRIBUTE.Name, name)
       pos=n;
   end
end

if (pos==0)
    error('CSXCAD::SetMaterialProperty: property not found');
    return;
end

for n=1:(nargin-2)/2
    if ( (ischar(varargin{2*n})) || isnumeric(varargin{2*n}))
        CSX.Properties.Material{pos}.Property.ATTRIBUTE = setfield(CSX.Properties.Material{pos}.Property.ATTRIBUTE, varargin{2*n-1},varargin{2*n});
    else 
        value = varargin{2*n};
        CSX.Properties.Material{pos}.Property.ATTRIBUTE.(varargin{2*n-1}) = value{1};
        CSX.Properties.Material{pos}.PropertyY.ATTRIBUTE.(varargin{2*n-1}) = value{2};
        CSX.Properties.Material{pos}.PropertyZ.ATTRIBUTE.(varargin{2*n-1}) = value{3};
    end
end
