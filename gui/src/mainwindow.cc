/*
 *  mcy-gui -- Mutation Cover with Yosys GUI
 *
 *  Copyright (C) 2019  Miodrag Milanovic <miodrag@symbioticeda.com>
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
#include <QGridLayout>
#include <QMessageBox>
#include <QSplitter>
#include <QTabBar>
#include "browserwidget.h"
#include "codeview.h"

static void initBasenameResource() { Q_INIT_RESOURCE(base); }

MainWindow::MainWindow(QString workingDir, QWidget *parent)
        : QMainWindow(parent), database(workingDir + "/database/db.sqlite3")
{
    initBasenameResource();
    qRegisterMetaType<std::string>();

    setObjectName(QStringLiteral("MainWindow"));
    resize(1024, 768);

    setWindowIcon(QIcon(":/icons/resources/symbiotic.png"));

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
    QMenu *menuHelp = new QMenu("&Help", menuBar);

    QAction *actionExit = new QAction("Exit", this);
    actionExit->setIcon(QIcon(":/icons/resources/exit.png"));
    actionExit->setShortcuts(QKeySequence::Quit);
    actionExit->setStatusTip("Exit the application");
    connect(actionExit, &QAction::triggered, this, &MainWindow::close);
    menuFile->addAction(actionExit);

    QAction *actionAbout = new QAction("&About", this);
    actionAbout->setStatusTip("Show the application's about box");
    connect(actionAbout, &QAction::triggered, this, &MainWindow::about);
    menuHelp->addAction(actionAbout);

    menuBar->addAction(menuFile->menuAction());
    menuBar->addAction(menuHelp->menuAction());
    setMenuBar(menuBar);

    mainToolBar = new QToolBar();
    addToolBar(Qt::TopToolBarArea, mainToolBar);

    statusBar = new QStatusBar();
    setStatusBar(statusBar);
}

void MainWindow::about()
{
    QMessageBox::about(this, tr("MCY-GUI"), tr("The <b>Mutation Cover with Yosys GUI</b> is part of "
                                               "<br/><b>SymbioticEDA</b> solution for formal verification."));
}

void MainWindow::openCodeViewTab(QString filename)
{
    if (!views.contains(filename)) {
        CodeView *code = new CodeView(filename, this);
        code->loadContent(database.getFileContent(filename).toLocal8Bit().constData());
        code->setCoverage(database.getCoverage(filename));
        views.insert(filename, code);
        centralTabWidget->addTab(code, QIcon(":/icons/resources/page_white_text.png"), filename);
        connect(code, &CodeView::updateUi, [=](int updated) {
            QString source = filename + ":" + QString::number(code->lineFromPosition(code->currentPos()) + 1);
            browser->selectSource(source);
        });
        connect(code, &ScintillaEdit::marginClicked, [=](int position, int modifiers, int margin) {
            code->selectLine(code->lineFromPosition(position) + 1);
        });
    } else {
        centralTabWidget->setCurrentWidget(views[filename]);
    }
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

void MainWindow::selectLine(QString filename, int line)
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