function [paramDebye, paramSarkar] = calcDjordjevicSarkarApprox(varargin)
% calcDjordjevicSarkarApprox - Approximate Djordjevic–Sarkar model with multi-term Debye model
%
% Fits a wideband dielectric model using a single permittivity and loss tangent
% measurement at one frequency. The Djordjevic–Sarkar [1] model is calculated from
% these values and approximated using multiple Debye terms. The poles are placed
% logarithmically between f1 and f2. One term per decade is used by default.
%
% INPUT PARAMETERS (Name-Value pairs):
%   Required:
%     'fMeas'      - Frequency of the Measurement [Hz]
%     'epsRMeas'   - Relative permittivity ε_r at 'fMeas'
%     'tandMeas'   - Loss tangent tan(δ) at 'fMeas'
%     'f2'         - Upper corner frequency of the Djordjevic–Sarkar model [Hz]
%
%   Depending on 'lowFreqEvalType':
%     'lowFreqEvalType' - Low-frequency behavior:
%                         0 = use 'f1' (default), typical Djordjevic–Sarkar
%                         1 = use 'epsRdc'
%     'f1'              - Lower corner frequency [Hz], required if lowFreqEvalType = 0
%     'epsRdc'          - DC permittivity, required if lowFreqEvalType = 1
%
%   Optional:
%     'sigmaDC'      - DC conductivity [S/m], usally neglected, default: 0
%     'nTermsPerDec' - Debye poles per frequency decade, default: 1
%     'plotEn'       - Enable plotting (0 = off, 1 = on), default: 0
%
% OUTPUT:
%   paramDebye  - Struct with multi-term Debye model:
%       .epsInf     : Permittivity at infinite frequency
%       .deltaEpsT  : Array of Δε'_i values
%       .wi         : Array of angular pole frequencies ω_i
%       .sigmaDC    : DC conductivity [S/m]
%
%   paramSarkar - Struct with Djordjevic–Sarkar model parameters:
%       .epsInf     : High-frequency permittivity
%       .deltaEpsT  : Total permittivity change (eps_r,DC - eps_inf)
%       .m1         : log10(lower angular freq)
%       .m2         : log10(upper angular freq)
%       .sigmaDC    : DC conductivity [S/m]
%
% EXAMPLE:
%   [pDebye, pSarkar] = calcDjordjevicSarkarApprox( ...
%     'fMeas', 1e9, 'epsRMeas', 4.2, 'tandMeas', 0.02, ...
%     'f1', 1e6, 'f2', 200e9, 'plotEn', 1);
%
% NOTES:
%   - Either 'f1' or 'epsRdc' must be specified depending on 'lowFreqEvalType'.
%   - Debye model is matched to the imaginary part of the DS model, sampled at
%     logarithmically spaced frequencies.
%
% See also: AddDebyeMaterial, AddLorentzMaterial
%
% [1] Djordjevic, Antonije R., et al. "Wideband frequency-domain
%     characterization of FR-4 and time-domain causality." IEEE Transactions
%     on electromagnetic compatibility 43.4 (2001): 662-667.
%
% -------------------------------------------------------------------------
% author: Tobias Ammann (2025)
%
% Version:
% v1.0  2025-06-30  Tobias Ammann  Initial release

  p = inputParser();
  p.FunctionName = 'calcDjordjevicSarkarApprox';

  % Required Parameters
  % Set as Parameters, checked manually to match openEMS name-value pair style
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
  p.parse(varargin{:});

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

  fMeas = p.Results.fMeas;
  epsRMeas = p.Results.epsRMeas;
  tandMeas = p.Results.tandMeas;

  f2 = p.Results.f2;

  lowFreqEvalType = p.Results.lowFreqEvalType;
  f1 = p.Results.f1;
  epsRdc = p.Results.epsRdc;
  sigmaDC = p.Results.sigmaDC; % S/m
  nTermsPerDec = p.Results.nTermsPerDec;

  m1 = log10(f1*2*pi); % Corner frequencies in log scale
  m2 = log10(f2*2*pi);

  eps0 = 8.8541878128e-12; %F/m

  wMeasSarkar = 2*pi*fMeas;

  % ----------------------------------------------------------------------------
  % -
  % - Calculate Djordjevic-Sarkar parameters
  % -
  % ----------------------------------------------------------------------------

  w2 = 10^m2;

  if lowFreqEvalType == 0
    % Standard Djordjevic Sarkar. Low frequency behaviour defined by w1 = 10^(m1).
    % Lower corner frequency, No approximations necessary --> use exact formulas
    w1 = 10^m1;

    k = log(10)*(-tandMeas*epsRMeas - sigmaDC/(eps0*wMeasSarkar));
    k = k/arg((w2 + 1i*wMeasSarkar)/(w1 + 1i*wMeasSarkar));

    epsInfSarkar = epsRMeas - k*log10(abs((w2 + 1i*wMeasSarkar)/(w1 + 1i*wMeasSarkar)));
    deltaEpsTsarkar = k*(m2-m1);

  else
    % Alternative definition: Specify epsRdc (permittivity at DC)
    % calculate the lower corner frequency 'w1' from this value
    k = log(10)*(-tandMeas*epsRMeas - sigmaDC/(eps0*wMeasSarkar))/atan2(-w2,wMeasSarkar);
    epsInfSarkar = epsRMeas - k*log10(sqrt(w2^2 + wMeasSarkar^2)/wMeasSarkar);

    deltaEpsTsarkar = epsRdc - epsInfSarkar;
    m1 = m2 - deltaEpsTsarkar/k;
    w1 = 10^m1;

    if m1 < 0
      error('DjordjevicSarkar m1 < 1. Value for ''epsRdc'' is too high, choose a lower one');
    end
  end

  % Model equation
  epsRSarkarEq = @(x) epsInfSarkar + deltaEpsTsarkar/(m2-m1)*log10((w2+1i*x)./(w1+1i*x)) - 1i*sigmaDC./(x*eps0);

  % Output struct
  paramSarkar.epsInf = epsInfSarkar;
  paramSarkar.m1 = m1;
  paramSarkar.m2 = m2;
  paramSarkar.deltaEpsT = deltaEpsTsarkar;
  paramSarkar.sigmaDC = sigmaDC;

  % ----------------------------------------------------------------------------
  % -
  % - Calculate Multi-Term Debye (approximation)
  % -
  % ----------------------------------------------------------------------------

  % Debye fist and last pole
  mMin = m1;
  mMax = m2;

  %mi = mMin:1/nTermsPerDec:mMax;
  %nTerms = length(mi);

  % Debye pole locations - nTermsPerDec evenly spaced in log-domain
  nTerms = ceil((mMax - mMin) * nTermsPerDec) + 1;
  mi = linspace(mMin, mMax, nTerms);
  wi = 10.^(mi.');

  % Pick frequencies to use for fit. Let´s call call them measured frequencies
  % because they are 'measured' samples of the DjordjevicSarkar model
  % Choose in a way that the imaginary part oscillates around the value of the Sarkar model
  msp = 10^((log10(wi(2)) - log10(wi(1)))/4);
  wMeasDebye = wi*msp; % Measured frequencies to use for fit

  % Determine deltaEpsilonTick_i from a known imaginary value
  A = 1./(wi.'./wMeasDebye + wMeasDebye./wi.');
  epsTTdebye = -imag(epsRSarkarEq(wMeasDebye));
  deltaEpsTdebye = A\epsTTdebye; % Solve linear equation system

  wx = wMeasSarkar;
  sumDebyeT = sum(deltaEpsTdebye./(1 + wx^2./wi.^2), 1); % real part
  epsInfDebye = epsInfSarkar + deltaEpsTsarkar/(m2-m1)*log10(abs((w2+1i*wx)/(w1+1i*wx))) - sumDebyeT;

  % Output struct
  paramDebye.epsInf = epsInfDebye;
  paramDebye.deltaEpsT = deltaEpsTdebye;
  paramDebye.wi = wi;
  paramDebye.sigmaDC = sigmaDC;

  % ----------------------------------------------------------------------------
  % -
  % - Visualization: Evaluate DjordjevicSarkar and Debye Models
  % -
  % ----------------------------------------------------------------------------
  if p.Results.plotEn

    % Generate frequency vector based on input corner frequencies
    mFreqMin = floor(log10(10^(m1-1)/2/pi));
    mFreqMax = ceil(log10(10^(m2+2)/2/pi));

    if mFreqMin < 10; mFreqMin = 1; end
    f = logspace(mFreqMin, mFreqMax, 1000);
    w = 2*pi*f;

    % --------------------------------------------------------------------------

    % Evaluate Djordjevic-Sarkar model (exact)
    epsRSarkar = epsRSarkarEq(w);
    tandSarkar = -imag(epsRSarkar)./real(epsRSarkar);

    % Approximations
    %epsRSarkarTapprox = epsInfSarkar + deltaEpsTsarkar/(m2-m1)*log10(w2./w);
    %epsRSarkarTTapprox = -deltaEpsTsarkar/(m2-m1)*(-pi/2)/log(10); % - 1i*sigmaDC./(w*eps0);

    % --------------------------------------------------------------------------

    % Evaluate Multi-Term Debye (approximation)
    sumDebye = sum(deltaEpsTdebye./(1 + 1i*w./wi), 1);
    epsRDebye = epsInfDebye + sumDebye - 1i*sigmaDC./(w*eps0);
    tandDebye = -imag(epsRDebye)./real(epsRDebye);

    % --------------------------------------------------------------------------
    % -
    % - Plots / Logarithmic frequency axis
    % -
    % --------------------------------------------------------------------------

    screenSize = get(0, 'screensize');
    figPos = [screenSize(3)/2, screenSize(4)/2, 800, 600];
    figLogPos = figPos + [-figPos(3), -figPos(4)/2, 0, 0] ;
    figLinPos = figPos + [ 0,         -figPos(4)/2, 0, 0] ;

    legendCell = {'Djordjevic-Sarkar (exact)', 'Multi-Term Debye (fit)', 'Measured/input data'};

    figLog = figure('Position', figLogPos);

    subplot(2,1,1);
    semilogx(f, real(epsRSarkar)); hold on;
    semilogx(f, real(epsRDebye));
    semilogx(fMeas, epsRMeas, 'ro');

    grid on;
    ylabel('\epsilon''');
    xlabel('Frequency / Hz');
    legend(legendCell);
    title(sprintf(['Wideband Dielectric Model: \\epsilon_r = %.1f, tan_\\delta =%.3f,'...
      ' f_{fit}=%.1fGHz, Order=%d'], epsRMeas, tandMeas, fMeas/1e9, nTerms));

    % --------------------------------------------------------------------------

    subplot(2,1,2);
    semilogx(f, -imag(epsRSarkar)); hold on;
    semilogx(f, -imag(epsRDebye));
    semilogx(fMeas, tandMeas*epsRMeas, 'ro');

    hold off;
    ylabel('\epsilon''''');
    xlabel('Frequency / Hz');
    grid on;

    % --------------------------------------------------------------------------
    % -
    % - Plots / Linear frequency axis
    % -
    % --------------------------------------------------------------------------

    % Set the maximum x-axis limit to one decade below the upper Djordjevic–Sarkar corner frequency
    maxPlotFreqLin = round(10^(m2-1)/2/pi);

    % Round up the maximum plot frequency to the next multiple of 10
    maxPlotFreqLin = ceil(maxPlotFreqLin/1e9/10)*10*1e9;
    [~, maxPlotIdxLin] = find(f >= maxPlotFreqLin, 1, 'first');
    figLin = figure('Position', figLinPos);

    subplot(2,1,1);
    plot(f(1:maxPlotIdxLin)/1e9, real(epsRSarkar(1:maxPlotIdxLin))); hold on;
    plot(f(1:maxPlotIdxLin)/1e9, real(epsRDebye(1:maxPlotIdxLin)));
    plot(fMeas/1e9, epsRMeas, 'ro');

    xlim([0, maxPlotFreqLin/1e9]);

    hold off;
    grid minor;
    ylabel('\epsilon''');
    xlabel('Frequency / GHz');
    legend(legendCell);
    title(sprintf('Wideband Dielectric Model: \\epsilon_r = %.1f, tan_\\delta =%.3f, f_{fit}=%.1fGHz, Order=%d', epsRMeas, tandMeas, fMeas/1e9, nTerms));

    % --------------------------------------------------------------------------

    subplot(2,1,2);
    plot(f(1:maxPlotIdxLin)/1e9, tandSarkar(1:maxPlotIdxLin)); hold on;
    plot(f(1:maxPlotIdxLin)/1e9, tandDebye(1:maxPlotIdxLin));
    plot(fMeas/1e9, tandMeas, 'ro');

    xlim([0, maxPlotFreqLin/1e9]);

    hold off;
    grid minor;
    ylabel('tan_\delta');
    xlabel('Frequency / GHz');
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
