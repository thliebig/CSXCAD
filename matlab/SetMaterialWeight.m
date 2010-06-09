function CSX = SetMaterialWeight(CSX, name, varargin)
% function CSX = SetMaterialWeight(CSX, name, varargin)
%
% Define the material weighting function(s)
%
% The functions can use the variables:
%   x,y,z
%   rho for the distance to z-axis
%   r   for the distance to origin
%   a   for alpha (as in cylindircal and spherical coord systems)
%   t   for theta (as in the spherical coord system
%   
%   all these variables are not weighted with the drawing unit defined by
%   the grid
% 
% example:
%     %material distribution as a rect-function with 4 periods
%     start=[-500 -100 -500];
%     stop =[ 500  100  500];
%     CSX = AddMaterial(CSX, 'material');
%     CSX = SetMaterialProperty(CSX, 'material', 'Epsilon', 1);
%     CSX = SetMaterialWeight(CSX, 'material', 'Epsilon', ['(sin(4*z / 1000 *2*pi)>0)+1']);
%     CSX = AddBox(CSX, 'material' ,10 , start, stop);
%
% See also AddMaterial, SetMaterialProperty, InitCSX, DefineRectGrid
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
        value = { varargin{2*n},  varargin{2*n},varargin{2*n}};
    else 
        value = varargin{2*n};
    end
    if ischar(value{1})
        CSX.Properties.Material{pos}.WeightX.ATTRIBUTE.(varargin{2*n-1})=['term:' value{1}];
    else
        CSX.Properties.Material{pos}.WeightX.ATTRIBUTE.(varargin{2*n-1})=value{1};
    end
    if ischar(value{2})
        CSX.Properties.Material{pos}.WeightY.ATTRIBUTE.(varargin{2*n-1})=['term:' value{2}];
    else
        CSX.Properties.Material{pos}.WeightY.ATTRIBUTE.(varargin{2*n-1})=value{2};
    end
    if ischar(value{3})
        CSX.Properties.Material{pos}.WeightZ.ATTRIBUTE.(varargin{2*n-1})=['term:' value{3}];
    else
        CSX.Properties.Material{pos}.WeightZ.ATTRIBUTE.(varargin{2*n-1})=value{3};
    end
end