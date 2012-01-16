function [isOct] = isOctave()
% function [isOct] = isOctave()
%
% function to test if matlab or octave is used
%
% openEMS matlab/octave interface
% -----------------------
% author: Thorsten Liebig (2011)

isOct = exist('OCTAVE_VERSION','builtin') ~= 0;
