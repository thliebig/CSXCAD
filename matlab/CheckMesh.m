function [EC pos E_type] = CheckMesh(lines, min_res, max_res, ratio, be_quiet)
% function [EC pos E_type] = CheckMesh(lines, min_res, max_res, ratio, be_quiet)
%
%   Check if mesh lines are valid
%
%   parameter:
%       min_res: minimal allowed mesh-diff
%       max_res: maximal allowed mesh-diff
%       ratio:   maximal allowed mesh-diff ratio
%       be_quiet: disable warnings
%
%   return:
%       EC:     error code (number of found errors)
%       pos:    line positions with error
%       E_type: error type
%
% CSXCAD matlab interface
% -----------------------
% author: Thorsten Liebig

if (nargin<5)
    be_quiet = 0;
end

diff_lines = diff(lines);
EC = 0;
E_type = [];

pos = [];
max_err = find(diff_lines>max_res);
if (~isempty(max_err) && ~be_quiet)
    warning('CSXCAD:CheckMesh','found resolution larger than max_res');
    pos = [pos max_err];
    EC = EC + numel(max_err);
    E_type = [E_type 1];
end

min_err = find(diff_lines<min_res);
if (~isempty(min_err) && ~be_quiet)
    warning('CSXCAD:CheckMesh','found resolution smaller than min_res');
    pos = [pos min_err];
    EC = EC + numel(min_err);
    E_type = [E_type 2];
end

for n=1:numel(diff_lines)-1
    if (diff_lines(n+1)/diff_lines(n) > ratio*1.01)
        str = ['lines: ' num2str(n) '@' num2str(lines(n)) '  ' num2str(n+1) '@' num2str(lines(n+1)) '  ' num2str(n+2) '@' num2str(lines(n+2))];
        if (~be_quiet)
            warning('CSXCAD:CheckMesh', [str '\nfound resolution increase larger than ratio: ' num2str(diff_lines(n+1)/diff_lines(n)) ' > ' num2str(ratio) ]);
        end
        pos = [pos n+1];
        EC = EC + 1;
        E_type = [E_type 3];
    end
    if (diff_lines(n+1)/diff_lines(n) < 1/ratio/1.01)
        str = ['lines: ' num2str(n) '@' num2str(lines(n)) '  ' num2str(n+1) '@' num2str(lines(n+1)) '  ' num2str(n+2) '@' num2str(lines(n+2))];
        if (~be_quiet)
            warning('CSXCAD:SmoothRange', [str '\nfound resolution decrease smaller than ratio: ' num2str(diff_lines(n+1)/diff_lines(n)) ' < 1/' num2str(ratio) '=' num2str(1/ratio) ]);
        end
        pos = [pos n];
        EC = EC + 1;
        E_type = [E_type 4];
    end
end
