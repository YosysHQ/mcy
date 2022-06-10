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

#include "finddialog.h"
#include "codeview.h"

#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QMessageBox>
#include <QPushButton>

FindDialog::FindDialog(QWidget *parent, CodeView *code) : QDialog(parent), code(code)
{
    QLabel *findLabel = new QLabel("Search for:");
    lineEdit = new QLineEdit;

    findNextButton = new QPushButton("Find &Next");
    findPrevButton = new QPushButton("Find &Previous");

    QHBoxLayout *layout = new QHBoxLayout;
    layout->addWidget(findLabel);
    layout->addWidget(lineEdit);
    layout->addWidget(findNextButton);
    layout->addWidget(findPrevButton);

    setLayout(layout);
    setWindowTitle("Find");
    connect(findNextButton, &QPushButton::clicked, [this]() { findClicked(true); });
    connect(findPrevButton, &QPushButton::clicked, [this]() { findClicked(false); });
}

void FindDialog::findClicked(bool forward)
{
    QString text = lineEdit->text();
    if (text.isEmpty()) {
        QMessageBox::information(this, "Empty Field", "Please enter text to search.");
    } else {
        code->find(text, forward);
    }
}

bool FindDialog::isForCodeView(CodeView *check)
{
    return check == code;
}
