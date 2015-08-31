#include "qspreadsheetdocument.h"

QSpreadsheetDocument::QSpreadsheetDocument(QObject *parent) :
    QObject(parent)
{
    p_tempDir = QApplication::applicationDirPath() + "/temp";
    p_tempNameSpace = "http://schemas.eidbeditorxmls.org/TempXML/1.0";
}

QSpreadsheetDocument::~QSpreadsheetDocument()
{
}

void QSpreadsheetDocument::clear()
{
    for( int i = p_sheetList.size()-1; i >= 0; --i )
    {
        QTable * sh = p_sheetList[i];
        if( sh ) delete sh;
    }
    p_sheetList.clear();
    p_sheetNames.clear();
    p_sheetsLTCell.clear();
}

void QSpreadsheetDocument::setTempDir( QString tempDir )
{
    p_tempDir = tempDir;
}

bool QSpreadsheetDocument::importSpreadsheet( QString fileName )
{
    clear();
    p_stringCache.clear();
    QFile file( fileName );
    if( !file.exists() ) return false;

    // Unzip
    QDir docDir( p_tempDir + "/xsheet" );
    docDir.mkpath( docDir.path() );
    clearDir( docDir.path() );
    if( !QtZipReader::unzip( fileName, docDir.path() ) )
    {
        removeDir( docDir.path() );
        return false;
    }

    // Read string cache
    readStringCache();

    // Make worksheets
    QFile sheetList( docDir.path() + "/xl/workbook.xml" );
    if( !sheetList.open( QFile::ReadOnly ) ) return false;
    QXmlStreamReader xml;
    xml.setDevice( &sheetList );
    if( xml.readNextStartElement() )
    {
        //qDebug() << "first : " << xml.name().toString();
        if( xml.name() != "workbook" ) return false;
        while( !xml.atEnd() && !xml.hasError() )
        {
            QXmlStreamReader::TokenType token = xml.readNext();
            //qDebug() << "second : " << xml.name().toString();
            if( ( token == QXmlStreamReader::StartElement ) && ( xml.name() == "sheet" ) )
            {
                QXmlStreamAttributes attrs = xml.attributes();
                QString name = attrs.value("name").toString();
                int sheetID = -1;
                QString sheetRID = attrs.value("r:id").toString();
                if( sheetRID.startsWith("rId") ) sheetID = s2i( sheetRID.mid(3), -1 );
                if( sheetID >= 0 )
                {
                    //qDebug() << "adding id=" << sheetID << ", name=" << name;
                    // Add worksheet and read it
                    QString sheetFileName = getSheetFile( sheetID );
                    QTable * newSheet = addSheet( name );
                    QPair<int,int> ltCell( 0, 0 );
                    readSheet( sheetFileName, newSheet, &ltCell );
                    p_sheetsLTCell.replace( sheetIndex( newSheet ), ltCell );
//                    QTable * sh = sheet(i);
//                    if( sh )
//                    {
//                        qDebug() << "Have sheet : " << sh->rowCount() << "x" << sh->columnCount();
//                        qDebug() << sh->row(0).cells();
//                    }
                }
            }
        }
    }
    xml.clear();
    sheetList.close();

    // The end
    removeDir( docDir.path() );
    return true;
}

bool QSpreadsheetDocument::exportSpreadsheet( QString fileName )
{
    QDir docDir( p_tempDir + "/xsheet" );
    if( !docDir.mkpath( docDir.absolutePath() ) ) return false;
    clearDir( docDir.absolutePath() );

    bool result = true;

    // Make format files
    if( !xl_makeContextTypes( docDir.absolutePath() + "/[Content_Types].xml" ) ) result = false;
    if( !xl_makeRels( docDir.absolutePath() + "/_rels/.rels" ) ) result = false;
    if( !xl_makeApp( docDir.absolutePath() + "/docProps/app.xml" ) ) result = false;
    if( !xl_makeCore( docDir.absolutePath() + "/docProps/core.xml" ) ) result = false;
    if( !xl_makeWorkbookXmlRels( docDir.absolutePath() + "/xl/_rels/workbook.xml.rels" ) ) result = false;
    if( !xl_makeTheme( docDir.absolutePath() + "/xl/theme/theme1.xml" ) ) result = false;
    if( !xl_makeStyles( docDir.absolutePath() + "/xl/styles.xml" ) ) result = false;
    if( !xl_makeWorkbook( docDir.absolutePath() + "/xl/workbook.xml" ) ) result = false;
    // Make data files
    if( !xl_makeSheets( docDir.absolutePath() + "/xl/worksheets", docDir.absolutePath() + "/xl/sharedStrings.xml" ) ) result = false;

    // Pack files
    if( !QtZipWriter::zip( fileName, docDir.absolutePath() ) ) result = false;

    // Remove temporary files
    removeDir( docDir.absolutePath() );

    return result;
}

QTable * QSpreadsheetDocument::addSheet( QString name )
{
    QTable * sh = new QTable();
    p_sheetList.append( sh );
    p_sheetNames.append( name );
    p_sheetsLTCell.append( QPair<int,int>(0,0) );
    return sh;
}

void QSpreadsheetDocument::insertSheet( int index, QString name )
{
    QTable * sh = new QTable();
    p_sheetList.insert( index, sh );
    p_sheetNames.insert( index, name );
    p_sheetsLTCell.insert( index, QPair<int,int>(0,0) );
}

void QSpreadsheetDocument::removeSheet( int index )
{
    p_sheetList.removeAt(index);
    p_sheetNames.removeAt(index);
    p_sheetsLTCell.removeAt(index);
}

void QSpreadsheetDocument::removeSheet( QString name )
{
    int index = p_sheetNames.indexOf(name);
    if( index < 0 ) return;
    removeSheet(index);
}

int QSpreadsheetDocument::sheetIndex( QTable * sheet )
{
    int index = -1;
    for( index = 0; index <= p_sheetList.size(); ++index )
        if( p_sheetList[index] == sheet ) break;
    return index;
}

QTable * QSpreadsheetDocument::sheet( int index )
{
    return p_sheetList.value(index,0);
}

QTable * QSpreadsheetDocument::sheet( QString name )
{
    return sheet(p_sheetNames.indexOf(name));
}

QString QSpreadsheetDocument::sheetName( int index )
{
    return p_sheetNames.value( index );
}

QString QSpreadsheetDocument::sheetName( QTable * sheet )
{
    return p_sheetNames.value( sheetIndex(sheet) );
}

QPair<int,int> QSpreadsheetDocument::sheetLeftTopCell( int index )
{
    return p_sheetsLTCell.value( index, QPair<int,int>(0,0) );
}

void QSpreadsheetDocument::setSheetLeftTopCell( int index, QPair<int,int> ltCell )
{
    p_sheetsLTCell.replace( index, ltCell );
}

int QSpreadsheetDocument::sheetCount()
{
    return p_sheetList.size();
}

QPair<int,int> QSpreadsheetDocument::parseCell( QString cell )
{
    //qDebug() << "In " << cell;
    QPair<int,int> result( 0, 0 );
    cell = cell.trimmed().toUpper();
    int digIndex;
    for( digIndex = 0; digIndex < cell.size(); ++digIndex )
        if( cell.at(digIndex).isDigit() ) break;
    result.second = 0;
    QString letters = cell.mid( 0, digIndex );
    int lpower = 1;
    for( int l = letters.size()-1; l >= 0; --l )
    {
        int dig = (int) ( letters[l].toAscii() - 'A' ) + 1;
        //qDebug() << QString( "out=%1 lpower=%2 dig=%3" ).arg(result.second).arg(lpower).arg(dig);
        dig *= lpower;
        lpower *= (int) ( 'Z' - 'A' ) + 1;
        result.second += dig;
    }
    result.second -= 1;
    result.first = s2i( cell.mid( digIndex, cell.size()-digIndex ), 0 ) - 1;
    //qDebug() << "Out " << result;
    return result;
}

QString QSpreadsheetDocument::encodeCell( QPair<int,int> cell )
{
    int letters = (int) ( 'Z' - 'A' ) + 1;
    int maxLettersForCell = 4;
    int lpower = 1;
    for( int i = 0; i < maxLettersForCell; ++i ) lpower *= letters;
    // Make letter part
    QString lPart;
    for( int i = 0; i <= maxLettersForCell; ++i )
    {
        if( i == maxLettersForCell ) cell.second++;
        int dig = ( cell.second / lpower );
        //qDebug() << "dig=" << dig << " lpower=" << lpower << " col=" << cell.second;
        if( dig > 0 )
        {
            char asc;
            asc = ((char)dig-1) + 'A';
            lPart.append( QChar( asc ) );
            cell.second -= lpower * dig;
        }
        lpower /= letters;
    }
    // Make cell text
    QString result = lPart + QString("%1").arg(cell.first+1);
    return result;
}

QString QSpreadsheetDocument::getSheetFile( int rid )
{
    QDir docDir( p_tempDir + "/xsheet" );
    QFile relsFile( docDir.path() + "/xl/_rels/workbook.xml.rels" );
    if( !relsFile.open( QFile::ReadOnly ) ) return QString();
    QXmlStreamReader xml;
    xml.setDevice( &relsFile );
    if( xml.readNextStartElement() )
    {
        //qDebug() << "first : " << xml.name().toString();
        if( xml.name() != "Relationships" ) return QString();
        while( !xml.atEnd() && !xml.hasError() )
        {
            QXmlStreamReader::TokenType token = xml.readNext();
            //qDebug() << "second : " << xml.name().toString();
            if( ( token == QXmlStreamReader::StartElement ) && ( xml.name() == "Relationship" ) )
            {
                QXmlStreamAttributes attrs = xml.attributes();
                QString id = attrs.value("Id").toString();
                QString target = attrs.value("Target").toString();
                if( id == QString("rId%1").arg(rid) ) return target;
            }
        }
    }
    return QString();
}

void QSpreadsheetDocument::readSheet( QString sheetFileName, QTable * sheet, QPair<int, int> * ltCell )
{
    if( sheetFileName.isEmpty() ) return;
    if( !sheet ) return;
    sheet->setRowCount(0);
    sheet->setColumnCount(0);

    QFile sfile( p_tempDir + QString("/xsheet/xl/%1").arg(sheetFileName) );
    if( !sfile.exists() ) return;

    QPair<int,int> topLeft;
    QPair<int,int> bottomRight;
    if( !sfile.open( QFile::ReadOnly ) ) return;
    QXmlStreamReader xml;
    xml.setDevice( &sfile );
    if( xml.readNextStartElement() )
    {
        if( xml.name() != "worksheet" ) return;
        while( !xml.atEnd() && !xml.hasError() )
        {
            QXmlStreamReader::TokenType token = xml.readNext();
            //qDebug() << "second : " << xml.name().toString();
            if( token != QXmlStreamReader::StartElement ) continue;
            if( xml.name() == "dimension" )
            {
                QXmlStreamAttributes attrs = xml.attributes();
                QString ref = attrs.value("ref").toString();
                QStringList dim = ref.split(":");
                if( dim.size()==2 )
                {
                    topLeft = parseCell(dim[0]);
                    bottomRight = parseCell(dim[1]);
                    if( ltCell ) *ltCell = topLeft;
                }
            }
            if( xml.name() == "row" )
            {
                while( !xml.atEnd() && !xml.hasError() )
                {
                    QXmlStreamReader::TokenType token = xml.readNext();
                    if( ( token == QXmlStreamReader::EndElement ) && ( xml.name() == "row" ) ) break;
                    if( token != QXmlStreamReader::StartElement ) continue;
                    if( xml.name() == "c" )
                    {
                        QXmlStreamAttributes attrs = xml.attributes();
                        QPair<int,int> cell = parseCell( attrs.value("r").toString() );
                        bool isCached = attrs.value("t").toString() == "s";
                        cell.first -= topLeft.first;
                        cell.second -= topLeft.second;
                        //qDebug() << "Have cell : r=" << cell.first << " c=" << cell.second;
                        while( !xml.atEnd() && !xml.hasError() )
                        {
                            QXmlStreamReader::TokenType token = xml.readNext();
                            if( ( token == QXmlStreamReader::EndElement ) && ( xml.name() == "c" ) ) break;
                            if( token != QXmlStreamReader::StartElement ) continue;
                            if( xml.name() == "v" )
                            {
                                QString value = xml.readElementText();
                                if( isCached ) value = p_stringCache.value( s2i(value,-1), value );
                                if( !value.isEmpty() )
                                {
                                    if( cell.first >= sheet->rowCount() ) sheet->setRowCount( cell.first+1 );
                                    if( cell.second >= sheet->columnCount() ) sheet->setColumnCount( cell.second+1 );
                                    sheet->set( cell.first, cell.second, value );
                                    //qDebug() << "set: " << cell << " " << value;
                                }
                            }
                        }
                    }
                }
            }
        }
    }
}

void QSpreadsheetDocument::readStringCache()
{
    p_stringCache.clear();
    QFile file( p_tempDir + "/xsheet/xl/sharedStrings.xml" );
    if( !file.exists() ) return;

    if( !file.open( QFile::ReadOnly ) ) return;
    QXmlStreamReader xml;
    xml.setDevice( &file );
    if( xml.readNextStartElement() )
    {
        if( xml.name() != "sst" ) return;
        QXmlStreamAttributes xattrs = xml.attributes();
        //int scount = s2i( xattrs.value("uniqueCount").toString(), 0 );
        while( !xml.atEnd() && !xml.hasError() )
        {
            QXmlStreamReader::TokenType token = xml.readNext();
            if( token != QXmlStreamReader::StartElement ) continue;
            if( xml.name() == "t" )
            {
                QString value = xml.readElementText();
                p_stringCache << value;
            }
        }
    }
}

void QSpreadsheetDocument::removeDir( QString dirPath )
{
    QDir dir( dirPath );
    if( !dir.exists() ) return;
    clearDir( dirPath );
    dir.rmdir( dirPath );
}

void QSpreadsheetDocument::clearDir( QString dirPath )
{
    QDir dir( dirPath );
    if( !dir.exists() ) return;
    dir.setFilter( QDir::AllEntries | QDir::Hidden | QDir::System | QDir::NoDotAndDotDot );
    QFileInfoList fileList = dir.entryInfoList();
    foreach( QFileInfo file, fileList )
    {
        if( file.isDir() ) removeDir( file.absoluteFilePath() );
        else dir.remove( file.absoluteFilePath() );
    }
}

int QSpreadsheetDocument::s2i( QString string, int defValue )
{
    bool ok = false;
    int result = string.toInt( &ok );
    return ( ok ? result : defValue );
}

double QSpreadsheetDocument::s2d( QString string, double defValue )
{
    bool ok = false;
    double result = string.toDouble( &ok );
    return ( ok ? result : defValue );
}

bool QSpreadsheetDocument::fileCopy( QString oldFile, QString newFile )
{
    QFile sfile( oldFile );
    QFile tfile( newFile );
    if( !sfile.open( QFile::ReadOnly ) ) return false;
    if( !tfile.open( QFile::WriteOnly ) ) return false;
    tfile.write( sfile.readAll() );
    sfile.close();
    tfile.close();
    return true;
}

bool QSpreadsheetDocument::addXmlHeader( QString fileName )
{
    QFile file( fileName );
    if( !file.open( QFile::ReadOnly ) ) return false;
    QByteArray fileData = file.readAll();
    file.close();
    if( !file.open( QFile::WriteOnly ) ) return false;
    file.write( QByteArray( "<?xml version=\"1.0\" encoding=\"UTF-8\" standalone=\"yes\"?>\n" ) );
    file.write( fileData );
    file.close();
    return true;
}

bool QSpreadsheetDocument::xl_makeContextTypes( QString fileName )
{
    // Make path
    QFileInfo fi( fileName );
    fi.absoluteDir().mkpath( fi.absolutePath() );

    // Run XQuery
    QFile xsFile( ":/res/qspreadsheet/xl_[Content_Types].xs" );
    if( !xsFile.open( QFile::ReadOnly ) ) return false;
    QXmlQuery query;
    query.bindVariable( "sheets", QVariant( p_sheetList.size() ) );
    query.setQuery( &xsFile );
    QFile xmlFile( fileName );
    if( !xmlFile.open( QFile::WriteOnly ) ) return false;
    QXmlSerializer serializer( query, &xmlFile );
    query.evaluateTo( &serializer );
    xsFile.close();
    xmlFile.close();

    addXmlHeader( fileName );
    return true;
}

bool QSpreadsheetDocument::xl_makeRels( QString fileName )
{
    // Make path
    QFileInfo fi( fileName );
    fi.absoluteDir().mkpath( fi.absolutePath() );

    // Run XQuery
    QFile xsFile( ":/res/qspreadsheet/xl_.rels.xs" );
    if( !xsFile.open( QFile::ReadOnly ) ) return false;
    QXmlQuery query;
    query.setQuery( &xsFile );
    QFile xmlFile( fileName );
    if( !xmlFile.open( QFile::WriteOnly ) ) return false;
    QXmlSerializer serializer( query, &xmlFile );
    query.evaluateTo( &serializer );
    xsFile.close();
    xmlFile.close();

    addXmlHeader( fileName );
    return true;
}

bool QSpreadsheetDocument::xl_makeApp( QString fileName )
{
    // Make path
    QFileInfo fi( fileName );
    fi.absoluteDir().mkpath( fi.absolutePath() );

    // Make sheetNames xml over xml stream writer
    QFile tmpFile( p_tempDir + "/xltmp_app.xml" );
    if( !tmpFile.open( QFile::WriteOnly ) ) return false;
    QXmlStreamWriter stream(&tmpFile);
    stream.writeStartDocument();
    stream.writeDefaultNamespace( p_tempNameSpace );
    stream.writeStartElement( "sheets" );
    for( int i = 0; i < p_sheetList.size(); ++i )
    {
        stream.writeTextElement( "sheetName", p_sheetNames[i] );
    }
    stream.writeEndElement();
    stream.writeEndDocument();
    tmpFile.close();

    // Run XQuery
    QFile xsFile( ":/res/qspreadsheet/xl_app.xs" );
    if( !xsFile.open( QFile::ReadOnly ) ) return false;
    QXmlQuery query;
    query.bindVariable( "sheets", QVariant( p_sheetList.size() ) );
    query.bindVariable( "sheetNamesFile", QVariant( p_tempDir + "/xltmp_app.xml" ) );
    query.setQuery( &xsFile );
    QFile xmlFile( fileName );
    if( !xmlFile.open( QFile::WriteOnly ) ) return false;
    QXmlSerializer serializer( query, &xmlFile );
    query.evaluateTo( &serializer );
    xsFile.close();
    xmlFile.close();

    // Remove blood prints
    tmpFile.remove();

    addXmlHeader( fileName );
    return true;
}

bool QSpreadsheetDocument::xl_makeCore( QString fileName )
{
    // Make path
    QFileInfo fi( fileName );
    fi.absoluteDir().mkpath( fi.absolutePath() );

    // Just copy
    fileCopy( ":/res/qspreadsheet/xl_core.xml", fileName );

    return true;
}

bool QSpreadsheetDocument::xl_makeWorkbookXmlRels( QString fileName )
{
    // Make path
    QFileInfo fi( fileName );
    fi.absoluteDir().mkpath( fi.absolutePath() );

    // Run XQuery
    QFile xsFile( ":/res/qspreadsheet/xl_workbook.xml.rels.xs" );
    if( !xsFile.open( QFile::ReadOnly ) ) return false;
    QXmlQuery query;
    query.bindVariable( "sheets", QVariant( p_sheetList.size() ) );
    query.setQuery( &xsFile );
    QFile xmlFile( fileName );
    if( !xmlFile.open( QFile::WriteOnly ) ) return false;
    QXmlSerializer serializer( query, &xmlFile );
    query.evaluateTo( &serializer );
    xsFile.close();
    xmlFile.close();

    addXmlHeader( fileName );
    return true;
}

bool QSpreadsheetDocument::xl_makeTheme( QString fileName )
{
    // Make path
    QFileInfo fi( fileName );
    fi.absoluteDir().mkpath( fi.absolutePath() );

    // Just copy
    fileCopy( ":/res/qspreadsheet/xl_theme1.xml", fileName );

    return true;
}

bool QSpreadsheetDocument::xl_makeStyles( QString fileName )
{
    // Make path
    QFileInfo fi( fileName );
    fi.absoluteDir().mkpath( fi.absolutePath() );

    // Just copy
    fileCopy( ":/res/qspreadsheet/xl_styles.xml", fileName );

    return true;
}

bool QSpreadsheetDocument::xl_makeWorkbook( QString fileName )
{
    // Make path
    QFileInfo fi( fileName );
    fi.absoluteDir().mkpath( fi.absolutePath() );

    // Make sheets xml over xml stream writer
    QFile tmpFile( p_tempDir + "/xltmp_workbook.xml" );
    if( !tmpFile.open( QFile::WriteOnly ) ) return false;
    QXmlStreamWriter stream(&tmpFile);
    stream.writeStartDocument();
    stream.writeDefaultNamespace( p_tempNameSpace );
    stream.writeStartElement( "sheets" );
    for( int i = 0; i < p_sheetList.size(); ++i )
    {
        stream.writeStartElement( "sheet" );
        stream.writeAttribute( "tname", p_sheetNames[i] );
        stream.writeAttribute( "tsheetId", QString("%1").arg(i+1) );
        stream.writeAttribute( "trid", QString("rId%1").arg(i+1) );
        stream.writeEndElement();
    }
    stream.writeEndElement();
    stream.writeEndDocument();
    tmpFile.close();

    // Run XQuery
    QFile xsFile( ":/res/qspreadsheet/xl_workbook.xs" );
    if( !xsFile.open( QFile::ReadOnly ) ) return false;
    QXmlQuery query;
    query.bindVariable( "sheets", QVariant( p_sheetList.size() ) );
    query.bindVariable( "sheetsFile", QVariant( p_tempDir + "/xltmp_workbook.xml" ) );
    //fileCopy( p_tempDir + "/xltmp_workbook.xml", "D:\\TEMPA.XML" );
    query.setQuery( &xsFile );
    QFile xmlFile( fileName );
    if( !xmlFile.open( QFile::WriteOnly ) ) return false;
    QXmlSerializer serializer( query, &xmlFile );
    query.evaluateTo( &serializer );
    xsFile.close();
    xmlFile.close();

    // Remove blood prints
    tmpFile.remove();

    addXmlHeader( fileName );
    return true;
}

bool QSpreadsheetDocument::xl_makeSheets( QString sheetsPath, QString sharedStringsFileName )
{
    // Make paths
    QDir sheetsDir( sheetsPath );
    sheetsDir.mkpath( sheetsDir.absolutePath() );
    QFileInfo fi( sharedStringsFileName );
    fi.absoluteDir().mkpath( fi.absolutePath() );

    // Prepare shared strings file
    p_stringCache.clear();
    int strCounter = 0;
    QFile tmpFile( p_tempDir + "/xltmp_shStrings.xml" );
    if( !tmpFile.open( QFile::WriteOnly ) ) return false;
    QXmlStreamWriter sxml(&tmpFile);
    sxml.writeStartDocument();
    sxml.writeDefaultNamespace( "http://schemas.openxmlformats.org/spreadsheetml/2006/main" );
    sxml.writeStartElement( "tsst" );

    // Make sheets
    for( int sh = 0; sh < p_sheetList.size(); ++sh )
    {
        QTable * data = sheet(sh);
        if( !data ) continue;
        int rows = data->rowCount();
        int cols = data->columnCount();

        QPair<int,int> ltCell = p_sheetsLTCell[sh];

        // Make temporary sheet xml
        QFile tshFile( p_tempDir + "/xltmp_sheet.xml" );
        if( !tshFile.open( QFile::WriteOnly ) ) return false;
        QXmlStreamWriter stream(&tshFile);
        stream.writeStartDocument();
        stream.writeDefaultNamespace( "http://schemas.openxmlformats.org/spreadsheetml/2006/main" );
        stream.writeStartElement( "sheetData" );

        // Make sheet data
        for( int r = 0; r < rows; ++r )
        {
            stream.writeStartElement( "row" );
            stream.writeAttribute( "r", QString("%1").arg(r+1+ltCell.first) );
            stream.writeAttribute( "spans", QString("%1:%2").arg(1+ltCell.second).arg(1+ltCell.second+cols) );
            for( int c = 0; c < cols; ++c )
            {
                // Get cell data
                QVariant inData = data->at( r, c );
                QString str = inData.toString();

//                QString cc = encodeCell( QPair<int,int>( r+ltCell.first, c+ltCell.second ) );
//                if( ( ( cc == "E4" ) || ( cc == "F8" ) ) && ( sh == 6 ) )
//                {
//                    qDebug() << cc << ": " << str;
//                }

                if( str.isEmpty() ) continue;
                // Is it number?
                bool isNumber = false;
                str.toDouble( &isNumber );
                // Is it realy number?
                bool isNumber2 = false;
                switch( inData.type() )
                {
                case QVariant::Double:
                case QVariant::Int:
                case QVariant::UInt:
                case QVariant::ULongLong:
                case QVariant::LongLong:
                case 135: // float
                    isNumber2 = true;
                    break;
                default:
                    //if( cc == "F30" ) qDebug() << inData.type() << " is " << (int) inData.type();
                    break;
                }
                if( !isNumber2 ) isNumber = false;
                // Is it realy realy true normal number that excel can normal read? :)
                bool isExpNumber = false;
                if( isNumber )
                {
                    if( str == "nan" ) isNumber = false;
                    else
                    {
                        double d = s2d( str, 0.0 );
                        if( d < 0 ) d *= 1.0;
                        if( ( d > 1E13 ) || ( ( d < 1E-13 ) && ( d > 0.0 ) ) )
                        {
                            isNumber = true;
                            isExpNumber = true;
                            //str.replace( ".", "," );
                        }
                        //if( ( d > 1E13 ) || ( ( d < 1E-13 ) ) ) isNumber = false;
                    }
                }
                // Prepare str (and add shared string if data is string)
                if( !isNumber )
                {
                    // Does this string is already present
                    int strIndex = p_stringCache.indexOf( str );
                    if( strIndex < 0 )
                    {
                        // Add shared string
                        sxml.writeStartElement( "si" );
                        sxml.writeTextElement( "t", str );
                        sxml.writeEndElement();
                        p_stringCache.append( str );
                        strIndex = p_stringCache.size()-1;
                    }
                    // Replacing and counting
                    str = QString("%1").arg( strIndex );
                    strCounter++;
                }
                else
                {
                    // Replace dot to right symbol
                    //str.replace( ".", "," );
                    if( !isExpNumber )
                        str = QString("%1").arg( s2d( str, 0.0 ), 0, 'f' );
                    else
                    {
                        str = QString("%1").arg( s2d( str, 0.0 ), 0, 'E' );
                    }
                }
                // Write cell
                stream.writeStartElement( "c" );
                stream.writeAttribute( "r", encodeCell( QPair<int,int>( r+ltCell.first, c+ltCell.second ) ) );
                if( !isNumber ) stream.writeAttribute( "t", "s" );
                //else stream.writeAttribute( "s", "1" );

//                if( ( ( cc == "E4" ) || ( cc == "F8" ) ) && ( sh == 6 ) )
//                {
//                    qDebug() << str;
//                }

                stream.writeTextElement( "v", str );
                stream.writeEndElement();
            }
            stream.writeEndElement();
        }

        // Finishing temporary sheet xml
        stream.writeEndElement();
        stream.writeEndDocument();
        tshFile.close();

        // Make sheet file
        QFile xsFile( ":/res/qspreadsheet/xl_sheet.xs" );
        if( !xsFile.open( QFile::ReadOnly ) ) return false;
        QXmlQuery query;
        query.bindVariable( "ltCell", QVariant( encodeCell( ltCell ) ) );
        QPair<int,int> rbCell( ltCell.first+rows-1, ltCell.second+cols-1 );
        query.bindVariable( "rbCell", QVariant( encodeCell( rbCell ) ) );
        query.bindVariable( "isActive", ( sh == 0 ) ? QVariant(1) : QVariant(0) );
        query.bindVariable( "sheetDataFile", QVariant( p_tempDir + "/xltmp_sheet.xml" ) );
        query.setQuery( &xsFile );
        QFile oxmlFile( sheetsDir.absolutePath() + QString("/sheet%1.xml").arg(sh+1) );
        if( !oxmlFile.open( QFile::WriteOnly ) ) return false;
        QXmlSerializer serializer( query, &oxmlFile );
        query.evaluateTo( &serializer );
        xsFile.close();
        oxmlFile.close();

        // Add xml header
        addXmlHeader( sheetsDir.absolutePath() + QString("/sheet%1.xml").arg(sh+1) );

        // Remove blood prints
        tshFile.remove();
    }

    // Finishing shared string file preparing
    sxml.writeEndElement();
    sxml.writeEndDocument();
    tmpFile.close();

    // Make shared string xml file
    QFile xsFile( ":/res/qspreadsheet/xl_sharedStrings.xs" );
    if( !xsFile.open( QFile::ReadOnly ) ) return false;
    QXmlQuery query;
    query.bindVariable( "strCount", QVariant( strCounter ) );
    query.bindVariable( "uniqCount", QVariant( p_stringCache.size() ) );
    query.bindVariable( "sharedStringsFile", QVariant( p_tempDir + "/xltmp_shStrings.xml" ) );
    query.setQuery( &xsFile );
    QFile oxmlFile( sharedStringsFileName );
    if( !oxmlFile.open( QFile::WriteOnly ) ) return false;
    QXmlSerializer serializer( query, &oxmlFile );
    query.evaluateTo( &serializer );
    xsFile.close();
    oxmlFile.close();

    // Add xml header
    addXmlHeader( sharedStringsFileName );

    // Remove blood prints
    tmpFile.remove();

    return true;
}
