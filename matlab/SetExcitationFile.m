function CSX = SetExcitationWeight(CSX, name, fileName)
% function CSX = SetExcitationWeight(CSX, name, fileName)
%
% Define the mode file source of the excitation
%
% example:
%
%     start=[0 0 0];
%     stop=[width height 0];
%     CSX = AddExcitation(CSX,'excite',0,[1 1 0]);
%     CSX = SetExcitationWeight(CSX,'excite','modeFile_E.csv');
%     CSX = AddBox(CSX,'excite',0 ,start,stop);
%
% See also AddExcitation, InitCSX, DefineRectGrid
%
% CSXCAD matlab interface
% -----------------------
% Author: (2023-2025) Gadi Lahav - gadi@rfwithcare.com

if ~isfield(CSX,'Properties')
    error('CSXCAD::SetExcitationFile: no properties not found');
end
if ~isfield(CSX.Properties,'Excitation')
    error('CSXCAD::SetExcitationFile: no excitation properties found');
end

pos=0;
for n=1:numel(CSX.Properties.Excitation)
   if  strcmp(CSX.Properties.Excitation{n}.ATTRIBUTE.Name, name)
       pos=n;
   end
end

if (pos==0)
    error('CSXCAD::SetExcitationFile: property not found');
    return;
end

CSX.Properties.Excitation{pos}.ATTRIBUTE.ModeFileName = fileName;