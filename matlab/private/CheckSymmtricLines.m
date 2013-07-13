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

% check all lines for symmetry
for n=1:NP/2
    if (abs((center-lines(n))-(lines(end-n+1)-center)) > range*tolerance/NP)
        return;
    end
end

% check central point to be symmetry-center
if (mod(NP,2))
    if (abs(lines((NP+1)/2)-center) > range*tolerance/NP)
        return;
    end
end

% if all checks pass, return true
result = true;
