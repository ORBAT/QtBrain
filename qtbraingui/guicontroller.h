#ifndef GUICONTROLLER_H
#define GUICONTROLLER_H

#include <QObject>

class GUIController : public QObject
{
    Q_OBJECT
public:
    explicit GUIController(QObject *parent = 0);

signals:

public slots:

};

#endif // GUICONTROLLER_H
