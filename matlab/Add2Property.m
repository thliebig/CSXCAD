function CSX = Add2Property(CSX, propName, newPrim, primName)

names = fieldnames(CSX.Properties);

for n=1:numel(names)
    for pos=1:numel(CSX.Properties.(names{n}))
        if strcmp(CSX.Properties.(names{n}){pos}.ATTRIBUTE.Name,propName)
             if ~isfield(CSX.Properties.(names{n}){pos},'Primitives')
                CSX.Properties.(names{n}){pos}.Primitives.(primName){1}=newPrim;
             elseif ~isfield(CSX.Properties.(names{n}){pos}.Primitives,primName)
                CSX.Properties.(names{n}){pos}.Primitives.(primName){1}=newPrim;
             else
                CSX.Properties.(names{n}){pos}.Primitives.(primName){end+1}=newPrim;
             end
        end
    end
end