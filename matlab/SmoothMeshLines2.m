function lines = SmoothMeshLines2( lines, max_res, ratio )
%lines = SmoothMeshLines2( lines, max_res [, ratio] )
%
% Create smooth mesh lines.
%
% input:
%   lines:   1xn vector of (fixed) mesh lines
%   max_res: maximum distance between any two lines (e.g. lambda/10)
%   ratio:   (optional) default: 1.3
%
% output:
%   lines:   1xn vector of (smoothed) mesh lines
%
% example:
%   mesh.x = [0 100 2300 2400];
%   mesh.x = SmoothMeshLines2( mesh.x, 43 );
%
% todo:
%  - if gaps cannot be completely filled because of the ratio restriction,
%    the spacing is not optimal. SmoothRange() needs to be optimized.
%  - SmoothRange() has special handling if stop_taper is to long; can this
%    also happen for start_taper?
%
% CSXCAD matlab interface
% -----------------------
% (C) 2010 Sebastian Held <sebastian.held@uni-due.de>
% See also SmoothMeshLines

if (numel(lines)<2)
    return
end

if (nargin<3)
    ratio = 1.3;
end

%
%    |        |            |            |
%    | gap(1) |   gap(2)   | ...        |
%    |        |            |            |
%
% lines(1)  lines(2) ...             lines(end)

lines = unique(sort(lines));

for n=1:(numel(lines)-1)
    old_gap(n).start_res = -1;
    old_gap(n).stop_res  = -1;
    old_gap(n).lines     = [];
end

if numel(lines) == 2
    % special case
    addLines = SmoothRange( lines(1), lines(2), [], [], max_res, ratio );
    lines = sort(unique([lines addLines]));
    [EC pos] = CheckMesh(lines,0,max_res,ratio);
    return
end

while 1
    gap = calc_gaps();
    
    % determine gap to process
    index = find( [old_gap(2:end).start_res] ~= [gap(2:end).start_res], 1, 'first' ) + 1;
    if isempty(index)
        index = find( [old_gap(1:end-1).stop_res] ~= [gap(1:end-1).stop_res], 1, 'first' );
    end
    if isempty(index)
        break; % done
    end
    
    start_res = min( max_res, calc_start_res(index) );
    stop_res  = min( max_res, calc_stop_res(index) );
    
    % create new lines
    old_gap(index).lines = SmoothRange( lines(index), lines(index+1), start_res, stop_res, max_res, ratio );

    % remember gap setting
    old_gap(index).start_res = start_res;
    old_gap(index).stop_res  = stop_res;
    
    % debug
    %plot_lines
end

% merge lines
for n=1:numel(old_gap)
    lines = [lines old_gap(n).lines];
end
lines = sort(unique(lines));
CheckMesh(lines,0,max_res,ratio);


%% ------------------------------------------------------------------------
function gap = calc_gaps()
    temp_lines = lines;
    for n=1:numel(old_gap)
        temp_lines = [temp_lines old_gap(n).lines];
    end
    temp_lines = sort(unique(temp_lines));
    
    if numel(temp_lines) == 2
        gap(1).start_res = inf; % resolution not fixed
        gap(1).stop_res  = max_res;
        return
    end
    
    gap(1).start_res = inf; % resolution not fixed
    idx = interp1( temp_lines, 1:numel(temp_lines), lines(2), 'nearest' );
    gap(1).stop_res  = min( max_res, temp_lines(idx+1) - temp_lines(idx) );
    for n=2:numel(lines)-2
        idx = interp1( temp_lines, 1:numel(temp_lines), lines(n), 'nearest' );
        gap(n).start_res = min( max_res, temp_lines(idx) - temp_lines(idx-1) );
        idx = interp1( temp_lines, 1:numel(temp_lines), lines(n+1), 'nearest' );
        gap(n).stop_res  = min( max_res, temp_lines(idx+1) - temp_lines(idx) );
        gap(n).lines = old_gap(n).lines;
    end
    idx = interp1( temp_lines, 1:numel(temp_lines), lines(end-1), 'nearest' );
    gap(numel(lines)-1).start_res = min( max_res, temp_lines(idx) - temp_lines(idx-1) );
    gap(numel(lines)-1).stop_res  = inf; % resolution not fixed
end % calc_gaps()


%% ------------------------------------------------------------------------
function plot_lines
    temp_lines = lines;
    for n=1:numel(old_gap)
        temp_lines = [temp_lines old_gap(n).lines];
    end
    temp_lines = sort(unique(temp_lines));
    
    plot( temp_lines, ones(size(temp_lines)), 'r+' );
    hold on
    plot( lines, ones(size(lines)), 'bo' );
    hold off
end % plot_lines


%% ------------------------------------------------------------------------
function res = calc_start_res( pos )
    if (pos < 2) || (pos > numel(lines))
        res = [];
        return
    end
    temp_lines = lines;
    for n=1:numel(old_gap)
        temp_lines = [temp_lines old_gap(n).lines];
    end
    temp_lines = sort(unique(temp_lines));
    
    idx = interp1( temp_lines, 1:numel(temp_lines), lines(pos), 'nearest' );
    res = temp_lines(idx) - temp_lines(idx-1);
end % calc_res()


%% ------------------------------------------------------------------------
function res = calc_stop_res( pos )
    if (pos < 1) || (pos >= numel(lines)-1)
        res = [];
        return
    end
    temp_lines = lines;
    for n=1:numel(old_gap)
        temp_lines = [temp_lines old_gap(n).lines];
    end
    temp_lines = sort(unique(temp_lines));
    
    idx = interp1( temp_lines, 1:numel(temp_lines), lines(pos+1), 'nearest' );
    res = temp_lines(idx+1) - temp_lines(idx);
end % calc_res()


%% ------------------------------------------------------------------------
function lines = SmoothRange(start, stop, start_res, stop_res, max_res, ratio)

if (nargin<6)
    ratio = 1.3;
end

if isempty(start_res) && isempty(stop_res)
    % special case: fill entire range with max_res
    n1 = ceil( (stop-start) / max_res ) + 1;
    lines = linspace( start, stop, n1 );
    return
end

if isempty(start_res)
    % special case: taper from stop_res at stop to max_res at start (if
    % possible)
    taper = stop_res*ratio;
    stop_taper = stop;
    while taper*ratio < max_res
        stop_taper = [stop_taper(1)-taper stop_taper];
        taper = taper*ratio;
    end
    if stop_taper(1) > start
        n1 = ceil( (stop_taper(1)-start) / max_res ) + 1;
        stop_taper = [linspace(start,stop_taper(1),n1) stop_taper];
    end
    lines = sort(unique(stop_taper(stop_taper>=start)));
    return
end

if isempty(stop_res)
    % special case: taper from stop_res at stop to max_res at start (if
    % possible)
    taper = start_res*ratio;
    start_taper = start;
    while taper*ratio < max_res
        start_taper = [start_taper start_taper(end)+taper];
        taper = taper*ratio;
    end
    if start_taper(end) < stop
        n1 = ceil( (stop-start_taper(end)) / max_res ) + 1;
        start_taper = [start_taper linspace(start_taper(end),stop,n1)];
    end
    lines = sort(unique(start_taper(start_taper<=stop)));
    return
end

taper = start_res*ratio;
start_taper = start;
while (taper*ratio<max_res)
    start_taper = [start_taper start_taper(end)+taper];
    taper = taper*ratio;
end

taper = stop_res*ratio;
stop_taper = stop;
while (taper*ratio<max_res)
    stop_taper = [stop_taper(1)-taper stop_taper];
    taper = taper*ratio;
end
if any(stop_taper<=start)
    % not enough space for entire taper
    stop_taper = stop_taper(stop_taper>=start); % remove illegal values
    if numel(stop_taper) > 1
        stop_taper(1) = []; % likely to near to start
        stop_taper = [(start+stop_taper(1))/2 stop_taper]; % create a centered line
    end
end

while ~isempty(stop_taper) && ( (abs(stop_taper(1) - start_taper(end)) < max_res) || (stop_taper(1) < start_taper(end)) )
    
    diff_start = diff(start_taper);
    if isempty(diff_start)
        diff_start = 0;
    end
    diff_stop = diff(stop_taper);
    if isempty(diff_stop)
        diff_stop = 0;
    end
    
    if (diff_start(end)>diff_stop(1))
        start_taper = start_taper(1:end-1);
    else
        stop_taper = stop_taper(2:end);
    end
end

if isempty(stop_taper)
    lines = [];
    return
end

% fill remaining space with equidistant lines
numL = ceil((stop_taper(1) - start_taper(end))/max_res)+1;
lines = unique([start_taper linspace(start_taper(end),stop_taper(1),numL) stop_taper]);
end % SmoothRange()

end % main function
