#include "qfilecontainer.h"

QFileContainer::QFileContainer(QObject *parent) :
    QObject(parent)
{
}

QFileContainer::~QFileContainer()
{
}

bool QFileContainer::readDir( QString dirPath, QString fileMask, bool subDirs )
{
    QString path = dirPath;
    if( !path.endsWith("/") ) path.append( "/" );
    QDirIterator it( path,
                     QDir::Files | (subDirs?(QDir::Dirs):(QDir::NoFilter)) | QDir::NoDotAndDotDot,
                     (subDirs?(QDirIterator::Subdirectories):(QDirIterator::NoIteratorFlags)) );
    while (it.hasNext())
    {
        QString file_path = it.next();
        QFileInfo finfo( file_path );

        if( !fileMask.isEmpty() )
        {
            QRegExp rx( fileMask );
            rx.setPatternSyntax(QRegExp::Wildcard);
            if( !rx.exactMatch(finfo.fileName()) ) continue;
        }

        if (it.fileInfo().isDir()) continue;
        if (it.fileInfo().isFile())
        {
            QFile file(file_path);
            if (!file.open(QIODevice::ReadOnly))
                continue;

            QByteArray data = file.readAll();
            addFile( file_path.remove(path), data );
        }
    }
    return true;
}

bool QFileContainer::readFile( QString fileName, QString virtFileName )
{
    QFile file( fileName );
    if( !file.open( QFile::ReadOnly ) ) return false;
    p_files.append( file.readAll() );
    p_names.append( virtFileName );
    return true;
}

bool QFileContainer::addFile( QString virtFileName, QByteArray data )
{
    p_files.append( data );
    p_names.append( virtFileName );
    return true;
}

bool QFileContainer::removeFile( QString fileMask )
{
    for( int i = 0; i < p_files.size(); ++i )
    {
        QRegExp rx( fileMask );
        rx.setPatternSyntax(QRegExp::Wildcard);
        QFileInfo vn( p_names[i] );
        if( rx.exactMatch( vn.fileName() ) )
        {
            p_files.removeAt(i);
            p_names.removeAt(i);
            --i;
        }
    }
    return true;
}

void QFileContainer::clear()
{
    p_files.clear();
    p_names.clear();
    p_nullFile.clear();
}

QStringList QFileContainer::entryList()
{
    return p_names;
}

QByteArray & QFileContainer::file( QString virtFileName )
{
    int index = p_names.indexOf( virtFileName );
    if( ( index < 0 ) || ( index >= p_files.size() ) ) return p_nullFile;
    return p_files[index];
}

bool QFileContainer::writeDir( QString dirPath, QString fileMask )
{
    QDir dir( dirPath );
    for( int i = 0; i < p_names.size(); ++i )
    {
        if( !fileMask.isEmpty() )
        {
            QFileInfo vn( p_names[1] );
            QRegExp rx( fileMask );
            rx.setPatternSyntax(QRegExp::Wildcard);
            if( !rx.exactMatch( vn.fileName() ) ) continue;
        }
        QString fileName = dir.absolutePath() + QString("/%1").arg( p_names[i] );
        writeFile( p_names[i], fileName );
    }
    return true;
}

bool QFileContainer::writeFile( QString virtFileName, QString fileName )
{
    int index = p_names.indexOf( virtFileName );
    if( ( index < 0 ) || ( index >= p_files.size() ) ) return false;

    QFileInfo fi( fileName );
    QDir().mkpath( fi.absolutePath() );

    QFile file( fileName );
    if( !file.open( QFile::WriteOnly ) ) return false;
    file.write( p_files[index] );
    return true;
}
