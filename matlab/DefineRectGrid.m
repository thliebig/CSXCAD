function CSX = DefineRectGrid(CSX, deltaUnit, mesh);
% function CSX = DefineRectGrid(CSX, deltaUnit, mesh);
%
% Create a rectiliniear grid.
% 
% example Cartesian mesh:
%     CSX = InitCSX();
%     mesh.x = SmoothMeshLines([0 a], 10);
%     mesh.y = SmoothMeshLines([0 b], 10);
%     mesh.z = SmoothMeshLines([0 length], 15);
%     CSX = DefineRectGrid(CSX, unit,mesh);
% 
% example Cylindrical mesh:
%     CSX = InitCSX('CoordSystem',1);
%     mesh.r = SmoothMeshLines([0 a], 10);
%     mesh.a = SmoothMeshLines([0 b], 10);
%     mesh.z = SmoothMeshLines([0 length], 15);
%     CSX = DefineRectGrid(CSX, unit,mesh);
% 
% See also InitCSX, SmoothMeshLines, SmoothMeshLines2
% 
% CSXCAD matlab interface
% -----------------------
% author: Thorsten Liebig

CSX.RectilinearGrid.ATTRIBUTE.DeltaUnit = deltaUnit;

if (isfield(mesh,'x'))
    CSX.RectilinearGrid.XLines = mesh.x;
elseif ( (isfield(mesh,'r')) && (CSX.ATTRIBUTE.CoordSystem==1))
    CSX.RectilinearGrid.XLines = mesh.r;
else
    error 'x/(r) direction not found'
end

if (isfield(mesh,'y'))
    CSX.RectilinearGrid.YLines = mesh.y;
elseif ((isfield(mesh,'a')) && (CSX.ATTRIBUTE.CoordSystem==1))
    CSX.RectilinearGrid.YLines = mesh.a;
else
    error 'y/(a) direction not found'
end

CSX.RectilinearGrid.ZLines = mesh.z;
