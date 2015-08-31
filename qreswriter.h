#ifndef QRESWRITER_H
#define QRESWRITER_H

#include <QtCore>

class QResWriter : public QObject
{
    Q_OBJECT

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
        qint32 nextIndex;
        quint32 size;
        quint32 offset;
        qint32 time;
        quint16 nameSize;
        quint32 nameOffset;
    };
#pragma pack(pop)   /* restore original alignment from stack */

    explicit QResWriter( QObject *parent = 0 );
    explicit QResWriter( QString fileName, QObject *parent = 0);
    virtual ~QResWriter();

    void setRes( QString fileName );

    void addFile( QString virtualFileName, QByteArray fileData );
    void addDir( QString dirPath, bool subDirs );
    QList<QString> filesList();
    bool writeRes();
    void clearRes();

    bool packAll( QString dirPath, bool subDirs = true );
    static bool packAll( QString resFileName, QString dirPath, bool subDirs = true );

private:
    QList<QResWriter::ResFileInfo> alphabetResort( QList<ResFileInfo> sourceList, QByteArray nameString );
    int calcAlphabetHash( QByteArray data, int divider );

signals:
    
public slots:

private:
    QString p_resFileName;
    QList<QString> p_names;
    QList<QByteArray> p_files;
    
};

#endif // QRESWRITER_H
