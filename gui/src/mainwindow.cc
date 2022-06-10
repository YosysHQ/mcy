/*
 *  mcy-gui -- Mutation Cover with Yosys GUI
 *
 *  Copyright (C) 2019  Miodrag Milanovic <micko@yosyshq.com>
 *
 *  Permission to use, copy, modify, and/or distribute this software for any
 *  purpose with or without fee is hereby granted, provided that the above
 *  copyright notice and this permission notice appear in all copies.
 *
 *  THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 *  WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 *  MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
 *  ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 *  WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
 *  ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
 *  OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 *
 */

#include "mainwindow.h"
#include <QDir>
#include <QGridLayout>
#include <QMessageBox>
#include <QSplitter>
#include <QTabBar>
#include "browserwidget.h"
#include "codeview.h"
#include "finddialog.h"

static void initBasenameResource() { Q_INIT_RESOURCE(base); }

MainWindow::MainWindow(QString dbFile, QString sourceDir, QWidget *parent)
        : QMainWindow(parent), database(dbFile), sourceDir(sourceDir), findDialog(nullptr)
{
    initBasenameResource();
    qRegisterMetaType<std::string>();

    setObjectName(QStringLiteral("MainWindow"));
    resize(1024, 768);

    setWindowIcon(QIcon(":/icons/resources/yosyshq.png"));

    // Create and deploy widgets on main screen
    QWidget *centralWidget = new QWidget(this);

    QSplitter *splitter_h = new QSplitter(Qt::Horizontal, centralWidget);

    centralTabWidget = new QTabWidget(this);
    centralTabWidget->setTabsClosable(true);
    centralTabWidget->setMovable(true);
    connect(centralTabWidget, &QTabWidget::tabCloseRequested, this, &MainWindow::closeCodeViewTab);

    splitter_h->addWidget(centralTabWidget);

    browser = new BrowserWidget(&database);
    browser->setMinimumWidth(350);
    connect(browser, &BrowserWidget::selectLine, this, &MainWindow::selectLine);
    connect(browser, &BrowserWidget::unselectLine, this, &MainWindow::unselectLine);

    splitter_h->addWidget(browser);
    splitter_h->setCollapsible(0, false);
    splitter_h->setCollapsible(1, false);
    splitter_h->setStretchFactor(0, 1);

    QGridLayout *gridLayout = new QGridLayout(centralWidget);
    gridLayout->setSpacing(6);
    gridLayout->setContentsMargins(2, 2, 2, 2);
    gridLayout->addWidget(splitter_h, 0, 0, 1, 1);
    setCentralWidget(centralWidget);

    createMenusAndBars();

    for (auto filename : database.getFileList()) {
        openCodeViewTab(filename);
    }
}

MainWindow::~MainWindow() {}

void MainWindow::createMenusAndBars()
{
    menuBar = new QMenuBar();
    QMenu *menuFile = new QMenu("&File", menuBar);
    QMenu *menuFind = new QMenu("Find", menuBar);
    QMenu *menuHelp = new QMenu("&Help", menuBar);

    QAction *actionExit = new QAction("Exit", this);
    actionExit->setIcon(QIcon(":/icons/resources/exit.png"));
    actionExit->setShortcuts(QKeySequence::Quit);
    actionExit->setStatusTip("Exit the application");
    connect(actionExit, &QAction::triggered, this, &MainWindow::close);
    menuFile->addAction(actionExit);

    QAction *actionFind = new QAction("Find", this);
    actionFind->setShortcuts(QKeySequence::Find);
    connect(actionFind, &QAction::triggered, this, &MainWindow::find);
    menuFind->addAction(actionFind);

    QAction *actionAbout = new QAction("&About", this);
    actionAbout->setStatusTip("Show the application's about box");
    connect(actionAbout, &QAction::triggered, this, &MainWindow::about);
    menuHelp->addAction(actionAbout);

    menuBar->addAction(menuFile->menuAction());
    menuBar->addAction(menuFind->menuAction());
    menuBar->addAction(menuHelp->menuAction());
    setMenuBar(menuBar);

    statusBar = new QStatusBar();
    setStatusBar(statusBar);
}

void MainWindow::about()
{
    QMessageBox::about(this, tr("MCY-GUI"),
                       tr("The <b>Mutation Cover with Yosys GUI</b> is part of "
                          "<br/><b>YosysHQ</b> solution for formal verification."));
}

void MainWindow::find()
{
    QWidget *current = centralTabWidget->currentWidget();
    if (current != nullptr) {
        if (std::string(current->metaObject()->className()) == "CodeView") {
            CodeView *code = (CodeView *)current;
            if (findDialog && !findDialog->isForCodeView(code)) {
                findDialog->close();
                delete findDialog;
                findDialog = nullptr;
            }
            if (!findDialog)
                findDialog = new FindDialog(this, code);
            findDialog->show();
            findDialog->raise();
            findDialog->activateWindow();
        }
    }
}

void MainWindow::openCodeViewTab(QString filename)
{
    if (!views.contains(filename)) {
        CodeView *code = new CodeView(filename, this);
        if (sourceDir.isEmpty()) {
            QString content = database.getFileContent(filename);
            if (content.isEmpty()) {
                QMessageBox::critical(this, tr("MCY-GUI"), tr("Database does not contain this file !!!"));
                return;
            }
            code->loadContent(content.toLocal8Bit().constData());
        } else {
            QDir path(sourceDir);
            QString filePath = path.filePath(filename);
            QFile file(filePath);
            if (file.open(QIODevice::ReadOnly | QIODevice::Text)) {
                QByteArray contents = file.readAll();
                code->loadContent(contents.constData());
            } else {
                QMessageBox::critical(this, tr("MCY-GUI"), tr("File does not exists !!!"));
                return;
            }
        }
        QFileInfo fi = QFileInfo(filename);
        code->setCoverage(database.getCoverage(filename), database.getLinesYetToCover(filename));
        views.insert(filename, code);
        int idx = centralTabWidget->addTab(code, QIcon(":/icons/resources/page_white_text.png"), fi.completeBaseName() + "." + fi.completeSuffix());
        centralTabWidget->setTabToolTip(idx, filename);
        connect(code, &ScintillaEdit::marginClicked, [=](int position, int modifiers, int margin) {
            QString source = filename + ":" + QString::number(code->lineFromPosition(position) + 1);
            source = browser->selectSource(source);
            QStringList param = source.split(':');
            selectLine(param.at(0), param.at(1));
        });
    }
    centralTabWidget->setCurrentWidget(views[filename]);
}

void MainWindow::closeCodeViewTab(int index)
{
    QWidget *current = centralTabWidget->widget(index);
    if (current != nullptr) {
        if (std::string(current->metaObject()->className()) == "CodeView") {
            CodeView *code = (CodeView *)current;
            views.remove(code->getFilename());
        }
    }
    centralTabWidget->removeTab(index);
}

void MainWindow::selectLine(QString filename, QString line)
{
    openCodeViewTab(filename);
    QWidget *current = centralTabWidget->currentWidget();
    if (current != nullptr) {
        if (std::string(current->metaObject()->className()) == "CodeView") {
            CodeView *code = (CodeView *)current;
            code->selectLine(line);
        }
    }
}

void MainWindow::unselectLine()
{
    QWidget *current = centralTabWidget->currentWidget();
    if (current != nullptr) {
        if (std::string(current->metaObject()->className()) == "CodeView") {
            CodeView *code = (CodeView *)current;
            code->unselectLine();
        }
    }
}