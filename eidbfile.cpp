#include "eidbfile.h"

EIDBItem::EIDBItem()
{
    p_id = 0;
    p_idCaps = 0;
    p_type = EIDB::UnknownItem;
    p_typeMod = 0;
    p_isFirst = false;
    p_isLast = false;
    p_codePage = "Windows-1251";
}

EIDBItem::~EIDBItem()
{
}

void EIDBItem::importData( QByteArray data, int type, int index, unsigned long size, unsigned long * offset, bool * ok )
{
    p_data.clear();
    if( data.size() < (signed int) (*offset+size) )
    {
        *offset = data.size();
        if( ok ) *ok = false;
        return;
    }
    p_data = data.mid( *offset, size );
    p_type = type;
    p_index = index;
    *offset += size;
    if( ok ) *ok = true;
}

QByteArray EIDBItem::exportData( int exportType )
{
    switch( exportType )
    {
    case EIDB::TextExportType:
    {
        QString str = value().toString();
        QByteArray data = str.toLocal8Bit();
        return data;
    }
    case EIDB::DBExportType:
    default:
    {
        switch( p_type )
        {
        case EIDB::IndexedFloatItem:
        {
            QByteArray result = p_data;
            EIDBTools::addIDnSize( result, (unsigned char) p_index, result.size() );
            return result;
        }
        default:
            return p_data;
        }
    }
    }
}

unsigned char EIDBItem::id() const
{
    return p_id;
}

void EIDBItem::setId( unsigned char id )
{
    p_id = id;
}

int EIDBItem::index() const
{
    return p_index;
}

void EIDBItem::setIndex( int index )
{
    p_index = index;
}

int EIDBItem::type() const
{
    return p_type;
}

void EIDBItem::setType( int type )
{
    p_type = type;
}

int EIDBItem::typeModifier() const
{
    return p_typeMod;
}

void EIDBItem::setTypeModifier( int typeMod )
{
    p_typeMod = typeMod;
}

QVariant EIDBItem::value() const
{
    switch( p_type )
    {
    case EIDB::StringItem:
    case EIDB::FixedStringList:
    case EIDB::FixedStringItem:
    case EIDB::AcksUniqueType:
    {
        QByteArray codecName = p_codePage.toLocal8Bit();
        QTextCodec *codec = QTextCodec::codecForName( codecName.data() );
        //qDebug() << "GET! CodecName: " << codecName.data() << " ... codec is " << ( (codec>0)?("valid"):("invalid") );
        if( !codec ) return QVariant();
        QByteArray prepared = p_data;
        while( ( !prepared.isEmpty() ) && ( prepared.at( prepared.size()-1 ) == 0 ) ) prepared.remove( prepared.size()-1, 1 );
        QString string = codec->toUnicode( prepared );
        //qDebug() << "String with size " << string.size() << " is : [" << string << "]";
        //string = "Test";
        //if( !string.isEmpty() ) qDebug() << "The last chr code is " << ( (int) string.at( string.size()-1 ).toAscii() );
        return string;
    }
    case EIDB::DWORDItem:
    {
        unsigned long dword = *( (unsigned long *) p_data.data() );
        return QVariant( (uint) dword );
    }
    case EIDB::LongItem:
    {
        signed long data = *( (signed long *) p_data.data() );
        return QVariant( (int) data );
    }
    case EIDB::ByteItem:
    {
        uchar data = *( (uchar *) p_data.data() );
        return QVariant( (int) data );
    }
    case EIDB::IndexedFloatItem:
    case EIDB::FloatItem:
    {
        float result = *( (float *) p_data.data() );
        return QVariant( result );
    }
    case EIDB::BitByteItem:
    {
        if( p_data.size() < 1 ) return QVariant(false);
        char data = p_data[0];
        //bool result = data > 0;
        return QVariant(data);
    }
    case EIDB::FixedLengthStringItem:
    {
        QByteArray codecName = p_codePage.toLocal8Bit();
        QTextCodec *codec = QTextCodec::codecForName( codecName.data() );
        if( !codec ) return QVariant();
        QByteArray prepared = p_data;
        while( ( !prepared.isEmpty() ) && ( prepared.size() > p_typeMod || prepared.at( prepared.size()-1 ) == 0 ) ) prepared.remove( prepared.size()-1, 1 );
        QString string = codec->toUnicode( prepared );
        return string;
    }
//    case EIDB::FloatListItem:
//    {
//        int count = p_data.size()/4;
//        QList<QVariant> list;
//        for( int i = 0; i < count; ++i )
//        {
//            float data = *( (float *) &p_data.data()[i*4] );
//            list << QVariant(data);
//        }
//        return QVariant( list );
//    }
    case EIDB::UnknownItem:
    default:
    {
        QByteArray out = p_data.toHex();
        return QVariant( out );
    }
    }
}

void EIDBItem::setValue( QVariant value )
{
    switch( p_type )
    {
    case EIDB::StringItem:
    case EIDB::FixedStringList:
    case EIDB::FixedStringItem:
    case EIDB::AcksUniqueType:
    {
        //qDebug() << "String!";
        QByteArray codecName = p_codePage.toLocal8Bit();
        QTextCodec *codec = QTextCodec::codecForName( codecName.data() );
//        qDebug() << "SET! CodecName: " << codecName.data() << " ... codec is " << ( (codec>0)?("valid"):("invalid") );
//        qDebug() << "Try see: " << p_codePage;
        if( !codec ) return;
        p_data = codec->fromUnicode(value.toString());
        if( ( p_data.size() < 1 ) || ( p_data.at(p_data.size()-1) != 0 ) ) p_data.append((char)0);
        break;
    }
    case EIDB::DWORDItem:
    {
        //qDebug() << "DWORD!";
        p_data.clear();
        double preVal = value.toDouble();
        unsigned long dword = (unsigned long) floor(preVal+0.5);
        char * dword_ptr = (char *) &dword;
        p_data.append( dword_ptr, 4 );
        break;
    }
    case EIDB::LongItem:
    {
        //qDebug() << "LONG!";
        p_data.clear();
        double preVal = value.toDouble();
        signed long data = (signed long) floor(preVal+0.5);
        char * ptr = (char *) &data;
        p_data.append( ptr, 4 );
        break;
    }
    case EIDB::ByteItem:
    {
        //qDebug() << "BYTE!";
        p_data.clear();
        double preVal = value.toDouble();
        uchar data = (uchar) floor(preVal+0.5);
        char * ptr = (char *) &data;
        p_data.append( ptr, 1 );
        break;
    }
    case EIDB::IndexedFloatItem:
    case EIDB::FloatItem:
    {
        //qDebug() << "FLOAT!";
        p_data.clear();
        float data = value.toFloat();
        char * ptr = (char *) &data;
        p_data.append( ptr, 4 );
        break;
    }
    case EIDB::BitByteItem:
    {
        //qDebug() << "BITS!";
        p_data.clear();
        double preVal = value.toDouble();
        char data = (char) floor(preVal+0.5);
        p_data.append( data );
        break;
    }
    case EIDB::FixedLengthStringItem:
    {
        QByteArray codecName = p_codePage.toLocal8Bit();
        QTextCodec *codec = QTextCodec::codecForName( codecName.data() );
        if( !codec ) return;
        p_data = codec->fromUnicode(value.toString());
        if( p_data.size()+1 > p_typeMod) p_data.remove(p_typeMod-1, p_data.size() - (p_typeMod-1));
        while( p_data.size() < p_typeMod) p_data.append((char)0);
        break;
    }
//    case EIDB::FloatListItem:
//    {
//        p_data.clear();
//        QList<QVariant> list = value.toList();
//        for( int i = 0; i < list.size(); ++i )
//        {
//            float data = list[i].toFloat();
//            char * ptr = (char *) &data;
//            p_data.append( ptr, 4 );
//        }
//        break;
//    }
    case EIDB::UnknownItem:
    default:
    {
        //qDebug() << "UNK!";
        QByteArray in = value.toByteArray();
        p_data = QByteArray::fromHex( in );
        break;
    }
    }
}

QByteArray EIDBItem::rawData() const
{
    return p_data;
}

void EIDBItem::setRawData( QByteArray data )
{
    p_data = data;
}

bool EIDBItem::isFirst() const
{
    return p_isFirst;
}

bool EIDBItem::isLast() const
{
    return p_isLast;
}

void EIDBItem::setFirst( bool yes )
{
    p_isFirst = yes;
}

void EIDBItem::setLast( bool yes )
{
    p_isLast = yes;
}

QString EIDBItem::codePage() const
{
    return p_codePage;
}

void EIDBItem::setCodePage( QString cp )
{
    p_codePage = cp;
}

unsigned char EIDBItem::idCaps() const
{
    return p_idCaps;
}

void EIDBItem::setIdCaps( unsigned char idcaps )
{
    p_idCaps = idcaps;
}

QByteArray EIDBItem::capsData() const
{
    return p_capsData;
}

void EIDBItem::setCapsData( QByteArray capsD )
{
    p_capsData = capsD;
}

void EIDBItem::print()
{
    //QString out = QString::fromAscii( p_data.data(), p_data.size() );
    qDebug() << "**** Item ID=" << p_id << "[" << p_index << "] with type=" << p_type << " has: " << value() << " (size:" << p_data.size() << ") data: <<" << p_data << ">>";
}

EIDBItem& EIDBItem::operator =(const EIDBItem& value)
{
    p_id = value.id();
    p_index = value.index();
    p_data = value.rawData();
    p_type = value.type();
    p_typeMod = value.typeModifier();
    p_isFirst = value.isFirst();
    p_isLast = value.isLast();
    p_codePage = value.codePage();
    p_idCaps = value.idCaps();
    p_capsData = value.capsData();
    return *this;
}

EIDBString::EIDBString()
{
    p_id = 0;
}

EIDBString::~EIDBString()
{
}

void EIDBString::importData( QByteArray data, QTable * itemTypesTable, unsigned long size, unsigned long * offset, bool * ok, QTable * blocksTable, EIDBItem defaultItem )
{
    unsigned long srcOffset = *offset;
    p_items.clear();
    // Parse string items
    int freezeCounter = 0;
    int itemTypesIndex = 0;
    //qDebug() << "Entering string. Offset is " << srcOffset << ". Size is " << size << ". Planning offset is " << (*offset+size) << ".";
    while( ( *offset < (srcOffset+size) ) && ( ++freezeCounter < 1000000 ) )
    {
        EIDBItem item = defaultItem;
        item.setCodePage( p_codePage );
        QPair<unsigned char,unsigned long> info;
        // Get item info
        bool p_ok = false;
        info = EIDBTools::getIDnSize( data, offset, &p_ok );
        if( !p_ok ) { if( ok ) *ok = false; return; }
        item.setId( info.first );
        if( itemTypesTable )
        {
            if( item.id() != (unsigned char) QtGenExt::variantToInt( itemTypesTable->at( itemTypesIndex, EIDB::ITID ) ) )
                itemTypesIndex = itemTypesTable->column( EIDB::ITID ).cells().indexOf(item.id());
        }
        // Parse item
        int size = info.second;
        int type = defaultItem.type();
        int typeValue = defaultItem.typeModifier();
        QByteArray typeData;
        if( ( itemTypesTable ) && ( itemTypesIndex >= 0 ) )
        {
            type = QtGenExt::variantToInt( itemTypesTable->at( itemTypesIndex, EIDB::ITType ), EIDB::UnknownItem );
            typeValue = QtGenExt::variantToInt( itemTypesTable->at( itemTypesIndex, EIDB::ITTypeValue ), 0 );
            typeData = itemTypesTable->at( itemTypesIndex, EIDB::ITTypeData ).toByteArray();
        }
        switch( type )
        {
        case EIDB::FloatListItem:
        {
            int itemCount = size/4;
            //qDebug() << "FloatListItem: count=" << itemCount;
            int ostatok = size%4;
            //qDebug() << "FloatListItem: ostatok=" << ostatok;
            for( int i = 0; i < itemCount; ++i )
            {
                EIDBItem subItem;
                subItem.setCodePage( p_codePage );
                subItem.setId( item.id() );
                p_ok = false;
                //qDebug() << "FloatListItem: import item [" << i << "] at offset " << (*offset) << ".";
                subItem.importData( data, EIDB::FloatItem, i, 4, offset, &p_ok );
                //qDebug() << "FloatListItem: imported item has value: " << subItem.value() << ". Offset after is " << (*offset);
                if( !p_ok ) { if( ok ) *ok = false; return; }
                if( i == 0 ) subItem.setFirst( true );
                if( i == (itemCount-1) ) subItem.setLast( true );
                p_items << subItem;
            }
            offset+=ostatok;
            break;
        }
        case EIDB::DWORDListItem:
        {
            int itemCount = size/4;
            int ostatok = size%4;
            for( int i = 0; i < itemCount; ++i )
            {
                EIDBItem subItem;
                subItem.setCodePage( p_codePage );
                subItem.setId( item.id() );
                p_ok = false;
                subItem.importData( data, EIDB::DWORDItem, i, 4, offset, &p_ok );
                if( !p_ok ) { if( ok ) *ok = false; return; }
                if( i == 0 ) subItem.setFirst( true );
                if( i == (itemCount-1) ) subItem.setLast( true );
                p_items << subItem;
            }
            offset+=ostatok;
            break;
        }
        case EIDB::ByteListItem:
        {
            int itemCount = size;
            for( int i = 0; i < itemCount; ++i )
            {
                EIDBItem subItem;
                subItem.setCodePage( p_codePage );
                subItem.setId( item.id() );
                p_ok = false;
                subItem.importData( data, EIDB::ByteItem, i, 1, offset, &p_ok );
                if( !p_ok ) { if( ok ) *ok = false; return; }
                if( i == 0 ) subItem.setFirst( true );
                if( i == (itemCount-1) ) subItem.setLast( true );
                p_items << subItem;
            }
            break;
        }
        case EIDB::IndexedFloatListItem:
        {
            // Parse sub string
            EIDBString str;
            str.setCodePage( p_codePage );
            str.setId( 255 );
            EIDBItem IndexedFloatDefItem;
            IndexedFloatDefItem.setCodePage( p_codePage );
            IndexedFloatDefItem.setId( 255 );
            IndexedFloatDefItem.setType( EIDB::FloatItem );
            bool p_ok = false;
            str.importData( data, 0, size, offset, &p_ok, 0, IndexedFloatDefItem );
            //qDebug() << str.items().size();
            //str.print();
            if( !p_ok ) { if( ok ) *ok = false; return; }
            // Extract items
            QList<EIDBItem> subItems = str.items();
            for( int i = 0; i < subItems.size(); ++i )
            {
                EIDBItem subItem;
                subItem.setCodePage( p_codePage );
                subItem.setId( item.id() );
                subItem.setIndex( (int) subItems[i].id() );
                subItem.setType( EIDB::IndexedFloatItem );
                subItem.setValue( subItems[i].value() );
                if( i == 0 ) subItem.setFirst( true );
                if( i == (subItems.size()-1) ) subItem.setLast( true );
                p_items << subItem;
            }
            break;
        }
        case EIDB::FixedStringList:
        {
            // Parse sub string
            EIDBString str;
            str.setCodePage( p_codePage );
            str.setId( 254 );
            EIDBItem FixedStringDefItem;
            FixedStringDefItem.setCodePage( p_codePage );
            FixedStringDefItem.setId( 255 );
            FixedStringDefItem.setType( EIDB::FixedStringItem );
            FixedStringDefItem.setTypeModifier( typeValue );
            bool p_ok = false;
            str.importData( data, 0, size, offset, &p_ok, 0, FixedStringDefItem );
            //qDebug() << str.items().size();
            //str.print();
            if( !p_ok ) { if( ok ) *ok = false; return; }
            // Extract items
            QList<EIDBItem> subItems = str.items();
            QString resultString;
            for( int i = 0; i < subItems.size(); ++i )
            {
                if( i ) resultString += ",";
                QString el = subItems[i].value().toString();
                el.replace( ',', ';' );
                resultString += el;
            }
            EIDBItem subItem;
            subItem.setCodePage( p_codePage );
            subItem.setId( item.id() );
            subItem.setIndex( 0 );
            subItem.setType( EIDB::FixedStringList );
            subItem.setTypeModifier( typeValue );
            subItem.setValue( resultString );
            p_items << subItem;
            break;
        }
        case EIDB::CustomTypeList:
        {
            if( blocksTable )
            {
                // Parse full featured sub string
                EIDBString str;
                str.setCodePage( p_codePage );
                int xBlockID = typeValue;
                int xBlockIndex = blocksTable->column( EIDB::BTID ).cells().indexOf(xBlockID);
                if( xBlockIndex >= 0 )
                {
                    QTable * itemIndexedUserTypesTable = EIDBFile::valueToLink( QtGenExt::variantToUIntPtr( blocksTable->at( xBlockIndex, EIDB::BTItemsLink ) ) );
                    str.setId( 255 );
                    bool p_ok = false;
                    str.importData( data, itemIndexedUserTypesTable, size, offset, &p_ok, blocksTable );
                    //qDebug() << str.items().size();
                    //str.print();
                    if( !p_ok ) { if( ok ) *ok = false; return; }
                    // Extract items
                    QList<EIDBItem> subItems = str.items();
                    for( int i = 0; i < subItems.size(); ++i )
                    {
                        //if( i == 0 ) qDebug() << "Start adding typed list";
                        EIDBItem subItem;
                        subItem.setCodePage( p_codePage );
                        subItem.setId( item.id() );
                        subItem.setIndex( (int) subItems[i].id() );
                        subItem.setType( subItems[i].type() );
                        subItem.setTypeModifier( EIDB::CustomTypeList );
                        subItem.setValue( subItems[i].value() );
                        if( i == 0 ) subItem.setFirst( true );
                        if( i == (subItems.size()-1) ) subItem.setLast( true );
                        //qDebug() << "Add: id=" << subItem.id() << ", index=" << subItem.index();
                        p_items << subItem;
                    }
                }
                else
                {
                    p_ok = false;
                    item.importData( data, type, 0, size, offset, &p_ok );
                    if( !p_ok ) { if( ok ) *ok = false; return; }
                    p_items << item;
                }
            }
            else
            {
                p_ok = false;
                item.importData( data, type, 0, size, offset, &p_ok );
                if( !p_ok ) { if( ok ) *ok = false; return; }
                p_items << item;
            }
            break;
        }
        case EIDB::AcksUniqueType:
        {
            if( blocksTable )
            {
                // Get acks string
                QString acksString;
                int acksCounter = 1;
                unsigned long acksOffset = *offset;
                //qDebug() << "Start size " << size;
                while( size > 0 )
                {
                    //qDebug() << "Size " << size;
                    // Parse full featured sub string
                    EIDBString str;
                    str.setCodePage( p_codePage );
                    int xBlockID = typeValue;
                    int xBlockIndex = blocksTable->column( EIDB::BTID ).cells().indexOf(xBlockID);
                    if( xBlockIndex < 0 ) { if( ok ) *ok = false; return; }
                    QTable * itemIndexedUserTypesTable = EIDBFile::valueToLink( QtGenExt::variantToUIntPtr( blocksTable->at( xBlockIndex, EIDB::BTItemsLink ) ) );
                    str.setId( 255 );
                    // Get capsulated info
                    p_ok = false;
                    info = EIDBTools::getIDnSize( data, offset, &p_ok );
                    if( !p_ok ) { if( ok ) *ok = false; return; }
                    // Parse sub string
                    p_ok = false;
                    str.importData( data, itemIndexedUserTypesTable, info.second, offset, &p_ok, blocksTable );
                    if( !p_ok ) { if( ok ) *ok = false; return; }

                    // Generate acks text
                    QString acksElement;
                    QList<EIDBItem> subItems = str.items();
                    for( int i = 0; i < subItems.size(); ++i )
                    {
                        if( i>0 ) acksElement += "##";
                        acksElement += QString( "%1=%2" ).arg( (int) subItems[i].id() ).arg( subItems[i].value().toString() );
                    }
                    acksString += QString( "{%1}" ).arg( acksElement );

                    // To next element
                    acksCounter++;
                    size -= (int) ((*offset) - acksOffset);
                    acksOffset = *offset;
                }
                // Make acks string element
                item.setIndex(0);
                item.setType( EIDB::AcksUniqueType );
                item.setTypeModifier( typeValue );
                item.setCodePage( p_codePage );
                item.setValue( acksString );
//                qDebug() << "";
//                qDebug() << "acksString :";
//                qDebug() << acksString;
//                qDebug() << "";
                // Add the items
                p_items << item;
            }
            else
            {
                p_ok = false;
                item.importData( data, type, 0, size, offset, &p_ok );
                if( !p_ok ) { if( ok ) *ok = false; return; }
                p_items << item;
            }
            break;
        }
        case EIDB::BitArray:
        {
            int itemCount = qMin( typeValue, size*8 );
            p_ok = false;
            item.importData( data, EIDB::UnknownItem, 0, size, offset, &p_ok );
            if( !p_ok ) { if( ok ) *ok = false; return; }
            QByteArray itemData = item.exportData();
            for( int i = 0; i < itemCount; ++i )
            {
                EIDBItem subItem;
                subItem.setCodePage( p_codePage );
                subItem.setId( item.id() );
                subItem.setIndex( i );
                subItem.setType( EIDB::BitByteItem );
                subItem.setTypeModifier( size );
                bool bit = ( itemData.data()[i/8] & (((char)0x01)<<(i%8)) ) > 0;
                subItem.setValue( bit );
                if( i == 0 ) subItem.setFirst( true );
                if( i == (itemCount-1) ) subItem.setLast( true );
                p_items << subItem;
            }
            break;
        }
        case EIDB::FixedLengthStringItem:
        {
            p_ok = false;
            item.setTypeModifier( typeValue );
            item.importData( data, type, 0, size, offset, &p_ok );
            if( !p_ok ) { if( ok ) *ok = false; return; }
            p_items << item;
            break;
        }
        default:
        {
            p_ok = false;
            item.importData( data, type, 0, size, offset, &p_ok );
            if( !p_ok ) { if( ok ) *ok = false; return; }
            p_items << item;
            break;
        }
        }
        itemTypesIndex++;
    }
    if( freezeCounter >= 1000000 ) qDebug() << "Error in string from offset " << srcOffset << " to offset " << *offset << "! Too many items (>1000000)!";
    //qDebug() << "String " << p_id << " is imported from offset " << srcOffset << " to offset " << (*offset) << ".";
    if( ok ) *ok = true;
}

QByteArray EIDBString::exportData( int exportType, QTable * blocksTable )
{
    switch( exportType )
    {
    case EIDB::TextExportType:
    {
        return QByteArray();
    }
    case EIDB::DBExportType:
    default:
    {
        QByteArray result;
        for( int i = 0; i < p_items.size(); ++i )
        {
            QByteArray itemData;
            unsigned char id = p_items[i].id();
            bool indexed = false;
            unsigned char capsId = 0;
            QByteArray capsData;
            bool useCapsulation = false;
            if( p_items[i].typeModifier() == EIDB::CustomTypeList ) indexed = true;
            if( p_items[i].typeModifier() == EIDB::CapsulatedCustomTypeList )
            {
                indexed = true;
                capsId = p_items[i].idCaps();
                capsData = p_items[i].capsData();
                useCapsulation = true;
            }
            if( p_items[i].isFirst() )
            {
                if( p_items[i].type() == EIDB::BitByteItem )
                {
                    //qDebug() << "have bit list item!";
                    int procSize = p_items[i].typeModifier();
                    QByteArray procData;
                    char procByte = 0;
                    int procIndex = 0;
                    do
                    {
                        if( i >= p_items.size() ) break;
                        if( procIndex >= (procSize*8) ) break;
                        //qDebug() << "append " << procIndex << " item";
                        if( p_items[i].value().toBool() ) procByte |= ((char)0x01)<<(procIndex%8);
                        if( (procIndex%8) == 7 )
                        {
                            procData.append( procByte );
                            procByte = 0;
                            procIndex = 0;
                        }
                        else procIndex++;
                    } while ( !p_items[i++].isLast() );
                    i--;
                    if( procIndex )
                    {
                        procData.append( procByte );
                        procByte = 0;
                        procIndex = 0;
                    }
                    while( procData.size() < procSize ) procData.append((char)0);
                    itemData.append( procData );
                    //qDebug() << "Bit list item is appednded!";
                }
                else
                {
                    do
                    {
                        if( i >= p_items.size() )
                            break;
                        {
                            QByteArray procData = p_items[i].exportData(exportType);
                            if( indexed ) EIDBTools::addIDnSize( procData, (unsigned char) p_items[i].index(), procData.size() );
                            itemData.append( procData );
                        }
                    } while ( !p_items[i++].isLast() );
                    i--;
                }
            }
            else
            {
                switch( p_items[i].type() )
                {
                case EIDB::FixedStringList:
                {
                    QString str = p_items[i].value().toString();
                    //qDebug() << "Pre> " << str;
                    QStringList list = str.split(',');
                    //qDebug() << list.size();
                    for( int it = 0; it < list.size(); ++it )
                    {
                        QString inlist = list.at(it).trimmed();
                        if( inlist.isEmpty() ) continue;
                        QByteArray elem;
                        elem.append( inlist );
                        if( elem.size() > p_items[i].typeModifier() ) elem.append((char)0);
                        while( elem.size() < p_items[i].typeModifier() ) elem.append((char)0);
                        EIDBTools::addIDnSize( elem, 1, elem.size() );
                        itemData.append( elem );
                    }
                    break;
                }
                case EIDB::AcksUniqueType:
                {
                    if( !blocksTable ) break;
                    int xBlockID = p_items[i].typeModifier();
                    int xBlockIndex = blocksTable->column( EIDB::BTID ).cells().indexOf(xBlockID);
                    if( xBlockIndex < 0 ) break;
                    //if( p_items[i].id() == 32 ) qDebug() << "xblock: " << xBlockID;
                    QTable * itemIndexedUserTypesTable = EIDBFile::valueToLink( QtGenExt::variantToUIntPtr( blocksTable->at( xBlockIndex, EIDB::BTItemsLink ) ) );
                    if( !itemIndexedUserTypesTable ) break;
                    QString acksString = p_items[i].value().toString();
                    QStringList acksList = acksString.split( "}{" );
                    if( acksList.size() < 1 ) break;
                    //qDebug() << p_items[i].exportData();
                    //qDebug() << " HEXXX: " << p_items[i].exportData().toHex();
                    for( int a = 0; a < acksList.size(); ++a )
                    {
                        QString acksItem = acksList[a].trimmed();
                        if( acksItem.isEmpty() ) continue;
                        QByteArray acksItemData;
                        if( ( a == 0 ) && ( acksItem.startsWith("{") ) ) acksItem.remove(0,1);
                        if( ( a == acksList.size()-1 ) && ( acksItem.endsWith("}") ) ) acksItem.remove( acksItem.size()-1, 1 );
                        QStringList acksItemList = acksItem.split("##");
                        for( int b = 0; b < acksItemList.size(); ++b )
                        {
                            // Make sub item data
                            QByteArray acksSubItemData;
                            int eqi = acksItemList[b].indexOf("=");
                            if( eqi < 1 ) continue;
                            int itID = QtGenExt::stringToInt( acksItemList[b].left(eqi), -1 );
                            if( itID < 0 ) continue;
                            QString acksText = acksItemList[b].mid( eqi+1 );
                            int itIndex = itemIndexedUserTypesTable->column( EIDB::ITID ).cells().indexOf( itID );
                            if( itIndex < 0 ) continue;
                            EIDBItem acksSubItem;
                            acksSubItem.setCodePage( p_items[i].codePage() );
                            acksSubItem.setType( QtGenExt::variantToInt( itemIndexedUserTypesTable->column( EIDB::ITType ).cells()[itIndex], EIDB::UnknownItem ) );
                            acksSubItem.setValue( acksText );
                            //qDebug() << acksItemList[b];
                            acksSubItemData = acksSubItem.exportData( exportType );
                            //qDebug() << acksSubItemData;
                            //qDebug() << " HEX:" << acksSubItemData.toHex();
                            // Make id size info
                            EIDBTools::addIDnSize( acksSubItemData, (unsigned char) itID, acksSubItemData.size() );
                            //if( p_items[i].id() == 32 ) qDebug() << "id: " << itID;
                            // Add sub item to item
                            acksItemData.append( acksSubItemData );
                        }
                        // Make id size info for item
                        EIDBTools::addIDnSize( acksItemData, 1, acksItemData.size() );
                        // Add item to main itemData
                        itemData.append( acksItemData );
                    }
                    break;
                }
                default:
                    itemData = p_items[i].exportData(exportType);
                    break;
                }
            }
            if( useCapsulation )
            {
                // Get info from caps data
                unsigned long spcOffset = 0;
                /*QPair<unsigned char,unsigned long> info = */EIDBTools::getIDnSize( capsData, &spcOffset );
                QByteArray compar = capsData.mid( (int) spcOffset );
//                qDebug() << "See:";
//                qDebug() << "ItemData: " << itemData.toHex();
//                qDebug() << "CapsData: " << compar.toHex();
//                qDebug() << "SrcCapsa: " << capsData.toHex();
                if( itemData == compar )
                {
                    // Have no capsulated data, so destroy elements
                    itemData = QByteArray();
                }
                else
                {
                    EIDBTools::addIDnSize( itemData, capsId, itemData.size() );
                }
            }
            EIDBTools::addIDnSize( itemData, id, itemData.size() );
            result.append( itemData );
        }
        return result;
    }
    }
}

EIDBItem * EIDBString::item( int itemIndex ) const
{
    if( ( itemIndex < 0 ) || ( itemIndex >= p_items.size() ) ) return (EIDBItem *) &p_nullItem;
    return (EIDBItem *) &p_items[itemIndex];
}

unsigned char EIDBString::id() const
{
    return p_id;
}

void EIDBString::setId( unsigned char id )
{
    p_id = id;
}

QList<EIDBItem> EIDBString::items() const
{
    return p_items;
}

void EIDBString::setItems( QList<EIDBItem> items )
{
    p_items = items;
}

QString EIDBString::codePage() const
{
    return p_codePage;
}

void EIDBString::setCodePage( QString cp )
{
    p_codePage = cp;
}

void EIDBString::print()
{
    qDebug() << "*** String with ID=" << p_id << " has " << p_items.count() << " items:";
    for( int i = 0; i < p_items.count(); ++i ) p_items[i].print();
}

EIDBString& EIDBString::operator =(const EIDBString& value)
{
    p_id = value.id();
    p_items = value.items();
//    QList<EIDBItem> its = value.items();
//    for( int i = 0; i < its.size(); ++i )
//        its[i].setValue( QVariant() );
//    p_items = its;
//    for( int i = 0; i < items.size(); ++i )
//    {
//        EIDBItem item = items[i];
//        QString buf = items[i].value().toString();
//        item.setValue( QVariant() );
//        p_items << item;
//    }
    p_codePage = value.codePage();
    return *this;
}

EIDBBlock::EIDBBlock()
{
    p_id = 0;
}

EIDBBlock::~EIDBBlock()
{
}

void EIDBBlock::importData( QByteArray data, QTable * itemTypesTable, unsigned long size, unsigned long * offset, bool * ok, QTable * blocksTable )
{
    p_content.clear();
    unsigned long srcOffset = *offset;
    // Parse strings
    int freezeCounter = 0;
    while( ( *offset < (srcOffset+size) ) && ( ++freezeCounter < 1000000 ) )
    {
        EIDBString str;
        str.setCodePage( p_codePage );
        QPair<unsigned char,unsigned long> info;
        // Get string info
        bool p_ok = false;
        //qDebug() << "Offset before string founding is " << (*offset);
        info = EIDBTools::getIDnSize( data, offset, &p_ok );
        //qDebug() << "Found string with id=" << info.first << " and size=" << info.second << ". Offset after is " << (*offset);
        if( !p_ok ) { if( ok ) *ok = false; return; }
        str.setId( info.first );
        // Parse string
        p_ok = false;
        str.importData( data, itemTypesTable, info.second, offset, &p_ok, blocksTable );
        if( !p_ok ) { if( ok ) *ok = false; return; }
        // Add string
        p_content << str;
    }
    if( freezeCounter >= 1000000 ) qDebug() << "Error in block " << p_id << "! Too many strings (>1000000)!";
    if( ok ) *ok = true;
}

QByteArray EIDBBlock::exportData( int exportType, QTable * blocksTable )
{
    switch( exportType )
    {
    case EIDB::TextExportType:
    {
        return QByteArray();
    }
    case EIDB::DBExportType:
    default:
    {
        QByteArray result;
        for( int i = 0; i < p_content.size(); ++i )
        {
            QByteArray strData;
            unsigned char id = p_content[i].id();
            strData = p_content[i].exportData(exportType,blocksTable);
            EIDBTools::addIDnSize( strData, id, strData.size() );
            result.append( strData );
        }
        return result;
    }
    }
}

EIDBItem * EIDBBlock::item( int stringIndex, int itemIndex ) const
{
    if( ( stringIndex < 0 ) || ( stringIndex >= p_content.size() ) ) return (EIDBItem *) &p_nullItem;
    return (EIDBItem *) p_content[stringIndex].item( itemIndex );
}

unsigned char EIDBBlock::id() const
{
    return p_id;
}

void EIDBBlock::setId( unsigned char id )
{
    p_id = id;
}

QList<EIDBString> EIDBBlock::content() const
{
    return p_content;
}

void EIDBBlock::setContent( QList<EIDBString> content )
{
    p_content = content;
}

QString EIDBBlock::codePage() const
{
    return p_codePage;
}

void EIDBBlock::setCodePage( QString cp )
{
    p_codePage = cp;
}

void EIDBBlock::print()
{
    qDebug() << "** Block with ID=" << p_id << " has " << p_content.count() << " strings:";
    for( int i = 0; i < p_content.count(); ++i ) p_content[i].print();
}

EIDBBlock& EIDBBlock::operator =(const EIDBBlock& value)
{
    p_id = value.id();
    p_content = value.content();
    p_codePage = value.codePage();
    return *this;
}

EIDBFile::EIDBFile(QObject *parent) :
    QObject(parent)
{
    p_id = 0;
}

EIDBFile::~EIDBFile()
{
}

void EIDBFile::readFile( QString fileName, QTable * filesTable )
{
    QFile file;
    file.setFileName( fileName );
    if( file.open(QFile::ReadOnly) )
    {
        QByteArray data = file.readAll();
        unsigned long offset = 0;
        bool ok = false;
        QTable * blocksTable = 0;
        if( filesTable )
        {
            QFileInfo fInfo( file );
            for( int i = 0; i < filesTable->rowCount(); ++i )
            {
                QString fileMask = filesTable->at( i, EIDB::FTMask ).toString();
                QRegExp rx(fileMask);
                rx.setPatternSyntax(QRegExp::Wildcard);
                if( rx.exactMatch(fInfo.fileName()) )
                {
                    blocksTable = valueToLink( QtGenExt::variantToUIntPtr( filesTable->at( i, EIDB::FTBlocksLink ) ) );
                    break;
                }
            }
        }
        importData( data, blocksTable, &offset, &ok );
        if( !ok ) qDebug() << "EIDBFile::readFile -> Error after inmportData function!";
        file.close();
    }
}

void EIDBFile::writeFile( QString fileName, QTable * blocksTable )
{
    QFile file;
    file.setFileName( fileName );
    if( file.open( QFile::WriteOnly ) )
    {
        QByteArray data = exportData(EIDB::DBExportType,blocksTable);
        file.write( data );
        file.close();
    }
}

void EIDBFile::importData( QByteArray data, QTable * blocksTable, unsigned long * offset, bool * ok )
{
    p_id = 0;
    p_blocks.clear();
    QPair<unsigned char,unsigned long> info;
    unsigned long srcOffset = *offset;
    // * Parse file
    bool p_ok = false;
    info = EIDBTools::getIDnSize( data, offset, &p_ok );
    if( !p_ok ) { if( ok ) *ok = false; return; }
    p_id = info.first;
    unsigned long size = info.second;
    //qDebug() << "File id is " << p_id << " and size is " << size;
    // * Parse blocks
    int freezeCounter = 0;
    while( ( *offset < (srcOffset+size) ) && ( ++freezeCounter < 15 ) )
    {
        EIDBBlock block;
        block.setCodePage( p_codePage );
        unsigned long blockSize;
        p_ok = false;
        // Get block info
        //qDebug() << "Fround block at offset " << (*offset);
        info = EIDBTools::getIDnSize( data, offset, &p_ok );
        //qDebug() << "Block id=" << info.first << ", size=" << info.second << ". Offset after founding is " << (*offset);
        if( !p_ok ) { if( ok ) *ok = false; return; }
        block.setId( info.first );
        blockSize = info.second;
        // Parse block
        p_ok = false;
        QTable * itemTypesTable = 0;
        if( blocksTable )
        {
            int bi = blocksTable->column( EIDB::BTID ).cells().indexOf( block.id() );
            if( bi >= 0 )
                itemTypesTable = valueToLink( QtGenExt::variantToUIntPtr( blocksTable->at( bi, EIDB::BTItemsLink ) ) );
        }
        block.importData( data, itemTypesTable, blockSize, offset, &p_ok, blocksTable );
        if( !p_ok ) { if( ok ) *ok = false; return; }
        // Add block
        p_blocks << block;
    }
    if( freezeCounter >= 15 ) qDebug() << "Error! Too many blocks in EIDB file (>15)!";
    if( ok ) *ok = true;
    //qDebug() << "Offset after all is " << (*offset);
}

QByteArray EIDBFile::exportData( int exportType, QTable * blocksTable )
{
    switch( exportType )
    {
    case EIDB::TextExportType:
    {
        return QByteArray();
    }
    case EIDB::DBExportType:
    default:
    {
        QByteArray result;
        for( int i = 0; i < p_blocks.size(); ++i )
        {
            QByteArray blockData;
            unsigned char id = p_blocks[i].id();
            blockData = p_blocks[i].exportData(exportType,blocksTable);
            EIDBTools::addIDnSize( blockData, id, blockData.size() );
            result.append( blockData );
        }
        // Append file info
        unsigned char id = p_id;
        EIDBTools::addIDnSize( result, id, result.size() );
        // Append appendix
        QByteArray signature("0000020C020801000000");
        QByteArray sigData = QByteArray::fromHex( signature );
        result.append( sigData );
        // Go out
        return result;
    }
    }
}

EIDBItem * EIDBFile::item( int blockIndex, int stringIndex, int itemIndex ) const
{
    if( ( blockIndex < 0 ) || ( blockIndex >= p_blocks.size() ) ) return (EIDBItem *) &p_nullItem;
    return (EIDBItem *) p_blocks[blockIndex].item( stringIndex, itemIndex );
}

EIDBBlock * EIDBFile::block( unsigned char blockId )
{
    EIDBBlock * result = &p_nullBlock;
    for( int i = 0; i < p_blocks.size(); ++i )
        if( p_blocks[i].id() == blockId )
        {
            result = &p_blocks[i];
            break;
        }
    return result;
}

unsigned char EIDBFile::id() const
{
    return p_id;
}

void EIDBFile::setId( unsigned char id )
{
    p_id = id;
}

QList<EIDBBlock> EIDBFile::blocks() const
{
    return p_blocks;
}

void EIDBFile::setBlocks( QList<EIDBBlock> blocks )
{
    p_blocks = blocks;
}

QString EIDBFile::codePage() const
{
    return p_codePage;
}

void EIDBFile::setCodePage( QString cp )
{
    p_codePage = cp;
}

void EIDBFile::print()
{
    qDebug() << "* File with ID=" << p_id << " has " << p_blocks.count() << " blocks:";
    for( int i = 0; i < p_blocks.count(); ++i ) p_blocks[i].print();
}

EIDBFile& EIDBFile::operator =(const EIDBFile& value)
{
    p_id = value.id();
    p_blocks = value.blocks();
    p_codePage = value.codePage();
    return *this;
}

quintptr EIDBFile::linkToValue( QTable * link )
{
    quintptr result = ( quintptr ) link;
    return result;
}

QTable * EIDBFile::valueToLink( quintptr value )
{
    QTable * result = (QTable*) value;
    return result;
}

QPair<unsigned char, unsigned long> EIDBTools::getIDnSize( QByteArray data, unsigned long * offset, bool * ok )
{
    QPair<unsigned char, unsigned long> result;
    result.first = 0;
    result.second = 0;
    // Get file id
    if( (data.size()-(*offset)) < 1 )
    {
        if( ok ) *ok = false;
        return result;
    }
    result.first = data[(int)*offset];
    *offset+=1;
    // Get file size
    if( (data.size()-(*offset)) < 1 )
    {
        if( ok ) *ok = false;
        return result;
    }
    result.second = (unsigned char) data.at(*offset);
    *offset+=1;
    if( result.second%2 )
    {
        *offset-=1;
        if( (data.size()-(*offset)) < 4 )
        {
            if( ok ) *ok = false;
            return result;
        }
        result.second = *( (unsigned long*) &data.data()[(int)*offset] ) - 1;
        *offset+=4;
    }
    result.second /= 2;
    result.second = qMin( data.size() - (*offset), result.second );
    if( ok ) *ok = true;
    return result;
}

void EIDBTools::addIDnSize( QByteArray & data, unsigned char id, unsigned long size )
{
    QByteArray addData;
    // Size
    unsigned long dSize = size * 2;
    if( dSize > (127*2) )
    {
        dSize++;
        addData.append( (const char *) &dSize, 4 );
    }
    else
        addData.append( (const char *) &dSize, 1 );
    data.prepend( addData );
    // ID
    unsigned char idd = id;
    addData.clear();
    addData.append( (const char *) &idd, 1 );
    data.prepend( addData );
}
