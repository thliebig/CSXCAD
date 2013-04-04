function [isOct,ver] = isOctave()
% [isOct,version] = isOctave()
%
% Function to test if matlab or octave is used.
%
% Output:
%  isOct: bool; true, if interpreter is Octave
%  version: struct
%    .major:   number; major version
%    .minor:   number; minor version
%    .release: number; release version
%
% example (Octave 3.6.1):
%   isOct = true
%   version.major = 3
%   version.minor = 6
%   version.release = 1
%
% example (Matlab 7.8.0.347 (R2009a)):
%   isOct = false
%   version.major = 7
%   version.minor = 8
%   version.release = 0
%
% openEMS matlab/octave interface
% -----------------------
% (C) 2011 Thorsten Liebig <thorsten.leibig@gmx.de>
% (C) 2012 Sebastian Held <sebastian.held@gmx.de>

isOct = exist('OCTAVE_VERSION','builtin') ~= 0;

if (isOct)
    ver_cell = strsplit( OCTAVE_VERSION, '.' );
else
    remain = version();
    ver_cell = {};
    while ~isempty(remain)
        [str, remain] = strtok(remain, '.');
        ver_cell{end+1} = str;
    end
end

ver = [];
ver.major   = str2double(ver_cell{1});
ver.minor   = str2double(ver_cell{2});
ver.release = str2double(ver_cell{3});
