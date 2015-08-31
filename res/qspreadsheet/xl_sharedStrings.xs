declare default element namespace "http://schemas.openxmlformats.org/spreadsheetml/2006/main";

declare namespace output = "http://www.w3.org/2010/xslt-xquery-serialization";
declare option output:method   "xml";
declare option output:encoding "UTF-8";
declare option output:indent   "yes";
declare option output:omit-xml-declaration  "yes";

doc($sharedStringsFile)/tsst/<sst count="{$strCount}" uniqueCount="{$uniqCount}">{./node()}</sst>
