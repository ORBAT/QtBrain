#include <QDebug>
#include "bfinterpreter_p.h"


namespace QtBrain {

    BfInterpreterPrivate::BfInterpreterPrivate(BfInterpreter * const qq) : q_ptr(qq)
    {
        qDebug() << "BfInterpreterPrivate()";
    }

    BfInterpreterPrivate::~BfInterpreterPrivate() {

    }

}
