#ifndef QSPREADSHEETDOCUMENT_H
#define QSPREADSHEETDOCUMENT_H

#include <QtCore>
#include <QtXml>
#include <QtXmlPatterns>

#include "qtzipreader.h"
#include "qtzipwriter.h"
#include "qtable.h"

class QSpreadsheetDocument : public QObject
{
    Q_OBJECT
public:
    explicit QSpreadsheetDocument(QObject *parent = 0);
    virtual ~QSpreadsheetDocument();

    void clear();
    void setTempDir( QString tempDir );

    bool importSpreadsheet( QString fileName );
    bool exportSpreadsheet( QString fileName );

    QTable * addSheet( QString name );
    void insertSheet( int index, QString name );
    void removeSheet( int index );
    void removeSheet( QString name );
    int sheetIndex( QTable * sheet );
    QTable * sheet( int index );
    QTable * sheet( QString name );
    QString sheetName( int index );
    QString sheetName( QTable * sheet );
    QPair<int,int> sheetLeftTopCell( int index );
    void setSheetLeftTopCell( int index, QPair<int,int> ltCell );
    int sheetCount();

signals:

public slots:

private:
    QPair<int,int> parseCell( QString cell );
    QString encodeCell( QPair<int,int> cell );
    QString getSheetFile( int rid );
    void readSheet( QString sheetFileName, QTable * sheet, QPair<int,int> * ltCell );
    void readStringCache();
    void removeDir( QString dirPath );
    void clearDir( QString dirPath );
    int s2i( QString string, int defValue = 0 );
    double s2d( QString string, double defValue = 0.0 );
    bool fileCopy( QString oldFile, QString newFile );
    bool addXmlHeader( QString fileName );

    bool xl_makeContextTypes( QString fileName );
    bool xl_makeRels( QString fileName );
    bool xl_makeApp( QString fileName );
    bool xl_makeCore( QString fileName );
    bool xl_makeWorkbookXmlRels( QString fileName );
    bool xl_makeTheme( QString fileName );
    bool xl_makeStyles( QString fileName );
    bool xl_makeWorkbook( QString fileName );
    bool xl_makeSheets( QString sheetsPath, QString sharedStringsFileName );

private:
    QList<QTable*> p_sheetList;
    QList<QString> p_sheetNames;
    QList< QPair<int,int> > p_sheetsLTCell;

    QString p_tempDir;
    QString p_tempNameSpace;

    QList<QString> p_stringCache;
};

#endif // QSPREADSHEETDOCUMENT_H
