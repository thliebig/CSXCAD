function WriteCSX(filename, CSX)
% function WriteCSX(filename, CSX)
%
% Write a CSX structure to an XML file.
%
% This is a convenience wrapper around struct_2_xml for writing CSXCAD
% structures created with InitCSX and the Add*/Define* functions.
%
% arguments:
%   filename  -- output file name, e.g. 'my_structure.xml'
%   CSX       -- CSXCAD structure as returned by InitCSX
%
% example:
%
%     CSX = InitCSX();
%     CSX = AddMetal(CSX, 'copper');
%     CSX = AddBox(CSX, 'copper', 10, [-1 -1 -1], [1 1 1]);
%     WriteCSX('my_structure.xml', CSX);
%
% See also InitCSX, DefineRectGrid, AddMetal, AddMaterial
%
% CSXCAD matlab interface
% -----------------------
% author: Thorsten Liebig

struct_2_xml(filename, CSX, 'ContinuousStructure');
