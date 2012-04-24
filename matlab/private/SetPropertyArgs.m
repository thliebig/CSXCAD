function CSX = SetPropertyArgs(CSX, type, name, property, varargin)
% CSX = SetPropertyArgs(CSX, type, name, varargin)
% 
% CSXCAD matlab interface
% -----------------------
% author: Thorsten Liebig

if ~isfield(CSX,'Properties')
    return
end
if ~isfield(CSX.Properties,type)
    return
end
if isempty(type)
    error('openEMS:SetPropertyArgs','type is empty, maybe the property you requested is undefined');
end

pos=0;
for n=1:numel(CSX.Properties.(type))
   if  strcmp(CSX.Properties.(type){n}.ATTRIBUTE.Name, name)
       pos=n;
   end
end

if (pos==0)
    error('CSXCAD::SetMaterialProperty: property not found');
    return;
end

for n=1:(nargin-4)/2
        CSX.Properties.(type){pos}.(property).ATTRIBUTE.(varargin{2*n-1}) = varargin{2*n};
    end
end
