function str = vector2str(vec, acc)
% str = vector2str( vec [, acc] )
%
% internal function for CSXCAD
% 
% (c) 2012-2015 Thorsten Liebig <thorsten.liebig@gmx.de>
% (C) 2012 Sebastian Held <sebastian.held@gmx.de>

str = '';

if (numel(vec)==0)
    return
end

if (nargin<2)
    acc = 9;
end

% Octave < 3.8.0 has a bug in num2str() (only 64bit versions)
% see bug report https://savannah.gnu.org/bugs/index.php?36121
[isOct,version] = isOctave();
if (isOct && version.major <= 3 && version.minor < 8)
    % affected Octave versions
    for n = 1:numel(vec)
        str = [str sprintf('%.*g', acc, vec(n)) ','];
    end
else
    % Matlab and non affected Octave
    for n = 1:numel(vec)
        str = [str num2str(vec(n),acc) ','];
    end
end

% remove the last ','
str = str(1:end-1);
