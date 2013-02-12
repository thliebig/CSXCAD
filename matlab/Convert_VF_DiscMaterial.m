function Convert_VF_DiscMaterial
% Script to convert the virtual family raw voxel data to a disc material
% property required by CSXCAD/openEMS
%
% Note: The conversion is (currently) done, converting the broad-band data
% into a relative permittivity and conductivity for a given frequency of
% interest. Thus the converted model is only valid within a small frequency
% range around the used conversion frequency.
%
% Requirements:
%   - Matlab, Octave is currently not supported due to missing hdf5 write
%   functions
%   - ~6GB of RAM to convert the largest voxel model
%   - Virtual Family voxel models
%       e.g. Duke_34y_V5_1mm.raw and .txt
%       See http://www.itis.ethz.ch/itis-for-health/virtual-population/overview/
%   - Virtual Family tissue database
%       e.g. DB_h5_20120711_SEMCADv14.8.h5
%       Download from: http://www.itis.ethz.ch/assets/Downloads/TissueDb/DBh5_20120711_SEMCADv14.8.zip
%
% (c) 2013 Thorsten Liebig

close all
clear
clc

%% settings (edit below)

center_Model = 1;

% name of the voxel model
% name = 'Billie_11y_V2_1mm';
% name = 'Duke_34y_V5_1mm'; %the 0.5mm requires large amounts of memory ~3GB, maybe use 1mm?
% name = 'Ella_26y_V2_1mm'; %the 0.5mm requires large amounts of memory ~3GB, maybe use 1mm?
% name = 'Thelonious_6y_V6_1mm';

% root path to virtual family models with each model in its own folder
VF_model_root_path = '<path/to/model/root>';

% tissue database file (incl. full path if necessary)
mat_db_file = 'DB_h5_20120711_SEMCADv14.8.h5';

% frequency of interest
freq = 298e6;

%% end of settings (do not edit below)

file_name = name;

if (center_Model>0)
    file_name = [file_name '_centered_'];
end

file_name = [file_name int2str(freq) '.h5'];

physical_constants
w = 2*pi*freq;

%%
fid = fopen([VF_model_root_path filesep name filesep name '.txt']);
material_list = textscan(fid,'%d	%f	%f	%f	%s');

frewind(fid)
line = [];
feof(fid);
while ~feof(fid)
    line = fgetl(fid);
    if strcmp(line,'Grid extent (number of cells)')
        n_cells = textscan(fid,'n%c	%f');
    end
    if strcmp(line,'Spatial steps [m]')
        d_cells = textscan(fid,'d%c	%f');
    end
end

fclose(fid);

nx = n_cells{2}(1);
ny = n_cells{2}(2);
nz = n_cells{2}(3);

disp(['body model contains ' num2str(nx*ny*nz) ' voxels']);

dx = d_cells{2}(1);
dy = d_cells{2}(2);
dz = d_cells{2}(3);

tic
disp('reading raw body data...');
fid = fopen([VF_model_root_path filesep name filesep name '.raw']);

% read in line by line to save memory
for n=1:nz
    data_plane = reshape(fread(fid,nx*ny),nx,ny);
    mat(:,:,n) = uint8(data_plane);
end
fclose(fid);

%
x = (0:nx)*dx;
y = (0:ny)*dy;
z = (0:nz)*dz;

if (center_Model>0)
    disp('centering model...');
    x = x - x(1) - (max(x)-min(x))/2;
    y = y - y(1) - (max(y)-min(y))/2;
    z = z - z(1) - (max(z)-min(z))/2;
end

disp(['Width (x) of the model: ' num2str(max(x)-min(x))])
disp(['Height (y) of the model: ' num2str(max(y)-min(y))])
disp(['Length (z) of the model: ' num2str(max(z)-min(z))])

mesh.x=x;
mesh.y=y;
mesh.z=z;

toc

%%
disp(['reading/analysing material database: ' mat_db_file]);

mat_db_info = h5info(mat_db_file);

%%
% there is no material in the list for number 0
mat_db.epsR(1)   = 1;
mat_db.kappa(1)  = 0;
mat_db.density(1) = 0;
mat_db.Name{1} = 'Background';

for n=1:numel(material_list{end})
    mat_para = GetColeData(mat_db_info, material_list{end}{n});
    eps_colo_cole = mat_para.ef + mat_para.sig/(1j*w*EPS0);
    if (mat_para.del1>0)
        eps_colo_cole = eps_colo_cole + mat_para.del1/(1 + (1j*w*mat_para.tau1*1e-12)^(1-mat_para.alf1));
    end
    if (mat_para.del2>0)
        eps_colo_cole = eps_colo_cole + mat_para.del2/(1 + (1j*w*mat_para.tau2*1e-9)^(1-mat_para.alf2));
    end
    if (mat_para.del3>0)
        eps_colo_cole = eps_colo_cole + mat_para.del3/(1 + (1j*w*mat_para.tau3*1e-6)^(1-mat_para.alf3));
    end
    if (mat_para.del4>0)
        eps_colo_cole = eps_colo_cole + mat_para.del4/(1 + (1j*w*mat_para.tau4*1e-3)^(1-mat_para.alf4));
    end
    mat_db.epsR(n+1)   = real(eps_colo_cole);
    mat_db.kappa(n+1)  = -1*imag(eps_colo_cole)*w*EPS0;
    mat_db.density(n+1) = mat_para.Dens;
    mat_db.Name{n+1} = mat_para.Name;
end
toc
%%

disp(['Creating DiscMaterial file: ' file_name]);
CreateDiscMaterial(file_name, mat, mat_db, mesh)
toc

disp('done..');

end

function [mat_para] = GetColeData(mat_db, mat_name)

% search for mat_name in all "AlternNames"

N_grp = numel(mat_db.Groups(1).Groups);

mat_name = regexp(mat_name, '/','split');
mat_name = mat_name{end};

for n=1:N_grp
    found = false;
    name = regexp(mat_db.Groups(1).Groups(n).Name, '/','split');
    alt_names = [name(end) regexp(mat_db.Groups(1).Groups(n).Attributes(end).Value, '/','split')];

    alt_names = regexprep(alt_names,' ','_');

    if (sum(strcmpi(alt_names, mat_name))>0)
        % we found it, break
        found = true;
        break;
    end
end

if (found==false)
    error(['property "' mat_name '" not found']);
end

for a = 1:numel(mat_db.Groups(1).Groups(n).Attributes)
    mat_para.(mat_db.Groups(1).Groups(n).Attributes(a).Name) = mat_db.Groups(1).Groups(n).Attributes(a).Value;
end

end
