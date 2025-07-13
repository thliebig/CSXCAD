function [CSX] = AddDjordjevicSarkarMaterial(CSX, matName, varargin)
% function [CSX] = AddDjordjevicSarkarMaterial(CSX, matName, varargin)
%
% Add a wideband dielectric material to a CSX struct using a multi-term
% Debye fit of the Djordjevic–Sarkar model.
%
% Calculates Debye parameters from a single (eps_r, tanD) measurement via
% `calcDjordjevicSarkarApprox` and adds the material to the CSX struct.
%
% - CSX          : CSX struct to which the material will be added
% - matName      : String name of the material
%
% required name-value pairs:
% - 'fMeas'     : Measurement frequency [Hz]
% - 'epsRMeas'  : Relative permittivity ε_r at 'fMeas'
% - 'tandMeas'  : Loss tangent tan(δ) at 'fMeas'
% - 'f2'        : Upper corner frequency of the Djordjevic–Sarkar model [Hz]
%
% optional name-value pairs:
% - 'lowFreqEvalType': Low-frequency behavior:
%                      0 = use 'f1' (default), typical Djordjevic–Sarkar
%                      1 = use 'epsRdc'
% - 'f1'             : Lower corner frequency [Hz] (used if `lowFreqEvalType` = 0)
% - 'epsRdc'         : Permittivity at DC (used if `lowFreqEvalType` = 1)
% - 'sigmaDC'        : DC conductivity [S/m]
% - 'nTermsPerDec'   : Number of Debye terms per frequency decade
% - 'plotEn'         : Enable/Disable plots of the model
%
% output:
% - CSX : Updated CSX struct including the defined wideband dielectric material
%
% note:
% - Internally uses `calcDjordjevicSarkarApprox` to generate model parameters.
% - See `calcDjordjevicSarkarApprox` for detailed description of the model
%   fitting.
%
% example:
%
%     CSX = AddDjordjevicSarkarMaterial(CSX, 'MyMaterial', ...
%         'fMeas', 1e9, 'epsRMeas', 4.2, 'tandMeas', 0.02, ...
%         'f1', 1e6, 'f2', 200e9);
%
% See also: calcDjordjevicSarkarApprox, AddDebyeMaterial
%
% Version History:
% v1.0  2025-06-30  Tobias Ammann  Initial version

  p = inputParser();
  p.FunctionName = 'AddDjordjevicSarkarMaterial';

  p.addRequired('CSX',      @isstruct);
  p.addRequired('matName',  @ischar);

  % Required Parameters
  % Set as Parameters, checked manually too match openEMS name-value pair style
  p.addParameter('fMeas',    [], @isPositiveScalar); % Hz
  p.addParameter('epsRMeas', [], @isPositiveScalar);
  p.addParameter('tandMeas', [], @isNonNegScalar);
  p.addParameter('f2',       [], @isPositiveScalar); % Upper fit frequency, Hz

  % Optional Parameters
  p.addParameter('lowFreqEvalType', 0,   @isIntegerScalar);   % 0 = use f1, 1 = use epsRdc
  p.addParameter('f1',              nan, @isPositiveScalar);  % lowFreqEvalType = 0, Hz
  p.addParameter('epsRdc',          inf, @isPositiveScalar);  % lowFreqEvalType = 1
  p.addParameter('sigmaDC',         0,   @isNonNegScalar);    % Siemens
  p.addParameter('nTermsPerDec',    1,   @(x) isScalar(x) && x >= 1); % Number of Debye terms per decade
  p.addParameter('plotEn',          0,   @isIntegerScalar);   % Enable/Disable plots of the model

  % Parse and manually verify required parameters
  p.parse(CSX, matName, varargin{:});

  requiredParams = {'fMeas', 'epsRMeas', 'tandMeas', 'f2'};
  for i = 1:numel(requiredParams)
    param = requiredParams{i};
    if ismember(param, p.UsingDefaults)
      error('%s: Missing required parameter ''%s''.', p.FunctionName, param);
    end
  end

  if ((p.Results.lowFreqEvalType == 0) && ismember('f1', p.UsingDefaults))
    error(['%s: For ''lowFreqEvalType=0'' a value for f1 (Djordjevic Sarkar ',...
          'low corner frequency)must be specified.'], p.FunctionName);
  end

  if ((p.Results.lowFreqEvalType == 1) && ismember('epsRdc', p.UsingDefaults))
    error(['%s: For ''lowFreqEvalType=1'', a value for epsRdc (value of ',...
           'EpsilonR at DC) must be specified.'], p.FunctionName);
  end

   % Fit the model and receive Debye model parametes for openEMS
   paramDebye = CalcDjordjevicSarkarApprox(...
      'fMeas', p.Results.fMeas,...
      'epsRMeas', p.Results.epsRMeas,...
      'tandMeas', p.Results.tandMeas,...
      'f1', p.Results.f1,...
      'f2', p.Results.f2,...
      'lowFreqEvalType', p.Results.lowFreqEvalType,...
      'epsRdc', p.Results.epsRdc,...
      'sigmaDC', p.Results.sigmaDC,...
      'nTermsPerDec', p.Results.nTermsPerDec,...
      'plotEn', p.Results.plotEn);

      CSX = AddDebyeMaterial(CSX, matName);
      CSX = SetMaterialProperty(CSX, matName, ...
                               'Epsilon', paramDebye.epsInf, ... % Epsilon here is acutally EpsilonRinf
                               'Kappa', paramDebye.sigmaDC);

      for i = 1:length(paramDebye.wi)

        CSX = SetMaterialProperty(CSX, matName,...
          ['EpsilonDelta_', int2str(i)],     paramDebye.deltaEpsT(i),...
          ['EpsilonRelaxTime_', int2str(i)], 1/paramDebye.wi(i));

      end
end

% Validation functions for input argument checks
function val = isNonNegScalar(x)
    val = isnumeric(x) && isscalar(x) && (x >= 0);
end

function val = isPositiveScalar(x)
    val = isnumeric(x) && isscalar(x) && (x > 0);
end

function val = isIntegerScalar(x)
    val = isnumeric(x) && isscalar(x) && (round(x) == x);
end

function val = isScalar(x)
    val = isnumeric(x) && isscalar(x);
end
