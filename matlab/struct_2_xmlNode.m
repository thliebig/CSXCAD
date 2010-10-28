function docElem = struct_2_xmlNode(docNode, docElem, mat_struct) 

if (isfield(mat_struct,'ATTRIBUTE'))
    names = fieldnames(mat_struct.ATTRIBUTE);
    for n=1:numel(names)
        if isnumeric(getfield(mat_struct.ATTRIBUTE,names{n})) || islogical(getfield(mat_struct.ATTRIBUTE,names{n}))
            docElem.setAttribute(names{n},num2str(getfield(mat_struct.ATTRIBUTE,names{n}),15));
        else
            docElem.setAttribute(names{n},getfield(mat_struct.ATTRIBUTE,names{n}));
        end
    end
    mat_struct = rmfield(mat_struct,'ATTRIBUTE');
end

names = fieldnames(mat_struct);

for n=1:numel(names)
    if isstruct(getfield(mat_struct,names{n}))
        docNewElem = docNode.createElement(names{n});
        docNewElem = struct_2_xmlNode(docNode, docNewElem, getfield(mat_struct,names{n}));
        docElem.appendChild(docNewElem);
    elseif iscell(getfield(mat_struct,names{n}))
        cellfield = getfield(mat_struct,names{n});
        for m=1:numel(cellfield)
            docNewElem = docNode.createElement(names{n});
            docNewElem = struct_2_xmlNode(docNode, docNewElem, cellfield{m});
            docElem.appendChild(docNewElem);
        end
    elseif isempty(getfield(mat_struct,names{n}))
        %do nothing...
    elseif isnumeric(getfield(mat_struct,names{n}))
        number = getfield(mat_struct,names{n});
        str = num2str(number(1),15);
        for i = 2:numel(number)
            str = [str ',' num2str(number(i),15)];
        end
        docNewElem = docNode.createElement(names{n});
        docNewElem.appendChild(docNode.createTextNode(str));
        docElem.appendChild(docNewElem);
    elseif ischar(getfield(mat_struct,names{n}))
        docNewElem = docNode.createElement(names{n});
        docNewElem.appendChild(docNode.createTextNode(getfield(mat_struct,names{n})));
        docElem.appendChild(docNewElem);
    end
end
