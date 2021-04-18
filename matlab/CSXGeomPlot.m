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
pathname = fileparts( filename );

if isunix
    AppCSXCAD_bin = searchBinary('AppCSXCAD.sh', ...
        {[pathname filesep '..' filesep '..' filesep 'AppCSXCAD' filesep], ...
         [pathname filesep '..' filesep '..' filesep '..' filesep 'bin' filesep]});
else % assume windows
    AppCSXCAD_bin = searchBinary('AppCSXCAD.exe',[pathname filesep '..' filesep]);
end

command = [AppCSXCAD_bin ' --disableEdit ' args_string ' ' CSX_filename];
disp( ['invoking AppCSXCAD, exit to continue script...'] );
if isOctave()
    fflush(stdout);
end

if ~isunix && isOctave() % assume Octave on windows
  old_qt_plugin_path=getenv('QT_PLUGIN_PATH');
  setenv('QT_PLUGIN_PATH',[pathname filesep '..' filesep 'qt5' filesep 'plugins']);
  system(command);
  setenv('QT_PLUGIN_PATH', old_qt_plugin_path);
else
  system(command);
end
