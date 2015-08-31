#include "eidbmodel.h"

EIDBModel::EIDBModel( QObject *parent )
{
    p_rows = 0;
    p_columns = 0;
    p_fileID = -1;
    p_blockID = -1;
}

EIDBModel::~EIDBModel()
{
}

EIDBBlock EIDBModel::eidb()
{
    return p_eidb;
}

void EIDBModel::setEIDB( EIDBBlock eidb )
{
    p_eidb = eidb;
    makeDefString();
    clearColumns();
    completeColumns();
    //foundIndexes(); already done with completeColumns()
    updateCounts();
    reset();
}

void EIDBModel::setHeaderInfo( QList<unsigned char> idList, QList<int> indexList, QList<int> widthList, QList<QString> nameList, QList<QString> descList, bool autoComplete )
{
    if( idList.size() < 1 ) return;
    if( idList.size() != indexList.size() ) return;
    if( nameList.size() != descList.size() ) return;
    if( idList.size() != nameList.size() ) return;

    p_headerItemIDList = idList;
    p_headerItemIndexList = indexList;
    p_headerItemWidthList = widthList;
    p_headerItemNameList = nameList;
    p_headerItemDescList = descList;

    if( autoComplete ) completeColumns();
    else foundIndexes();
    updateCounts();
    reset();
}

int EIDBModel::getHeaderWidth( int index )
{
    if( index < 0 ) return 0;
    if( index >= p_columns ) return 0;
    return p_headerItemWidthList[index];
}

void EIDBModel::setFileID( int fileID )
{
    p_fileID = fileID;
}

void EIDBModel::setBlockID( int blockID )
{
    p_blockID = blockID;
}

int EIDBModel::fileID()
{
    return p_fileID;
}

int EIDBModel::blockID()
{
    return p_blockID;
}

int EIDBModel::fieldID( int columnIndex )
{
    return (int) p_headerItemIDList.value( columnIndex, -1 );
}

int EIDBModel::fieldIndex( int columnIndex )
{
    return p_headerItemIndexList.value( columnIndex, -1 );
}

void EIDBModel::clearHeaderInfo()
{
    clearColumns();
    foundIndexes();
    updateCounts();
    reset();
}

int EIDBModel::columnCount ( const QModelIndex & parent ) const
{
    if( parent.isValid() ) return 0;
    return p_columns;
}

int EIDBModel::rowCount ( const QModelIndex & parent ) const
{
    if( parent.isValid() ) return 0;
    return p_rows;
}

bool EIDBModel::hasIndex ( int row, int column, const QModelIndex & parent ) const
{
    if( parent.isValid() ) return false;
    if( ( row < 0 ) || ( column < 0 ) ) return false;
    if( column >= p_columns ) return false;
    if( row >= p_rows ) return false;
    return true;
}

QModelIndex EIDBModel::index ( int row, int column, const QModelIndex & parent ) const
{
    if( parent.isValid() ) return QModelIndex();
    return createIndex(row,column);
}

QVariant EIDBModel::data ( const QModelIndex & index, int role ) const
{
    if( !index.isValid() ) return QVariant();
    if( ( role != Qt::DisplayRole ) && ( role != Qt::EditRole ) ) return QVariant();
    int row = index.row();
    int column = index.column();
    if( column >= p_indexes.size() ) return QVariant();
    if( !hasIndex( row, column ) ) return QVariant();
    //qDebug() << "<<Requested content>> at r=" << row << " c=" << column << " with index=" << p_indexes[column] << "(rows=" << p_rows << ",cols=" << p_columns << ")";
    return p_eidb.content()[row].items()[p_indexes[column]].value();
}

Qt::ItemFlags EIDBModel::flags ( const QModelIndex & index ) const
{
    if( !index.isValid() ) return Qt::NoItemFlags;
    if( !hasIndex( index.row(), index.column() ) ) return Qt::NoItemFlags;
    return (Qt::ItemFlags) (Qt::ItemIsSelectable | Qt::ItemIsEnabled | Qt::ItemIsEditable);
}

bool EIDBModel::setData ( const QModelIndex & index, const QVariant & value, int role )
{
    if( !index.isValid() ) return false;
    if( role != Qt::EditRole ) return false;
    int row = index.row();
    int column = index.column();
    if( column >= p_indexes.size() ) return false;
    if( !hasIndex( row, column ) ) return false;
    p_eidb.item( row, p_indexes[column] )->setValue( value );
    emit dataChanged(index,index);
    if( column == 0 )
    {
        emit headerDataChanged(Qt::Vertical,row,row);
    }
    return true;
}

void EIDBModel::setField( int row, int fID, int fIndex, QVariant data )
{
    if( p_eidb.content().size() < 1 ) return;
    for( int i = 0; i < p_headerItemIDList.size(); ++i )
    {
        int id = (int) p_headerItemIDList[i];
        int index = p_headerItemIndexList[i];
        if( ( fID == id ) && ( fIndex == index ) )
        {
            setData( this->index( row, i ), data );
            break;
        }
    }
}

QVariant EIDBModel::headerData ( int section, Qt::Orientation orientation, int role ) const
{
    if( section < 0 ) return QVariant();
    switch( orientation )
    {
    case Qt::Horizontal:
        if( section >= p_columns ) return QVariant();
        switch( role )
        {
//        case Qt::SizeHintRole:
//        {
//            QSize size( p_headerItemWidthList[section], 24 );
//            return QVariant( size );
//        }
        case Qt::ToolTipRole:
            return QVariant( p_headerItemDescList[section] );
        case Qt::DisplayRole:
            return QVariant( p_headerItemNameList[section] );
        default:
            return QVariant();
        }
    case Qt::Vertical:
        if( section >= p_rows ) return QVariant();
        switch( role )
        {
        case Qt::DisplayRole:
        {
            QString str = QString("%1").arg(section);
//            if( ( p_eidb.content().size()>section ) && ( p_eidb.content().at(section).items().size()>0 ) )
//                str = str + QString(". %2").arg( p_eidb.content().at(section).item(0)->value().toString() );
            return QVariant( str );
        }
        default:
            return QVariant();
        }
    default:
        return QVariant();
    }
}

bool EIDBModel::setHeaderData ( int section, Qt::Orientation orientation, const QVariant & value, int role )
{
    return false;
}

bool EIDBModel::insertColumns ( int column, int count, const QModelIndex & parent )
{
    return false;
}

bool EIDBModel::insertRows ( int row, int count, const QModelIndex & parent )
{
    if( parent.isValid() ) return false;
    if( row < 0 ) return false;
    if( row > p_rows ) return false;
    if( count < 1 ) return false;
    beginInsertRows( parent, row, row+count-1 );
    QList<EIDBString> strings = p_eidb.content();
    for( int i = 0; i < count; ++i )
    {
        EIDBString eistr = p_defString;

        // Todo: find the mistake
        QList<EIDBItem> items = eistr.items();
        for( int ii = 0; ii < items.size(); ++ii )
            items[ii].setValue( QVariant() );
        eistr.setItems( items );

        strings.insert( row+i, eistr );
    }
    p_eidb.setContent( strings );
    p_rows += count;
    endInsertRows();
    return true;
}

bool EIDBModel::removeColumns ( int column, int count, const QModelIndex & parent )
{
    return false;
}

bool EIDBModel::removeRows ( int row, int count, const QModelIndex & parent )
{
    if( parent.isValid() ) return false;
    if( row < 0 ) return false;
    if( row >= p_rows ) return false;
    if( count < 1 ) return false;
    if( (row+count) > p_rows ) count = p_rows - row;
    beginRemoveRows( parent, row, row+count-1 );
    QList<EIDBString> strings = p_eidb.content();
    for( int i = 0; i < count; ++i )
        strings.removeAt( row );
    p_eidb.setContent( strings );
    p_rows -= count;
    endRemoveRows();
    return true;
}

bool EIDBModel::moveRows( int fromRow, int toRow, int count, const QModelIndex & parent )
{
    if( parent.isValid() ) return false;
    if( ( fromRow < 0 ) || ( toRow < 0 ) ) return false;
    if( count < 1 ) return false;
    if( (fromRow+count) > p_rows ) return false;
    if( toRow > (p_rows-count) ) return false;
    QList<EIDBString> strings = p_eidb.content();
    QList<EIDBString> moved = strings.mid( fromRow, count );
    for( int i = 0; i < count; ++i )
        strings.removeAt(fromRow);
    for( int i = 0; i < count; ++i )
        strings.insert( toRow+i, moved[i] );
    if( !removeRows( fromRow, count ) ) return false;
    if( !insertRows( toRow, count ) ) return false;
    p_eidb.setContent( strings );
    emit dataChanged( index( toRow, 0 ), index( toRow+count-1, p_columns-1 ) );
    return true;
}

void EIDBModel::makeDefString()
{
    if( p_eidb.content().size() < 1 )
    {
        p_defString = EIDBString();
        return;
    }
    EIDBString eistr = p_eidb.content().first();
    QList<EIDBItem> items = eistr.items();
    for( int i = 0; i < items.size(); ++i )
        items[i].setValue( QVariant() );
    eistr.setItems( items );
    p_defString = eistr;
}

void EIDBModel::clearColumns()
{
    p_headerItemIDList.clear();
    p_headerItemIndexList.clear();
    p_headerItemWidthList.clear();
    p_headerItemNameList.clear();
    p_headerItemDescList.clear();
}

void EIDBModel::completeColumns()
{
    if( p_eidb.content().size() < 1 ) return;
    int columns = p_eidb.content()[0].items().size();
    if( columns < 1 ) return;
    QList<EIDBItem> items = p_eidb.content()[0].items();
    for( int col = 0; col < columns; ++col )
    {
        // Search for already existing columns
        int inHeaderIndex;
        for( inHeaderIndex = 0; inHeaderIndex < p_headerItemIDList.size(); ++inHeaderIndex )
        {
            if(
                ( p_headerItemIDList[inHeaderIndex] == items[col].id() ) &&
                ( p_headerItemIndexList[inHeaderIndex] == items[col].index() )
               )
                break;
        }
        if( inHeaderIndex >= p_headerItemIDList.size() )
        {
            // Adding absent item
            p_headerItemIDList << items[col].id();
            p_headerItemIndexList << items[col].index();
            p_headerItemWidthList << 0;
            p_headerItemNameList << QString( "Unk%1-%2" ).arg(items[col].id()).arg(items[col].index());
            p_headerItemDescList << QString();
        }
    }
    foundIndexes();
}

void EIDBModel::foundIndexes()
{
    p_indexes.clear();
    if( p_headerItemIDList.size() < 1 ) return;
    if( p_eidb.content().size() < 1 ) return;
    QList<EIDBItem> items = p_eidb.content()[0].items();
    if( items.size() < 1 ) return;
    // Make index list
    for( int i = 0; i < p_headerItemIDList.size(); ++i )
    {
        int j;
        for( j = 0; j < items.size(); ++j  )
        {
            if( ( items[j].id() == p_headerItemIDList[i] ) && ( items[j].index() == p_headerItemIndexList[i] ) )
            {
                p_indexes << j;
                break;
            }
        }
        if( j >= items.size() )
        {
            p_headerItemIDList.removeAt(i);
            p_headerItemIndexList.removeAt(i);
            p_headerItemNameList.removeAt(i);
            p_headerItemDescList.removeAt(i);
            p_headerItemWidthList.removeAt(i);
        }
    }
    updateCounts();
}

void EIDBModel::updateCounts()
{
    p_columns = 0;
    p_rows = 0;
    if( p_eidb.content().size() < 1 ) return;
    p_columns = p_headerItemIDList.size();
    p_rows = p_eidb.content().size();
}
