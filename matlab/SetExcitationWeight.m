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
    return
end
if ~isfield(CSX.Properties,'Electrode')
    return
end

pos=0;
for n=1:numel(CSX.Properties.Electrode)
   if  strcmp(CSX.Properties.Electrode{n}.ATTRIBUTE.Name, name)
       pos=n;
   end
end

if (pos==0)
    error('CSXCAD::SetExcitationWeight: property not found');
    return;
end

if ischar(weight{1})
    CSX.Properties.Electrode{pos}.Weight.ATTRIBUTE.X = ['term:' weight{1}];
else
    CSX.Properties.Electrode{pos}.Weight.ATTRIBUTE.X = weight{1};
end
if ischar(weight{2})
    CSX.Properties.Electrode{pos}.Weight.ATTRIBUTE.Y = ['term:' weight{2}];
else
    CSX.Properties.Electrode{pos}.Weight.ATTRIBUTE.Y = weight{2};
end
if ischar(weight{3})
    CSX.Properties.Electrode{pos}.Weight.ATTRIBUTE.Z = ['term:' weight{3}];
else
    CSX.Properties.Electrode{pos}.Weight.ATTRIBUTE.Z = weight{3};
end
