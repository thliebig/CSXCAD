function CSX = SetExcitationWeight(CSX, name, weight)
% function CSX = SetExcitationWeight(CSX, name, weight)
%
% Define weighting functions for x-, y- and z-direction of excitation
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
%     start=[0 0 0];
%     stop=[width height 0];
%     CSX = AddExcitation(CSX,'excite',0,[1 1 0]);
%     weight{1} = '2*cos(0.0031416*x)*sin(0.0062832*y)';
%     weight{2} = '1*sin(0.0031416*x)*cos(0.0062832*y)';
%     weight{3} = 0;
%     CSX = SetExcitationWeight(CSX,'excite',weight);
%     CSX = AddBox(CSX,'excite',0 ,start,stop);
%
% See also AddExcitation, InitCSX, DefineRectGrid
% 
% CSXCAD matlab interface
% -----------------------
% author: Thorsten Liebig

if ~isfield(CSX,'Properties')
    error('CSXCAD::SetExcitationWeight: no properties not found');
end
if ~isfield(CSX.Properties,'Excitation')
    error('CSXCAD::SetExcitationWeight: no excitation properties found');
end

pos=0;
for n=1:numel(CSX.Properties.Excitation)
   if  strcmp(CSX.Properties.Excitation{n}.ATTRIBUTE.Name, name)
       pos=n;
   end
end

if (pos==0)
    error('CSXCAD::SetExcitationWeight: property not found');
    return;
end

CSX.Properties.Excitation{pos}.Weight.ATTRIBUTE.X = weight{1};
CSX.Properties.Excitation{pos}.Weight.ATTRIBUTE.Y = weight{2};
CSX.Properties.Excitation{pos}.Weight.ATTRIBUTE.Z = weight{3};
