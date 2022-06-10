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

#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include "database.h"

#include <QMainWindow>
#include <QMenuBar>
#include <QStatusBar>
#include <QTabWidget>

Q_DECLARE_METATYPE(std::string)

class CodeView;
class BrowserWidget;
class FindDialog;

class MainWindow : public QMainWindow
{
    Q_OBJECT

  public:
    explicit MainWindow(QString dbFile, QString sourceDir, QWidget *parent = 0);
    virtual ~MainWindow();

  protected:
    void createMenusAndBars();
    void openCodeViewTab(QString filename);
    void closeCodeViewTab(int index);

  protected Q_SLOTS:
    void selectLine(QString filename, QString line);
    void unselectLine();
    void about();
    void find();

  protected:
    QTabWidget *centralTabWidget;
    BrowserWidget *browser;
    QMap<QString, CodeView *> views;
    QMenuBar *menuBar;
    QStatusBar *statusBar;
    DbManager database;
    QString sourceDir;
    FindDialog *findDialog;
};

#endif // MAINWINDOW_H
