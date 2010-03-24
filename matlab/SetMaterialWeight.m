function CSX = SetMaterialWeight(CSX, name, varargin)
% function CSX = SetMaterialWeight(CSX, name, varargin)
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
    if ischar(varargin{2*n})
        CSX.Properties.Material{pos}.WeightX.ATTRIBUTE.(varargin{2*n-1})=['term:' varargin{2*n}];
    else
        CSX.Properties.Material{pos}.WeightX.ATTRIBUTE.(varargin{2*n-1})=varargin{2*n};
    end
    if ischar(varargin{2*n})
        CSX.Properties.Material{pos}.WeightY.ATTRIBUTE.(varargin{2*n-1})=['term:' varargin{2*n}];
    else
        CSX.Properties.Material{pos}.WeightY.ATTRIBUTE.(varargin{2*n-1})=varargin{2*n};
    end
    if ischar(varargin{2*n})
        CSX.Properties.Material{pos}.WeightZ.ATTRIBUTE.(varargin{2*n-1})=['term:' varargin{2*n}];
    else
        CSX.Properties.Material{pos}.WeightZ.ATTRIBUTE.(varargin{2*n-1})=varargin{2*n};
    end
end