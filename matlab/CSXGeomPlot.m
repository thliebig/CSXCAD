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

if isunix
    AppCSXCAD_bin = searchBinary('AppCSXCAD.sh', ...
        {[dir filesep '..' filesep '..' filesep 'AppCSXCAD' filesep], ...
         [dir filesep '..' filesep '..' filesep '..' filesep 'bin' filesep]});
else % assume windows
    AppCSXCAD_bin = searchBinary('AppCSXCAD.exe',[dir filesep '..' filesep]);
end

command = [AppCSXCAD_bin ' --disableEdit ' args_string ' ' CSX_filename];
disp( ['invoking AppCSXCAD, exit to continue script...'] );
if isOctave()
    fflush(stdout);
end
system(command);
