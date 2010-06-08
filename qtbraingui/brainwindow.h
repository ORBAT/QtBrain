#ifndef BRAINWINDOW_H
#define BRAINWINDOW_H

#include <QMainWindow>

namespace Ui {
    class BrainWindow;
}

class BrainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit BrainWindow(QWidget *parent = 0);
    ~BrainWindow();

signals:
    void testSig(const int&);

protected:
    void changeEvent(QEvent *e);

private:
    Ui::BrainWindow *ui;
};

#endif // BRAINWINDOW_H
