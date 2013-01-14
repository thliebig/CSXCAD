function CreateDiscMaterial(filename, data, mat_db, mesh)
% function CreateDiscMaterial(filename, data, mat_db, mesh)
%
% Create the discrete material hdf5 file (version 2) usable by AddDiscMaterial
%
% Note: This function currently requires Matlab. Octave is missing the
% necessary hdf5 write functions.
%
% arguments:
%   filename:   hdf5 file to create (must not exist)
%   data:       voxel based index data, index as used in mat_db or 0 for
%               background material
%               example:
%
%   mat_db:     material database
%   mesh:       used voxel mesh
%
% example:
%   mat_db.epsR    = [1     3      4];    %relative permittivity
%   mat_db.kappa   = [0     0.2    0.4];  %electric conductivity (S/m)
%   mat_db.density = [0     1000   1010]; %material density (kg/m³)
%   mat_db.Name    = {'Air','mat1','mat2'};
%
%   data = [0 1 0; 2 2 2; 0 3 0];
%   mesh.x = [0    0.1 0.2];
%   mesh.y = [-0.1 0   0.1];
%   mesh.z = [0    0.4 0.8];
%
%   CreateDiscMaterial_v2('test_mat.h5', data, mat_db, mesh);
%
% See also AddDiscMaterial
%
% CSXCAD matlab interface
% -----------------------
% author: Thorsten Liebig (2013)


if isOctave
    error('CreateDiscMaterial currently does not support Octave, due to missing hdf5 functions!');
end

data_size = [numel(mesh.x) numel(mesh.y) numel(mesh.z)];

if (exist(filename,'file'))
    error(['file "' filename '" already exist. Delete/rename first!']);
end

h5create(filename, '/DiscData',data_size, 'Datatype', 'int32', 'ChunkSize',data_size, 'Deflate',9);
h5write(filename, '/DiscData', data - 1);

h5writeatt(filename, '/DiscData', 'DB_Size', int32(numel(mat_db.epsR)));

h5writeatt(filename, '/DiscData', 'epsR', mat_db.epsR);
h5writeatt(filename, '/DiscData', 'kappa', mat_db.kappa);
h5writeatt(filename, '/DiscData', 'density', mat_db.density);
h5writeatt(filename, '/DiscData', 'Name', strjoin(mat_db.Name,','));

h5create(filename, '/mesh/x', data_size(1));
h5write(filename, '/mesh/x', mesh.x);

h5create(filename, '/mesh/y', data_size(2));
h5write(filename, '/mesh/y', mesh.y);

h5create(filename, '/mesh/z', data_size(3));
h5write(filename, '/mesh/z', mesh.z);

h5writeatt(filename, '/', 'Version', 2);



function out = strjoin(names, delimitier)

out = names{1};
for n=2:numel(names)
    out = [out delimitier names{n}];
end
