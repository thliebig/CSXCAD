function results = AnalyseMesh(lines)
% function results = AnalyseMesh(lines)
%
%   Analyse a given mesh line vector
%
% output structure:
%   results.numLines:       number of lines
%   results.max_res:        max. resolution found
%   results.min_res:        min. resolution found
%   results.max_ratio:      max. grading ratio found
%   results.homogeneous:    true/false for homogeneous mesh
%   results.symmetric:      true/false for symmetric mesh
%
% CSXCAD matlab interface
% -----------------------
% author: Thorsten Liebig (C) 2012

results = [];

lines = sort(unique(lines));

if (numel(lines)<=1)
    warning('CSXCAD:AnalyseMesh', 'more than one line needed to analyse mesh');
end

diff_lines = diff(lines);

results.numLines = numel(lines);

results.max_res = max(diff_lines);
results.min_res = min(diff_lines);
if (results.max_res==results.min_res)
    results.homogeneous = 1;
else
    results.homogeneous = 0;
end

results.symmetric = CheckSymmtricLines(lines);

ratio_lines = diff_lines(1:end-1)./diff_lines(2:end);

results.max_ratio = max([ratio_lines 1./ratio_lines]);
