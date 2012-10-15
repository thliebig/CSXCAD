function [ lines ] = SmoothRange(start, stop, start_res, stop_res, max_res, ratio)
%function [ lines ] = SmoothRange(start, stop, start_res, stop_res, max_res, ratio)
%
%   internal function only, use SmoothMeshLines instead
%
% See also SmoothMeshLines
%
% CSXCAD matlab interface
% -----------------------
% author: Thorsten Liebig

if (nargin<6)
    ratio = 1.3;
end

taper = start_res*ratio;
start_taper = start;
while (taper<max_res)
    start_taper = [start_taper start_taper(end)+taper];
    taper = taper*ratio;
end

taper = stop_res*ratio;
stop_taper = stop;
while (taper<max_res)
    stop_taper = [stop_taper stop_taper(end)-taper];
    taper = taper*ratio;
end
stop_taper = sort(stop_taper);

while ( (abs(stop_taper(1) - start_taper(end)) < max_res) || (stop_taper(1) < start_taper(end)) )
    
    diff_start = diff(start_taper);
    if isempty(diff_start)
        diff_start = 0;
    end
    diff_stop = diff([stop_taper]);
    if isempty(diff_stop)
        diff_stop = 0;
    end
    
    if (diff_start(end)>diff_stop(1))
        start_taper = start_taper(1:end-1);
    else
        stop_taper = stop_taper(2:end);
    end
    
    if (numel(stop_taper)==0) || (numel(start_taper)==0)
        break
    end
end

if (numel(stop_taper)==0) || (numel(start_taper)==0)
    lines = unique([start_taper stop_taper]);
else
    numL = ceil((stop_taper(1) - start_taper(end))/max_res)+1;
    lines = unique([start_taper linspace(start_taper(end),stop_taper(1),numL) stop_taper]);
end