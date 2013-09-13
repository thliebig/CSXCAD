function [binary_location] = searchBinary(name, searchpath, err_fail)
% [binary_location] = searchBinary(name, searchpath)
%
% Function to search for executable. If the executable isn't found
% in searchpath, look in environment search path.
%
% parameter:
%   name:         name of the binary to search
%   searchpath:   (list of) search paths
%   err_fail:     0/1, throw an error if binary is not found (default is 1)
%
% Output:
%  binary_location:
%     if found in searchpath:                full path of binary
%     if found in environment search path:   name of binary
%     if not found:                          empty string
%
%
% openEMS matlab/octave interface
% -----------------------
% (C) 2013 Stefan Mahr <dac922@gmx.de>

if (nargin<3)
    err_fail = 1;
end

if ischar(searchpath)
    searchpath = {searchpath};
end

% try all given search paths
for n=1:numel(searchpath)
    binary_location = [searchpath{n} name];
    if exist(binary_location, 'file')
        return
    end
end

% binary not found in given search path, try environment PATH

path1=path;
warn1=warning('off');
addpath(getenv('PATH'));
warning(warn1);

if (exist(name,'file'))
    binary_location = name;
else
    binary_location = '';
end

path(path1);

if ((err_fail~=0) && isempty(binary_location))
    error('CSXCAD:binary_location', [name ' binary not found!']);
end
