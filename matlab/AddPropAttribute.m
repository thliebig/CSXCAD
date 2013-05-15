function CSX = AddPropAttribute(CSX, name, att_name, att_value)
% CSX = AddPropAttribute(CSX, name, att_name, att_value)
%
% Add a given attribute (name and value) to the given property
%
% CSXCAD matlab interface
% -----------------------
% author: Thorsten Liebig (c) 2013

type = GetPropertyType(CSX, name);

pos=0;
for n=1:numel(CSX.Properties.(type))
   if  strcmp(CSX.Properties.(type){n}.ATTRIBUTE.Name, name)
       pos=n;
   end
end

if (pos==0)
    error('CSXCAD::AddPropAttribute: property not found');
    return;
end

CSX.Properties.(type){pos}.Attributes.ATTRIBUTE.(att_name) = att_value;
