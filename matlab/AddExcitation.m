function CSX = AddExcitation(CSX, name, type, excite, varargin)
% function CSX = AddExcitation(CSX, name, type, excite, varargin)
%
% Creates an E-field or H-field excitation.
%
% CSX: CSX-struct created by InitCSX
% name: property name for the excitation
% type: 0=E-field soft excitation  1=E-field hard excitation
%       2=H-field soft excitation  3=H-field hard excitation
% excite: e.g. [2 0 0] for excitation of 2 V/m in x-direction
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
% See also SetExcitationWeight AddBox

if ~ischar(name)
    error('CSXCAD::AddExcitation: name must be a string');
end

if isfield(CSX.Properties,'Electrode')
    CSX.Properties.Electrode{end+1}.ATTRIBUTE.Name=name;    
else
	CSX.Properties.Electrode = {}; % create cell array
    CSX.Properties.Electrode{1}.ATTRIBUTE.Name=name;
end

CSX.Properties.Electrode{end}.Excitation.ATTRIBUTE.Type=type;
CSX.Properties.Electrode{end}.Excitation.ATTRIBUTE.Excit_X=excite(1);
CSX.Properties.Electrode{end}.Excitation.ATTRIBUTE.Excit_Y=excite(2);
CSX.Properties.Electrode{end}.Excitation.ATTRIBUTE.Excit_Z=excite(3);

% CSX.Properties.Electrode{end}.Weight.ATTRIBUTE = [];

for n=1:(nargin-4)/2
    CSX.Properties.Electrode{end}.ATTRIBUTE.(varargin{2*n-1})=varargin{2*n};
end