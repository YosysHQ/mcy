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
#include "selectdir.h"

SelectDirectoryPage::SelectDirectoryPage(QWidget *parent)
    : QWizardPage(parent)
{
    setTitle(tr("Select working directory"));

    QHBoxLayout *layout = new QHBoxLayout;
    
    QPushButton *browseButton = new QPushButton(tr("&Browse..."), this);
    connect(browseButton, &QAbstractButton::clicked, this, &SelectDirectoryPage::browse);
    directory = new QLineEdit();
    directory->setReadOnly(true);
    directory->setText(QDir::toNativeSeparators(QDir::currentPath()));
    layout->addWidget(directory);
    layout->addWidget(browseButton);
    setLayout(layout);
    registerField("directory*", directory);
}

void SelectDirectoryPage::browse()
{
    QString dir =
        QDir::toNativeSeparators(QFileDialog::getExistingDirectory(this, tr("Select Working Directory..."), QDir::currentPath()));

    if (!dir.isEmpty()) {
        directory->setText(dir);
        Q_EMIT completeChanged();
    }
}

bool SelectDirectoryPage::isComplete() const
{
    QFileInfo check(directory->text());
    return (check.exists() && check.isDir());
}

int SelectDirectoryPage::nextId() const
{
    return CreateWizard::Page_DesignSetup;
}

