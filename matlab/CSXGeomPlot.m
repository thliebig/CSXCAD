function CSXGeomPlot(CSX_filename, args_string)
% function CSXGeomPlot(CSX_filename<, args_string>)
%
% Show the geometry stored in the CSX file using AppCSXCAD
%
% Optional AppCSXCAD arguments (args_string):
%   '--RenderDiscMaterial', enable material rendering
%
%  exports:
%   '--export-polydata-vtk=<path-for-export>'
%   '--export-STL=<path-for-export>'
%
% See also InitCSX, DefineRectGrid
%
% CSXCAD matlab interface
% -----------------------
% author: Thorsten Liebig

if nargin < 1
    error 'specify the xml file to open'
end

if nargin < 2
    args_string = '';
end

filename = mfilename('fullpath');
dir = fileparts( filename );
AppCSXCAD_Path = [dir filesep '../../AppCSXCAD' filesep];
   
if (~exist(AppCSXCAD_Path,'dir'))
    AppCSXCAD_Path = [dir filesep '..' filesep];
end

if isunix
	AppCSXCAD_Path = [AppCSXCAD_Path 'AppCSXCAD.sh'];
else
	AppCSXCAD_Path = [AppCSXCAD_Path 'AppCSXCAD'];
end

command = [AppCSXCAD_Path ' --disableEdit ' args_string ' ' CSX_filename];
disp( ['invoking AppCSXCAD, exit to continue script...'] );
if isOctave()
    fflush(stdout);
end
system(command);
