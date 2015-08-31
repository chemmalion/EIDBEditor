#ifndef QTXMLREADER_H
#define QTXMLREADER_H

#include <QtCore>
#include <QtXml>

class QtXmlReader : public QObject
{
    Q_OBJECT
public:
    explicit QtXmlReader(QObject *parent = 0);
    virtual ~QtXmlReader();

    bool start( QString xmlFileName );
    bool finish();

signals:

public slots:

};

#endif // QTXMLREADER_H
