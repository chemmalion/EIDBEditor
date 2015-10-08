#include "qreswriter.h"

QResWriter::QResWriter(QObject *parent) :
    QObject(parent)
{
}

QResWriter::QResWriter( QString fileName, QObject *parent) :
    QObject(parent)
{
    setRes( fileName );
}

QResWriter::~QResWriter()
{
}

void QResWriter::setRes( QString fileName )
{
    p_resFileName = fileName;
}

void QResWriter::addFile( QString virtualFileName, QByteArray fileData )
{
    p_names.append( virtualFileName );
    p_files.append( fileData );
}

void QResWriter::addDir( QString dirPath, bool subDirs )
{
    QString path = dirPath;
    if( !path.endsWith("/") ) path.append( "/" );
    QDirIterator it( path,
                     QDir::Files | (subDirs?(QDir::Dirs):(QDir::NoFilter)) | QDir::NoDotAndDotDot,
                     (subDirs?(QDirIterator::Subdirectories):(QDirIterator::NoIteratorFlags)) );
    while (it.hasNext())
    {
        QString file_path = it.next();
        if (it.fileInfo().isDir()) continue;
        if (it.fileInfo().isFile())
        {
            QFile file(file_path);
            if (!file.open(QIODevice::ReadOnly))
                continue;

            QByteArray ba = file.readAll();
            addFile(file_path.remove(path), ba);
        }
    }
}

QList<QString> QResWriter::filesList()
{
    return p_names;
}

bool QResWriter::writeRes()
{
    QFile resFile( p_resFileName );
    if( !resFile.open( QFile::WriteOnly ) ) return false;

    // Write header
    ResHeader resHdr;
    resHdr.id = 0x019CE23C;
    resHdr.fileCount = p_names.size();
    resHdr.footerOffset = (int) sizeof(ResHeader);
    resHdr.nameStringSize = 0;
    resFile.write( (char*) &resHdr, (qint64) sizeof(ResHeader) );

    QByteArray nameString;

    // Write files
    QList<ResFileInfo> fileInfoList;
    for( int i = 0; i < p_files.size(); ++i )
    {
        // Make res file info entry
        ResFileInfo resInfo;
        resInfo.offset = resHdr.footerOffset;
        resInfo.size = p_files[i].size();
        resInfo.nameOffset = nameString.size();
        resInfo.nameSize = p_names[i].size();
        resInfo.nextIndex = -1;
        resInfo.time = 0;
        // Add name to namestring
        QByteArray nameData = p_names[i].toLocal8Bit();
        nameString.append( nameData );

        // Update header info
        resHdr.footerOffset += resInfo.size /*+ (16-((p_files[i].size())%16))*/;

        // Add file info to list
        fileInfoList.append( resInfo );

        // Write file data
        resFile.write( p_files[i] );
//        // Write align bytes
//        resFile.write( QByteArray( 16-((p_files[i].size())%16), 0 ) );
    }

    // Resort file entries list by alphabet hash
    fileInfoList = alphabetResort( fileInfoList, nameString );

    // Write file info list
    for( int i = 0; i < fileInfoList.size(); ++i )
    {
        ResFileInfo & resInfo = fileInfoList[i];
        resFile.write( (char*) &resInfo, sizeof(ResFileInfo) );
    }

    // Write namestring
    resFile.write( nameString );

    // Update res file header
    resHdr.nameStringSize = nameString.size();
    resFile.seek(0);
    resFile.write( (char*) &resHdr, (qint64) sizeof(ResHeader) );

    resFile.close();

    return true;
}

void QResWriter::clearRes()
{
    p_names.clear();
    p_files.clear();
}

bool QResWriter::packAll( QString dirPath, bool subDirs )
{
    clearRes();
    addDir( dirPath, subDirs );
    return writeRes();
}

bool QResWriter::packAll( QString resFileName, QString dirPath, bool subDirs )
{
    QResWriter res( resFileName );
    return res.packAll( dirPath, subDirs );
}

QList<QResWriter::ResFileInfo> QResWriter::alphabetResort( QList<ResFileInfo> sourceList, QByteArray nameString )
{
    // Preparing
    QList<ResFileInfo> result;
    for( int i = 0; i < sourceList.size(); ++i )
    {
        ResFileInfo info;
        info.offset = 0;
        info.nextIndex = -1;
        result.append( info );
    }
    // Sorting
    for( int i = 0; i < sourceList.size(); ++i )
    {
        int index = calcAlphabetHash( nameString.mid( sourceList[i].nameOffset, sourceList[i].nameSize ), sourceList.size() );
        if( result[index].offset > 0 )
        {
            while( result[index].nextIndex != -1 )
                index = result[index].nextIndex;
            int lastFreeIndex = sourceList.size()-1;;
            while( result[lastFreeIndex].offset > 0 )
                --lastFreeIndex;
            result[index].nextIndex = lastFreeIndex;
            index = lastFreeIndex;
        }
        result[index] = sourceList[i];
        result[index].nextIndex = -1;
    }
    return result;
}

int QResWriter::calcAlphabetHash( QByteArray data, int divider )
{
    int result = 0;
    for( int i = data.size()-1; i >=0; --i ) result += tolower(data[i]);
    return (result%divider);
}
