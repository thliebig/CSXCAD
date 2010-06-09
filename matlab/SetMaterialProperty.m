function CSX = SetMaterialProperty(CSX, name, varargin)
% function CSX = SetMaterialProperty(CSX, name, varargin)
%
% examples:
% CSX = AddMaterial( CSX, 'RO3010' );
% CSX = SetMaterialProperty( CSX, 'RO3010', 'Epsilon', 10.2, 'Mue', 1 );
% CSX = AddBox( CSX, 'RO3010', 0, [0 0 0], [100 1000 1000] );
%
% %unisotropic material
% CSX = AddMaterial( CSX, 'sheet','Isotropy',0);
% CSX = SetMaterialProperty(CSX, 'sheet', 'Kappa', [0 0 kappa]);
% CSX = AddBox( CSX, 'sheet', 0, [0 0 0], [10 1000 1000] );
%
% See also AddMaterial, SetMaterialWeight
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
