declare default element namespace "http://schemas.openxmlformats.org/officeDocument/2006/extended-properties";
declare namespace xtemp = "http://schemas.eidbeditorxmls.org/TempXML/1.0";

declare namespace output = "http://www.w3.org/2010/xslt-xquery-serialization";
declare option output:method   "xml";
declare option output:encoding "UTF-8";
declare option output:indent   "yes";
declare option output:omit-xml-declaration  "yes";

<Properties xmlns:vt="http://schemas.openxmlformats.org/officeDocument/2006/docPropsVTypes">

<Application>EIDBEditor</Application>
<DocSecurity>0</DocSecurity>
<ScaleCrop>false</ScaleCrop>
<HeadingPairs>
	<vt:vector size="2" baseType="variant">
		<vt:variant>
			<vt:lpstr>Pages</vt:lpstr>
		</vt:variant>
		<vt:variant>
			<vt:i4>{$sheets}</vt:i4>
		</vt:variant>
	</vt:vector>
</HeadingPairs>
<TitlesOfParts>
	<vt:vector size="{$sheets}" baseType="lpstr">
		{doc($sheetNamesFile)//xtemp:sheetName/<vt:lpstr>{./node()}</vt:lpstr>}
	</vt:vector>
</TitlesOfParts>
<Company>https://sourceforge.net/projects/eidbeditor/</Company>
<LinksUpToDate>false</LinksUpToDate>
<SharedDoc>false</SharedDoc>
<HyperlinksChanged>false</HyperlinksChanged>
<AppVersion>1.4</AppVersion>

</Properties>