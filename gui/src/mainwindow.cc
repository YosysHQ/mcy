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
#include <QSplitter>
#include <QTabBar>
#include "browserwidget.h"

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

    QTabWidget *centralTabWidget = new QTabWidget();
    centralTabWidget->setTabsClosable(true);

    splitter_h->addWidget(centralTabWidget);

    BrowserWidget *browser = new BrowserWidget(&database);
    browser->setMinimumWidth(300);

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
}

MainWindow::~MainWindow() {}

void MainWindow::createMenusAndBars()
{
    menuBar = new QMenuBar();
    QMenu *menu_File = new QMenu("&File", menuBar);
    QMenu *menu_Help = new QMenu("&Help", menuBar);

    menuBar->addAction(menu_File->menuAction());
    menuBar->addAction(menu_Help->menuAction());
    setMenuBar(menuBar);

    mainToolBar = new QToolBar();
    addToolBar(Qt::TopToolBarArea, mainToolBar);

    statusBar = new QStatusBar();
    setStatusBar(statusBar);
}
