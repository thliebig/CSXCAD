function CSXGeomPlot(CSX_filename, args_string)
% function CSXGeomPlot(CSX_filename<, args_string>)
%
% Show the geometry stored in the CSX file using csxcad-view, the viewer of
% the CSXCAD python interface, or AppCSXCAD if csxcad-view is not installed
%
% Optional arguments (args_string), understood by both:
% - '--RenderDiscMaterial', enable material rendering
%
% exports:
% - '--export-polydata-vtk=<path-for-export>'
% - '--export-STL=<path-for-export>'
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
prefix = [pathname filesep '..' filesep '..' filesep '..' filesep];

% prefer the python viewer (csxcad-view), e.g. from the venv next to the install
if isunix
    viewer_bin = searchBinary('csxcad-view', {[prefix 'bin' filesep], [prefix 'venv' filesep 'bin' filesep]}, 0);
else % assume windows
    viewer_bin = searchBinary('csxcad-view.exe', [pathname filesep '..' filesep], 0);
end
if ~isempty(viewer_bin)
    disp( ['invoking csxcad-view, exit to continue script...'] );
    if isOctave()
        fflush(stdout);
    end
    system([viewer_bin ' --disableEdit ' args_string ' ' CSX_filename]);
    return
end

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
