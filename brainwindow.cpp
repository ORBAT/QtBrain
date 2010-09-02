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

#include "brainwindow.h"
#include "bfcompiler.h"
#include "ui_brainwindow.h"
#include "bfhighlighter.h"
#include <QDebug>
#include <QPalette>
#include <QMessageBox>
#include <QStandardItemModel>
#include <QStandardItem>
#include <QFileDialog>


using namespace QtBrain;

BrainWindow::BrainWindow(QWidget *parent) :
        QMainWindow(parent),
        m_vm(new BfVM(this)),
        m_compiler(new BfCompiler::BfCompiler(this)),
        m_jmps(NULL),
        m_mappings(NULL),
        m_memMap(new Memtype[BfVM::MAX_MEM_ADDR+1]), /* +1 because MAX_MEM_ADDR only gives us
                                                        the largest possible _address_, not
                                                        the size of the memory */
        m_vmNeedsInput(false),
        m_documentDirty(false),
        ui(new Ui::BrainWindow)

{
    m_vm->start();
    m_compiler->start();



    connect(this, SIGNAL(compile(const QString&)), m_compiler,
            SLOT(compile(const QString&)));


    connect(m_compiler, SIGNAL(compiled(QList<BfOpcode>,
                                        BiHash<IPType,IPType>&,
                                        BiHash<IPType,quint32>&)),
            this, SLOT(compiled(QList<BfOpcode>,
                                BiHash<IPType,IPType>&,
                                BiHash<IPType,quint32>&)));


    ui->setupUi(this);



    clearMemMap();
    connectToVM();
    additionalUISetup();
    disableRunActions();
}

void BrainWindow::clearMemMap() {
    for(int i = BfVM::MAX_MEM_ADDR+1; i --> 0;) {
        m_memMap[i] = 0;
    }
}

void BrainWindow::connectToVM() {
    /* connect a signal to the VM's public initialize() slots so the user can load new
       programs into the VM */
    //connect(this,SIGNAL(initialize(QString)),m_vm, SLOT(initialize(QString)));
    connect(this,SIGNAL(initialize(const QList<BfOpcode>&)), m_vm,
            SLOT(initialize(const QList<BfOpcode>&)));


    connect(ui->slTickDelay, SIGNAL(valueChanged(int)), m_vm, SLOT(changeDelay(int)));

    /* detect when the user presses return in the input QLineEdit and then
       use the sendOutput() slot to send the contents of the QLineEdit */
    // NOTE: You really do need to press Enter to send input data to the VM
    connect(ui->leInput, SIGNAL(returnPressed()), this, SLOT(sendOutput()));

    /* connect the GUI's output signal to the input slot of the VM so the user can change
       the input buffer of the VM */
    connect(this, SIGNAL(output(const QString&)), m_vm, SLOT(input(const QString&)));

    connect(m_vm, SIGNAL(inited()), this, SLOT(vmInited()));
    connect(m_vm, SIGNAL(cleared()), this, SLOT(vmCleared()));
    connect(m_vm, SIGNAL(resetted()), this, SLOT(vmReset()));
    connect(m_vm, SIGNAL(running(bool)), this, SLOT(vmRunning(bool)));
    connect(m_vm, SIGNAL(finish()), this, SLOT(vmFinished()));
    connect(m_vm, SIGNAL(needInput()), this, SLOT(vmNeedInput()));
    /* remove characters from the input QLineEdit when the VM signals it has consumed
       input */
    connect(m_vm, SIGNAL(inputConsumed()), this, SLOT(vmConsumedInput()));

    // receive IP information
    connect(m_vm, SIGNAL(heartBeat(const IPType&)), this, SLOT(vmHeartBeat(const IPType&)));

    // receive output from the VM
    connect(m_vm, SIGNAL(output(const Memtype&)), this, SLOT(vmOutput(const Memtype&)));

    connect(m_vm, SIGNAL(DPChanged(DPType)), this, SLOT(vmDPChanged(DPType)));
    connect(m_vm, SIGNAL(memChanged(DPType,Memtype)), this,
            SLOT(vmMemChanged(DPType,Memtype)));
    connect(m_vm, SIGNAL(breakpoint(IPType,DPType)),this,SLOT(vmBreakPoint(IPType,DPType)));


}

void BrainWindow::additionalUISetup() {
    // Note the use of the *Sig signals to control the state of the VM.
    connect(ui->actionStep, SIGNAL(triggered()), m_vm, SIGNAL(stepSig()));
    connect(ui->actionRun, SIGNAL(triggered()), m_vm, SIGNAL(toggleRunSig()));
    connect(ui->actionReset, SIGNAL(triggered()), m_vm, SIGNAL(resetSig()));
    connect(ui->actionClear, SIGNAL(triggered()), m_vm, SIGNAL(clearSig()));
    connect(ui->action_Quit, SIGNAL(triggered()), this, SLOT(close()));

    // some standard icons for the actions
    ui->actionOpen->setIcon(style()->standardIcon(QStyle::SP_DialogOpenButton));
    ui->actionRun->setIcon(style()->standardIcon(QStyle::SP_MediaPlay));
    ui->actionStep->setIcon(style()->standardIcon(QStyle::SP_ArrowForward));
    ui->actionReset->setIcon(style()->standardIcon(QStyle::SP_BrowserReload));
    ui->actionSave->setIcon(style()->standardIcon(QStyle::SP_DialogSaveButton));
    ui->actionNew->setIcon(style()->standardIcon(QStyle::SP_FileIcon));
    ui->actionLoad_program->setIcon(style()->standardIcon(QStyle::SP_ComputerIcon));

    // receive error messages from the compiler
    connect(m_compiler, SIGNAL(error(const QString&,quint32)), this,
            SLOT(compilerError(const QString&,quint32)));


    // start out in the IDE tab
    ui->viewsTab->setCurrentIndex(0);

    /* The UI designer keeps removing my vertical header, so this kluge should force it
       to be visible */
    ui->tblMemory->verticalHeader()->setVisible(true);

    /* memoize the header for DPs 0-4, since most BF programs spend most of their time
       around there anyhow */
    m_memStartHeader = new QStringList();
    for(int i = 0; i <= 8; ++i) {
        m_memStartHeader->append(tr("%1").arg(i));
    }


    /* allocates the QTableWidgetItems used in the memory table. When the table contents
       change these items should be changed instead of creating new ones each time. */
    QTableWidgetItem *twi;
    for(int i = 0; i < 9; ++i) {
        twi = new QTableWidgetItem(tr("0"));
        twi->setTextAlignment(Qt::AlignHCenter);
        ui->tblMemory->setItem(i,0, twi);
    }


    ui->tblMemory->setVerticalHeaderLabels(*m_memStartHeader);

    resizeMemTable();

    // set m_debuggingMode to whatever state the QAction is in
    m_debuggingMode = ui->actionDebugging_mode->isChecked();

    // the highlighter is only used in the IDE right now
    m_highlighter = new BfHighlighter(ui->teIde->document());


    // store the original palette of the VM text input widget
    m_inputOriginalPalette = ui->leInput->palette();

    // set the color for the attention palette
    m_attentionPalette = QPalette(m_inputOriginalPalette);
    m_attentionPalette.setColor(QPalette::Base, Qt::red);

    // start out with no document name
    setCurrentDocument(QString());
    // start up in "clean" mode so the editor will not ask to save an empty file
//    setDocumentIsDirty();

    // notify the GUI if the document is edited
    connect(ui->teIde, SIGNAL(textChanged()), this, SLOT(setDocumentIsDirty()));
}

/*
  disa defaults to true, so the default action is to disable all... uh... actions
  */
void BrainWindow::disableRunActions(bool disa) {
    ui->actionClear->setDisabled(disa);
    ui->actionReset->setDisabled(disa);
    ui->actionRun->setDisabled(disa);
    ui->actionStep->setDisabled(disa);
}

BrainWindow::~BrainWindow()
{
    delete ui;
    delete m_jmps;
    delete m_mappings;
    delete m_memStartHeader;
    delete m_memMap;
}

void BrainWindow::changeEvent(QEvent *e)
{
    QMainWindow::changeEvent(e);
    switch (e->type()) {
    case QEvent::LanguageChange:
        ui->retranslateUi(this);
        break;
    default:
        break;
    }
}



void BrainWindow::vmInited() {
    qDebug() << "VM is initialized";
    disableRunActions(false);
}

void BrainWindow::vmNeedInput() {
    m_vmNeedsInput = !m_vmNeedsInput;
    if(m_vmNeedsInput) {
        ui->leInput->setPalette(m_attentionPalette);
    } else {
        ui->leInput->setPalette(m_inputOriginalPalette);
    }
}

void BrainWindow::compiled(const QList<BfOpcode> &src, BiHash<IPType, IPType> &jmps, BiHash<IPType, quint32> &mappings) {

    programToDebugger();

    // get the VM initialized
    emit initialize(src);


    delete m_jmps;     // deleting a NULL pointer is perfectly safe
    delete m_mappings;

    m_jmps = new BiHash<IPType,IPType>(jmps);
    m_mappings = new BiHash<IPType,quint32>(mappings);
    qDebug("BrainWindow::compiled()");
}


void BrainWindow::compilerError(const QString &msg, quint32 pos) {
    QMessageBox::critical(this, trUtf8("Compilation error"),
                          trUtf8("There was an error in the source somewhere near "
                                 "position %1:\n%2").
                          arg(pos).arg(msg));
    QTextCursor tc = ui->teIde->textCursor();
    tc.setPosition(pos);
    ui->teIde->setTextCursor(tc);
}


void BrainWindow::programToDebugger() {
    ui->teDebugProgram->setPlainText(ui->teIde->toPlainText());
}

void BrainWindow::vmOutput(const Memtype &data) {
    //ui->teOutput->append(QChar::fromAscii(data));
    ui->teOutput->insertPlainText(QChar::fromAscii(data));
    ui->teOutput->moveCursor(QTextCursor::End);
    qDebug("vmOutput();");

}

void BrainWindow::vmConsumedInput() {
    qDebug("BrainWindow::vmConsumedInput()");
    QString orig = ui->leInput->text();
    QString consumed;
    if(orig.size() == 1) {
        consumed = QString();
    } else {
        // "pop" one character
        consumed = orig.right(orig.size()-1);
    }

    ui->leInput->setText(consumed);
}

void BrainWindow::vmDPChanged(DPType dp) {
    // TODO: CHANGE MEMORY TABLE!
    if(m_debuggingMode) {
        /* FIXME: QLineEdit::setText() is hideously slow and calling it repeatedly will make X11
           beg for mercy */
        ui->leDP->setText(tr("%1").arg(dp));
        changeMemView(dp);
    }
}

/* shows 4 cells of memory to the left and right of the DP, so total number of cells
   shown is 9 (4 cells + the cell the DP is on + 4 cells). If DP is less than 4, it just
   shows cells 0 - 9, and if the DP wanders near BfVM::MAX_MEM_ADDR-4 it just shows
   everything up to the largest address */
void BrainWindow::changeMemView(DPType dp) {
    ui->tblMemory->setVerticalHeaderLabels(makeHeaderForPos(dp));
    int min, selected = 4;
    if(dp <= 4) {
        min = 0;
        selected = dp;
    } else if(dp > 4 && dp < BfVM::MAX_MEM_ADDR-4) {
        min = dp-4;
    } else {
        min = BfVM::MAX_MEM_ADDR - 8;
    }

    for(int i = 0; i <= 8; ++i) {
        ui->tblMemory->item(i,0)->setText(tr("%1").arg(m_memMap[min+i]));
    }

    resizeMemTable();
    ui->tblMemory->selectRow(selected);

}

void BrainWindow::vmMemChanged(DPType dp, Memtype mt) {
    m_memMap[dp] = mt;
    int dpPos = 4;
    if(m_debuggingMode) {
        if(dp <= 4) {
            dpPos = dp;
        } else if (dp > BfVM::MAX_MEM_ADDR-4) {
            dpPos = 4+(dp - (BfVM::MAX_MEM_ADDR-4));
        }
        ui->tblMemory->item(dpPos,0)->setText(tr("%1").arg(mt));
        resizeMemTable();
    }
}


/**
  The VM sends the reset and clear signals when it enters the clear state, so
  clearing doesn't have to explicitly do the same things as reset
  */
void BrainWindow::vmCleared() {
    qDebug() << "VM cleared";
    disableRunActions(true);
    ui->teDebugProgram->setPlainText(QString());
}

void BrainWindow::vmReset() {
    qDebug() << "VM reset";

    ui->leDP->setText(tr("0"));
    ui->leIP->setText(tr("0"));
    ui->leInput->setText(QString());

    if(m_debuggingMode) {
        // reset the debugger program view cursor too
        moveDbgCursor(ui->teDebugProgram, 0, true);
    }

    // clears the "local copy" of the VM's memory we're keeping around
    clearMemMap();
    // clears the memory view in the GUI.
    clearMemoryTbl();

}

void BrainWindow::vmFinished() {
    qDebug() << "VM finished";
    // Ensure all actions are in the right state
    ui->actionRun->setDisabled(true);
    ui->actionStep->setDisabled(true);

    ui->actionClear->setEnabled(true);
    ui->actionReset->setEnabled(true);
}

void BrainWindow::vmHeartBeat(const IPType &ip) {
    qDebug("VM heartbeat, IP %d",ip);

    if(m_debuggingMode) {

        /* FIXME: QLineEdit::setText() is hideously slow and calling it repeatedly will make X11
       beg for mercy */
        ui->leIP->setText(QString::number((unsigned int)ip));
        /* should I replace call to m_mappings->value with an array lookup? The array
       would be the same length as the _compiled_ program itself, and contain the mapped
       values in it. So looking up m_mapArry[i] would look up the mapping for the command
       at IP position i. This would, of course, mean that I'd be trading off processing time
       for memory usage, and a program of maximum allowed size (2^32 instructions) would
       cause the GUI to eat up a whopping 16 gigaBYTES of memory. On the other hand, how
       likely are we to see a Brainfuck program with 137 438 953 472 commands in it? */

        const quint32 srcPos = m_mappings->value(ip);
        // moves the cursor to the corresponding position in the source
        moveDbgCursor(ui->teDebugProgram, srcPos);
    }
}

void BrainWindow::vmBreakPoint(IPType ip, DPType dp) {
    qDebug("BrainWindow::vmBreakPoint();");
    ui->actionDebugging_mode->setChecked(true);
    vmHeartBeat(ip);
    vmDPChanged(dp);
}

void BrainWindow::closeEvent(QCloseEvent *e) {
    if(maybeSave()) {
        e->accept();
    } else {
        e->ignore();
    }
}

void BrainWindow::on_actionLoad_program_triggered()
{
    emit compile(ui->teIde->toPlainText());
}

void BrainWindow::sendOutput() {
    qDebug("BrainWindow::sendOutput()");
    emit output(ui->leInput->text());
}

void BrainWindow::moveDbgCursor(QPlainTextEdit* te, const quint32 dir, bool reset) {
    QTextCursor tc = te->textCursor();

    // reset cursor position or move it to an absolute position
    if(reset) {
        tc.setPosition(0);
    } else {
        tc.setPosition(dir);
        /* moves the cursor forwards but keeps the anchor in place. Now the
           first character should be selected */
        tc.movePosition(QTextCursor::NextCharacter, QTextCursor::KeepAnchor);
    }
    te->setTextCursor(tc);
}


void BrainWindow::vmRunning(bool running) {
    qDebug() << "vmRunning()" << running;
    ui->actionRun->setChecked(running);
    /* disable the ability to change the text input buffer while the VM is running */
    ui->leInput->setDisabled(running);

}

void BrainWindow::clearMemoryTbl() {
    ui->tblMemory->setVerticalHeaderLabels(makeHeaderForPos(0));
    for(int i = 0; i < 9; ++i) {
        ui->tblMemory->item(0,i)->setText(tr("0"));
    }
    resizeMemTable();
}



QStringList BrainWindow::makeHeaderForPos(DPType dp) {
    QStringList result;
    DPType min, max;


    /* the DP is always at the center of the table, at position 4 */

    if(dp<=4) {
        return *m_memStartHeader;
    } else if(dp > 4 && dp < BfVM::MAX_MEM_ADDR-4) {
        min = dp-4;
    } else { // the last 9 elements
        max = BfVM::MAX_MEM_ADDR;
        min = max - 8;
    }

    for(int i = min; i <= min+8; ++i) {
        result << tr("%1").arg(i);
    }
    return result;
}

void BrainWindow::setCurrentDocument(const QString &text) {

    ui->teIde->document()->setModified(false);
    setWindowModified(false);
    //: translations
    setWindowFilePath(text.isEmpty()? trUtf8("UNTITLED") : text);
    m_documentName = text;
    m_documentDirty = false;
}

inline void BrainWindow::resizeMemTable() {
    //ui->tblMemory->resizeColumnsToContents(); // FIXME: not doing anything. Why?
    //ui->tblMemory->resizeRowsToContents();
    ui->tblMemory->horizontalHeader()->setStretchLastSection(true);
    ui->tblMemory->verticalHeader()->setStretchLastSection(true);
}


void BrainWindow::setDocumentIsDirty() {
    /* TODO: the compile action is now enabled EVERY time the document changes.
       Might want to do this only once... */
    ui->actionLoad_program->setEnabled(true);

    bool dirty = ui->teIde->document()->isModified();

    qDebug() << "BrainWindow::setDocumentIsDirty()"<<dirty;
    m_documentDirty = dirty;

    // if the window title doesn't contain anything, setWindowModified() will complain
    this->setWindowModified(dirty);
    ui->actionSave->setEnabled(dirty);
}




bool BrainWindow::saveAs() {
    QString fileName = QFileDialog::getSaveFileName(this,
                                                    trUtf8("Save Brainfuck source as..."));
    if(fileName.isEmpty())
        return false;

    qDebug() << "on_actionSaveAs_triggered() filename" <<fileName;

    return writeToFile(ui->teIde->toPlainText(), fileName);

}


bool BrainWindow::maybeSave() {
    if(m_documentDirty) {
        QMessageBox::StandardButton ret;
        ret = QMessageBox::warning(this, trUtf8("QtBrain"),
                                   tr("The document has been modified.\n"
                                      "Do you want to save your changes?"),
                                   QMessageBox::Save | QMessageBox::Discard | QMessageBox::Cancel);
        if (ret == QMessageBox::Save)
            return save();
        else if (ret == QMessageBox::Cancel)
            return false;
    }
    return true;
}


bool BrainWindow::save() {
    if(m_documentName.isEmpty())
        return saveAs();
    else
        return writeToFile(ui->teIde->toPlainText(), m_documentName);
}


bool BrainWindow::writeToFile(QString data, QString fileName) {
    QFile file(fileName);
    if(!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        QMessageBox::warning(this, trUtf8("QtBrain"),
                             trUtf8("Error writing to file %1:\n%2").arg(file.fileName())
                             .arg(file.errorString()));
        return false;
    }

    QTextStream out(&file);
    out << data;
    setCurrentDocument(file.fileName());
    statusBar()->showMessage(trUtf8("File saved"), 3000);
    return true;
}


void BrainWindow::on_slTickDelay_valueChanged(int value)
{
    ui->leTickDelay->setText(QString::number(value));
}


void BrainWindow::open() {
    if(maybeSave()) {
        QString fileName = QFileDialog::getOpenFileName(this);
        if(!fileName.isEmpty()) {
            QFile file(fileName);
            if(!file.open(QFile::ReadOnly | QFile::Text)) {
                QMessageBox::warning(this, trUtf8("QtBrain"),
                                     trUtf8("Error reading file %1: %2")
                                     .arg(fileName).arg(file.errorString()));
                return;
            }
            QTextStream in(&file);
#ifndef QT_NO_CURSOR
            QApplication::setOverrideCursor(Qt::WaitCursor);
#endif
            ui->teIde->setPlainText(in.readAll());
#ifndef QT_NO_CURSOR
            QApplication::restoreOverrideCursor();
#endif
            setCurrentDocument(fileName);
            statusBar()->showMessage(trUtf8("File loaded"), 3000);
        }


    }
}



void BrainWindow::on_actionOpen_triggered()
{
    open();
}


void BrainWindow::on_actionSave_triggered()
{
    save();
}

void BrainWindow::on_actionSaveAs_triggered()
{
    saveAs();
}

void BrainWindow::on_actionNew_triggered()
{
    if(maybeSave()) {
        ui->teIde->clear();
        setCurrentDocument(QString());
        ui->actionLoad_program->setEnabled(false);
    }
}

void BrainWindow::on_actionDebugging_mode_toggled(bool checked)
{
    m_debuggingMode = checked;
    ui->actionStep->setEnabled(checked);
    ui->leDP->setEnabled(checked);
    ui->leIP->setEnabled(checked);
    ui->tblMemory->setEnabled(checked);
    if(!checked) { // if debugging mode is not on, hide the "IP cursor" in the debugger
        clearMemoryTbl();
        ui->teDebugProgram->moveCursor(QTextCursor::Start);
    }
}
