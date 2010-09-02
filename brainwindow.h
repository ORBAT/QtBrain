/*
Copyright 2010 Tom Eklof. All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are
permitted provided that the following conditions are met:

   1. Redistributions of source code must retain the above copyright notice, this list of
      conditions and the following disclaimer.

   2. Redistributions in binary form must reproduce the above copyright notice, this list
      of conditions and the following disclaimer in the documentation and/or other materials
      provided with the distribution.

THIS SOFTWARE IS PROVIDED BY TOM EKLOF ``AS IS'' AND ANY EXPRESS OR IMPLIED
WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND
FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL TOM EKLOF OR
CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON
ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF
ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/
#ifndef BRAINWINDOW_H
#define BRAINWINDOW_H

#include "bfvm.h"
#include "bihash.h"
#include <QMainWindow>
#include <QFile>

namespace QtBrain {
    class BfCompiler;
    class BfHighlighter;
}

namespace Ui {
    class BrainWindow;
}

using namespace QtBrain;

class QPlainTextEdit;
class QStandardItemModel;


class BrainWindow : public QMainWindow {
    Q_OBJECT
public:
    /////////////////////////////////////////////////////////////////////////////////////
    //// PUBLIC METHODS
    ///////////////////
    BrainWindow(QWidget *parent = 0);
    ~BrainWindow();

    /////////////////////////////////////////////////////////////////////////////////////
    //// SIGNALS
    ////////////
signals:
    void initialize(const QString&); // sent to the VM to initialize it
    void initialize(const QList<BfOpcode>&); /* sent to the VM to initialize it with
                                                compiled code */
    void output(const QString&); // to send data to the VM

    void compile(const QString&); // to send data to the compiler


    ///////////////////////////////////////////////////////////////////////////////////////
    //// PROTECTED SLOTS
    ////////////////////
    /* TODO: reduce the number of slots somehow */
protected slots:
    void vmInited();        // used to receive the inited() signal from the VM
    void vmCleared();       // used to receive the cleared() signal from the VM
    void vmReset();         // used to receive the resetted() signal from the VM
    void vmFinished();      // used to receive the finish() signal from the VM
    void vmRunning(bool);   // is the VM running or not?
    void vmNeedInput();     // VM needs input
    void vmHeartBeat(const IPType&); // used to receive the heartbeat() signal from the VM
    void vmConsumedInput();   /* used to receive the inputConsumed() signal from the VM */

    void vmDPChanged(DPType); // used to receive DP change signals from the VM

    /* when a cell of memory is changed, this method caches the contents and then shows the
       change in the table. */
    void vmMemChanged(DPType,Memtype);

    void vmOutput(const Memtype&);

    void sendOutput();      /* when the user presses enter in the Input field,
                               the QLineEdit emits a signal. This signal is sent to
                               sendOutput() which in turn sends the contents of the
                               QLineEdit to the VM */

    void programToDebugger();/* loads the source from the IDE tab to the debugger tab */

    void vmBreakPoint(IPType, DPType);




    void compilerError(const QString&, quint32);
    /* receives error messages from the compiler along with the possible position of the
       error */

    // data from the compiler. Look in bfcompiler.h for more information
    void compiled(const QList<BfOpcode>&, BiHash<IPType,IPType> &jmps,
                  BiHash<IPType,quint32> &mappings);





protected:
    /////////////////////////////////////////////////////////////////////////////////////////
    //// PROTECTED METHODS
    //////////////////////
    void changeEvent(QEvent *e);
    void closeEvent(QCloseEvent *);

private:
    /////////////////////////////////////////////////////////////////////////////////////////
    //// PRIVATE MEMBER VARIABLES
    ///////////////////////////////
    BfVM                            *m_vm;         // QThread for the Brainfuck VM
    BfCompiler                      *m_compiler;   // the QThread for the compiler
    BiHash<IPType,IPType>           *m_jmps;       // brace matching hash

    /* should I replace call to m_mappings->value with an array lookup? The array
       would be the same length as the _compiled_ program itself, and contain the mapped
       values in it. So looking up m_mapArry[i] would look up the mapping for the command
       at IP position i. This would, of course, mean that I'd be trading off processing time
       for memory usage, and a program of maximum allowed size (2^32 instructions) would
       cause the GUI to eat up a whopping 16 gigaBYTES of memory. On the other hand, how
       likely are we to see a Brainfuck program with 137 438 953 472 commands in it? */
    BiHash<IPType,quint32>          *m_mappings;   // bytecode <-> source position mappings
    BfHighlighter                   *m_highlighter;// syntax highlighter
    Memtype                         *m_memMap;     // just a duplicate of the VM's memory...

    QPalette                        m_inputOriginalPalette;
    QPalette                        m_attentionPalette;

    QStringList                     *m_memStartHeader;// the table header for DPs 0-4

    bool                            m_vmNeedsInput;/* set to true when the vm needs input.
                                                      Dirty hack... */

    bool                            m_debuggingMode;// if true, enable debugging mode

    bool                            m_documentDirty;/* if the IDE document is "dirty", it has
                                                       been edited but not saved */

    QString                         m_documentName;/* the name of the document being
                                                      edited */

    Ui::BrainWindow *ui;




    /////////////////////////////////////////////////////////////////////////////////////////
    //// PRIVATE METHODS
    ////////////////////

    // Zeroes the memory table contents and header
    void clearMemoryTbl();

    void resetUi();                          // resets memory state, DP, IP etc widgets
    void disableRunActions(bool wot = true); // disables and enables VM-related actions
    void additionalUISetup();// connects local QActions to state change signals in the VM etc
    void connectToVM(); // connects local slots to VM state information signals
    void connectToCompiler(); // connects local slots to compiler information signals

    void clearMemMap(); // zeroes the memory map

    // creates the proper header for the memory table
    QStringList makeHeaderForPos(DPType dp);

    // when the DP moves, this function changes the Table to reflect the change.
    void changeMemView(DPType);


    /* Moves the "cursor" in the debugging view of the program.

       If reset is true, the cursor is "reset", ie. nothing is selected.

       This _SHOULD_ probably be done by subclassing QTextEdit and then making a
       custom caret for it by painting on the widget, but this'll do for
       now.
    */
    void moveDbgCursor(QPlainTextEdit* te, const quint32 dir=0, bool reset=false);

    // makes the memory table the right size
    void resizeMemTable();

    // changes the current document
    void setCurrentDocument(const QString &);

    // writes the given QString to the given QFile and returns true if succesful
    bool writeToFile(QString, QString);


    bool save();
    bool saveAs();
    bool maybeSave();
    bool saveFile(const QString &fileName);
    void open();

    /////////////////////////////////////////////////////////////////////////////////////////
    //// PRIVATE SLOTS
    //////////////////
private slots:



    /* I used the toggled(bool) slot since I need to change the state of the button
       from the program as well, and triggered() only responds to user action */
    void on_actionDebugging_mode_toggled(bool checked);

    void on_actionNew_triggered();
    void on_actionSaveAs_triggered();
    void on_actionSave_triggered();
    void on_actionOpen_triggered();
    // debugging mode-related stuff like enabling/disabling controls and actions
    void on_slTickDelay_valueChanged(int value);
    void on_actionLoad_program_triggered();

    // sets whether the document needs saving or not. Default to true
    void setDocumentIsDirty();

};

#endif // BRAINWINDOW_H
