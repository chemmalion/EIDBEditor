#ifndef QTGENEXT_H
#define QTGENEXT_H

#include <QtCore>
#include <QtGui>

class QtGenExt
{

public:

    // This Variant "overloads" replace "bool *ok" to default values
    static double variantToDouble( QVariant variant, double defValue = 0.0 );
    static float variantToFloat( QVariant variant, float defValue = 0.0 );
    static int variantToInt( QVariant variant, int defValue = 0 );
    static qlonglong variantToLongLong( QVariant variant, qlonglong defValue = 0 );
    static qreal variantToReal( QVariant variant, qreal defValue = 0.0 );
    static uint variantToUInt( QVariant variant, uint defValue = 0 );
    static qulonglong variantToULongLong( QVariant variant, qulonglong defValue = 0 );
    static quintptr variantToUIntPtr( QVariant variant, quintptr defValue = 0 );

    static QVariant uintptrToVariant( quintptr value );

    // This String "overloads" replace "bool *ok" to default values
    static int stringToInt( QString string, int defValue = 0 );

    // This is additional functions to work with lists
    template <class T> static QList<T> getIntersection( QList<T> one, QList<T> two )
    {
        QList<T> min, max, inter;
        if( one.size() < two.size() ) { min = one; max = two; }
        else { min = two; max = one; }
        for( int i = 0; i < min.size(); ++i )
            if( max.contains(min[i]) ) inter << min[i];
        return inter;
    }
    template <class T> static QList<T> getDifference( QList<T> one, QList<T> two )
    {
        QList<T> diff;
        for( int i = 0; i < one.size(); ++i )
            if( !two.contains(one[i]) ) diff << one[i];
        for( int i = 0; i < two.size(); ++i )
            if( !one.contains(two[i]) ) diff << two[i];
        return diff;
    }
    template <class T> static QList<T> getSubstraction( QList<T> from, QList<T> what )
    {
        for( int i = 0; i < what.size(); ++i )
            if( from.contains(what[i]) ) from.removeAll( what[i] );
        return from;
    }

    // Functions to compile sql queries
    static QString compileQuery( QStringList listSelect, QStringList listFrom, QStringList listWhere );
    static QString compileIntersectListQuery( QString compiledQuery, QStringList listIntersectWhere );

    // File and dir functions
    static void removeDir( QString dirPath );
    static void clearDir( QString dirPath );
    static void makeEmptyDir( QString dirPath );

    // Functions to deal with file name strings
    static QString getFileBasename( QString filepath );
    static QString getFileCompleteBasename( QString filepath );
    static QString getFileDirPath( QString filepath );
    static QString getFileCanonicalDirPath( QString filepath );
    static QString getCanonicalDirPath( QString dirpath );

    // HTML
    static QString encodeCharsForHtml( QString text );
    static QString decodeCharsFromHtml( QString html );

    // Russian functions :)
    static QString getRussianNumberedWord( int number, QString single, QString pare, QString multiple );

    // Graphics
    static QIcon uniteIcons( const QIcon& icon1, const QIcon& icon2 );

private:
    QtGenExt();
    virtual ~QtGenExt();

};

#endif // QTGENEXT_H
