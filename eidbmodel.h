#ifndef EIDBMODEL_H
#define EIDBMODEL_H

#include <QtCore>

#include "eidbfile.h"

class EIDBModel : public QAbstractTableModel
{
    Q_OBJECT

public:
    explicit EIDBModel( QObject *parent = 0 );
    virtual ~EIDBModel();

    EIDBBlock eidb();
    void setEIDB( EIDBBlock eidb );
    void setHeaderInfo( QList<unsigned char> idList, QList<int> indexList, QList<int> widthList, QList<QString> nameList, QList<QString> descList, bool autoComplete = true );
    void clearHeaderInfo();

    int getHeaderWidth( int index );

    void setFileID( int fileID );
    void setBlockID( int blockID );
    int fileID();
    int blockID();

    int fieldID( int columnIndex );
    int fieldIndex( int columnIndex );

    virtual int	columnCount ( const QModelIndex & parent = QModelIndex() ) const;
    virtual int	rowCount ( const QModelIndex & parent = QModelIndex() ) const;
    bool hasIndex ( int row, int column, const QModelIndex & parent = QModelIndex() ) const;
    virtual QModelIndex	index ( int row, int column, const QModelIndex & parent = QModelIndex() ) const;

    virtual QVariant data ( const QModelIndex & index, int role = Qt::DisplayRole ) const;
    virtual Qt::ItemFlags flags ( const QModelIndex & index ) const;

    virtual bool setData ( const QModelIndex & index, const QVariant & value, int role = Qt::EditRole );
    void setField( int row, int fID, int fIndex, QVariant data );

    virtual QVariant headerData ( int section, Qt::Orientation orientation, int role = Qt::DisplayRole ) const;
    virtual bool setHeaderData ( int section, Qt::Orientation orientation, const QVariant & value, int role = Qt::EditRole );

    virtual bool insertColumns ( int column, int count, const QModelIndex & parent = QModelIndex() );
    virtual bool insertRows ( int row, int count, const QModelIndex & parent = QModelIndex() );
    virtual bool removeColumns ( int column, int count, const QModelIndex & parent = QModelIndex() );
    virtual bool removeRows ( int row, int count, const QModelIndex & parent = QModelIndex() );

    virtual bool moveRows( int fromRow, int toRow, int count, const QModelIndex & parent = QModelIndex() );

private:
    void makeDefString();
    void clearColumns();
    void completeColumns();
    void foundIndexes();
    void updateCounts();

private:
    EIDBBlock p_eidb;
    EIDBString p_defString;

    int p_fileID;
    int p_blockID;

    QList<unsigned char> p_headerItemIDList;
    QList<int> p_headerItemIndexList;
    QList<QString> p_headerItemNameList;
    QList<QString> p_headerItemDescList;
    QList<int> p_headerItemWidthList;

    QList<int> p_indexes;

    int p_rows;
    int p_columns;

};

#endif // EIDBMODEL_H
