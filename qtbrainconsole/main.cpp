#include <QtCore/QCoreApplication>
#include "brainconsole.h"
#include "bfinterpreter.h"
#include <QDebug>

using namespace QtBrain;

int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);
    BrainConsole b;


    return a.exec();
}
