function struct_2_xml(filename, xml_struct, rootName);
% function struct_2_xml(filename, xml_struct, rootName)
%
% Write a Matlab/Octave structure to an XML file.
%
% This is the low-level XML writer behind the CSXCAD structure files.
% Field names become element names, and a field named ATTRIBUTE becomes the
% attributes of its element. To write a CSXCAD structure, use WriteCSX,
% which wraps this function.
%
% arguments:
%   filename    -- output file name, e.g. 'my_structure.xml'
%   xml_struct  -- structure to write
%   rootName    -- name of the XML root element
%
% example:
%
%     CSX = InitCSX();
%     CSX = AddMetal(CSX, 'copper');
%     struct_2_xml('my_structure.xml', CSX, 'ContinuousStructure');
%
% See also WriteCSX, InitCSX
%
% CSXCAD matlab interface
% -----------------------
% author: Thorsten Liebig

if ~isOctave()
	docNode = com.mathworks.xml.XMLUtils.createDocument(rootName);
	docElem = docNode.getDocumentElement;
	
	docElem = struct_2_xmlNode(docNode, docElem, xml_struct);
	
	% Save the sample XML document.
	xmlFileName = [filename];
	xmlwrite(xmlFileName,docNode);
else
%	% for octave you need the octave_xmltoolbox (C) 2007 Thomas Geiger
%	% http://wiki.octave.org/wiki.pl?XMLToolboxPort
%	xml_struct = octave_struct2xml( xml_struct, rootName );
%	xml_save( filename, xml_struct, 'any' );

%	xml_toolbox is buggy (sequence of elements is not preserved)
	fid = fopen( filename, 'w' );
	fprintf( fid, '<?xml version="1.0" encoding="UTF-8" standalone="yes" ?>\n' );
	fprintf( fid, octave_struct2xml_2(xml_struct,rootName,'') );
	fclose( fid );
end
