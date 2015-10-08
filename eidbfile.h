#ifndef EIDBFILE_H
#define EIDBFILE_H

#include <QtCore>

#include "qtable.h"
#include "qtgenext.h"

namespace EIDB
{
    enum Items
    {
        UnknownItem = 0,
        StringItem = 1,
        DWORDItem = 2,
        DWORDListItem = 3,
        FloatItem = 4,
        FloatListItem = 5,
        IndexedFloatItem = 6,
        IndexedFloatListItem = 7,
        ByteItem = 8,
        ByteListItem = 9,
        LongItem = 10,
        FixedStringList = 11,
        FixedStringItem = 12,
        CustomTypeList = 13,
        BitArray = 14,
        BitByteItem = 15,
        StringList = 16,
        CapsulatedCustomTypeList = 17,
        AcksUniqueType = 18,
        ItemCount = 19,
        FixedLengthStringItem = 20
    };
    enum ExportType
    {
        DBExportType = 0,
        TextExportType = 1,
        ExportTypeCount = 2
    };
}

namespace EIDB
{
    enum FilesTableEnum
    {
        FTID = 0,
        FTName = 1,
        FTMask = 2,
        FTIcon = 3,
        FTTitle = 4,
        FTDesc = 5,
        FTBlocksLink = 6,
        FTColumns = 7
    };
    enum BlocksTableEnum
    {
        BTFTID = 0,
        BTID = 1,
        BTName = 2,
        BTIcon = 3,
        BTTitle = 4,
        BTDesc = 5,
        BTItemsLink = 6,
        BTHeadersLink = 7,
        BTColumns = 8
    };
    enum ItemTypesTableEnum
    {
        ITID = 0,
        ITType = 1,
        ITTypeValue = 2,
        ITTypeData = 3,
        ITColumns = 4
    };
    enum HeadersTableEnum
    {
        HTID = 0,
        HTIndex = 1,
        HTWidth = 2,
        HTName = 3,
        HTDesc = 4,
        HTColumns = 5
    };
}

class EIDBItem
{
public:
    EIDBItem();
    virtual ~EIDBItem();

    void importData( QByteArray data, int type, int index, unsigned long size, unsigned long * offset = 0, bool * ok = 0 );
    QByteArray exportData( int exportType = EIDB::DBExportType );

    unsigned char id() const;
    void setId( unsigned char id );

    int index() const;
    void setIndex( int index );

    int type() const;
    void setType( int type );

    int typeModifier() const;
    void setTypeModifier( int typeMod );

    QVariant value() const;
    void setValue( QVariant value );

    QByteArray rawData() const;
    void setRawData( QByteArray data );

    bool isFirst() const;
    bool isLast() const;
    void setFirst( bool yes );
    void setLast( bool yes );

    QString codePage() const;
    void setCodePage( QString cp );

    unsigned char idCaps() const;
    void setIdCaps( unsigned char idcaps );

    QByteArray capsData() const;
    void setCapsData( QByteArray capsD );

    void print();

    EIDBItem& operator =(const EIDBItem& value);

private:
    unsigned char p_id;
    int p_index;
    QByteArray p_data;
    int p_type;
    int p_typeMod;
    bool p_isFirst;
    bool p_isLast;
    QString p_codePage;
    unsigned char p_idCaps;
    QByteArray p_capsData;
};

class EIDBString
{
public:
    EIDBString();
    virtual ~EIDBString();

    void importData( QByteArray data, QTable * itemTypesTable, unsigned long size, unsigned long * offset = 0, bool * ok = 0, QTable * blocksTable = 0, EIDBItem defaultItem = EIDBItem() );
    QByteArray exportData( int exportType = EIDB::DBExportType, QTable * blocksTable = 0 );

    EIDBItem * item( int itemIndex ) const;

    unsigned char id() const;
    void setId( unsigned char id );

    QList<EIDBItem> items() const;
    void setItems( QList<EIDBItem> items );

    QString codePage() const;
    void setCodePage( QString cp );

    void print();

    EIDBString& operator =(const EIDBString& value);

private:
    unsigned char p_id;
    QList<EIDBItem> p_items;
    QString p_codePage;
    EIDBItem p_nullItem;
};

class EIDBBlock
{
public:
    EIDBBlock();
    virtual ~EIDBBlock();

    void importData( QByteArray data, QTable * itemTypesTable, unsigned long size, unsigned long * offset = 0, bool * ok = 0, QTable * blocksTable = 0 );
    QByteArray exportData( int exportType = EIDB::DBExportType, QTable * blocksTable = 0 );

    EIDBItem * item( int stringIndex, int itemIndex ) const;

    unsigned char id() const;
    void setId( unsigned char id );

    QList<EIDBString> content() const;
    void setContent( QList<EIDBString> content );

    QString codePage() const;
    void setCodePage( QString cp );

    void print();

    EIDBBlock& operator =(const EIDBBlock& value);

private:
    unsigned char p_id;
    QList<EIDBString> p_content;
    QString p_codePage;
    EIDBItem p_nullItem;
};

class EIDBFile : public QObject
{
    Q_OBJECT
public:
    explicit EIDBFile(QObject *parent = 0);
    virtual ~EIDBFile();

    void readFile( QString fileName, QTable * filesTable );
    void writeFile( QString fileName, QTable * blocksTable = 0 );

    void importData( QByteArray data, QTable * blocksTable, unsigned long * offset = 0, bool * ok = 0 );
    QByteArray exportData( int exportType = EIDB::DBExportType, QTable * blocksTable = 0 );

    EIDBItem * item( int blockIndex, int stringIndex, int itemIndex ) const;
    EIDBBlock * block( unsigned char blockId );

    unsigned char id() const;
    void setId( unsigned char id );

    QList<EIDBBlock> blocks() const;
    void setBlocks( QList<EIDBBlock> blocks );

    QString codePage() const;
    void setCodePage( QString cp );

    void print();

    EIDBFile& operator =(const EIDBFile& value);

    static quintptr linkToValue( QTable * link );
    static QTable * valueToLink( quintptr value );

private:
    unsigned char p_id;
    QList<EIDBBlock> p_blocks;
    EIDBItem p_nullItem;
    EIDBBlock p_nullBlock;
    QString p_codePage;
};

class EIDBTools
{
public:
    static QPair<unsigned char, unsigned long> getIDnSize( QByteArray data, unsigned long * offset, bool * ok = 0 );
    static void addIDnSize( QByteArray & data, unsigned char id, unsigned long size );
};

#endif // EIDBFILE_H
