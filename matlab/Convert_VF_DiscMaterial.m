function Convert_VF_DiscMaterial(raw_filesuffix, mat_db_file, out_filename, varargin)
% function Convert_VF_DiscMaterial(raw_filesuffix, mat_db_file, out_filename, varargin)
%
% function to convert the virtual family raw voxel data to a disc material
% property required by CSXCAD/openEMS
%
% Note: The conversion is (currently) done, converting the broad-band data
% into a relative permittivity and conductivity for a given frequency of
% interest. Thus the converted model is only valid within a small frequency
% range around the used conversion frequency.
%
% required arguments:
%   raw_filesuffix:     suffix for the virtual family body model files
%                       the files:
%                       <raw_filesuffix>.txt and <raw_filesuffix>.raw
%                       must be found!
%                       example: '/tmp/Ella_26y_V2_1mm'
%   mat_db_file:        tissue database file (incl. full path if necessary)
%                       example: '/tmp/DB_h5_20120711_SEMCADv14.8.h5'
%   out_filename:       outfile name, e.g. 'Ella_298MHz.h5'
%
% variable arguments (key/value):
%   'Frequency':        specifiy the frequency of interest (required!)
%   'Center':           0/1 make the model centered around (0,0,0)
%                       (default is off)
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
% example:
% Convert_VF_DiscMaterial('/tmp/Ella_26y_V2_1mm', ...
%                         '/tmp/DB_h5_20120711_SEMCADv14.8.h5', ...
%                         'Ella_centered_298MHz.h5', ...
%                         'Frequency', 2.4e9);
%
% (c) 2013 Thorsten Liebig

if exist(out_filename,'file')
    disp(['requested model file: "' out_filename '" already exists, skipping']);
    return
end

%% default settings
center_Model = 0; % make the model centered around (0,0,0)
freq = nan; % frequency of interest
range = {[],[],[]};

% internal
range_used = 0;

for n=1:2:numel(varargin)
    if (strcmp(varargin{n},'Frequency'))
        freq = varargin{n+1};
    elseif (strcmp(varargin{n},'Center'))
        center_Model = varargin{n+1};
    elseif (strcmp(varargin{n},'Range'))
        range = varargin{n+1};
        range_used = 1;
    else
        warning('CSXCAD:Convert_VF_DiscMaterial',['unknown argument: ' varagin{n}]);
    end
end

if (isnan(freq))
    error('CSXCAD:Convert_VF_DiscMaterial','a frequency of interest must be specified');
end

%% end of settings (do not edit below)
physical_constants
w = 2*pi*freq;

%%
disp(['reading raw body specs: ' raw_filesuffix '.txt']);
fid = fopen([raw_filesuffix '.txt']);
material_list = textscan(fid,'%d	%f	%f	%f	%s');

frewind(fid)
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

disp('Body model mesh range (original):');
disp(['x: ' num2str(x(1)) ' --> ' num2str(x(end)) ', width: ' num2str(max(x)-min(x))]);
disp(['y: ' num2str(y(1)) ' --> ' num2str(y(end)) ', width: ' num2str(max(y)-min(y))]);
disp(['z: ' num2str(z(1)) ' --> ' num2str(z(end)) ', width: ' num2str(max(z)-min(z))]);

%% map to range
if (isempty(range{1}))
    x_idx = 1:nx;
else
    x_idx = find(x>=range{1}(1) & x<=range{1}(2));
end

if (isempty(range{2}))
    y_idx = 1:ny;
else
    y_idx = find(y>=range{2}(1) & y<=range{2}(2));
end

if (isempty(range{3}))
    z_idx = 1:nz;
else
    z_idx = find(z>=range{3}(1) & z<=range{3}(2));
end

tic
disp(['reading raw body data: ' raw_filesuffix '.raw']);
fid = fopen([raw_filesuffix '.raw']);

skip=(z(z_idx(1))-z(1))/dz*nx*ny;
fseek(fid,skip,'bof');

% read in line by line to save memory
for n=1:length(z_idx)
    data_plane = reshape(fread(fid,nx*ny),nx,ny);
    mat(1:numel(x_idx),1:numel(y_idx),n) = uint8(data_plane(x_idx,y_idx));
end
fclose(fid);

% resize to range
x = x(x_idx);x=[x x(end)+dx];
y = y(y_idx);y=[y y(end)+dx];
z = z(z_idx);z=[z z(end)+dx];

if (range_used)
    disp('Body model mesh range (new):');
    disp(['x: ' num2str(x(1)) ' --> ' num2str(x(end)) ', width: ' num2str(max(x)-min(x))]);
    disp(['y: ' num2str(y(1)) ' --> ' num2str(y(end)) ', width: ' num2str(max(y)-min(y))]);
    disp(['z: ' num2str(z(1)) ' --> ' num2str(z(end)) ', width: ' num2str(max(z)-min(z))]);
end

nx = numel(x_idx);
ny = numel(y_idx);
nz = numel(z_idx);

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

disp(['Creating DiscMaterial file: ' out_filename]);
CreateDiscMaterial(out_filename, mat, mat_db, mesh)
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
