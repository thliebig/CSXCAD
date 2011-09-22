function CSX = AddMetal(CSX, name)
%function CSX = AddMetal(CSX, name)
%
% Add a metal property (PEC) to CSX with the given name.
% Remember to add at least one or more geometrical primitives to any
% property.
%
%   example:
%       CSX = AddMetal(CSX,'metal'); %create PEC with propName 'metal'
%       CSX = AddBox(CSX,'metal',10,[0 0 0],[100 100 200]); %assign box
%
% See also AddMaterial, AddExcitation, AddProbe, AddDump, AddBox
% 
% CSXCAD matlab interface
% -----------------------
% author: Thorsten Liebig

CSX = AddProperty(CSX, 'Metal', name);
