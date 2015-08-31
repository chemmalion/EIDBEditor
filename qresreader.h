#ifndef QRESREADER_H
#define QRESREADER_H

#include <QtCore>

#include "qtable.h"
#include "qtgenext.h"

class QResReader : public QObject
{
    Q_OBJECT
public:

    enum
    {
        FName = 0,
        FOffset = 1,
        FSize = 2,
        FCount_ = 3
    };

    struct ResHeader
    {
        unsigned long id;
        unsigned long fileCount;
        unsigned long footerOffset;
        unsigned long nameStringSize;
    };

#pragma pack(push)  /* push current alignment to stack */
#pragma pack(1)     /* set alignment to 1 byte boundary */
    struct ResFileInfo
    {
        quint32 id;
        quint32 size;
        quint32 offset;
        quint32 hasah;
        quint16 nameSize;
        quint32 nameOffset;
    };
#pragma pack(pop)   /* restore original alignment from stack */

    explicit QResReader( QObject *parent = 0 );
    explicit QResReader( QString fileName, QObject *parent = 0 );
    virtual ~QResReader();

    bool setRes( QString fileName );

    QList<QString> files();
    QByteArray readFile( QString fileName );
    void extractAll( QString targetPath );

    static void unpack( QString resFileName, QString targetPath );

signals:

public slots:

private:
    QString p_fileName;
    QTable p_fileList;

};

#endif // QRESREADER_H
