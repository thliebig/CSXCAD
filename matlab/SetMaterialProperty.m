function CSX = SetMaterialProperty(CSX, name, varargin)

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
    CSX.Properties.Material{pos}.Property.ATTRIBUTE = setfield(CSX.Properties.Material{pos}.Property.ATTRIBUTE, varargin{2*n-1},varargin{2*n});
end