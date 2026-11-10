function CSX = AddLossyMetal(CSX, name, conductivity, varargin)
%function CSX = AddLossyMetal(CSX, name, conductivity, varargin)
%
% Add a lossy (and optionally rough) metal property to CSX with the given
% name. Unlike a conducting sheet it is meant for ordinary 3D bodies: the
% solver applies the (rough) surface impedance to the faces of the body.
%
% arguments:
%   conductivity:   conductivity in S/m, e.g. 5.8e7
%
% optional (key/value pairs):
%   'RoughnessModel': 'none' (default) or 'gradient'
%   'Rq':             rms surface roughness in m, e.g. 1.8e-6
%   'Thickness':      thickness in m, only used for zero-thickness (2D)
%                     primitives
%
% example:
%
%     % rough copper, gradient model, Rq = 1.8um
%     CSX = AddLossyMetal(CSX,'copper',5.8e7,'RoughnessModel','gradient','Rq',1.8e-6);
%     % a 35um thick trace
%     CSX = AddBox(CSX,'copper',10,[0 -250 0],[10000 250 35]);
%
% See also AddMetal, AddConductingSheet, AddMaterial, AddBox
%
% CSXCAD matlab interface
% -----------------------

if (conductivity<=0)
    error('CSXCAD:AddLossyMetal','the conductivity must be positive');
end

args = {'Conductivity', conductivity};
for n=1:2:numel(varargin)
    key = varargin{n};
    val = varargin{n+1};
    switch lower(key)
        case 'roughnessmodel'
            if ~any(strcmpi(val, {'none','gradient'}))
                error('CSXCAD:AddLossyMetal','unknown roughness model "%s"', val);
            end
            args(end+1:end+2) = {'RoughnessModel', lower(val)};
        case 'rq'
            if (val<0)
                error('CSXCAD:AddLossyMetal','a negative Rq is invalid');
            end
            args(end+1:end+2) = {'Rq', val};
        case 'thickness'
            if (val<0)
                error('CSXCAD:AddLossyMetal','a negative thickness is invalid');
            end
            args(end+1:end+2) = {'Thickness', val};
        otherwise
            error('CSXCAD:AddLossyMetal','unknown argument "%s"', key);
    end
end

CSX = AddProperty(CSX, 'LossyMetal', name, args{:});
