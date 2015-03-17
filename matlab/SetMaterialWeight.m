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

CSX = SetPropertyArgs(CSX, GetPropertyType(CSX,name), name, 'Weight', varargin{:});
