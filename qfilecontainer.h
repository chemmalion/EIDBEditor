#ifndef QFILECONTAINER_H
#define QFILECONTAINER_H

#include <QtCore>

class QFileContainer : public QObject
{
    Q_OBJECT

public:

    explicit QFileContainer(QObject *parent = 0);
    virtual ~QFileContainer();

    bool readDir( QString dirPath, QString fileMask = QString(), bool subDirs = true );
    bool readFile( QString fileName, QString virtFileName );

    bool addFile( QString virtFileName, QByteArray data );
    bool removeFile( QString fileMask );
    void clear();

    QStringList entryList();

    QByteArray & file( QString virtFileName );

    bool writeDir( QString dirPath, QString fileMask = QString() );
    bool writeFile( QString virtFileName, QString fileName );
    
signals:
    
public slots:

private:
    QList<QByteArray> p_files;
    QStringList p_names;
    QByteArray p_nullFile;
    
};

#endif // QFILECONTAINER_H
