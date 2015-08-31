#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QtCore>
#include <QtGui>

#include "cdialogabout.h"
#include "cprogressdialog.h"

#include "qtzipreader.h"
#include "qtable.h"
#include "qspreadsheetdocument.h"
#include "qfilecontainer.h"

#include "eidbstructure.h"
#include "eidbfile.h"
#include "eidbmodel.h"

#include "qtableviewex.h"

namespace Ui {
    class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

    bool load( QList<QString> files = QList<QString>() );
    bool save();
    bool saveAsDatabaseRes( QString fileName = QString() );
    bool saveAsDatabaseXlsx( QString fileName = QString(), bool silent = false );
    bool saveAsDatabaseDBs( QString targetPath = QString() );

    bool isFileRes( QString fileName );
    bool importFileRes( QString fileName );

    bool isFileXlsx( QString fileName );
    bool importFileXlsx( QString fileName, bool silent = false );

    bool isFileDB( QString fileName );
    bool importFileDB( QString fileName );

    bool realyDiscardChanges();
    void setNoChanges();
    void setNoCleared();

    void setEIDBModelHeaders( EIDBModel & model, QTable * headers );
//    void setViewAutoWidthForHeaders( QTableViewEx * view );
    QTableViewEx * getCurrentTableView();
    EIDBModel * getCurrentEIDBModel();
    EIDBModel * getEIDBModel( int fileID, int blockID );
    EIDBBlock getEIDBBlock( int fileID, int blockID );
    EIDBBlock getEIDBBlock( QString fileName, QString blockName );

    void prepareTempDir();

    bool importDBInits();
    void clearBlock( int fileID, int blockID );
    void clearBlocks();

public slots:
    void qTableViewKostyl();
    void haveDataChanged();
    bool doQuit();
    void updateState();
    void updateNavigation();
    void updateLastLoads();

    void loadSettings();
    void loadSettingsForLastLoads();
    void saveSettings();
    void saveSettingsForLastLoads();

    void longQuit();
    void forceQuit();

    void myOn_mainTabChanged( int index );
    void myOn_subTabChanged( int index );
    void myOn_viewClicked( QModelIndex index );
    void myOn_viewKeyPressed( QKeyEvent * event );
    void myOn_changeClipboard( QClipboard::Mode mode );

protected:
    void closeEvent(QCloseEvent *event);

signals:
    void doneSaving();
    void doneLoading();

private:
    void addLastLoads( QStringList loadList );

private slots:
    void myOn_recentLoad();

private slots:
    void on_actionClear_triggered();
    void on_actionLoad_triggered();
    void on_actionSave_triggered();
    void on_actionSaveAsDatabaseRes_triggered();
    void on_actionSaveAsDatabaseXlsx_triggered();
    void on_actionSaveAsDatabaseDBFiles_triggered();
    void on_actionExit_triggered();
    void on_actionAbout_triggered();
    void on_actionInsertRow_triggered();
    void on_actionAppendRow_triggered();
    void on_actionRemoveRow_triggered();
    void on_actionMoveRowUp_triggered();
    void on_actionMoveRowDown_triggered();
    void on_actionCopy_triggered();
    void on_actionPaste_triggered();

private:
    Ui::MainWindow *ui;

    EIDBStructure eist;

    QTabWidget * p_tabWidget;
    QList<QTabWidget*> p_blockTabWidgets;
    int p_currentTab;
    int p_currentSubTab;
    QHash< QPair<int,int>, QTableViewEx* > p_tableViewList;
    QHash< QPair<int,int>, EIDBModel* > p_modelList;

    CDialogAbout p_about;
    CProgressDialog p_progress;

    bool p_changed;
    bool p_cleared;
    QString p_resFileName;
    QFileContainer p_fileContainer;

    // Settings
    QString p_loadPath;
    QString p_savePath;
    QString p_codePage;
    QList<QStringList> p_lastLoads;

    QList<QAction*> p_llActions;

    QString p_tempDir;
};

#endif // MAINWINDOW_H
