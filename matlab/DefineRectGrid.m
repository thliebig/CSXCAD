function CSX = DefineRectGrid(CSX, deltaUnit, mesh);
% function CSX = DefineRectGrid(CSX, deltaUnit, mesh);
%
% Create a rectiliniear grid.
% 
% example:
%     CSX = InitCSX();
%     mesh.x = SmoothMeshLines([0 a], 10);
%     mesh.y = SmoothMeshLines([0 b], 10);
%     mesh.z = SmoothMeshLines([0 length], 15);
%     CSX = DefineRectGrid(CSX, unit,mesh);
% 
% See also InitCSX, SmoothMeshLines, SmoothMeshLines2
% 
% CSXCAD matlab interface
% -----------------------
% author: Thorsten Liebig

CSX.RectilinearGrid.ATTRIBUTE.DeltaUnit = deltaUnit;
CSX.RectilinearGrid.XLines = mesh.x;
CSX.RectilinearGrid.YLines = mesh.y;
CSX.RectilinearGrid.ZLines = mesh.z;
