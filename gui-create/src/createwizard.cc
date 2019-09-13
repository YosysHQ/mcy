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

CreateWizard::CreateWizard(QWidget *parent)
    : QWizard(parent)
{
    setPage(Page_Intro, new IntroPage);
    setPage(Page_SelectDirectory, new SelectDirectoryPage);
    setPage(Page_DesignSetup, new DesignSetupPage);
    setPage(Page_TestSetup, new TestSetupPage);

    setStartId(Page_Intro);
#ifndef Q_OS_MAC
    setWizardStyle(ModernStyle);
#endif
    setOption(HaveHelpButton, true);
    setPixmap(QWizard::LogoPixmap, QPixmap(":/icons/resources/symbiotic.png"));

    connect(this, &QWizard::helpRequested, this, &CreateWizard::showHelp);
    setWindowTitle(tr("Create Wizard"));
}

void CreateWizard::showHelp()
{
    QString message;

    switch (currentId()) {
    case Page_Intro:
        message = tr("TODO: Intro page help message.");
        break;
    case Page_DesignSetup:
        message = tr("TODO: Select files help message.");
        break;
    default:
        message = tr("This help is likely not to be of any help.");
    }

    QMessageBox::information(this, tr("Create Wizard Help"), message);
}

void CreateWizard::accept()
{
    QByteArray content;
    content += "[options]"; content += "\n";
    content += QString("size ") + field("mutations_size").toString(); content += "\n";
   
    content += "\n";
    
    content += "[script]"; content += "\n";
    content += field("script").toString();
    content += "\n";
    content += "\n";

    content += "[files]"; content += "\n";
    QStringList fileList = field("theFileList").toStringList();
    for (auto item : fileList) {
        content += QString("read_verilog ") + item;
        content += "\n";
    }
    content += "\n";

    content += "[logic]"; content += "\n";
    content += "tag(\"NOC\")"; content += "\n";
    content += "\n";

    QFile headerFile("config.mcy");
    if (!headerFile.open(QFile::WriteOnly | QFile::Text)) {
        QMessageBox::warning(nullptr, QObject::tr("Create Wizard"),
                             QObject::tr("Cannot write file %1:\n%2")
                             .arg(headerFile.fileName())
                             .arg(headerFile.errorString()));
        return;
    }
    headerFile.write(content);

    QDialog::accept();
}

IntroPage::IntroPage(QWidget *parent)
    : QWizardPage(parent)
{
    setTitle(tr("Introduction"));

    topLabel = new QLabel(tr("This wizard will help you create configuration for "
                             "<i>Mutation Cover with Yosys</i>."));
    topLabel->setWordWrap(true);

    QVBoxLayout *layout = new QVBoxLayout;
    layout->addWidget(topLabel);
    setLayout(layout);
}

int IntroPage::nextId() const
{
    return CreateWizard::Page_SelectDirectory;
}

SelectDirectoryPage::SelectDirectoryPage(QWidget *parent)
    : QWizardPage(parent)
{
    setTitle(tr("Select working directory"));

    QHBoxLayout *layout = new QHBoxLayout;
    
    QPushButton *browseButton = new QPushButton(tr("&Browse..."), this);
    connect(browseButton, &QAbstractButton::clicked, this, &SelectDirectoryPage::browse);
    directory = new QLineEdit();
    directory->setReadOnly(true);
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
        QDir::toNativeSeparators(QFileDialog::getExistingDirectory(this, tr("Find Files"), QDir::currentPath()));

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

DesignSetupPage::DesignSetupPage(QWidget *parent)
    : QWizardPage(parent)
{
    setTitle("Design Setup...");

    top = new QLineEdit();
    
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

    QWidget *filesel = new QWidget();
    filesel->setLayout(filesel_layout);
    QWidget *scripted = new QWidget();
    scripted->setLayout(script_layout);
    
    QVBoxLayout *layout = new QVBoxLayout;
    layout->addWidget(new QLabel("Top:"));
    layout->addWidget(top);
    layout->addWidget(new QLabel("Design files:"));
    layout->addWidget(filesel);
    layout->addWidget(new QLabel("Script:"));
    layout->addWidget(scripted);
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

void DesignSetupPage::updateScript()
{
    script->clear();
    for(int i = 0; i < fileList->count(); ++i)
    {
        script->append(QString("read_verilog ") + fileList->item(i)->text());
    }
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
}

void DesignSetupPage::resetScript()
{
    addButton->setEnabled(true);
    deleteButton->setEnabled(true);
    editButton->setEnabled(true);
    resetButton->setEnabled(false);
    script->setReadOnly(true);
    updateScript();
}

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

    QDialogButtonBox *buttonBox_test = new QDialogButtonBox(Qt::Vertical, this);
    addTestButton = buttonBox_test->addButton("Add", QDialogButtonBox::ActionRole);
    delTestButton = buttonBox_test->addButton("Delete", QDialogButtonBox::ActionRole);

    QObject::connect(addTestButton, &QPushButton::clicked, this, &TestSetupPage::addTest);
    QObject::connect(delTestButton, &QPushButton::clicked, this, &TestSetupPage::delTest);


    refTestList = new QListWidget;
    refTestList->setDragDropMode(QAbstractItemView::InternalMove);
    refTestList->setSelectionMode(QAbstractItemView::MultiSelection);

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

    QWidget *testWidget = new QWidget();
    testWidget->setLayout(test_layout);
    QWidget *refTestWidget = new QWidget();
    refTestWidget->setLayout(refTest_layout);

    QVBoxLayout *layout = new QVBoxLayout;
    layout->addWidget(new QLabel("Mutation size:"));
    layout->addWidget(mutations_size);
    layout->addWidget(new QLabel("Tests:"));
    layout->addWidget(testWidget);
    layout->addWidget(new QLabel("Reference tests:"));
    layout->addWidget(refTestWidget);

    setLayout(layout);
}

int TestSetupPage::nextId() const
{
    return -1;
}

void TestSetupPage::addTest()
{
}

void TestSetupPage::delTest()
{    
}

void TestSetupPage::addRefTest()
{
}

void TestSetupPage::delRefTest()
{
}
