#ifndef EIDBSTRUCTURE_H
#define EIDBSTRUCTURE_H

#include <QtCore>

#include "qtable.h"

class EIDBStructure
{
public:
    EIDBStructure();

    void parseDBFiles( QString fileName );
    void parseDBBlocks( QString fileName );
    void parseDBTypes( QString fileName );
    void parseDBHeaders( QString fileName );

    int fileId( QString fileName );
    int fileIndex( int fileID );
    int blocksIndex( int fileID );
    QTable * blocksOfFile( int fileID );

    int blockIndexInBlocksTable( int blockIndexOfFiles, int blockID );

    int itemTypesTableIndex( int fileID, int blockID );
    QTable * itemTypesTable( int fileID, int blockID );
    int itemTypesIndexOfTable( QTable * itemTypes, int itemID );
    int itemType( QTable * itemTypes, int itemIndex );

    int headersTableIndex( int fileID, int blockID );
    QTable * headersTable( int fileID, int blockID );

    QList<int> filesSaveSequence();
    QList<int> blocksSaveSequence( int fileID );

    QPair<int,int> blockForName( QString name );
    QString blockName( int fileID, int blockID );

    QTable dbFiles;
    QList<QTable*> dbBlocks;
    QList<QTable*> dbItemTypes;
    QList<QTable*> dbHeaders;
    QList< QPair<int,int> > dbSaveSequence;

private:
    QHash<QString,int> typeNames;
};

#endif // EIDBSTRUCTURE_H
