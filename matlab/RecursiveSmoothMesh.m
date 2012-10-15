function resultMeshPoints = RecursiveSmoothMesh(fixedMeshPoints, max_res, ratio, varargin)
% function resultMeshPoints = RecursiveSmoothMesh(fixedMeshPoints, max_res, ratio, varargin)
%
% RecursiveSmoothMesh: This function tries to generate an optimal mesh
%  between the given fixedMeshPoints. The space between the fixed points is
%  filled with the largest possible mesh step size considdering mesh max
%  resolution and mesh increase/decrease ratio.
%
% Algorithm: Mesh creation is done via a simple try and error approach:
%  The next mesh point could be between lastDistance*ratio and
%  lastDistance/ratio away from the last mesh point. If a fixed mesh point
%  exist in this distance, this point is used. If there is a fixed mesh
%  point below this distance, the last (determined) mesh point is wrong, go
%  back and try with a mesh point a few mm below. (If that is not possible
%  because the distance to the mesh point behind the last point is to low,
%  also update this mesh point).
%  The algorithm seems to work very well, except if the ratio is to low.
%  At a value of 1.2, the calculation takes a very long time.
%
%
% Parameter:    fixedMeshPoints:    List containing points at which a mesh
%                                    line should appear
%               max_res:            Maximum distance between two mesh lines
%               ratio:              Maximum grading ratio between two
%                                    neighbour mesh lines
%
% optional variable arguments ('key', value):
%      CheckMesh:          Do a final mesh check (default is true)
%      allowed_max_ratio:  allow only a given max. grading ratio
%                           (default --> ratio*1.25)
%
% Returns:      resultMeshPoints:   List containing the positions of all
%                                     mesh lines. If a empty list is
%                                     returned, no resolution could be
%                                     found.
%
% CSXCAD matlab interface
% -----------------------
% Author: Florian Pfanner
% Date: 28.09.2012


if (nargin < 3)
    ratio = 1.4;
end

if (ratio < 1.2)
    warning('ratio must be > 1.2, set it to 1.2');
    ratio = 1.2;
end

check_mesh = true;
max_ratio = ratio*1.25;

for vn=1:2:numel(varargin)
    if (strcmpi(varargin{vn},'CheckMesh'))
        check_mesh = varargin{vn+1};
    end
    if (strcmpi(varargin{vn},'allowed_max_ratio'))
        max_ratio = varargin{vn+1};
    end
end

fixedMeshPoints = sort(unique(fixedMeshPoints(:)'));
if (length(fixedMeshPoints) < 2)
    error('Not enoughts points to create mesh!');
end


% special behaviour to generate a symetric mesh (If the fixedMeshPoints are
% given in a symetric way.)
symMesh = CheckSymmtricLines(fixedMeshPoints);
if (symMesh)
    nPoints = length(fixedMeshPoints);

    symMeshEven = mod(nPoints,2)==0;

    if (symMeshEven)
        center = mean(fixedMeshPoints);
        if (center-fixedMeshPoints(nPoints/2) > 0.5*max_res)
            fixedMeshPoints = [fixedMeshPoints(1:nPoints/2) center];
        else
            % this is not working!!!
            % fixedMeshPoints = fixedMeshPoints(1:nPoints/2+1);
            fixedMeshPoints = [fixedMeshPoints(1:nPoints/2) center];
        end
    else
        % For symetric mesh, only the first half of the fixed mesh points is
        % needed. But add center of the symetric mesh to the fixed mesh points
        % to ensure the center is included.
        fixedMeshPoints = fixedMeshPoints(1:(nPoints+1)/2);
    end
end


minDistance = min(fixedMeshPoints(2:end)-fixedMeshPoints(1:end-1));

% two cases:
%  * minDistance is smaller than max_res
%       -> try spaces for the first mesh between max_res and minDistance/2
%  * max_res is smaller than minDistance
%       -> try spaces for the first mesh between max_res and max_res/10
if (minDistance < max_res)
    trySpaces = linspace(max_res, minDistance/2, 10);
else
    trySpaces = linspace(max_res, max_res/10, 10);
end
for k=1:length(trySpaces)
    [resultMeshPoints, success] = recursiveMeshSearch(fixedMeshPoints(1), ...
        trySpaces(k), fixedMeshPoints(2:end), max_res, ratio);
    if (success)
        resultMeshPoints = [fixedMeshPoints(1) resultMeshPoints];
        break;
    end
end


if (symMesh)
    resultMeshPoints = [resultMeshPoints(1:end-1) max(resultMeshPoints)*2-resultMeshPoints(end:-1:1)];
end

% check result
if (check_mesh)
    CheckMesh(resultMeshPoints,0,max_res,ratio,0);
end

end % main function

function [meshPoints, success] = recursiveMeshSearch(lastMeshPoint, ...
    lastSpace, fixedMeshPoints, max_res, ratio)

nextMeshPointMax = lastMeshPoint + min(lastSpace*ratio, max_res);
nextMeshPointMin = lastMeshPoint + lastSpace/ratio;

% check if below ratio is a fixed mesh point -> abbort
if (fixedMeshPoints(1) < nextMeshPointMin)
    meshPoints = [];
    success = false;
    return;
end

% check if in range of ratio is a fixed mesh point:
if (fixedMeshPoints(1) >= nextMeshPointMin && fixedMeshPoints(1) <= nextMeshPointMax)
    % yes, use this fixed mesh point
    nextMeshPoint = fixedMeshPoints(1);
    if (length(fixedMeshPoints) > 1)
        [meshPointsCall, success] = recursiveMeshSearch(nextMeshPoint, ...
            nextMeshPoint-lastMeshPoint, fixedMeshPoints(2:end), ...
            max_res, ratio);
        if (success)    % backtracking was successful, return result
            meshPoints = [nextMeshPoint meshPointsCall];
        else            % no possible resulution found, last step has to be repeated!
            meshPoints = [];
        end
    else            % this was the last mesh point, finish!
        meshPoints = nextMeshPoint;
        success = true;
    end
    return;
end

% try to set next mesh point, begin with highest width
trySpace = linspace(nextMeshPointMax, nextMeshPointMin, 10);
for i=1:length(trySpace)
    [meshPointsCall, success] = recursiveMeshSearch(trySpace(i), ...
        trySpace(i)-lastMeshPoint, fixedMeshPoints, max_res, ...
        ratio);
    if (success)    % backtracking was successful, return
        meshPoints = [trySpace(i) meshPointsCall];
        return;
    end
end
% no sucessful points found, return false
meshPoints = [];
success = false;
end
