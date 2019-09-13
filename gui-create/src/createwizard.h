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

#ifndef CREATEWIZARD_H
#define CREATEWIZARD_H

#include <QWizard>
#include <QLabel>
#include <QListWidget>
#include <QTextEdit>

class CreateWizard : public QWizard
{
    Q_OBJECT

public:
    enum { Page_Intro, Page_SelectDirectory, Page_SelectFiles, Page_ScriptEdit, Page_Options };
    CreateWizard(QWidget *parent = 0);

    void accept() override;
private Q_SLOTS:
    void showHelp();    
};

class IntroPage : public QWizardPage
{
    Q_OBJECT

public:
    IntroPage(QWidget *parent = 0);

    int nextId() const override;

private:
    QLabel *topLabel;
};

class SelectDirectoryPage : public QWizardPage
{
    Q_OBJECT

public:
    SelectDirectoryPage(QWidget *parent = 0);

    int nextId() const override;
    bool isComplete() const override;
    
private:
    void browse();

    QLineEdit *directory;
};

class SelectFilesPage : public QWizardPage
{
    Q_OBJECT
    Q_PROPERTY(QStringList theFileList READ theFileList NOTIFY theFileListChanged)
public:
    SelectFilesPage(QWidget *parent = 0);

    int nextId() const override;

    QStringList theFileList() const;
private:
    QListWidget *fileList;
    QPushButton *addButton;
    QPushButton *deleteButton;
private Q_SLOTS:
    void addFiles();
    void deleteFiles();
Q_SIGNALS:
    void theFileListChanged();
};

class ScriptEditPage : public QWizardPage
{
    Q_OBJECT
    Q_PROPERTY(QString theScript READ theScript NOTIFY theScriptChanged)

public:
    ScriptEditPage(QWidget *parent = 0);

    int nextId() const override;
    void initializePage() override;

    QString theScript() const;
private:
    QTextEdit *text;
Q_SIGNALS:
    void theScriptChanged();    
};

class OptionsPage : public QWizardPage
{
    Q_OBJECT

public:
    OptionsPage(QWidget *parent = 0);

    int nextId() const override;

private:
    QLineEdit *mutations_size;
    QLineEdit *pick_cover_prcnt;
    QLineEdit *weight_cover;
    QLineEdit *weight_pq_w;
    QLineEdit *weight_pq_b;
    QLineEdit *weight_pq_c;
    QLineEdit *weight_pq_s;

    QLineEdit *weight_pq_mw;
    QLineEdit *weight_pq_mb;
    QLineEdit *weight_pq_mc;
    QLineEdit *weight_pq_ms;
};

#endif // CREATEWIZARD_H
