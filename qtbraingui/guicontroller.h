#ifndef GUICONTROLLER_H
#define GUICONTROLLER_H

#include <QObject>

namespace QtBrain {
    class BfInterpreter;
}

class GUIController : public QObject
{
    Q_OBJECT
public:
    explicit GUIController(QObject *parent = 0);

protected:

    QtBrain::BfInterpreter *bfi;

signals:

public slots:

};

#endif // GUICONTROLLER_H
