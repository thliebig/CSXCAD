function CSX = SetPropertyArgs(CSX, type, name, property, varargin)
% CSX = SetPropertyArgs(CSX, type, name, property, varargin)
% 
% CSXCAD matlab interface
% -----------------------
% author: Thorsten Liebig

pos = GetPropertyPosition(CSX, type, name);

if (pos==0)
    error('CSXCAD:SetPropertyArgs',['property "' name '" of type "' type '" not found!']);
end

for n=1:numel(varargin)/2
    CSX.Properties.(type){pos}.(property).ATTRIBUTE.(varargin{2*n-1}) = varargin{2*n};
end
