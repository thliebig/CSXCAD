function CSX = SetExcitationWeight(CSX, name, weight)
% function CSX = SetExcitationWeight(CSX, name, weight)
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
if ischar(weight{2})
    CSX.Properties.Electrode{pos}.Weight.ATTRIBUTE.Z = ['term:' weight{3}];
else
    CSX.Properties.Electrode{pos}.Weight.ATTRIBUTE.Z = weight{3};
end
