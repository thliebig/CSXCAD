function CSX = AddExcitation(CSX, name, type, excite, varargin)
% function CSX = AddExcitation(CSX, name, type, excite, varargin)
%
% CSXCAD matlab interface
% -----------------------
% author: Thorsten Liebig

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