function out = octave_struct2xml( in, rootName )

out = [];
out.NAME = rootName;

if ~isstruct( in )
	if ~ischar( in )
		temp = num2str(in(1));
		for a=2:numel(in)
			temp = [temp ', ' num2str(in(a))];
		end
		in = temp;
	end
	out.TEXT = in;
	return
end

fnames = fieldnames( in );
for n=1:numel(fnames)
	current_field = fnames{n};
	if strcmp( current_field, 'ATTRIBUTE' )
		attributes = fieldnames( in.ATTRIBUTE );
		for m=1:numel( attributes )
			temp = in.ATTRIBUTE.(attributes{m});
			if ~ischar( temp )
				temp = num2str( temp );
			end
			out.ATTRS.(attributes{m}) = temp;
		end
		continue
	end
    
	if iscell( in.(current_field) )
	        out.(current_field) = {};
		for m=1:numel( in.(current_field) )
			out.(current_field){m} = octave_struct2xml( in.(current_field){m}, current_field );
		end
	else
		out.(current_field){1} = octave_struct2xml( in.(current_field), current_field );
	end
end
