function result = CheckSymmtricLines(lines)
% function result = CheckSymmtricLines(lines)
%
% check mesh lines for symmetry
%
% Note: make sure lines are sorted and unique
%
% CSXCAD matlab interface
% -----------------------
% author: Thorsten Liebig (C) 2012

result = false;

tolerance = 1e-10;
NP = numel(lines);
range = lines(end)-lines(1);
center = 0.5*(lines(end)+lines(1));

if (abs(mean(lines)-center) < range*tolerance/NP)
    result = true;
end
