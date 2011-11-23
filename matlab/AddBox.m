function CSX = AddBox(CSX, propName, prio, start, stop, varargin)
% function CSX = AddBox(CSX, propName, prio, start, stop, varargin)
%
% Add a box to CSX and assign to a property with name <propName>.
%
%  start:   box start coordinates
%  stop :   box stop  coordinates
%  prio :   primitive priority
%
% optional:
%   Transformation: perform a transformation on a primitive by adding
%      e.g.: 'Transform', {'Scale','1,1,2','Rotate_X',pi/4,'Translate','0,0,100'}
%      Note: This will only affect the 3D material/metal discretisation
%
%   example:
%       CSX = AddMetal(CSX,'metal'); %create PEC with propName 'metal'
%       CSX = AddBox(CSX,'metal',10,[0 0 0],[100 100 200]); %assign box
%
%   with transformation:
%       CSX = AddBox(CSX,'metal',10,[0 0 0],[100 100 200], ...
%                        'Transform', {Rotate_Z, pi/4});
%
% See also AddCylinder, AddCylindricalShell, AddSphere, AddSphericalShell,
% AddCurve, AddWire, AddMetal
%
% CSXCAD matlab interface
% -----------------------
% author: Thorsten Liebig

box.ATTRIBUTE.Priority = prio;

box.P1.ATTRIBUTE.X=start(1);
box.P1.ATTRIBUTE.Y=start(2);
box.P1.ATTRIBUTE.Z=start(3);

box.P2.ATTRIBUTE.X=stop(1);
box.P2.ATTRIBUTE.Y=stop(2);
box.P2.ATTRIBUTE.Z=stop(3);

box = AddPrimitiveArgs(box,varargin{:});

CSX = Add2Property(CSX,propName, box, 'Box');