#ifndef QRESPACKER_H
#define QRESPACKER_H

#include <QtCore>

class QResPacker : public QObject
{
    Q_OBJECT

/** Data **/

public:

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
        quint32 hash;
        quint16 nameSize;
        quint32 nameOffset;
    };
#pragma pack(pop)   /* restore original alignment from stack */

/** Methods **/

public:
    explicit QResPacker( QObject *parent = 0 );
//    explicit QResPacker( QString resFileName, QObject *parent = 0 );
//    virtual ~QResPacker();

//    void clear();
//    void setRes( QString resFileName );

//    // Reading
//    void read();

signals:
    
public slots:
    
};

#endif // QRESPACKER_H
