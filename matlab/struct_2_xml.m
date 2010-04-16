function struct_2_xml(filename, xml_struct, rootName);

Octave = exist('OCTAVE_VERSION') ~= 0;

if ~Octave
	docNode = com.mathworks.xml.XMLUtils.createDocument(rootName);
	docElem = docNode.getDocumentElement;
	
	docElem = struct_2_xmlNode(docNode, docElem, xml_struct);
	
	% Save the sample XML document.
	xmlFileName = [filename];
	xmlwrite(xmlFileName,docNode);
else
	% for octave you need the octave_xmltoolbox (C) 2007 Thomas Geiger
	% http://wiki.octave.org/wiki.pl?XMLToolboxPort
	xml_struct = octave_struct2xml( xml_struct, rootName );
	xml_save( filename, xml_struct, 'any' );
end
