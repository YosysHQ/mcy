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

#ifndef DESIGNSETUP_H
#define DESIGNSETUP_H

#include <QWizard>
#include <QLabel>
#include <QListWidget>
#include <QTextEdit>

class DesignSetupPage : public QWizardPage
{
    Q_OBJECT
    Q_PROPERTY(QStringList theFileList READ theFileList NOTIFY theFileListChanged)
    Q_PROPERTY(QString theScript READ theScript NOTIFY theScriptChanged)
public:
    DesignSetupPage(QWidget *parent = 0);

    int nextId() const override;

    QStringList theFileList() const;
    QString theScript() const;
private:
    void updateScript();

    QLineEdit *top;
    QListWidget *fileList;
    QPushButton *addButton;
    QPushButton *deleteButton;
    QTextEdit *script;
    QPushButton *editButton;
    QPushButton *resetButton;
private Q_SLOTS:
    void addFiles();
    void deleteFiles();
    void editScript();
    void resetScript();
Q_SIGNALS:
    void theFileListChanged();
    void theScriptChanged();    
};

#endif // DESIGNSETUP_H
