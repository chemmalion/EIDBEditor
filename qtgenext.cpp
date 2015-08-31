#include "qtgenext.h"

QtGenExt::QtGenExt()
{
}

QtGenExt::~QtGenExt()
{
}

double QtGenExt::variantToDouble( QVariant variant, double defValue )
{
    bool ok = false;
    double result = variant.toDouble( &ok );
    return ( ok ? result : defValue );
}

float QtGenExt::variantToFloat( QVariant variant, float defValue )
{
    bool ok = false;
    float result = variant.toFloat( &ok );
    return ( ok ? result : defValue );
}

int QtGenExt::variantToInt( QVariant variant, int defValue )
{
    bool ok = false;
    int result = variant.toInt( &ok );
    return ( ok ? result : defValue );
}

qlonglong QtGenExt::variantToLongLong( QVariant variant, qlonglong defValue )
{
    bool ok = false;
    qlonglong result = variant.toLongLong( &ok );
    return ( ok ? result : defValue );
}

qreal QtGenExt::variantToReal( QVariant variant, qreal defValue )
{
    bool ok = false;
    qreal result = variant.toReal( &ok );
    return ( ok ? result : defValue );
}

uint QtGenExt::variantToUInt( QVariant variant, uint defValue )
{
    bool ok = false;
    uint result = variant.toUInt( &ok );
    return ( ok ? result : defValue );
}

qulonglong QtGenExt::variantToULongLong( QVariant variant, qulonglong defValue )
{
    bool ok = false;
    qulonglong result = variant.toULongLong( &ok );
    return ( ok ? result : defValue );
}

quintptr QtGenExt::variantToUIntPtr( QVariant variant, quintptr defValue )
{
    bool ok = false;
    qulonglong subResult = variant.toULongLong( &ok );
    quintptr result;
    if( ok ) result = (quintptr) subResult;
    else result = defValue;
    return result;
}

QVariant QtGenExt::uintptrToVariant( quintptr value )
{
    qulonglong subResult = (qulonglong) value;
    QVariant result(subResult);
    return result;
}

int QtGenExt::stringToInt( QString string, int defValue )
{
    bool ok = false;
    int result = string.toInt( &ok );
    return ( ok ? result : defValue );
}

//template <class T> QList<T> QtGenExt::getIntersection( QList<T> one, QList<T> two )
//template <class T> QList<T> QtGenExt::getDifference( QList<T> one, QList<T> two )
//template <class T> QList<T> QtGenExt::getSubstraction( QList<T> from, QList<T> what )

QString QtGenExt::compileQuery( QStringList listSelect, QStringList listFrom, QStringList listWhere )
{
    QString queryString;
    for( int i = 0; i < listSelect.size(); ++i )
    {
        if( i == 0 ) queryString += "SELECT ";
        queryString += listSelect[i];
        if( i < (listSelect.size()-1) ) queryString += ", ";
    }
    for( int i = 0; i < listFrom.size(); ++i )
    {
        if( i == 0 ) queryString += " FROM ";
        queryString += listFrom[i];
        if( i < (listFrom.size()-1) ) queryString += ", ";
    }
    for( int i = 0; i < listWhere.size(); ++i )
    {
        if( i == 0 ) queryString += " WHERE ";
        queryString += listWhere[i];
        if( i < (listWhere.size()-1) ) queryString += " AND ";
    }
    return queryString;
}

QString QtGenExt::compileIntersectListQuery( QString compiledQuery, QStringList listIntersectWhere )
{
    if( listIntersectWhere.size() )
    {
        QString interString;
        for( int i = 0; i < listIntersectWhere.size(); ++i )
        {
            QString prepared = compiledQuery;
            prepared.replace( "##intersectedWhere##", listIntersectWhere[i] );
            interString += prepared;
            if( i < (listIntersectWhere.size()-1) ) interString += " INTERSECT ";
        }
        compiledQuery = interString;
    }
    return compiledQuery;
}

void QtGenExt::removeDir( QString dirPath )
{
    QDir dir( dirPath );
    if( !dir.exists() ) return;
    clearDir( dirPath );
    dir.rmdir( dirPath );
}

void QtGenExt::clearDir( QString dirPath )
{
    QDir dir( dirPath );
    if( !dir.exists() ) return;
    dir.setFilter( QDir::AllEntries | QDir::Hidden | QDir::System | QDir::NoDotAndDotDot );
    QFileInfoList fileList = dir.entryInfoList();
    foreach( QFileInfo file, fileList )
    {
        if( file.isDir() ) removeDir( file.absoluteFilePath() );
        else dir.remove( file.absoluteFilePath() );
    }
}

void QtGenExt::makeEmptyDir( QString dirPath )
{
    QDir dir( dirPath );
    if( !dir.exists() ) dir.mkpath( dirPath );
    else QtGenExt::clearDir( dirPath );
}

QString QtGenExt::getFileBasename( QString filepath )
{
    QFileInfo fileInfo( filepath );
    return fileInfo.baseName();
}

QString QtGenExt::getFileCompleteBasename( QString filepath )
{
    QFileInfo fileInfo( filepath );
    return fileInfo.completeBaseName();
}

QString QtGenExt::getFileDirPath( QString filepath )
{
    QFileInfo fileInfo( filepath );
    return fileInfo.path();
}

QString QtGenExt::getFileCanonicalDirPath( QString filepath )
{
    if( QFile::exists( filepath ) )
    {
        QFileInfo fileInfo( filepath );
        return fileInfo.canonicalPath();
    }
    else
    {
        filepath.replace( "\\", "/" );
        QStringList path = filepath.split("/");
        if( path.size() > 0 ) path.removeLast();
        QString result = path.join( "/" );
        return result;
    }
}

QString QtGenExt::getCanonicalDirPath( QString dirpath )
{
    QDir dir( dirpath );
    return dir.canonicalPath();
}

// ALPHA ("a" to "z" and "A" to "Z") / DIGIT (0 to 9) / "-" / "." / "_" / "~"
QString QtGenExt::encodeCharsForHtml( QString text )
{
    QString result;
    int textSize = text.size();
    for( int i = 0; i < textSize; ++i )
    {
        QChar chr = text[i];
        if( ( chr >= 'a' && chr >= 'z' ) ||
            ( chr >= 'A' && chr >= 'Z' ) ||
            ( chr >= '0' && chr >= '9' ) ||
            ( chr == '-' ) ||
            ( chr == '.' ) ||
            ( chr == '_' ) ||
            ( chr == '~' ) ||
            ( chr == ' ' )
          )
//        if( chr.unicode() >= 0x20 && chr.unicode() < 0x7F )
        {
            result += chr;
            continue;
        }
        QString encoded = QString( "&#x%1;" ).arg( chr.unicode(), 0, 16 );
        result += encoded;
    }
    return result;
}

QString QtGenExt::decodeCharsFromHtml( QString html )
{
    return html;
}

QString QtGenExt::getRussianNumberedWord( int number, QString single, QString pare, QString multiple )
{
    enum wordtypes{ single_word = 1, pare_word = 2, multiple_word = 3 };
    int word;
    number = number % 100; // russian fonetic variations use only two small digits
    if( number >= 11 && number <= 19 ) word = multiple_word;
    else
    {
        number = number % 10; // in many cases only last digit means
        if( number == 1 ) word = single_word;
        else if( number >= 2 && number <= 4 ) word = pare_word;
        else word = multiple_word;
    }
    switch( word )
    {
    case single_word:   return single;
    case pare_word:     return pare;
    default:
    case multiple_word: return multiple;
    }
}

QIcon QtGenExt::uniteIcons( const QIcon& icon1, const QIcon& icon2 )
{
    // Best regards to :
    // 1)
    // trdm (http://www.rsdn.ru/Users/34191.aspx)
    // source info: http://www.rsdn.ru/forum/cpp.applied/3495972.flat.aspx

    // Size
    QRect rect = QRect( 0, 0, 16, 16 );
    QRect rect1 = QRect( QPoint(0,0), icon1.availableSizes().value( 0, QSize(16,16) ) );
    QRect rect2 = QRect( QPoint(0,0), icon2.availableSizes().value( 0, QSize(16,16) ) );
    rect = rect.united( rect1 ).united( rect2 );
    QSize size = rect.size();

    QImage img = QImage( size, QImage::Format_ARGB32 );
    img.fill( 0 );
    QPainter painter(&img);

    painter.setCompositionMode(QPainter::CompositionMode_SourceOver);
    painter.setRenderHint(QPainter::Antialiasing);
    painter.setRenderHint(QPainter::HighQualityAntialiasing);

    painter.drawPixmap(0, 0, icon1.pixmap(size));
    painter.drawPixmap(0, 0, icon2.pixmap(size));

    return QIcon( QPixmap::fromImage(img) );
}
