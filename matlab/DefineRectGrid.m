function CSX = DefineRectGrid(CSX, deltaUnit, mesh)
% function CSX = DefineRectGrid(CSX, deltaUnit, mesh);
%
% Create a rectiliniear grid.
% 
% example Cartesian mesh:
%     CSX = InitCSX();
%     mesh.x = AutoSmoothMeshLines([0 a], 10);
%     mesh.y = AutoSmoothMeshLines([0 b], 10);
%     mesh.z = AutoSmoothMeshLines([0 length], 15);
%     CSX = DefineRectGrid(CSX, unit,mesh);
% 
% example Cylindrical mesh:
%     CSX = InitCSX('CoordSystem',1);
%     mesh.r = AutoSmoothMeshLines([0 a], 10);
%     mesh.a = AutoSmoothMeshLines([0 2*pi], pi/30);
%     mesh.z = AutoSmoothMeshLines([-length 0 length], 15);
%     CSX = DefineRectGrid(CSX, unit,mesh);
% 
% See also InitCSX, SmoothMesh, AutoSmoothMeshLines, DetectEdges
% 
% CSXCAD matlab interface
% -----------------------
% author: Thorsten Liebig

CSX.RectilinearGrid.ATTRIBUTE.DeltaUnit = deltaUnit;

if (isfield(CSX,'ATTRIBUTE'))
    if (isfield(CSX.ATTRIBUTE,'CoordSystem'))
        CSX.RectilinearGrid.ATTRIBUTE.CoordSystem = CSX.ATTRIBUTE.CoordSystem;
    end
end

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
