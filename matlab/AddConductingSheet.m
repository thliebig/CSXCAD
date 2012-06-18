function CSX = AddConductingSheet(CSX, name, conductivity, thickness)
%function CSX = AddConductingSheet(CSX, name, conductivity, thickness)
%
% Add a conducting sheet property to CSX with the given name.
% Remember to add at least one or more 2D!! geometrical primitives to this
% property.
%
% Hint:
%   Set the thickness to 0 to fall back to a perfect metal (AddMetal)
%
%   example:
%       % create the conducting material peroperty, e.g. 40um thick copper
%       CSX = AddConductingSheet(CSX,'copper',56e6,40e-6);
%       % assign box the 2D box --> 40um thick sheet
%       CSX = AddBox(CSX,'copper',10,[0 -50 200],[1000 50 200]);
%
% See also AddMaterial, AddMetal, AddExcitation, AddBox
%
% CSXCAD matlab interface
% -----------------------
% author: Thorsten Liebig 2012


% fall back to ideal pec for t==0 or c==0
if ((thickness==0) || (conductivity==0))
    CSX = AddMetal(CSX,name);
    return;
end

if ((conductivity<0) || (thickness<0))
    error('CSXCAD:AddConductingSheet','a negative conductivity or thickness is invalid');
end

if (conductivity<1e6)
    warning('CSXCAD:AddConductingSheet','a conductivity below 1MA/Vm is not recommended');
end

if (thickness>500e-6)
    warning('CSXCAD:AddConductingSheet','a thickness greater than 500um is not recommended');
end


if (thickness<1e-6)
    warning('CSXCAD:AddConductingSheet','a thickness lower than 1um is not recommended');
end

CSX = AddProperty(CSX, 'ConductingSheet', name,'Conductivity',conductivity,'Thickness',thickness);
