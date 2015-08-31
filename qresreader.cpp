#include "qresreader.h"

QResReader::QResReader( QObject *parent ) :
    QObject(parent)
{
}

QResReader::QResReader(QString fileName, QObject *parent) :
    QObject(parent)
{
    setRes( fileName );
}

QResReader::~QResReader()
{
}

bool QResReader::setRes( QString fileName )
{
    p_fileList.setColumnCount( FCount_ );
    p_fileList.setRowCount(0);
    p_fileName = fileName;

    QFile file( fileName );
    if( !file.exists() ) return false;
    if( !file.open( QFile::ReadOnly ) ) return false;

    ResHeader rh;
    if( file.read( (char*) &rh, sizeof(rh) ) < sizeof(rh) ) return false;
    if( rh.id != 0x019CE23C ) return false;

    //qDebug() << QString( "ResHeader: id=%1 files=%2 footerOffset=%3 namesSize=%4" ).arg( rh.id ).arg( rh.fileCount ).arg( rh.footerOffset ).arg( rh.nameStringSize );

    // Read name string
    if( !file.seek( file.size() - rh.nameStringSize ) ) return false;
    QByteArray nameString = file.read( rh.nameStringSize );
    if( nameString.size() < (int) rh.nameStringSize ) return false;

    //qDebug() << "NameString:";
    //qDebug() << nameString;

    // Build file list
    if( !file.seek( rh.footerOffset ) ) return false;
    p_fileList.setRowCount( rh.fileCount );
    for( int i = 0; i < (int) rh.fileCount; ++i )
    {
        ResFileInfo rfi;
        if( file.read( (char*) &rfi, sizeof(rfi) ) < sizeof(rfi) ) return false;

        //qDebug() << QString( "Item(%6): id=%1 offset=%2 size=%3 nameOffset=%4 nameSize=%5" ).arg(rfi.id).arg(rfi.offset).arg(rfi.size).arg(rfi.nameOffset).arg(rfi.nameSize).arg((int)sizeof(rfi));

        QByteArray nameData = nameString.mid( (int) rfi.nameOffset, (int) rfi.nameSize );
        //qDebug() << "NameData: " << nameData;
        QString name = QString::fromLocal8Bit( nameData.data(), nameData.size() );
        p_fileList.set( i, FName, name );
        p_fileList.set( i, FOffset, (uint) rfi.offset );
        p_fileList.set( i, FSize, (uint) rfi.size );
        //qDebug() << name << " [ at " << rfi.offset << " with size " << rfi.size << " ]";
    }

    file.close();
    return true;
}

QList<QString> QResReader::files()
{
    QList<QString> result;
    int count = p_fileList.rowCount();
    for( int i = 0; i < count; ++i )
        result << p_fileList.at( i, FName ).toString();
    return result;
}

QByteArray QResReader::readFile( QString fileName )
{
    QByteArray result;
    int fi = p_fileList.column( FName ).cells().indexOf( fileName );
    if( fi < 0 ) return result;
    quint32 offset = QtGenExt::variantToInt( p_fileList.at( fi, FOffset ), -1 );
    quint32 size = QtGenExt::variantToInt( p_fileList.at( fi, FSize ), -1 );
    if( offset < 0 || size < 0 ) return result;

    QFile file( p_fileName );
    if( !file.open( QFile::ReadOnly ) ) return result;
    //qDebug() << "Offset " << offset << " size " << size;
    if( !file.seek( offset ) ) return result;

    result = file.read( size );

    return result;
}

void QResReader::extractAll( QString targetPath )
{
    while( targetPath.endsWith("/") ) targetPath.remove( targetPath.size()-1, 1 );
    QDir tp( targetPath );
    if( !tp.mkpath( targetPath ) ) return;

    QStringList fileNames = files();
    foreach( QString fileName, fileNames )
    {
        //qDebug() << ( targetPath + "/" + fileName );
        QFileInfo fi( targetPath + "/" + fileName );
        QString absPath = fi.absolutePath();
        if( !tp.mkpath( absPath ) ) continue;

        QFile file( fi.absoluteFilePath() );
        if( !file.open( QFile::WriteOnly ) ) continue;

        file.write( readFile( fileName ) );
    }
}

void QResReader::unpack( QString resFileName, QString targetPath )
{
    QResReader resFile( resFileName );
    resFile.extractAll( targetPath );
}
