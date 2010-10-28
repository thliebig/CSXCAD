function [EC pos] = CheckMesh(lines, min_res, max_res, ratio)
% function [EC pos] = CheckMesh(lines, min_res, max_res, ratio)
%
%   Check if mesh lines are valid
%
%   parameter:
%       min_res: minimal allowed mesh-diff
%       max_res: maximal allowed mesh-diff
%       ratio:   maximal allowed mesh-diff ratio
%
%   return:
%       EC:     error code (number of found errors)
%       pos:    line positions with error
%
% CSXCAD matlab interface
% -----------------------
% author: Thorsten Liebig

diff_lines = diff(lines);
EC = 0;

pos = [];
max_err = find(diff_lines>max_res);
if ~isempty(max_err)
    warning('CSXCAD:CheckMesh','found resolution larger than max_res');
    pos = [pos max_err];
    EC = EC + numel(pos);
end

min_err = find(diff_lines<min_res);
if ~isempty(max_err)
    warning('CSXCAD:CheckMesh','found resolution smaller than min_res');
    pos = [pos min_err];
    EC = EC + numel(pos);
end

for n=1:numel(diff_lines)-1
    if (diff_lines(n+1)/diff_lines(n) > ratio*1.01)
        str = ['lines: ' num2str(n) '@' num2str(lines(n)) '  ' num2str(n+1) '@' num2str(lines(n+1)) '  ' num2str(n+2) '@' num2str(lines(n+2))];
        warning('CSXCAD:CheckMesh', [str '\nfound resolution increase larger than ratio: ' num2str(diff_lines(n+1)/diff_lines(n)) ' > ' num2str(ratio) ]);
        pos = [pos n];
        EC = EC + 1;
    end
    if (diff_lines(n+1)/diff_lines(n) < 1/ratio/1.01)
        str = ['lines: ' num2str(n) '@' num2str(lines(n)) '  ' num2str(n+1) '@' num2str(lines(n+1)) '  ' num2str(n+2) '@' num2str(lines(n+2))];
        warning('CSXCAD:SmoothRange', [str '\nfound resolution decrease smaller than ratio: ' num2str(diff_lines(n+1)/diff_lines(n)) ' < 1/' num2str(ratio) '=' num2str(1/ratio) ]);
        pos = [pos n];
        EC = EC + 1;
    end
end
