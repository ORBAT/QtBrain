#ifndef BRAINWINDOW_H
#define BRAINWINDOW_H

#include <QMainWindow>
#include "bfinterp_global.h"

namespace Ui {
    class BrainWindow;
}

using namespace QtBrain;

class BrainWindow : public QMainWindow
{
    Q_OBJECT
    //////////////////////////////////////////////////////////////////////////////////////////////
    //// PUBLIC METHODS
    ///////////////////
public:
    explicit BrainWindow(QWidget *parent = 0);
    ~BrainWindow();

    // when the DP moves, this function changes the Table to reflect the change.
    void changeMemView(DPType);





    //////////////////////////////////////////////////////////////////////////////////////////////
    //// SIGNALS
    ////////////
signals:
    void testSig(const int&);

    //////////////////////////////////////////////////////////////////////////////////////////////
    //// PROTECTED METHODS
    //////////////////////
protected:
    void changeEvent(QEvent *e);
    void closeEvent(QCloseEvent *);

    //////////////////////////////////////////////////////////////////////////////////////////////
    //// PRIVATE MEMBERS
    ////////////////////
private:
    Ui::BrainWindow *ui;
    QPalette                        m_inputOriginalPalette;
    QPalette                        m_attentionPalette;

    QStringList                     *m_memStartHeader;// the table header for DPs 0-4

    bool                            m_documentDirty;/* if the IDE document is "dirty", it has
                                                       been edited but not saved */

    QString                         m_documentName;/* the name of the document being
                                                      edited */
};

#endif // BRAINWINDOW_H
