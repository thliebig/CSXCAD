function lines = SmoothMeshLines( lines, max_res, ratio, varargin)
%function lines = SmoothMeshLines( lines, max_res, ratio, varargin)
%
% create smooth mesh lines
% 
%   Warning: This function may not always produce a desired output.
% 
%   lines:      given fixed lines to create a smooth mesh in between
%   max_res:    desired max. resolution
%   ratio:      max. neighboring line-delta ratio, (optional, default is 1.3)
%
% optional variable arguments ('key', value)
%   recursive:          SmoothMeshLines a couple of times recursivly (default is 0)
%   CheckMesh:          Do a final mesh check (default is true)
%   allowed_max_ratio:  allow only a given max. grading ratio
%                           (default --> ratio*1.25)
%
% example:
%   % create a x-mesh with lines at 0, 50 and 200 an a desired mesh
%   resolution of 5
%   mesh.x = SmoothMeshLines([0 50 200],5,1.3);
%
% See also InitCSX, DefineRectGrid
% 
% CSXCAD matlab interface
% -----------------------
% author: Thorsten Liebig

if (numel(lines)<2)
    return
end

if (nargin<3)
    ratio = 1.3;
end

recursive = 0;
check_mesh = true;
max_ratio = ratio*1.25;

for vn=1:2:numel(varargin)
    if (strcmpi(varargin{vn},'recursive'))
        recursive = varargin{vn+1};
    end
    if (strcmpi(varargin{vn},'CheckMesh'))
        check_mesh = varargin{vn+1};
    end
    if (strcmpi(varargin{vn},'allowed_max_ratio'))
        max_ratio = varargin{vn+1};
    end
end

lines = unique(sort(lines));

diff_Lines = diff(lines);

index = find(diff_Lines>(1.001*max_res));

% for n=1:numel(diff_Lines)-1
%     if ( (diff_Lines(n+1)/diff_Lines(n) > ratio) )
%         index = [index n+1];
%     end
% end

index = unique(index);

addLines = [];

for n=1:numel(index)
    if (index(n)==1)
        start_res = max_res;
    else
        start_res = lines(index(n)) - lines(index(n)-1);
    end
    
    if ((index(n)+1)==numel(lines))
        stop_res = max_res;
    else
        stop_res = lines(index(n)+2) - lines(index(n)+1);
    end
    
    addLines = [addLines SmoothRange(lines(index(n)),lines(index(n)+1),start_res,stop_res,max_res,ratio)];
end


lines = unique(sort([lines addLines]));

addLines = [];
% relax ratio for test
ratio_relax = ratio + (ratio-1) * 1;

[EC pos E_type] = CheckMesh(lines,0,max_res,ratio_relax,1);
diff_Lines = diff(lines);

for n=1:EC
    if pos(n)>1
        start_res = diff_Lines(pos(n)-1);
    else
        start_res = diff_Lines(pos(n));
    end
    if pos(n) >= numel(diff_Lines)
        stop_res  = diff_Lines(end);
    else
        stop_res  = diff_Lines(pos(n)+1);
    end
    max_res_R = max([start_res stop_res])/2/ratio;
    addLines = [addLines SmoothRange(lines(pos(n)),lines(pos(n)+1),start_res,stop_res,max_res_R,ratio)];    
end

lines = unique(sort([lines addLines]));

for n=1:recursive
    old_numL = numel(lines);
    lines = SmoothMeshLines( lines, max_res, ratio, 0);
    if numel(lines) == old_numL
        return
    end
end

if (check_mesh)
    CheckMesh(lines,0,max_res,max_ratio,0);
end
