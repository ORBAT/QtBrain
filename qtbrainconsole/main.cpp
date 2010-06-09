#include <QtCore/QCoreApplication>
#include "brainconsole.h"

int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);
    BrainConsole b;
    return a.exec();
}
