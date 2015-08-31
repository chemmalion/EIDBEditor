declare default element namespace "http://schemas.openxmlformats.org/spreadsheetml/2006/main";
declare namespace xtemp = "http://schemas.eidbeditorxmls.org/TempXML/1.0";

declare namespace output = "http://www.w3.org/2010/xslt-xquery-serialization";
declare option output:method   "xml";
declare option output:encoding "UTF-8";
declare option output:indent   "yes";
declare option output:omit-xml-declaration  "yes";

<workbook xmlns:r="http://schemas.openxmlformats.org/officeDocument/2006/relationships">
	<fileVersion appName="xl" lastEdited="5" lowestEdited="5" rupBuild="9302"/>
	<workbookPr defaultThemeVersion="124226"/>
	<bookViews>
		<workbookView xWindow="120" yWindow="150" windowWidth="24915" windowHeight="12075"/>
	</bookViews>
	<sheets>
		{
			for $i in doc($sheetsFile)//xtemp:sheet
			order by number($i/@tsheetId) ascending
			return <sheet name="{$i/@tname}" sheetId="{$i/@tsheetId}" r:id="{$i/@trid}" />
		}
	</sheets>
	<calcPr calcId="145621"/>
</workbook>
