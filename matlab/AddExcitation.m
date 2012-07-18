function CSX = AddExcitation(CSX, name, type, excite, varargin)
% function CSX = AddExcitation(CSX, name, type, excite, varargin)
%
% Creates an E-field or H-field excitation.
%
% CSX: CSX-struct created by InitCSX
% name: property name for the excitation
% type: 0=E-field soft excitation  1=E-field hard excitation
%       2=H-field soft excitation  3=H-field hard excitation
%       10=plane wave excitation
%
% excite: e.g. [2 0 0] for excitation of 2 V/m in x-direction
% 
% additional options for openEMS:
%   'Delay'  : setup an excitation time delay in seconds
%   'PropDir': direction of plane wave propagation (plane wave excite only)
%
% example:
%   CSX = AddExcitation( CSX, 'infDipole', 1, [1 0 0] );
%   start = [-dipole_length/2, 0, 0];
%   stop  = [+dipole_length/2, 0, 0];
%   CSX = AddBox( CSX, 'infDipole', 1, start, stop );
%
%
% CSXCAD matlab interface
% -----------------------
% author: Thorsten Liebig
%
% See also SetExcitationWeight, AddMetal, AddExcitation, AddProbe,
% AddDump, AddBox

CSX = AddProperty(CSX, 'Excitation', name, 'Type', type, 'Excite', excite, varargin{:});
