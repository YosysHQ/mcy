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

#include <QtWidgets>
#include "createwizard.h"
#include "designsetup.h"

DesignSetupPage::DesignSetupPage(QWidget *parent)
    : QWizardPage(parent)
{
    setTitle("Design Setup...");

    top = new QLineEdit();
    top->setReadOnly(false);
    QObject::connect(top,  &QLineEdit::textChanged, this, &DesignSetupPage::textChanged);

    fileList = new QListWidget;
    fileList->setDragDropMode(QAbstractItemView::InternalMove);
    fileList->setSelectionMode(QAbstractItemView::MultiSelection);

    QDialogButtonBox *buttonBox_filesel = new QDialogButtonBox(Qt::Vertical, this);
    addButton = buttonBox_filesel->addButton("Add", QDialogButtonBox::ActionRole);
    addButton->setEnabled(true);
    deleteButton = buttonBox_filesel->addButton("Delete", QDialogButtonBox::ActionRole);
    deleteButton->setEnabled(true);

    QObject::connect(addButton,  &QPushButton::clicked, this, &DesignSetupPage::addFiles);
    QObject::connect(deleteButton, &QPushButton::clicked, this, &DesignSetupPage::deleteFiles);

    QDialogButtonBox *buttonBox_script = new QDialogButtonBox(Qt::Vertical, this);
    editButton = buttonBox_script->addButton("Edit", QDialogButtonBox::ActionRole);
    editButton->setEnabled(true);
    resetButton = buttonBox_script->addButton("Reset", QDialogButtonBox::ActionRole);
    resetButton->setEnabled(false);

    QObject::connect(editButton,  &QPushButton::clicked, this, &DesignSetupPage::editScript);
    QObject::connect(resetButton, &QPushButton::clicked, this, &DesignSetupPage::resetScript);

    script = new QTextEdit();
    script->show();
    script->setReadOnly(true);

    QHBoxLayout *filesel_layout = new QHBoxLayout;
    filesel_layout->setContentsMargins(0, 0, 0, 0);
    filesel_layout->addWidget(fileList);
    filesel_layout->addWidget(buttonBox_filesel);

    QHBoxLayout *script_layout = new QHBoxLayout;
    script_layout->setContentsMargins(0, 0, 0, 0);
    script_layout->addWidget(script);
    script_layout->addWidget(buttonBox_script);
    
    QVBoxLayout *layout = new QVBoxLayout;
    layout->addWidget(new QLabel("Top:"));
    layout->addWidget(top);
    layout->addWidget(new QLabel("Design files:"));
    layout->addLayout(filesel_layout);
    layout->addWidget(new QLabel("Script:"));
    layout->addLayout(script_layout);
    setLayout(layout);

    registerField("top*", top);
    registerField("theFileList*", this, "theFileList");
    registerField("script*", this, "theScript");
}

QString DesignSetupPage::theScript() const
{
    return script->toPlainText();
}

int DesignSetupPage::nextId() const
{
    return CreateWizard::Page_TestSetup;
}

void DesignSetupPage::textChanged(const QString &text)
{
    updateScript();
}

void DesignSetupPage::updateScript()
{
    script->clear();
    for(int i = 0; i < fileList->count(); ++i)
    {
        script->append(QString("read_verilog ") + fileList->item(i)->text());
    }
    script->append(QString("prep -top ") + top->text());
    script->append("");
}

void DesignSetupPage::addFiles()
{
    QFileDialog dialog(this);
    dialog.setFileMode(QFileDialog::ExistingFiles);
    dialog.setDirectory(field("directory").toString());
    dialog.setNameFilter(trUtf8("Design files (*.v *.sv *.vh)"));
    QDir dir(field("directory").toString());
    if (dialog.exec()) {
        for(auto file : dialog.selectedFiles())
            fileList->addItem(dir.relativeFilePath(file));        
    }
    updateScript();
    Q_EMIT completeChanged();
}

QStringList DesignSetupPage::theFileList() const
{
    QStringList list;
    for(int i = 0; i < fileList->count(); ++i)
    {
        list << fileList->item(i)->text();
    }    
    return list;
}

void DesignSetupPage::deleteFiles()
{
    for(auto name : fileList->selectedItems()) {
        delete fileList->takeItem(fileList->row(name));
    }
    updateScript();
    Q_EMIT completeChanged();
}

void DesignSetupPage::editScript()
{
    addButton->setEnabled(false);
    deleteButton->setEnabled(false);
    editButton->setEnabled(false);
    resetButton->setEnabled(true);
    script->setReadOnly(false);
    top->setReadOnly(true);
}

void DesignSetupPage::resetScript()
{
    addButton->setEnabled(true);
    deleteButton->setEnabled(true);
    editButton->setEnabled(true);
    resetButton->setEnabled(false);
    script->setReadOnly(true);
    top->setReadOnly(false);
    updateScript();
}
