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
#include "testsetup.h"
#include "addtest.h"

TestSetupPage::TestSetupPage(QWidget *parent)
    : QWizardPage(parent)
{
    setTitle("Test Setup...");

    mutations_size = new QLineEdit();
    mutations_size->setValidator( new QIntValidator(1, 100000, this) );
    registerField("mutations_size*", mutations_size);

    testList = new QListWidget;
    testList->setDragDropMode(QAbstractItemView::InternalMove);
    testList->setSelectionMode(QAbstractItemView::MultiSelection);
    QObject::connect(testList, &QListWidget::itemDoubleClicked, this, &TestSetupPage::editTest);

    QDialogButtonBox *buttonBox_test = new QDialogButtonBox(Qt::Vertical, this);
    addTestButton = buttonBox_test->addButton("Add", QDialogButtonBox::ActionRole);
    delTestButton = buttonBox_test->addButton("Delete", QDialogButtonBox::ActionRole);

    QObject::connect(addTestButton, &QPushButton::clicked, this, &TestSetupPage::addTest);
    QObject::connect(delTestButton, &QPushButton::clicked, this, &TestSetupPage::delTest);


    refTestList = new QListWidget;
    refTestList->setDragDropMode(QAbstractItemView::InternalMove);
    refTestList->setSelectionMode(QAbstractItemView::MultiSelection);
    QObject::connect(refTestList, &QListWidget::itemDoubleClicked, this, &TestSetupPage::editTest);

    QDialogButtonBox *buttonBox_refTest = new QDialogButtonBox(Qt::Vertical, this);
    addRefTestButton = buttonBox_refTest->addButton("Add", QDialogButtonBox::ActionRole);
    delRefTestButton = buttonBox_refTest->addButton("Delete", QDialogButtonBox::ActionRole);

    QObject::connect(addRefTestButton, &QPushButton::clicked, this, &TestSetupPage::addRefTest);
    QObject::connect(delRefTestButton, &QPushButton::clicked, this, &TestSetupPage::delRefTest);


    QHBoxLayout *test_layout = new QHBoxLayout;
    test_layout->setContentsMargins(0, 0, 0, 0);
    test_layout->addWidget(testList);
    test_layout->addWidget(buttonBox_test);

    QHBoxLayout *refTest_layout = new QHBoxLayout;
    refTest_layout->setContentsMargins(0, 0, 0, 0);
    refTest_layout->addWidget(refTestList);
    refTest_layout->addWidget(buttonBox_refTest);

    QVBoxLayout *layout = new QVBoxLayout;
    layout->addWidget(new QLabel("Mutation size:"));
    layout->addWidget(mutations_size);
    layout->addWidget(new QLabel("Tests:"));
    layout->addLayout(test_layout);
    layout->addWidget(new QLabel("Reference tests:"));
    layout->addLayout(refTest_layout);

    setLayout(layout);
}

int TestSetupPage::nextId() const
{
    return -1;
}

void TestSetupPage::addTest()
{
    AddTestDialog dlg(QDir::cleanPath(field("directory").toString()), false, nullptr, this);
    dlg.setModal(true);
    if (dlg.exec() == QDialog::Accepted)
    {
        TestFile item = dlg.getItem();
        QListWidgetItem* newItem = new QListWidgetItem(item.name, testList);
        newItem->setData(Qt::UserRole, QVariant::fromValue(item));
    }
}

void TestSetupPage::delTest()
{    
}

void TestSetupPage::addRefTest()
{
    AddTestDialog dlg(QDir::cleanPath(field("directory").toString()), true, nullptr, this);
    dlg.setModal(true);
    if (dlg.exec() == QDialog::Accepted)
    {
        TestFile item = dlg.getItem();
        QListWidgetItem* newItem = new QListWidgetItem(item.name, refTestList);
        newItem->setData(Qt::UserRole, QVariant::fromValue(item));
    }
}

void TestSetupPage::delRefTest()
{
}

bool TestSetupPage::isNameValid(QString name)
{
    for (int i = 0; i < testList->count(); ++i) {        
        if (testList->item(i)->data(Qt::UserRole).value<TestFile>().name == name) {
            return true;
        }
    }
    for (int i = 0; i < refTestList->count(); ++i) {
        if (refTestList->item(i)->data(Qt::UserRole).value<TestFile>().name == name) {
            return true;
        }
    }
}

void TestSetupPage::editTest(QListWidgetItem* item)
{
    TestFile data = item->data(Qt::UserRole).value<TestFile>();
    AddTestDialog dlg(QDir::cleanPath(field("directory").toString()), data.reference, &data, this);
    dlg.setModal(true);
    if (dlg.exec() == QDialog::Accepted)
    {
        item->setData(Qt::UserRole, QVariant::fromValue(dlg.getItem()));
    }
}
