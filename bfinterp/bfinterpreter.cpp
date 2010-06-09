#include <QDebug>
#include "bfinterpreter.h"
#include "bfinterpreter_p.h"

namespace QtBrain {

    /*
     Qt 4.6 documentation, "Threads and QObjects" states:
     "The child of a QObject must always be created in the thread where the parent
     was created. This implies, among other things, that you should never pass the
     QThread object (this) as the parent of an object created in the thread (since the
     QThread object itself was created in another thread)."

     However, I need to pass a pointer to the public implementation to
     BfInterpreterPrivate's constructor. How do I solve this? Does it even *need* solving
     in this case?
     */
    BfInterpreter::BfInterpreter(QObject *parent) : QObject(parent), dpt(new BfInterpreterPrivate(this))
    {
        qDebug() << "BfInterpreter()";

        qDebug() << "BfInterpreter() thread"<< thread()->currentThreadId();

    }

    BfInterpreter::BfInterpreter(BfInterpreterPrivate &dd, QObject *parent) : QObject(parent), dpt(&dd)
    {}

    BfInterpreter::~BfInterpreter() {
        qDebug() << "~BfInterpreter()";
    }

    /////////////////////////////////////////////////////////////////////////////////////
    //// SLOTS FOR EXTERNAL USE
    ///////////////////////////
    void BfInterpreter::changeDelay(const int &delay) {
        qDebug() << "BfInterpreter() thread"<<thread()->currentThreadId();
        Q_D(BfInterpreter);
        d->changeDelay(delay);
    }

    void BfInterpreter::initialize(const QList<BfOpcode> &src) {
        qDebug() << "BfInterpreter() thread"<<thread()->currentThreadId();
        Q_D(BfInterpreter);
        d->doinit(src);
    }



    void BfInterpreter::initialize(QList<BfOpcode> const *src) {
        qDebug() << "BfInterpreter() thread"<<thread()->currentThreadId();
        QList<BfOpcode> derp = *src;
        Q_D(BfInterpreter);
        d->doinit(derp);
    }

    void BfInterpreter::input(const QString &in){
        qDebug() << "BfInterpreter() thread"<<thread()->currentThreadId();
        Q_D(BfInterpreter);
        d->input(in);
    }

    void BfInterpreter::setDebugging(const bool &set) {
        qDebug() << "BfInterpreter() thread"<<thread()->currentThreadId();
        if(set) {
            Q_D(BfInterpreter);
            d->setDebugging(set);
        }
    }


    //////////////////////////////////////////////////////////////////////////////////
    //// PROTECTED METHODS
    //////////////////////

//    void BfInterpreter::run() {
//       exec();
//    }

}
