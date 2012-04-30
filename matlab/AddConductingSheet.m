function CSX = AddConductingSheet(CSX, name, conductivity, thickness)
%function CSX = AddConductingSheet(CSX, name, conductivity, thickness)
%
% Add a conducting sheet property to CSX with the given name.
% Remember to add at least one or more 2D!! geometrical primitives to this
% property.
%
%   example:
%       % create the conducting material peroperty, e.g. 40um thick copper
%       CSX = AddConductingSheet(CSX,'copper',56e6,40e-6);
%       % assign box the 2D box --> 40um thick sheet
%       CSX = AddBox(CSX,'copper',10,[0 -50 200],[1000 50 200]);
%
% See also AddMaterial, AddMetal, AddExcitation, AddBox
%
% CSXCAD matlab interface
% -----------------------
% author: Thorsten Liebig 2012

CSX = AddProperty(CSX, 'ConductingSheet', name,'Conductivity',conductivity,'Thickness',thickness);
