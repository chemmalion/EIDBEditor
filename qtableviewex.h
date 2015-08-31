#ifndef QTABLEVIEWEX_H
#define QTABLEVIEWEX_H

#include <QtCore>
#include <QtGui>

#include "freezetablewidget.h"

class QTableViewEx : public FreezeTableWidget
{
    Q_OBJECT
public:
    explicit QTableViewEx ( QAbstractItemModel * model, QWidget * parent = 0 );
    virtual ~QTableViewEx ();

    QModelIndex myMoveCursor( int cursorAction, Qt::KeyboardModifiers modifiers );

protected:
    virtual void keyPressEvent( QKeyEvent * event );

signals:
    void afterKeyPressed( QKeyEvent * event );

public slots:

};

#endif // QTABLEVIEWEX_H
