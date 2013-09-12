function [binary_location] = searchBinary(name,searchpath)
% [binary_location] = searchBinary(name,searchpath)
%
% Function to search for executable. If the executable isn't found
% in searchpath, look in environment search path.
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

binary_location = [searchpath name];

if (~exist(binary_location,'file'))

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
   
end
