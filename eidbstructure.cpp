#include "eidbstructure.h"

#include "eidbfile.h"

EIDBStructure::EIDBStructure()
{
    // Type names
    typeNames["Hex"]=EIDB::UnknownItem;
    typeNames["String"]=EIDB::StringItem;

    typeNames["UnsignedLong"]=EIDB::DWORDItem;
    typeNames["UnsignedLongList"]=EIDB::DWORDListItem;
    typeNames["Float"]=EIDB::FloatItem;
    typeNames["FloatList"]=EIDB::FloatListItem;
    typeNames["IndexedFloat"]=EIDB::IndexedFloatItem;
    typeNames["IndexedFloatList"]=EIDB::IndexedFloatListItem;
    typeNames["Byte"]=EIDB::ByteItem;
    typeNames["ByteList"]=EIDB::ByteListItem;
    typeNames["SignedLong"]=EIDB::LongItem;
    typeNames["MinfixedStringList"]=EIDB::FixedStringList;
    //typeNames["MinfixedString"]=EIDB::FixedStringItem;
    typeNames["TypeList"]=EIDB::CustomTypeList;
    typeNames["CapsulatedTypeList"]=EIDB::CapsulatedCustomTypeList;
    typeNames["BitList"]=EIDB::BitArray;
    typeNames["BitByte"]=EIDB::BitByteItem;
    typeNames["StringList"]=EIDB::StringList;
    typeNames["AcksUniqueType"]=EIDB::AcksUniqueType;
    typeNames["FixedString"]=EIDB::FixedLengthStringItem;
}

void EIDBStructure::parseDBFiles( QString fileName )
{
    dbFiles.setColumnCount(EIDB::FTColumns);
    dbFiles.setRowCount(0);
    QFile file;
    file.setFileName( fileName );
    if( !file.exists() ) return;
    if( !file.open( QFile::ReadOnly | QFile::Text ) ) return;
    QTextStream in(&file);
    QString line;
    //qDebug() << "*** Files:";
    while( !( line = in.readLine() ).isNull() )
    {
        if( line.isEmpty() ) continue;
        if( line[0] == '#' ) continue;
        line = line.trimmed();
        if( line.isEmpty() ) continue;
        // Parse data
        QStringList list = line.split( '\t' );
        int fileID = 0;
        if( list.size() > 0 ) fileID = QtGenExt::stringToInt( list[0], 0 );
        QString name = QString( "DBFile%1" ).arg(fileID);
        if( list.size() > 1 ) name = list[1];
        QString fileMask = "*.*";
        if( list.size() > 2 ) fileMask = list[2];
        QString fileIcon;
        if( list.size() > 3 ) fileIcon = list[3];
        QString fileTitle = name;
        if( list.size() > 4 ) fileTitle = list[4];
        if( fileTitle.isEmpty() ) fileTitle = name;
        QString fileDesc;
        if( list.size() > 5 ) fileDesc = list[5];
        // Addings
        dbFiles.appendRow( QTableLine( fileID, name, fileMask, fileIcon, fileTitle, fileDesc ) );
        //qDebug() << dbFiles.row( dbFiles.rowCount()-1 ).cells();
    }
}

void EIDBStructure::parseDBBlocks( QString fileName )
{
    dbSaveSequence.clear();
    QFile file;
    file.setFileName( fileName );
    if( !file.exists() ) return;
    if( !file.open( QFile::ReadOnly | QFile::Text ) ) return;
    QTextStream in(&file);
    QString line;
    //qDebug() << "*** Blocks:";
    while( !( line = in.readLine() ).isNull() )
    {
        if( line.isEmpty() ) continue;
        if( line[0] == '#' ) continue;
        line = line.trimmed();
        if( line.isEmpty() ) continue;
        // Parse data
        QStringList list = line.split( '\t' );
        int fileID = 0;
        if( list.size() > 0 )
        {
            QString fileName = list[0];
            QVector<QVariant> names = dbFiles.column(EIDB::FTName).cells();
            int row = names.indexOf(fileName);
            if( row >= 0 ) fileID = QtGenExt::variantToInt( dbFiles.at( row, EIDB::FTID ), 0 );
        }
        int blockID = 0;
        if( list.size() > 1 ) blockID = QtGenExt::stringToInt( list[1], 0 );
        QString name = QString( "DBBlock%1File%2" ).arg(blockID).arg(fileID);
        if( list.size() > 2 ) name = list[2];
        QString icon;
        if( list.size() > 3 ) icon = list[3];
        QString title = name;
        if( list.size() > 4 ) title = list[4];
        QString desc;
        if( list.size() > 5 ) desc = list[5];
        // Addings
        // * New blocks table if not exists
        int bi = blocksIndex(fileID);
        if( bi < 0 )
        {
            QTable * tablePtr = new QTable( &dbFiles );
            tablePtr->setColumnCount( EIDB::BTColumns );
            tablePtr->setRowCount(0);
            dbBlocks << tablePtr;
            int fi = fileIndex(fileID);
            dbFiles.set( fi, EIDB::FTBlocksLink, QtGenExt::uintptrToVariant( EIDBFile::linkToValue( tablePtr ) ) );
            bi = dbBlocks.size()-1;
        }
        // * add
        dbBlocks[bi]->appendRow( QTableLine( fileID, blockID, name, icon, title, desc ) );
        //qDebug() << dbBlocks[bi]->row( dbBlocks[bi]->rowCount()-1 ).cells();
        // * add save sequence
        if( blockID >= 20 ) continue;
        dbSaveSequence << QPair<int,int>( fileID, blockID );
    }
}

void EIDBStructure::parseDBTypes( QString fileName )
{
    QFile file;
    file.setFileName( fileName );
    if( !file.exists() ) return;
    if( !file.open( QFile::ReadOnly | QFile::Text ) ) return;
    QTextStream in(&file);
    QString line;
    int fileID = 0;
    while( !( line = in.readLine() ).isNull() )
    {
        if( line.isEmpty() ) continue;
        if( line[0] == '#' ) continue;
        line = line.trimmed();
        if( line.isEmpty() ) continue;
        if( ( line[0] == '[' ) && ( line[line.size()-1] == ']' ) )
        {
            // Parse file code name
            line = line.mid( 1, line.size()-2 );
            QString fileCodeName = line;
            fileID = fileId(fileCodeName);
            continue;
        }
        // Parse data
        QStringList list = line.split( '\t' );
        int blockID = 0;
        if( list.size() > 0 ) blockID = QtGenExt::stringToInt( list[0], 0 );
        int itemID = 0;
        if( list.size() > 1 ) itemID = QtGenExt::stringToInt( list[1], 0 );
        int itemType = EIDB::UnknownItem;
        int itemTypeValue = 0;
        QByteArray itemTypeData;
        if( list.size() > 2 )
        {
            QString itemTypeName = list[2];
            itemTypeName = itemTypeName.trimmed();
            if( itemTypeName == typeNames.key(EIDB::StringList) )
            {
                itemType = EIDB::FixedStringList;
                itemTypeValue = 0;
            }
            else if( itemTypeName.mid( 0, typeNames.key(EIDB::CustomTypeList).size() ) == typeNames.key(EIDB::CustomTypeList) )
            {
                itemType = EIDB::CustomTypeList;
                QString suffix = itemTypeName.mid( typeNames.key(EIDB::CustomTypeList).size() ).trimmed();
                if( suffix.size() > 2 ) suffix = suffix.mid( 1, suffix.size()-2 );
                itemTypeValue = QtGenExt::stringToInt( suffix );
            }
            else if( itemTypeName.mid( 0, typeNames.key(EIDB::CapsulatedCustomTypeList).size() ) == typeNames.key(EIDB::CapsulatedCustomTypeList) )
            {
                itemType = EIDB::CapsulatedCustomTypeList;
                QString suffix = itemTypeName.mid( typeNames.key(EIDB::CapsulatedCustomTypeList).size() ).trimmed();
                if( suffix.size() > 2 ) suffix = suffix.mid( 1, suffix.size()-2 );
                QStringList parList = suffix.split( "," );
                if( parList.size() > 0 ) itemTypeValue = QtGenExt::stringToInt( parList[0] );
                if( parList.size() > 1 )
                {
                    QByteArray d = parList[1].toLocal8Bit();
                    itemTypeData = QByteArray::fromHex( d );
                }
            }
            else if( itemTypeName.mid( 0, typeNames.key(EIDB::AcksUniqueType).size() ) == typeNames.key(EIDB::AcksUniqueType) )
            {
                itemType = EIDB::AcksUniqueType;
                QString suffix = itemTypeName.mid( typeNames.key(EIDB::AcksUniqueType).size() ).trimmed();
                if( suffix.size() > 2 ) suffix = suffix.mid( 1, suffix.size()-2 );
                itemTypeValue = QtGenExt::stringToInt( suffix );
            }
            else if ( itemTypeName.mid( 0, typeNames.key(EIDB::BitArray).size() ) == typeNames.key(EIDB::BitArray) )
            {
                itemType = EIDB::BitArray;
                QString suffix = itemTypeName.mid( typeNames.key(EIDB::BitArray).size() ).trimmed();
                if( suffix.size() > 2 ) suffix = suffix.mid( 1, suffix.size()-2 );
                itemTypeValue = QtGenExt::stringToInt( suffix );
            }
            else if ( itemTypeName.mid( 0, typeNames.key(EIDB::FixedStringList).size() ) == typeNames.key(EIDB::FixedStringList) )
            {
                itemType = EIDB::FixedStringList;
                QString suffix = itemTypeName.mid( typeNames.key(EIDB::FixedStringList).size() ).trimmed();
                if( suffix.size() > 2 ) suffix = suffix.mid( 1, suffix.size()-2 );
                itemTypeValue = QtGenExt::stringToInt( suffix );
            }
            else if ( itemTypeName.mid( 0, typeNames.key(EIDB::FixedLengthStringItem).size() ) == typeNames.key(EIDB::FixedLengthStringItem) )
            {
                itemType = EIDB::FixedLengthStringItem;
                QString suffix = itemTypeName.mid( typeNames.key(EIDB::FixedLengthStringItem).size() ).trimmed();
                if( suffix.size() > 2 ) suffix = suffix.mid( 1, suffix.size()-2 );
                itemTypeValue = QtGenExt::stringToInt( suffix );
            }
            else
            {
                itemType = typeNames.value( itemTypeName, EIDB::UnknownItem );
            }
        }
        // Addings
        // * New itemTypes table if not exists
        int ii = itemTypesTableIndex(fileID,blockID);
        if( ii < 0 )
        {
            int bi = blocksIndex(fileID);
            if( bi < 0 ) continue;
            int bii = blockIndexInBlocksTable(bi,blockID);
            // * New block row in blocks table if not exists
            if( bii < 0 )
            {
                // * add
                dbBlocks[bi]->appendRow( QTableLine( fileID, blockID ) );
                // * try anew
                bii = blockIndexInBlocksTable(bi,blockID);
                if( bii < 0 ) continue;
            }
            QTable * blockPtr = dbBlocks[bi];
            QTable * tablePtr = new QTable( blockPtr );
            tablePtr->setColumnCount( EIDB::ITColumns );
            tablePtr->setRowCount(0);
            dbItemTypes << tablePtr;
            blockPtr->set( bii, EIDB::BTItemsLink, QtGenExt::uintptrToVariant( EIDBFile::linkToValue( tablePtr ) ) );
            ii = dbItemTypes.size()-1;
        }
        // * add
        dbItemTypes[ii]->appendRow( QTableLine( itemID, itemType, itemTypeValue, itemTypeData ) );
    }
}

void EIDBStructure::parseDBHeaders( QString fileName )
{
    QFile file;
    file.setFileName( fileName );
    if( !file.exists() ) return;
    if( !file.open( QFile::ReadOnly | QFile::Text ) ) return;
    QTextStream in(&file);
    QString line;
    int fileID = 0;
    while( !( line = in.readLine() ).isNull() )
    {
        if( line.isEmpty() ) continue;
        if( line[0] == '#' ) continue;
        line = line.trimmed();
        if( line.isEmpty() ) continue;
        if( ( line[0] == '[' ) && ( line[line.size()-1] == ']' ) )
        {
            // Parse file code name
            line = line.mid( 1, line.size()-2 );
            QString fileCodeName = line;
            fileID = fileId(fileCodeName);
            continue;
        }
        // Parse data
        QStringList list = line.split( '\t' );
        int blockID = 0;
        if( list.size() > 0 ) blockID = QtGenExt::stringToInt( list[0], 0 );
        int itemID = 0;
        if( list.size() > 1 ) itemID = QtGenExt::stringToInt( list[1], 0 );
        int itemIndex = 0;
        if( list.size() > 2 ) itemIndex = QtGenExt::stringToInt( list[2], 0 );
        int headerWidth = 0;
        if( list.size() > 3 ) headerWidth = QtGenExt::stringToInt( list[3], 0 );
        QString headerName;
        if( list.size() > 4 ) headerName = list.at(4).trimmed();
        QString headerDesc;
        if( list.size() > 5 ) headerDesc = list[5];
        // Addings
        // * New headers table if not exists
        int hi = headersTableIndex(fileID,blockID);
        if( hi < 0 )
        {
            int bi = blocksIndex(fileID);
            if( bi < 0 ) continue;
            int bii = blockIndexInBlocksTable(bi,blockID);
            if( bii < 0 ) continue;
            QTable * blockPtr = dbBlocks[bi];
            QTable * tablePtr = new QTable( blockPtr );
            tablePtr->setColumnCount( EIDB::HTColumns );
            tablePtr->setRowCount(0);
            dbHeaders << tablePtr;
            blockPtr->set( bii, EIDB::BTHeadersLink, QtGenExt::uintptrToVariant( EIDBFile::linkToValue( tablePtr ) ) );
            hi = dbHeaders.size()-1;
        }
        // * add
        dbHeaders[hi]->appendRow( QTableLine( itemID, itemIndex, headerWidth, headerName, headerDesc ) );
    }
}

int EIDBStructure::fileId( QString fileName )
{
    int row = dbFiles.column(EIDB::FTName).cells().indexOf(fileName);
    if( row < 0 ) return 0;
    return QtGenExt::variantToInt( dbFiles.at( row, EIDB::FTID ) );
}

int EIDBStructure::fileIndex( int fileID )
{
    return dbFiles.column( EIDB::FTID ).cells().indexOf( fileID );
}

int EIDBStructure::blocksIndex( int fileID )
{
    QVector<QVariant> fileIDList = dbFiles.column( EIDB::FTID ).cells();
    int row = fileIDList.indexOf( fileID );
    if( row < 0 ) return -1;
    int i;
    for( i = 0; i < dbBlocks.size(); ++i )
    {
        QTable * bLink = dbBlocks[i];
        QTable * fLink = EIDBFile::valueToLink( QtGenExt::variantToUIntPtr( dbFiles.at( row, EIDB::FTBlocksLink ) ) );
        if( bLink == fLink ) break;
    }
    if( i < dbBlocks.size() ) return i;
    return -1;
}

QTable * EIDBStructure::blocksOfFile( int fileID )
{
    int bi = blocksIndex(fileID);
    if( bi < 0 ) return 0;
    return dbBlocks[bi];
}

int EIDBStructure::blockIndexInBlocksTable( int blockIndexOfFiles, int blockID )
{
    return dbBlocks[blockIndexOfFiles]->column( EIDB::BTID ).cells().indexOf( blockID );
}

int EIDBStructure::itemTypesTableIndex( int fileID, int blockID )
{
    QTable * blocks = blocksOfFile(fileID);
    if( !blocks ) return -1;
    QVector<QVariant> blockIDList = blocks->column( EIDB::BTID ).cells();
    int row = blockIDList.indexOf( blockID );
    if( row < 0 ) return -1;
    int i;
    for( i = 0; i < dbItemTypes.size(); ++i )
    {
        QTable * iLink = dbItemTypes[i];
        QTable * bLink = EIDBFile::valueToLink( QtGenExt::variantToUIntPtr( blocks->at( row, EIDB::BTItemsLink ) ) );
        if( iLink == bLink ) break;
    }
    if( i < dbItemTypes.size() ) return i;
    return -1;
}

QTable * EIDBStructure::itemTypesTable( int fileID, int blockID )
{
    int ii = itemTypesTableIndex(fileID,blockID);
    if( ii < 0 ) return 0;
    return dbItemTypes[ii];
}

int EIDBStructure::itemTypesIndexOfTable( QTable * itemTypes, int itemID )
{
    if( !itemTypes ) return -1;
    return itemTypes->column( EIDB::ITID ).cells().indexOf(itemID);
}

int EIDBStructure::itemType( QTable * itemTypes, int itemIndex )
{
    if( !itemTypes ) return EIDB::UnknownItem;
    if( itemIndex < 0 ) return EIDB::UnknownItem;
    if( itemIndex >= itemTypes->rowCount() ) return EIDB::UnknownItem;
    return QtGenExt::variantToInt( itemTypes->at( itemIndex, EIDB::ITType ), EIDB::UnknownItem );
}

int EIDBStructure::headersTableIndex( int fileID, int blockID )
{
    QTable * blocks = blocksOfFile(fileID);
    if( !blocks ) return -1;
    QVector<QVariant> blockIDList = blocks->column( EIDB::BTID ).cells();
    int row = blockIDList.indexOf( blockID );
    if( row < 0 ) return -1;
    int i;
    for( i = 0; i < dbHeaders.size(); ++i )
    {
        QTable * hLink = dbHeaders[i];
        QTable * bLink = EIDBFile::valueToLink( QtGenExt::variantToUIntPtr( blocks->at( row, EIDB::BTHeadersLink ) ) );
        if( hLink == bLink ) break;
    }
    if( i < dbHeaders.size() ) return i;
    return -1;
}

QTable * EIDBStructure::headersTable( int fileID, int blockID )
{
    int hi = headersTableIndex(fileID,blockID);
    if( hi < 0 ) return 0;
    return dbHeaders[hi];
}

QList<int> EIDBStructure::filesSaveSequence()
{
    QList<int> result;
    for( int i = 0; i < dbSaveSequence.size(); ++i )
        if( !result.contains(dbSaveSequence[i].first) ) result << dbSaveSequence[i].first;
    return result;
}

QList<int> EIDBStructure::blocksSaveSequence( int fileID )
{
    QList<int> result;
    for( int i = 0; i < dbSaveSequence.size(); ++i )
        if( fileID == dbSaveSequence[i].first ) result << dbSaveSequence[i].second;
    return result;
}

QPair<int,int> EIDBStructure::blockForName( QString name )
{
    QPair<int,int> result(-1,-1);
    for( int i = 0; i < dbBlocks.size(); ++i )
    {
        QTable * blocks = dbBlocks[i];
        if( !blocks ) continue;
        QVector<QVariant> names = blocks->column( EIDB::BTName ).cells();
        int b = names.indexOf( name );
        if( b >= 0 )
        {
            result.first = QtGenExt::stringToInt( blocks->column( EIDB::BTFTID ).cells().at(b).toString(), -1 );
            result.second = QtGenExt::stringToInt( blocks->column( EIDB::BTID ).cells().at(b).toString(), -1 );
        }
    }
    return result;
}

QString EIDBStructure::blockName( int fileID, int blockID )
{
    if( ( fileID < 0 ) || ( blockID < 0 ) ) return QString();
    for( int i = 0; i < dbBlocks.size(); ++i )
    {
        QTable * blocks = dbBlocks[i];
        if( !blocks ) continue;
        for( int bi = 0; bi < blocks->rowCount(); ++bi )
        {
            int biFileID = QtGenExt::stringToInt( blocks->column( EIDB::BTFTID ).cells().at(bi).toString(), -1 );
            int biBlockID = QtGenExt::stringToInt( blocks->column( EIDB::BTID ).cells().at(bi).toString(), -1 );
            if( ( biFileID == fileID ) && ( biBlockID == blockID ) )
                return blocks->column( EIDB::BTName ).cells().at(bi).toString();
        }
    }
    return QString();
}
