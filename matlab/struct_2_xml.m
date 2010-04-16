function struct_2_xml(filename, xml_struct, rootName);


docNode = com.mathworks.xml.XMLUtils.createDocument(rootName);
docElem = docNode.getDocumentElement;

docElem = struct_2_xmlNode(docNode, docElem, xml_struct);

% Save the sample XML document.
xmlFileName = [filename];
xmlwrite(xmlFileName,docNode);