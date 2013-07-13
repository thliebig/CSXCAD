function CSX = AddDiscMaterial(CSX, name, varargin)
% function CSX = AddDiscMaterial(CSX, name, varargin)
%
% Add a discretized material model property to CSX with the given name.
% Discretized model has to be stored in an hdf5 file.
% Use Transform option to perfom some transformation actions.
%
% variable arguments (key/value)
%  'File' (mandatory): define the filename of the discrete material
%  'Scale':            scale the discrete material
%                      e.g. to your drawing units: 'Scale', 1/unit
%  'Transform':        Apply a transformation, see AddBox for more infos
%  'UseDBBackground':  set to 0, to use the properties background material
%                      instead of the database material with index 0 (default)
%
% examples:
% %add human body model
% CSX = AddDiscMaterial(CSX, 'Ella', 'Scale', 1/unit, ...
%   'Transform', {'Rotate_Z',pi/2,'Translate','300,300,500'}, ...
%   'File', 'model_file_name.h5' );
% start = [mesh.x(1)   mesh.y(1)   mesh.z(1)];
% stop  = [mesh.x(end) mesh.y(end) mesh.z(end)];
% CSX = AddBox(CSX,'Ella', 0 ,start,stop);
%
% See also AddBox, AddMetal, AddExcitation, AddProbe, AddDump 
%
% CSXCAD matlab interface
% -----------------------
% author: Thorsten Liebig

transform = [];

for n=1:2:(nargin-2)
    if (strcmp(varargin{n},'Transform'))
        transform = varargin([n:n+1]);
        varargin([n:n+1]) = [];
        break
    end
end

CSX = AddProperty(CSX, 'DiscMaterial', name, varargin{:});

if ~isempty(transform)
    for n=1:2:numel(transform{2})
        CSX.Properties.DiscMaterial{end}.Transformation.(transform{2}{n}).ATTRIBUTE.Argument=transform{2}{n+1};
    end
end
