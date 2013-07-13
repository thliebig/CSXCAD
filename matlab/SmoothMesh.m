function [mesh] = SmoothMesh( mesh, max_res, ratio, varargin)
% function [mesh] = SmoothMesh( mesh, max_res, <ratio, varargin>)
%
%   Convienent function to create a smooth mesh in all directions.
%   Generate smooth mesh by choosing an appropriate algorithm in each direction.
%
%   Currently supported smoothing algorithm:
%       SmoothMeshLines, SmoothMeshLines2 and RecursiveSmoothMesh
%
%  arguments:
%   lines:      given fixed lines to create a smooth mesh in between
%   max_res:    scalar or vector of desired max. allowed resolution
%   ratio:      grading ratio: scalar or vector of desired neighboring
%                   line-delta ratio (optional, default is 1.5)
%                   - see also 'allowed_max_ratio' argument
%
%  variable arguments ('keyword',value):
%   algorithm:          define subset of tried algorihm, e.g. [1 3]
%   symmetric:          0/1 force symmetric mesh (default is input symmetry)
%   homogeneous:        0/1 force homogeneous mesh
%   allowed_min_res:    allow a given min resolution only
%   allowed_max_ratio:  allow only a given max. grading ratio
%                           (default --> ratio*1.25)
%   debug:              0/1 off/on
%
% example:
%     mesh.x = [-BoundBox 0 BoundBox];
%     mesh.y = [-BoundBox 0 BoundBox];
%     mesh.z = [0 BoundBox];
%     mesh = SmoothMesh(mesh, lambda/20/unit);
%     CSX = DefineRectGrid(CSX, unit, mesh);
%
% See also AutoSmoothMeshLines, InitCSX, DefineRectGrid, DetectEdges
%
% CSXCAD matlab interface
% -----------------------
% author: Thorsten Liebig (C) 2012

if (nargin<3)
    ratio = 1.5;
end

if (numel(max_res)==1)
    max_res = [max_res max_res max_res];
end
if (numel(ratio)==1)
    ratio = [ratio ratio ratio];
end
if (numel(max_res)~=1)
    max_res = [max_res max_res max_res];
end

if isfield(mesh,'x')
    mesh.x = AutoSmoothMeshLines(mesh.x, max_res(1), ratio(1), varargin{:});
elseif isfield(mesh,'r')
    mesh.r = AutoSmoothMeshLines(mesh.r, max_res(1), ratio(1), varargin{:});
else
    error 'x/(r) direction not found'
end

if isfield(mesh,'y')
    mesh.y = AutoSmoothMeshLines(mesh.y, max_res(2), ratio(2), varargin{:});
elseif isfield(mesh,'a')
    mesh.a = AutoSmoothMeshLines(mesh.a, max_res(2), ratio(2), varargin{:});
else
    error 'y/(a) direction not found'
end

if isfield(mesh,'z')
    mesh.z = AutoSmoothMeshLines(mesh.z, max_res(3), ratio(3), varargin{:});
else
    error 'z direction not found'
end
