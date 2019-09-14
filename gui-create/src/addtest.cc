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
#include "addtest.h"

AddTestDialog::AddTestDialog(QString path,QWidget *parent)
    : QDialog(parent), path(path)
{
    setWindowTitle("Add Test...");

    name = new QLineEdit();        
    QDialogButtonBox *buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);

    connect(buttonBox, &QDialogButtonBox::accepted, this, &QDialog::accept);
    connect(buttonBox, &QDialogButtonBox::rejected, this, &QDialog::reject);
    
    QStringList testTypeList;
    testTypeList << "icarus" << "isim" << "modelsim" << "verilator" << "xsim" << "custom";

    testType = new QComboBox();
    testType->addItems(testTypeList);    

    QHBoxLayout *fileLayout = new QHBoxLayout;    
    QPushButton *browseButton = new QPushButton(tr("&Browse..."), this);
    connect(browseButton, &QAbstractButton::clicked, this, &AddTestDialog::browseFile);
    file = new QLineEdit();
    file->setReadOnly(true);
    fileLayout->addWidget(file);
    fileLayout->addWidget(browseButton);    

    QVBoxLayout *layout = new QVBoxLayout;
    layout->addWidget(new QLabel("Name:"));    
    layout->addWidget(name);    
    layout->addWidget(new QLabel("Type:"));    
    layout->addWidget(testType);    
    layout->addWidget(new QLabel("File:"));
    layout->addLayout(fileLayout);
    layout->addSpacerItem(new QSpacerItem(20, 40, QSizePolicy::Minimum, QSizePolicy::Expanding));
    layout->addWidget(buttonBox);

    setLayout(layout);
}

void AddTestDialog::browseFile()
{
    QString fileName =
        QDir::toNativeSeparators(QFileDialog::getOpenFileName(this, tr("Select Test File..."), path, "Test Files (*.*)"));

    if (!fileName.isEmpty()) {
        file->setText(fileName);
    }
}

