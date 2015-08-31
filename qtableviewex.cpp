#include "qtableviewex.h"

QTableViewEx::QTableViewEx(QAbstractItemModel * model, QWidget * parent) :
    FreezeTableWidget(model,parent)
{
}

QTableViewEx::~QTableViewEx ()
{
}

QModelIndex QTableViewEx::myMoveCursor( int cursorAction, Qt::KeyboardModifiers modifiers )
{
    return moveCursor( (QAbstractItemView::CursorAction) cursorAction, modifiers );
}

void QTableViewEx::keyPressEvent( QKeyEvent * event )
{
    QTableView::keyPressEvent(event);
    emit afterKeyPressed( event );
}
