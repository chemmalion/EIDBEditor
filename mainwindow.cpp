#include "mainwindow.h"
#include "ui_mainwindow.h"

#include "qresreader.h"
#include "qreswriter.h"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    // Main
    ui->setupUi(this);
    p_cleared = true;
    p_changed = false;
    p_currentTab = 0;
    p_currentSubTab = 0;
    connect( QApplication::clipboard(), SIGNAL(changed(QClipboard::Mode)), this, SLOT(myOn_changeClipboard(QClipboard::Mode)) );
    // Load settings
    loadSettings();
    // Temp directory
    p_tempDir = QApplication::applicationDirPath() + "/temp";
    //prepareTempDir();
    // EIDB Structure configuring
    eist.parseDBFiles( QApplication::applicationDirPath() + "/dbfiles.txt" );
    eist.parseDBBlocks( QApplication::applicationDirPath() + "/dbblocks.txt" );
    eist.parseDBTypes( QApplication::applicationDirPath() + "/dbtypes.txt" );
    eist.parseDBHeaders( QApplication::applicationDirPath() + "/dbheaders.txt" );
    // Views configuring
    p_tableViewList.clear();
    p_modelList.clear();
    p_tabWidget = 0;
    if( eist.dbFiles.rowCount() > 0 )
    {
        QGridLayout * mainLayout = new QGridLayout();
        p_tabWidget = new QTabWidget( ui->centralWidget );
        mainLayout->addWidget( p_tabWidget );
        ui->centralWidget->setLayout( mainLayout );
        connect( p_tabWidget, SIGNAL(currentChanged(int)), this, SLOT(myOn_mainTabChanged(int)) );
        for( int i = 0; i < eist.dbFiles.rowCount(); ++i )
        {
            QWidget * widget = new QWidget( p_tabWidget );
            QString title = eist.dbFiles.at( i, EIDB::FTTitle ).toString();
            QString desc = eist.dbFiles.at( i, EIDB::FTDesc ).toString();
            QString iconString = eist.dbFiles.at( i, EIDB::FTIcon ).toString();
            iconString.replace( "%APP%", QApplication::applicationDirPath() );
            QIcon icon = QIcon( iconString );
            int fileID = QtGenExt::variantToInt( eist.dbFiles.at(i,EIDB::FTID),0 );
            widget->setObjectName( QString( "dFile_%1" ).arg(fileID) );
            p_tabWidget->addTab( widget, icon, title );
            p_tabWidget->setTabToolTip( p_tabWidget->count()-1, desc );
            // Make sub tabs for blocks
            QTable * blocks = EIDBFile::valueToLink( QtGenExt::variantToUIntPtr( eist.dbFiles.at( i, EIDB::FTBlocksLink ) ) );
            if( blocks )
            {
                QGridLayout * subLayout = new QGridLayout();
                QTabWidget * subTabWidget = new QTabWidget( widget );
                p_blockTabWidgets << subTabWidget;
                subTabWidget->setObjectName( QString( "dFileTab_%1" ).arg( fileID ) );
                subLayout->addWidget( subTabWidget );
                widget->setLayout( subLayout );
                connect( subTabWidget, SIGNAL(currentChanged(int)), this, SLOT(myOn_subTabChanged(int)) );
                for( int j = 0; j < blocks->rowCount(); ++j )
                {
                    int blockID = QtGenExt::variantToInt( blocks->at( j, EIDB::BTID ) );
                    if( blockID >= 20 ) continue;
                    QGridLayout * deepSubLayout = new QGridLayout();
                    QWidget * subWidget = new QWidget( subTabWidget );
                    subWidget->setLayout( deepSubLayout );
                    QString subTitle = blocks->at( j, EIDB::BTTitle ).toString();
                    QString subDesc = blocks->at( j, EIDB::BTDesc ).toString();
                    QString subIconString = blocks->at( j, EIDB::BTIcon ).toString();
                    subIconString.replace( "%APP%", QApplication::applicationDirPath() );
                    QIcon subIcon = QIcon( subIconString );
                    subWidget->setObjectName( QString( "dBlock_%1_%2" ).arg(fileID).arg(blockID) );
                    subTabWidget->addTab( subWidget, subIcon, subTitle );
                    subTabWidget->setTabToolTip( subTabWidget->count()-1, subDesc );
                    // Make id
                    QPair<int,int> id;
                    id.first = fileID;
                    id.second = blockID;
                    // Make table model
                    EIDBModel * model = new EIDBModel( subWidget );
                    connect( model, SIGNAL(dataChanged(QModelIndex,QModelIndex)), this, SLOT(haveDataChanged()) );
                    model->setObjectName( QString( "dModel_%1_%2" ).arg(fileID).arg(blockID) );
                    model->setFileID( fileID );
                    model->setBlockID( blockID );
                    p_modelList[id] = model;
                    // Make table viewer
                    QTableViewEx * tableView = new QTableViewEx( model, subWidget );
                    deepSubLayout->addWidget( tableView );
                    tableView->setObjectName( QString( "dView_%1_%2" ).arg(fileID).arg(blockID) );
                    p_tableViewList[id] = tableView;
                    connect( tableView, SIGNAL(clicked(QModelIndex)), this, SLOT(myOn_viewClicked(QModelIndex)) );
                    connect( tableView, SIGNAL(afterKeyPressed(QKeyEvent*)), this, SLOT(myOn_viewKeyPressed(QKeyEvent*)) );
                    // Link viewer and model
                    //tableView->setModel( model );
                }
            }
        }
    }
    // Import db inits
    importDBInits();
    // Update state
    p_changed = false;
    updateState();
    updateLastLoads();
}

MainWindow::~MainWindow()
{
    delete ui;
}

bool MainWindow::load( QList<QString> files )
{
    if( !realyDiscardChanges() ) return false;
    // Get name list if it empty
    if( files.size() < 1 )
    {
        // Get name list
        files = QFileDialog::getOpenFileNames( this, "Open Evil Islands database files", p_loadPath, "Evil Islands Database (database*.res);;Evil Islands Excel Database (database*.xlsx);;Evil Islands DB files (*.*db);;All files (*.*)" );
        if( files.size() < 1 ) return true; // Nothing to load, so nothing to error :)
        // Remember load path
        p_loadPath = QDir( QFileInfo(files[0]).path() ).path();
        // Add to last loads
        addLastLoads( files );
    }
    // Parse files
    bool ok = true;
    for( int i = 0; i < files.size(); ++i )
    {
        QString fileName = files[i];
        // Res file
        if( isFileRes( fileName ) )
        {
            if( !importFileRes( fileName ) ) ok = false;
            continue;
        }
        // Xlsx file
        if( isFileXlsx( fileName ) )
        {
            if( !importFileXlsx( fileName ) ) ok = false;
            continue;
        }
        // DB file
        if( !importFileDB( fileName ) ) ok = false;
    }
    // Break the changes flag
    p_changed = false;
    if( ok ) p_cleared = false;
    return ok;
}

bool MainWindow::save()
{
    bool ok = saveAsDatabaseRes( p_resFileName );
    if( ok )
    {
        // Break the changes flag
        p_changed = false;
    }
    return ok;
}

bool MainWindow::saveAsDatabaseRes( QString fileName )
{
    // Get name for res if it empty
    if( fileName.isEmpty() )
    {
        fileName = QFileDialog::getSaveFileName( this, "Save Evil Islands Database file", p_savePath, "Evil Islands Database (database*.res);;All files (*.*)" );
        if( fileName.isEmpty() ) return false;
        // Remember save path
        p_savePath = QDir( QFileInfo(fileName).path() ).path();
    }
    // Prepare res db folder
    QtGenExt::makeEmptyDir( p_tempDir + "/res" );
    if( !p_resFileName.isEmpty() ) // Unpack source res files if we have it
        p_fileContainer.writeDir( p_tempDir + "/res" );
    else if( QFile::exists( fileName ) ) // Unpack target res files if no source and rewrite target
        QResReader::unpack( fileName, p_tempDir + "/res" );
    // Prepare res db files
    if( !saveAsDatabaseDBs( p_tempDir + "/res" ) ) return false;
    // Pack res archive
    bool ok = true;
    if( !QResWriter::packAll( fileName, p_tempDir + "/res", true ) ) ok = false;
    // Remove blood prints
    QtGenExt::removeDir( p_tempDir + "/res" );
    // The End
    emit doneSaving();
    return ok;
}

bool MainWindow::saveAsDatabaseXlsx( QString fileName, bool silent )
{
    // Get name for xlsx if it empty
    if( fileName.isEmpty() )
    {
        fileName = QFileDialog::getSaveFileName( this, "Save Evil Islands Excel Database file", p_savePath, "Evil Islands Excel Database (database*.xlsx);;All files (*.*)" );
        if( fileName.isEmpty() ) return false;
        // Remember save path
        p_savePath = QDir( QFileInfo(fileName).path() ).path();
    }
    // Saving to xlsx
    QSpreadsheetDocument spd;
    QtGenExt::makeEmptyDir( p_tempDir + "/xlsx" );
    spd.setTempDir( p_tempDir + "/xlsx" );

    // Get save sequence
    QList<int> fileIDList = eist.filesSaveSequence();

    // Show progress bar
    int stepCounter = 0;
    if( !silent )
    {
        // Count all rows
        int allsteps = 1;
        for( int fi = 0; fi < fileIDList.size(); ++fi )
        {
            int fileID = fileIDList[fi];
            QList<int> blockIDList = eist.blocksSaveSequence( fileID );
            for( int bi = 0; bi < blockIDList.size(); ++bi )
            {
                int blockID = blockIDList[bi];
                // Add sheet
                EIDBModel * model = getEIDBModel( fileID, blockID );
                if( !model ) continue;
                allsteps += model->rowCount() * model->columnCount();
            }
        }
        // Set progress maximum
        p_progress.setProgressMaximum( allsteps );
        // Show progress
        p_progress.setProgress( 0 );
        p_progress.setLabel( "Saving..." );
        p_progress.show();
    }

    // Make sheets
    for( int fi = 0; fi < fileIDList.size(); ++fi )
    {
        int fileID = fileIDList[fi];
        QList<int> blockIDList = eist.blocksSaveSequence( fileID );
        for( int bi = 0; bi < blockIDList.size(); ++bi )
        {
            int blockID = blockIDList[bi];
            // Add sheet
            EIDBModel * model = getEIDBModel( fileID, blockID );
            if( !model ) continue;
            if( model->rowCount() < 1 ) continue;
            QTable * sh = spd.addSheet( eist.blockName( fileID, blockID ) );
            if( !sh ) continue;
            // Make sheet header
            spd.setSheetLeftTopCell( spd.sheetIndex(sh), QPair<int,int>(1,1) );
            sh->setColumnCount( model->columnCount() );
            sh->setRowCount( 2 );
            for( int c = 0; c < sh->columnCount(); ++c )
            {
                sh->set( 0, c, model->headerData( c, Qt::Horizontal ) );
                sh->set( 1, c, QString("FLD%1-%2").arg( model->fieldID(c) ).arg( model->fieldIndex(c) ) );
            }
            // Make sheet data
            sh->appendRows( model->rowCount() );
            for( int r = 0; r < model->rowCount(); ++r )
            {
                // Export data
                for( int c = 0; c < model->columnCount(); ++c )
                    sh->set( r+2, c, model->data( model->index( r, c ) ) );
                // Update the progress
                stepCounter += model->columnCount();
                if( !silent ) p_progress.setProgress( stepCounter );
            }
        }
    }
    // Write xlsx
    spd.exportSpreadsheet( fileName );

    // Remove blood prints
    QtGenExt::removeDir( p_tempDir + "/xlsx" );

    // Hide progress
    if( !silent ) p_progress.hide();

    emit doneSaving();
    return true;
}

bool MainWindow::saveAsDatabaseDBs( QString targetPath )
{
    // Get target path for DB files if it empty
    if( targetPath.isEmpty() )
    {
        targetPath = QFileDialog::getExistingDirectory( this, "Save Evil Islands DB files", p_savePath );
        if( targetPath.isEmpty() ) return false;
        // Remember save path
        p_savePath = targetPath;
    }
    // Some path corrections
    while( targetPath.endsWith("/") ) targetPath.remove( targetPath.size()-1, 1 );
    // Saving DB files
    QList<EIDBBlock> blocks;
    EIDBFile eifile;
    eifile.setCodePage( p_codePage );
    QList<int> fileIDList = eist.filesSaveSequence();
    for( int i = 0; i < fileIDList.size(); ++i )
    {
        int fileID = fileIDList[i];
        blocks.clear();
        QList<int> blocksList = eist.blocksSaveSequence( fileID );
        bool haveInfo = false;
        for( int j = 0; j < blocksList.size(); ++j )
        {
            EIDBBlock blk = getEIDBBlock(fileID,blocksList[j]);
            if( blk.content().size() > 0 ) haveInfo = true;
            blocks << blk;
        }
        if( !haveInfo ) continue;
        eifile.setBlocks( blocks );
        eifile.setId(1);

        int fi = eist.fileIndex(fileID);
        QString fileName = eist.dbFiles.at( fi, EIDB::FTName ).toString();
        QString fileMask = eist.dbFiles.at( fi, EIDB::FTMask ).toString();
        QStringList list = fileMask.split('.');
        if( list.size() < 1 ) list << "xdb";
        QString suffix = list.last();
        fileName = targetPath + QString("/") + fileName + QString(".") + suffix;
        fileName = fileName.toLower();

        eifile.writeFile( fileName, eist.blocksOfFile( fileID ) );
    }
    // The End
    emit doneSaving();
    return true;
}

bool MainWindow::isFileRes( QString fileName )
{
    QRegExp rx( "*.res" );
    rx.setPatternSyntax(QRegExp::Wildcard);
    QFileInfo fi( fileName );
    return rx.exactMatch(fi.fileName());
}

bool MainWindow::importFileRes( QString fileName )
{
    // Find suitable directory for extraction
    QString unpath = QString("%1/resunp").arg(p_tempDir);
    for( int resFolderCounter = 0; QFileInfo( unpath = QString("%1/resunp%2").arg(p_tempDir).arg(resFolderCounter) ).exists(); ++resFolderCounter );
    // Extract res archive
    QResReader res( fileName );
    res.extractAll( unpath );
    // Import db files
    QDir dir( unpath );
    QStringList dbList = dir.entryList( QStringList() << "*.*db" );
    for( int i = 0; i < dbList.size(); ++i )
        importFileDB( unpath + "/" + dbList[i] );
    // Save all files to support unsupported files
    p_fileContainer.clear();
    p_fileContainer.readDir( unpath );
    // Remove blood prints
    QtGenExt::removeDir( unpath );
    // Save res file name for Ctrl+S function
    p_resFileName = fileName;
    // The End
    emit doneLoading();
    return true;
}

bool MainWindow::isFileXlsx( QString fileName )
{
    QRegExp rx( "*.xlsx" );
    rx.setPatternSyntax(QRegExp::Wildcard);
    QFileInfo fi( fileName );
    return rx.exactMatch(fi.fileName());
}

bool MainWindow::importFileXlsx( QString fileName, bool silent )
{
    // Read the spreadsheet
    QSpreadsheetDocument spd;
    spd.setTempDir( p_tempDir + "/xlsx" );
    if( !spd.importSpreadsheet( fileName ) ) return false;
    QtGenExt::removeDir( p_tempDir + "/xlsx" );

    // Import sheets

    // * Progress bar support
    int maximum = 1;
    int progressCounter = 0;
    if( !silent )
    {
        // * Get maximum for progress
        for( int i = 0; i < spd.sheetCount(); ++i )
        {
            QTable * sheet = spd.sheet(i);
            if( !sheet ) continue;
            maximum += sheet->rowCount();
        }
        // * Show progress
        p_progress.setProgressMaximum( maximum );
        p_progress.setProgress(0);
        p_progress.setLabel("Preparing...");
        p_progress.goShow(this);
    }

    // * Main code
    for( int i = 0; i < spd.sheetCount(); ++i )
    {
        QTable * sheet = spd.sheet(i);
        if( !sheet ) continue;
        // Get block id
        int blockID = -1;
        int fileID = -1;
        QString sheetName = spd.sheetName(i);
        if( !silent ) p_progress.setLabel( sheetName );
        if( sheetName.startsWith( "BL" ) )
        {
            sheetName = sheetName.mid( 2 );
            QStringList data = sheetName.split( "-" );
            if( data.size()==2 )
            {
                fileID = QtGenExt::stringToInt( data[0], -1 );
                blockID = QtGenExt::stringToInt( data[1], -1 );
            }
        }
        else
        {
            QPair<int,int> info = eist.blockForName( sheetName );
            fileID = info.first;
            blockID = info.second;
        }
        //qDebug() << QString( "%1: f%2 b%3" ).arg( sheetName ).arg( fileID ).arg( blockID );
        if( ( fileID < 0 ) || ( blockID < 0 ) ) continue;
        // Build column map
        QHash< int, QPair<int,int> > colMap;
        // * Find indentification row
        int idr = -1;
        for( int r = 0; r < sheet->rowCount(); ++r )
        {
            for( int c = 0; c < sheet->columnCount(); ++c )
           {
                QString data = sheet->at( r, c ).toString();
                //qDebug() << QString("%1x%2 : %3").arg(r).arg(c).arg(data);
                // * building map
                if( data.startsWith("FLD") )
                {
                    //qDebug()<<"Found!";
                    if( idr < 0 ) idr = r;
                    QString inf = data.mid( 3 );
                    QStringList field = inf.split( "-" );
                    int fID = 0;
                    int fIndex = 0;
                    if( field.size()>=1 )fID = QtGenExt::stringToInt( field[0], 0 );
                    if( field.size()>=2 )fIndex = QtGenExt::stringToInt( field[1], 0 );
                    colMap[c] = QPair<int,int>(fID,fIndex);
                }
            }
            if( idr >=0 ) break;
        }
        if( idr < 0 ) return false;
        // Get data
        clearBlock( fileID, blockID );
        EIDBModel * model = getEIDBModel( fileID, blockID );
        if( !model ) continue;
        model->insertRows( 0, sheet->rowCount()-(idr+1) );
        for( int r = idr+1; r < sheet->rowCount(); ++r )
        {
            //qDebug() << sheet->row(r).cells();
            //model->insertRow( r-idr-1 );
            bool haveRowData = false;
            QHashIterator< int, QPair<int,int> > qhi(colMap);
            while( qhi.hasNext() )
            {
                qhi.next();
                QPair<int,int> field = qhi.value();
                QVariant data = sheet->at( r, qhi.key() );
                if( ( i == 6 ) && ( r == 3 ) && ( qhi.key() == 7 ) )
                    qDebug() << data;
                if( !haveRowData && !data.toString().isEmpty() )
                {
                    haveRowData = true;
                    //qDebug() << "have data at " << r << " is " << data.toString();
                }
                //qDebug() << QString( "SRC r%1c%3: %2" ).arg(r-(idr+1)).arg(data.toString()).arg(qhi.key());
                model->setField( r-(idr+1), field.first, field.second, data );
                //qDebug() << QString( "TRGN r%1c%3: %2" ).arg(r-(idr+1)).arg( model->data(model->index(r-(idr+1),0)).toString() ).arg(qhi.key());
                //if( (r-(idr+1)) > 0 ) qDebug() << QString( "TRGO r%1c%3: %2" ).arg(r-(idr+1)-1).arg( model->data(model->index(r-(idr+1)-1,0)).toString() ).arg(qhi.key());
            }
            if( !haveRowData )
            {
                //qDebug() << "trying remove from " << (r-(idr+1)) << " count " << (sheet->rowCount() - (r-(idr+1)));
                model->removeRows( r-(idr+1), model->rowCount() - (r-(idr+1)) );
                break;
            }
            if( !silent ) p_progress.setProgress( ++progressCounter );
        }
    }

    // * Hide progress
    if( !silent ) p_progress.hide();

    emit doneLoading();
    return true;
}

bool MainWindow::isFileDB( QString fileName )
{
    for( int i = 0; i < eist.dbFiles.rowCount(); ++i )
    {
        QRegExp rx( eist.dbFiles.at( i, EIDB::FTMask ).toString() );
        rx.setPatternSyntax(QRegExp::Wildcard);
        QFileInfo fi( fileName );
        if( rx.exactMatch(fi.fileName()) ) return true;
    }
    return false;
}

bool MainWindow::importFileDB( QString fileName )
{
    // Try to find known type of db file
    QFileInfo qfi( fileName );
    QString baseName = qfi.fileName();
    for( int i = 0; i < eist.dbFiles.rowCount(); ++i )
    {
        QString mask = eist.dbFiles.at( i, EIDB::FTMask ).toString();
        QRegExp rx( mask );
        rx.setPatternSyntax(QRegExp::Wildcard);
        if( rx.exactMatch( baseName ) )
        {
            //qDebug() << "mask: " << mask << " == " << baseName;
            EIDBFile dbfile;
            dbfile.setCodePage( p_codePage );
            dbfile.readFile( fileName, &eist.dbFiles );

            int fileID = QtGenExt::variantToInt( eist.dbFiles.at( i, EIDB::FTID ), 0 );
            QHashIterator< QPair<int,int>, EIDBModel* > qhi(p_modelList);
            while( qhi.hasNext() )
            {
                qhi.next();
                QPair<int,int> id = qhi.key();
                if( id.first == fileID )
                {
                    EIDBModel * model = qhi.value();
                    if( !model ) continue;
                    model->setEIDB( *dbfile.block(id.second) );
                    QTable * headers = eist.headersTable( id.first, id.second );
                    if( !headers ) continue;
                    setEIDBModelHeaders( *model, headers );
                }
            }
            // Known db file was imported
            emit doneLoading();
            return true;
        }
        else
        {
            //qDebug() << "mask: " << mask << " != " << baseName;
        }
    }
    //  Unknown db file was not imported
    return false;
}

bool MainWindow::realyDiscardChanges()
{
    if ( p_changed )
    {
        QMessageBox::StandardButton response;
        response = QMessageBox::question( this, "Confirm", "Changes do not saved. Do you really want to discard changes?", QMessageBox::Save | QMessageBox::Cancel | QMessageBox::Discard );
        if( response == QMessageBox::Save )
        {
            save();
            if( p_changed ) return false;
        }
        else if( response != QMessageBox::Discard ) return false;
    }
    return true;
}

void MainWindow::setNoChanges()
{
    p_changed = false;
    updateState();
}

void MainWindow::setNoCleared()
{
    p_cleared = false;
    updateState();
}

void MainWindow::setEIDBModelHeaders( EIDBModel & model, QTable * headers )
{
    if( !headers ) return;
    QList<unsigned char> idList;
    QList<int> indexList;
    QList<int> widthList;
    QList<QString> nameList;
    QList<QString> descList;
    for( int i = 0; i < headers->rowCount(); ++i )
    {
        idList << (unsigned char) QtGenExt::variantToInt( headers->at( i, EIDB::HTID ), EIDB::UnknownItem );
        indexList << QtGenExt::variantToInt( headers->at( i, EIDB::HTIndex ), 0 );
        widthList << QtGenExt::variantToInt( headers->at( i, EIDB::HTWidth ), 0 );
        nameList << headers->at( i, EIDB::HTName ).toString();
        descList << headers->at( i, EIDB::HTDesc ).toString();
    }
    model.setHeaderInfo( idList, indexList, widthList, nameList, descList );
    QTimer::singleShot( 0, this, SLOT(qTableViewKostyl()) );
}

//void MainWindow::setViewAutoWidthForHeaders( QTableViewEx * view )
//{
//    if( !view ) return;
//    QFontMetrics metr = view->fontMetrics();
//    EIDBModel * model = view->model();
//    if( !model ) return;
//    int columns = model->columnCount();
//    for( int c = 0; c < columns; ++c )
//    {

//    }
//}

QTableViewEx * MainWindow::getCurrentTableView()
{
    QTableViewEx * result = 0;
    QPair<int,int> id = QPair<int,int>(-1,-1);
    QHashIterator< QPair<int,int>, QTableViewEx* > qhi(p_tableViewList);
    while( qhi.hasNext() )
    {
        qhi.next();
        QTableViewEx * view = qhi.value();
        if( !view ) continue;
        if( view->isVisible() )
        {
            id = qhi.key();
            result = view;
            break;
        }
    }
//    qDebug() << "fileID:" << id.first << " blockID:" << id.second;
    return result;
}

EIDBModel * MainWindow::getCurrentEIDBModel()
{
//    // Get current file ID
//    if( !p_tabWidget ) return 0;
//    QWidget * fileWidget = p_tabWidget->currentWidget();
//    if( !fileWidget ) return 0;
//    QStringList currentFileNameList = fileWidget->objectName().split('_');
//    if( ( currentFileNameList.size() < 2 ) || ( currentFileNameList[0] != "dFile" ) ) return 0;
//    int fileID = QtGenExt::stringToInt( currentFileNameList[1] );
//    // Get current block ID
//    int fileTabIndex = p_tabWidget->currentIndex();
//    QTabWidget * fileTabWidget = p_blockTabWidgets[fileTabIndex];
//    if( !fileTabWidget ) return 0;
//    QWidget * blockWidget = fileTabWidget->currentWidget();
//    if( !blockWidget ) return 0;
//    QStringList currentBlockNameList = blockWidget->objectName().split('_');
//    if( ( currentBlockNameList.size() < 3 ) || ( currentBlockNameList[0] != "dBlock" ) ) return 0;
//    int blockFileID = QtGenExt::stringToInt( currentFileNameList[1] );
//    if( fileID != blockFileID ) return 0;
//    int blockID = QtGenExt::stringToInt( currentFileNameList[2] );
//    // Get model
//    qDebug() << "fileID:" << fileID << " blockID:" << blockID;
//    return p_modelList.value( QPair<int,int>(fileID,blockID), (EIDBModel*) 0 );
    QTableViewEx * view = getCurrentTableView();
    if( !view ) return 0;
    return (EIDBModel*) view->model();
}

EIDBModel * MainWindow::getEIDBModel( int fileID, int blockID )
{
    return p_modelList.value( QPair<int,int>(fileID,blockID), (EIDBModel*) 0 );
}

EIDBBlock MainWindow::getEIDBBlock( int fileID, int blockID )
{
    EIDBModel * model = p_modelList.value( QPair<int,int>(fileID,blockID), (EIDBModel*) 0 );
    if( !model ) return EIDBBlock();
    return model->eidb();
}

EIDBBlock MainWindow::getEIDBBlock( QString fileName, QString blockName )
{
    int fileID = eist.dbFiles.column( EIDB::FTName ).cells().indexOf( fileName );
    if( fileID < 0 ) return EIDBBlock();
    QTable * blocks = eist.blocksOfFile(fileID);
    if( !blocks ) return EIDBBlock();
    int blockID = blocks->column( EIDB::BTName ).cells().indexOf( blockName );
    if( blockID < 0 ) return EIDBBlock();
    return getEIDBBlock( fileID, blockID );
}

void MainWindow::prepareTempDir()
{
    QDir tempDir( p_tempDir );
    if( !tempDir.exists() ) tempDir.mkpath( p_tempDir );
    else QtGenExt::clearDir( p_tempDir );
}

bool MainWindow::importDBInits()
{
    QString initsTemp = p_tempDir + "/dbinits";
    QtGenExt::makeEmptyDir( initsTemp );
    if( !QtZipReader::unzip( QApplication::applicationDirPath() + "/dbinits.zip", initsTemp ) ) return false;
    QDir temp( initsTemp );
    temp.setFilter( QDir::Files | QDir::NoDotAndDotDot );
    temp.setNameFilters( QStringList() << "*.*db" );
    QStringList fileNames = temp.entryList();
    bool ok = true;
    for( int i = 0; i < fileNames.size(); ++i )
    {
        QString fileName = initsTemp + QString("/%1").arg( fileNames[i] );
        if( !importFileDB( fileName ) ) ok = false;
    }
    clearBlocks();
    return ok;
}

void MainWindow::clearBlock( int fileID, int blockID )
{
    EIDBModel * model = p_modelList.value( QPair<int,int>( fileID, blockID ), 0 );
    if( !model ) return;
    model->removeRows( 0, model->rowCount() );
}

void MainWindow::clearBlocks()
{
    QHashIterator< QPair<int,int>, EIDBModel * > qhi(p_modelList);
    while( qhi.hasNext() )
    {
        qhi.next();
        EIDBModel * model = qhi.value();
        if( !model ) continue;
        model->removeRows( 0, model->rowCount() );
    }
    p_fileContainer.clear();
    p_cleared = true;
}

void MainWindow::qTableViewKostyl()
{
    QList< QTableViewEx* > viewList = p_tableViewList.values();
    foreach( QTableViewEx* view, viewList )
    {
        if( !view ) continue;
        EIDBModel * model = (EIDBModel *) view->model();
        if( !model ) continue;
        QString name = view->objectName();
        QStringList list = name.split('_');
        if( ( list.size() < 1 ) || ( list[0] != "dView" ) ) continue;
        int fileID = QtGenExt::stringToInt( list[1], 0 );
        int blockID = QtGenExt::stringToInt( list[2], 0 );
        QTable * headers = eist.headersTable( fileID, blockID );
        if( !headers ) continue;
        QFontMetrics fm = view->horizontalHeader()->fontMetrics();
        for( int i = 0; i < model->columnCount(); ++i )
        {
            int htWidth = QtGenExt::variantToInt( headers->at( i, EIDB::HTWidth ) );
            //if( (fileID == 1) && (blockID == 1) && (i == 23) ) qDebug() << model->headerData(i,Qt::Horizontal) << " " << htWidth;
            if( htWidth > 0 ) view->setColumnWidth( i, htWidth );
            else view->setColumnWidth( i, fm.width( model->headerData( i, Qt::Horizontal ).toString() )+15 );
        }
    }
}

void MainWindow::haveDataChanged()
{
    p_changed = true;
    p_cleared = false;
    updateState();
}

bool MainWindow::doQuit()
{
    if( !realyDiscardChanges() ) return false;

    saveSettings();
    QtGenExt::removeDir( p_tempDir );

    return true;
}

void MainWindow::updateState()
{
    ui->actionSave->setEnabled( p_changed );
    ui->actionSaveAsDatabaseRes->setEnabled( !p_cleared );
    ui->actionSaveAsDatabaseXlsx->setEnabled( !p_cleared );
    ui->actionSaveAsDatabaseDBFiles->setEnabled( !p_cleared );
    if( p_resFileName.isEmpty() ) this->setWindowTitle( "Evil Islands DB Editor" );
    else this->setWindowTitle( QString("Evil Islands DB Editor - [ %1 ]").arg( p_resFileName ) );
    updateNavigation();
}

void MainWindow::updateNavigation()
{
    ui->actionInsertRow->setEnabled( false );
    ui->actionAppendRow->setEnabled( false );
    ui->actionRemoveRow->setEnabled( false );
    ui->actionMoveRowUp->setEnabled( false );
    ui->actionMoveRowDown->setEnabled( false );
    ui->actionCopy->setEnabled( false );
    ui->actionPaste->setEnabled( false );

    QTableViewEx * view = getCurrentTableView();
    if( !view ) return;
    EIDBModel * model = (EIDBModel*) view->model();
    if( !model ) return;
    int rows = model->rowCount();
    bool haveData = rows > 0;
    bool haveColumns = ( model->columnCount() > 0 );
    QModelIndex index = view->currentIndex();
    bool haveCursor = index.isValid();
    int row = index.row();
    bool atFirst = ( row == 0 );
    bool atLast = ( row == (rows-1) );

    QClipboard * clip = QApplication::clipboard();
    bool haveClipboard = ( clip > 0 ) && ( !clip->text().isEmpty() );

    ui->actionInsertRow->setEnabled( haveColumns );
    ui->actionAppendRow->setEnabled( haveColumns );
    ui->actionRemoveRow->setEnabled( haveData && haveCursor );
    ui->actionMoveRowUp->setEnabled( haveCursor && !atFirst );
    ui->actionMoveRowDown->setEnabled( haveCursor && !atLast );
    ui->actionCopy->setEnabled( haveData );
    ui->actionPaste->setEnabled( haveClipboard && haveColumns );
}

void MainWindow::updateLastLoads()
{
    //qDebug() << "Before updatings: p_llActions.size=" << p_llActions.size() << ", p_lastLoads.size=" << p_lastLoads.size();
    while( p_llActions.size() < p_lastLoads.size() )
    {
        //qDebug() << QString( "p_llActions: %1 -> %2" ).arg(p_llActions.size()).arg(p_lastLoads.size());
        QAction * act = new QAction( QIcon(":/main/icons/script-attribute.png"), "No list", this );
        if( !act ) return;
        p_llActions.append( act );
        ui->menuRecentLoads->insertAction( 0, act );
        act->setObjectName( QString("LL%1").arg( p_llActions.size()-1 ) );
        connect( act, SIGNAL(triggered()), this, SLOT(myOn_recentLoad()) );
    }
    while( p_llActions.size() > p_lastLoads.size() )
    {
        //qDebug() << QString( "p_llActions: %1 -> %2" ).arg(p_llActions.size()).arg(p_lastLoads.size());
        QAction * act = p_llActions.last();
        if( !act ) return;
        p_llActions.removeLast();
        ui->menuRecentLoads->removeAction( act );
        disconnect( act, SIGNAL(triggered()), this, SLOT(myOn_recentLoad()) );
        delete act;
    }
    for( int i = 0; i < p_lastLoads.size(); ++i )
    {
        //qDebug() << "Update Action at " << i;
        QAction * act = p_llActions[i];
        if( !act ) continue;
        // Build text
        QString text;
        QStringList list = p_lastLoads[i];
        if( list.size() < 1 ) text = "[ No list ]";
        else
        {
            text += list.first();
            for( int j = 1; j < list.size(); ++j )
            {
                QFileInfo fi( list[j] );
                text += QString( "; %1" ).arg( fi.fileName() );
            }
        }
        act->setText( text );
    }
}

void MainWindow::loadSettings()
{
    QSettings settings( QApplication::applicationDirPath() + "/settings.ini", QSettings::IniFormat );
    restoreGeometry( QByteArray::fromHex( settings.value( "Main/WindowGeo" ).toByteArray() ) );
    p_loadPath = settings.value( "Main/LoadPath" ).toString();
    p_savePath = settings.value( "Main/SavePath" ).toString();
    p_codePage = settings.value( "Main/DataCodePage", "Windows-1251" ).toString();
    loadSettingsForLastLoads();
}

void MainWindow::loadSettingsForLastLoads()
{
    p_lastLoads.clear();
    QSettings settings( QApplication::applicationDirPath() + "/settings.ini", QSettings::IniFormat );
    for( int i = 0; i < 5; ++i )
    {
        QStringList loadList = settings.value( QString("LastLoads/LoadList%1").arg(i+1) ).toStringList();
        if( loadList.isEmpty() ) continue;
        p_lastLoads.append( loadList );
    }
}

void MainWindow::saveSettings()
{
    QSettings settings( QApplication::applicationDirPath() + "/settings.ini", QSettings::IniFormat );
    settings.setValue( "Main/WindowGeo", saveGeometry().toHex() );
    settings.setValue( "Main/LoadPath", p_loadPath );
    settings.setValue( "Main/SavePath", p_savePath );
    settings.setValue( "Main/DataCodePage", p_codePage );
    saveSettingsForLastLoads();
}

void MainWindow::saveSettingsForLastLoads()
{
    QSettings settings( QApplication::applicationDirPath() + "/settings.ini", QSettings::IniFormat );
    for( int i = 0; i < 5; ++i )
    {
        QStringList loadList;
        if( i < p_lastLoads.size() ) loadList = p_lastLoads[i];
        settings.setValue( QString("LastLoads/LoadList%1").arg(i+1), loadList );
    }
}

void MainWindow::longQuit()
{
    p_changed = false;
    QTimer::singleShot( 500, this, SLOT(forceQuit()) );
}

void MainWindow::forceQuit()
{
    QApplication::quit();
}

void MainWindow::myOn_mainTabChanged( int index )
{
    p_currentTab = index;
    updateNavigation();
}

void MainWindow::myOn_subTabChanged( int index )
{
    p_currentSubTab = index;
    updateNavigation();
}

void MainWindow::myOn_viewClicked( QModelIndex index )
{
    updateNavigation();
}

void MainWindow::myOn_viewKeyPressed( QKeyEvent * event )
{
    updateNavigation();
}

void MainWindow::myOn_changeClipboard( QClipboard::Mode mode )
{
    if( mode != QClipboard::Clipboard ) return;
    updateNavigation();
}

void MainWindow::closeEvent(QCloseEvent *event)
{
    if( doQuit() ) event->accept();
    else event->ignore();
}

void MainWindow::addLastLoads( QStringList loadList )
{
//    for( int i = 0; i < 5; ++i )
//    {
//        if( p_lastLoads[i] == loadList )
//        {
//            p_lastLoads.removeAt(i);
//            break;
//        }
//    }
    p_lastLoads.removeAll( loadList );
    p_lastLoads.prepend( loadList );
    while( p_lastLoads.size() > 5 ) p_lastLoads.removeLast();
    updateLastLoads();
}

void MainWindow::myOn_recentLoad()
{
    QAction * act = (QAction*) QObject::sender();
    if( !act ) return;
    if( !act->objectName().startsWith("LL") ) return;
    int actIndex = QtGenExt::stringToInt( act->objectName().mid(2), -1 );
    if( actIndex < 0 ) return;
    if( actIndex >= p_lastLoads.size() ) return;
    load( p_lastLoads[actIndex] );
    addLastLoads( p_lastLoads[actIndex] );
    updateState();
}

void MainWindow::on_actionClear_triggered()
{
    if( !realyDiscardChanges() ) return;
    clearBlocks();
    p_changed = false;
    updateState();
}

void MainWindow::on_actionLoad_triggered()
{
    load();
    updateState();
}

void MainWindow::on_actionSave_triggered()
{
    save();
    updateState();
}

void MainWindow::on_actionSaveAsDatabaseRes_triggered()
{
    if( saveAsDatabaseRes() ) p_changed = false;
    updateState();
}

void MainWindow::on_actionSaveAsDatabaseXlsx_triggered()
{
    if( saveAsDatabaseXlsx() ) p_changed = false;
    updateState();
}

void MainWindow::on_actionSaveAsDatabaseDBFiles_triggered()
{
    if( saveAsDatabaseDBs() ) p_changed = false;
    updateState();
}

void MainWindow::on_actionExit_triggered()
{
    close();
}

void MainWindow::on_actionAbout_triggered()
{
    p_about.fire(this);
}

void MainWindow::on_actionInsertRow_triggered()
{
    QTableViewEx * view = getCurrentTableView();
    if( !view ) return;
    EIDBModel * model = (EIDBModel*) view->model();
    if( !model ) return;
    QModelIndex index = view->currentIndex();
    if( !index.isValid() )
    {
        on_actionAppendRow_triggered();
        return;
    }
    model->insertRow( index.row() );

    QKeyEvent event( QEvent::KeyPress, Qt::Key_Up, Qt::NoModifier );
    QApplication::sendEvent( view, &event );

    haveDataChanged();
}

void MainWindow::on_actionAppendRow_triggered()
{
    QTableViewEx * view = getCurrentTableView();
    if( !view ) return;
    EIDBModel * model = (EIDBModel*) view->model();
    if( !model ) return;
    model->insertRow( model->rowCount() );

    QKeyEvent event( QEvent::KeyPress, Qt::Key_Down, Qt::ControlModifier );
    for( int i = 0; i < 100; ++i ) QApplication::sendEvent( view, &event );

    haveDataChanged();
}

void MainWindow::on_actionRemoveRow_triggered()
{
    QTableViewEx * view = getCurrentTableView();
    if( !view ) return;
    QModelIndex index = view->currentIndex();
    if( !index.isValid() ) return;
    EIDBModel * model = (EIDBModel*) view->model();
    if( !model ) return;
    model->removeRow( index.row() );

    QKeyEvent event( QEvent::KeyPress, Qt::Key_Down, Qt::NoModifier );
    QApplication::sendEvent( view, &event );

    haveDataChanged();
}

void MainWindow::on_actionMoveRowUp_triggered()
{
    QTableViewEx * view = getCurrentTableView();
    if( !view ) return;
    EIDBModel * model = (EIDBModel*) view->model();
    if( !model ) return;
    QModelIndex index = view->currentIndex();
    if( !index.isValid() ) return;
    int row = index.row();
    if( row < 1 ) return;
    model->moveRows( row, row-1, 1 );

    QKeyEvent event( QEvent::KeyPress, Qt::Key_Up, Qt::NoModifier );
    QApplication::sendEvent( view, &event );

    haveDataChanged();
}

void MainWindow::on_actionMoveRowDown_triggered()
{
    QTableViewEx * view = getCurrentTableView();
    if( !view ) return;
    EIDBModel * model = (EIDBModel*) view->model();
    if( !model ) return;
    QModelIndex index = view->currentIndex();
    if( !index.isValid() ) return;
    int row = index.row();
    if( row >= (model->rowCount()-1) ) return;
    model->moveRows( row, row+1, 1 );

    QKeyEvent event( QEvent::KeyPress, Qt::Key_Down, Qt::NoModifier );
    QApplication::sendEvent( view, &event );
    QApplication::sendEvent( view, &event );

    haveDataChanged();
}

void MainWindow::on_actionCopy_triggered()
{
    QTableViewEx * view = getCurrentTableView();
    if( !view ) return;
    EIDBModel * model = (EIDBModel*) view->model();
    if( !model ) return;
    QModelIndexList list = view->selectionModel()->selectedIndexes();
    if( list.size() < 1 ) return;
    // Find minimum and maximum row and column
    int rowUp = -1, rowDown = -1;
    int columnLeft = -1, columnRight = -1;
    for( int i = 0; i < list.size(); ++i )
    {
        QModelIndex index = list[i];
        if( i == 0 )
        {
            rowUp = index.row();
            rowDown = index.row();
            columnLeft = index.column();
            columnRight = index.column();
            continue;
        }
        if( rowUp > index.row() ) rowUp = index.row();
        if( rowDown < index.row() ) rowDown = index.row();
        if( columnLeft > index.column() ) columnLeft = index.column();
        if( columnRight < index.column() ) columnRight = index.column();
    }
    if( (rowUp<0) || (rowDown<0) || (columnLeft<0) || (columnRight<0) ) return;
    // Begin formatting
    QString data;
    bool haveHeader = false;
    // Formatting headers for whole table
    if( (rowUp==0) && (rowDown==(model->rowCount()-1)) && (columnLeft==0) && (columnRight==(model->columnCount()-1)) && !model->eidb().content().isEmpty() )
    {
        haveHeader = true;
        QList<EIDBItem> items = model->eidb().content().first().items();
        for( int r = 0; r < 2; ++r )
        {
            QString str;
            for( int c = 0; c <= columnRight; ++c )
            {
                QString pre;
                if( r == 0 ) pre = model->headerData( c, Qt::Horizontal ).toString();
                if( r == 1 ) pre = QString( "FLD%1-%2" ).arg( (int) items[c].id() ).arg( items[c].index() );
                if( c > 0 ) str += "\t";
                str += pre;
            }
            if( r > 0 ) data += "\n";
            data += str;
        }
    }
    // Data formatting
    for( int r = rowUp; r <= rowDown; ++r )
    {
        QString str;
        for( int c = columnLeft; c <= columnRight; ++c )
        {
            if( c > columnLeft ) str += "\t";
            QString pre = model->data( model->index(r,c) ).toString();
            bool test = false;
            model->data( model->index(r,c) ).toFloat(&test);
            if( test ) pre.replace( ".", "," );
            str += pre;
        }
        if( ( r > rowUp ) || haveHeader ) data += "\n";
        data += str;
    }
    // Write to clipboard
    QClipboard  * clip = QApplication::clipboard();
    if( !clip ) return;
    clip->setText( data );
}

void MainWindow::on_actionPaste_triggered()
{
    QTableViewEx * view = getCurrentTableView();
    if( !view ) return;
    EIDBModel * model = (EIDBModel*) view->model();
    if( !model ) return;
    QModelIndex index = view->currentIndex();
    if( !index.isValid() ) return;
    int row = index.row();
    int column = index.column();
    int mRows = model->rowCount();
    int mColumns = model->columnCount();
    // Read from clipboard
    QClipboard  * clip = QApplication::clipboard();
    if( !clip ) return;
    QString data = clip->text();
    data = data.trimmed();
    // Data parsing
    QStringList rows = data.split( '\n' );
    // * adding necessary rows
    if( rows.size()+row > mRows )
    {
        model->insertRows( mRows, (rows.size()+row)-mRows );
        mRows = model->rowCount();
    }
    for( int r = 0; r < rows.size(); ++r )
    {
        if( (row+r) >= mRows ) break;
        QStringList columns = rows[r].split( '\t' );
        for( int c = 0; c < columns.size(); ++c )
        {
            if( (column+c) >= mColumns ) break;
            model->setData( model->index( row+r, column+c ), QVariant( columns[c] ) );
        }
    }
    haveDataChanged();
}
