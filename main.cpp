#include <QtCore>
#include <QtGui>

#include "qtable.h"
#include "mainwindow.h"

int main(int argc, char *argv[])
{
    QTextCodec::setCodecForCStrings(QTextCodec::codecForName("UTF-8"));
    QTextCodec::setCodecForTr(QTextCodec::codecForName("UTF-8"));
    QTextCodec::setCodecForLocale(QTextCodec::codecForName("Windows-1251"));
    QApplication a(argc, argv);
    QApplication::setOrganizationName( "chemmalion" );
    QApplication::setOrganizationDomain( "http://chemmalion.info/" );
    QApplication::setApplicationName( "DBEditor" );

    MainWindow w;

    if( argc > 1 )
    {
        QString inFileName( QString::fromLocal8Bit( argv[1] ) );
        QFile inFile( inFileName );

        if( inFile.exists() )
        {
            // Get console settings
            QSettings settings( a.applicationDirPath() + "/settings.ini", QSettings::IniFormat );
            bool silent = settings.value( "Console/Silent" ).toBool();
            settings.setValue( "Console/Silent", QVariant(silent) );

            a.connect( &w, SIGNAL(doneSaving()), &w, SLOT(longQuit()) );
            if( w.isFileRes( inFileName ) )
            {
                QList<QString> loadList;
                loadList << inFileName;
                w.load( loadList );
                QFileInfo fi( inFileName );
                QString outFileName = fi.absolutePath() + "/" + fi.completeBaseName() + ".xlsx";
                w.saveAsDatabaseXlsx( outFileName, silent );
            }
            else if( w.isFileXlsx( inFileName ) )
            {
                QList<QString> loadList;
                loadList << inFileName;
                w.importFileXlsx( inFileName, silent );
                w.setNoChanges();
                w.setNoCleared();
                QFileInfo fi( inFileName );
                QString outFileName = fi.absolutePath() + "/" + fi.completeBaseName() + ".res";
                w.saveAsDatabaseRes( outFileName );
            }
            QtGenExt::removeDir( QApplication::applicationDirPath() + "/temp" );
        }
        else
        {
            // Quit with no work
            QTimer::singleShot( 500, &a, SLOT(quit()) );
        }
    }
    else
    {
        w.show();
    }

    return a.exec();
}
