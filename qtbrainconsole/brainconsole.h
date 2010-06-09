#ifndef BRAINCONSOLE_H
#define BRAINCONSOLE_H

#include <QObject>
#include "bfinterpreter.h"

//namespace QtBrain {
//    class BfInterpreter;
//}

using namespace QtBrain;

class BrainConsole : public QObject
{
    Q_OBJECT
public:
    explicit BrainConsole(QObject *parent = 0);
    ~BrainConsole();

signals:
    void iniVm(const QList<BfOpcode>&);
    void iniVm(QList<BfOpcode> const *);

public slots:
    void bfiTest();

private:
    BfInterpreter *bfi;
    QThread *bfiThread;

};

#endif // BRAINCONSOLE_H
