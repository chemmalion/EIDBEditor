declare default element namespace "http://schemas.openxmlformats.org/package/2006/relationships";

<Relationships>

{
	for $i in (1 to $sheets)
	return <Relationship Id="rId{$i}" Type="http://schemas.openxmlformats.org/officeDocument/2006/relationships/worksheet" Target="worksheets/sheet{$i}.xml"/>
}

<Relationship Id="rId{$sheets + 1}" Type="http://schemas.openxmlformats.org/officeDocument/2006/relationships/theme" Target="theme/theme1.xml"/>
<Relationship Id="rId{$sheets + 2}" Type="http://schemas.openxmlformats.org/officeDocument/2006/relationships/styles" Target="styles.xml"/>
<Relationship Id="rId{$sheets + 3}" Type="http://schemas.openxmlformats.org/officeDocument/2006/relationships/sharedStrings" Target="sharedStrings.xml"/>

</Relationships>