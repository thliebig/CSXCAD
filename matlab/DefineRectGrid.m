function CSX = DefineRectGrid(CSX, deltaUnit, mesh);
% function CSX = DefineRectGrid(CSX, deltaUnit, mesh);
%
% CSXCAD matlab interface
% -----------------------
% author: Thorsten Liebig

CSX.RectilinearGrid.ATTRIBUTE.DeltaUnit = deltaUnit;
CSX.RectilinearGrid.XLines = mesh.x;
CSX.RectilinearGrid.YLines = mesh.y;
CSX.RectilinearGrid.ZLines = mesh.z;
