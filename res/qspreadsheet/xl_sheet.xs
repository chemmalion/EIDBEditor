declare default element namespace "http://schemas.openxmlformats.org/spreadsheetml/2006/main";

declare namespace output = "http://www.w3.org/2010/xslt-xquery-serialization";
declare option output:method   "xml";
declare option output:encoding "UTF-8";
declare option output:indent   "yes";
declare option output:omit-xml-declaration  "yes";

<worksheet xmlns:r="http://schemas.openxmlformats.org/officeDocument/2006/relationships" xmlns:mc="http://schemas.openxmlformats.org/markup-compatibility/2006" mc:Ignorable="x14ac" xmlns:x14ac="http://schemas.microsoft.com/office/spreadsheetml/2009/9/ac">
	<dimension ref="{$ltCell}:{$rbCell}"/>
	<sheetViews>
			<sheetView tabSelected="{$isActive}" workbookViewId="0">
				<selection activeCell="{$ltCell}" sqref="{$ltCell}"/>
			</sheetView>
	</sheetViews>
	<sheetFormatPr defaultRowHeight="15" x14ac:dyDescent="0.25"/>
	{doc($sheetDataFile)/sheetData/<sheetData>{./node()}</sheetData>}
	<pageMargins left="0.7" right="0.7" top="0.75" bottom="0.75" header="0.3" footer="0.3"/>
</worksheet>
