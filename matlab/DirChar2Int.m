function n = DirChar2Int(dir_char)
% function n = DirChar2Int(dir_char)
%
% internal function to convert a character like 'x','y','z' into a numeric
% direction: 0..2!
% If input already is a numeric value from 0..2, it will just be copied!
% Everything else will raise an error!
%
% CSXCAD matlab interface
% -----------------------
% author: Thorsten Liebig (c) 2013

if (numel(dir_char)>1)
    error('CSXCAD:DirChar2Int','invalid normal direction')
end

if (ischar(dir_char))
    if (strcmp(dir_char,'x') || strcmp(dir_char,'r'))
        n = 0; return;
    elseif (strcmp(dir_char,'y') || strcmp(dir_char,'a'))
        n = 1; return;
    elseif strcmp(dir_char,'z')
        n = 2; return;
    else
        error('CSXCAD:DirChar2Int','invalid normal direction')
    end
elseif (isnumeric(dir_char) && ((dir_char==0) || (dir_char==1) || (dir_char==2)))
    n = dir_char;
else
    error('CSXCAD:DirChar2Int','invalid normal direction')
end
