function type_name = GetPropertyType(CSX,name)
% function type_name = GetPropertyType(CSX,name)
%
% internal function to get the type of a given property

type_name = '';
prop_types = fieldnames(CSX.Properties);
for n=1:numel(prop_types)
    for p = 1:numel(CSX.Properties.(prop_types{n}))
        if (strcmp(CSX.Properties.(prop_types{n}){p}.ATTRIBUTE.Name,name))
            type_name = prop_types{n};
            return;
        end
    end
end
