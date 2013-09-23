function CSX = Add2Property(CSX, propName, newPrim, primName)
% function CSX = Add2Property(CSX, propName, newPrim, primName)
%
% meant for internal use!!
%
% CSXCAD matlab interface
% -----------------------
% author: Thorsten Liebig

type = GetPropertyType(CSX, propName);
if isempty(type)
    error('CSXCAD:Add2Property',['the type for the property "' propName '" cannot be found']);
end

pos = GetPropertyPosition(CSX, type, propName);
if (pos==0)
    error('CSXCAD:Add2Property',['property "' propName '" of type "' type '" not found!']);
end

if ~isfield(CSX.Properties.(type){pos}, 'Primitives')
    CSX.Properties.(type){pos}.Primitives.(primName){1}=newPrim;
elseif ~isfield(CSX.Properties.(type){pos}.Primitives, primName)
    CSX.Properties.(type){pos}.Primitives.(primName){1}=newPrim;
else
    CSX.Properties.(type){pos}.Primitives.(primName){end+1}=newPrim;
end
