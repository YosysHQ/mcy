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
    setPage(Page_SelectFiles, new SelectFilesPage);
    setPage(Page_ScriptEdit, new ScriptEditPage);
    setPage(Page_Options, new OptionsPage);

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
    case Page_SelectFiles:
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
    
    if (!field("weight_pq_w").toString().isEmpty()) {
        content += QString("weight_pq_w ") + field("weight_pq_w").toString(); content += "\n";
    }
    if (!field("weight_pq_mw").toString().isEmpty()) {
        content += QString("weight_pq_mw ") + field("weight_pq_mw").toString(); content += "\n";
    }

    if (!field("weight_pq_b").toString().isEmpty()) {
        content += QString("weight_pq_b ") + field("weight_pq_b").toString(); content += "\n";
    }
    if (!field("weight_pq_mb").toString().isEmpty()) {
        content += QString("weight_pq_mb ") + field("weight_pq_mb").toString(); content += "\n";
    }

    if (!field("weight_pq_c").toString().isEmpty()) {
        content += QString("weight_pq_c ") + field("weight_pq_c").toString(); content += "\n";
    }
    if (!field("weight_pq_mc").toString().isEmpty()) {
        content += QString("weight_pq_mc ") + field("weight_pq_mc").toString(); content += "\n";
    }

    if (!field("weight_pq_s").toString().isEmpty()) {
        content += QString("weight_pq_s ") + field("weight_pq_s").toString(); content += "\n";
    }
    if (!field("weight_pq_ms").toString().isEmpty()) {
        content += QString("weight_pq_ms ") + field("weight_pq_ms").toString(); content += "\n";
    }

    content += QString("weight_cover ") + field("weight_cover").toString(); content += "\n";
    content += QString("pick_cover_prcnt ") + field("pick_cover_prcnt").toString(); content += "\n";

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
    return CreateWizard::Page_SelectFiles;
}

SelectFilesPage::SelectFilesPage(QWidget *parent)
    : QWizardPage(parent)
{
    setTitle(tr("Select design files..."));

    fileList = new QListWidget;
    fileList->setDragDropMode(QAbstractItemView::InternalMove);
    fileList->setSelectionMode(QAbstractItemView::MultiSelection);

    QDialogButtonBox *buttonBox = new QDialogButtonBox(Qt::Vertical, this);
    addButton = buttonBox->addButton("Add", QDialogButtonBox::ActionRole);
    deleteButton = buttonBox->addButton("Delete", QDialogButtonBox::ActionRole);

    QObject::connect(addButton,  &QPushButton::clicked, this, &SelectFilesPage::addFiles);
    QObject::connect(deleteButton, &QPushButton::clicked, this, &SelectFilesPage::deleteFiles);

    QHBoxLayout *layout = new QHBoxLayout;
    layout->addWidget(fileList);
    layout->addWidget(buttonBox);
    setLayout(layout);

    registerField("theFileList*", this, "theFileList");
}

int SelectFilesPage::nextId() const
{
    return CreateWizard::Page_ScriptEdit;
}

void SelectFilesPage::addFiles()
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
    Q_EMIT completeChanged();
}

QStringList SelectFilesPage::theFileList() const
{
    QStringList list;
    for(int i = 0; i < fileList->count(); ++i)
    {
        list << fileList->item(i)->text();
    }    
    return list;
}

void SelectFilesPage::deleteFiles()
{
    for(auto name : fileList->selectedItems()) {
        delete fileList->takeItem(fileList->row(name));
    }
    Q_EMIT completeChanged();
}

OptionsPage::OptionsPage(QWidget *parent)
    : QWizardPage(parent)
{
    setTitle(tr("Options"));

    QGridLayout *layout = new QGridLayout;
    
    layout->addWidget(new QLabel(tr("size")),0,0,1,1);
    mutations_size = new QLineEdit();
    mutations_size->setValidator( new QIntValidator(1, 100000, this) );
    layout->addWidget(mutations_size,0,1,1,1);
    registerField("mutations_size*", mutations_size);

    layout->addWidget(new QLabel(tr("pick_cover_prcnt")),1,0,1,1);
    pick_cover_prcnt = new QLineEdit();
    pick_cover_prcnt->setValidator( new QIntValidator(0, 1000, this) );
    layout->addWidget(pick_cover_prcnt,1,1,1,1);
    registerField("pick_cover_prcnt*", pick_cover_prcnt);
    

    layout->addWidget(new QLabel(tr("weight_cover")),2,0,1,1);
    weight_cover = new QLineEdit();
    weight_cover->setValidator( new QIntValidator(0, 1000, this) );
    layout->addWidget(weight_cover,2,1,1,1);
    registerField("weight_cover*", weight_cover);


    layout->addWidget(new QLabel(tr("weight_pq_w")),3,0,1,1);
    weight_pq_w = new QLineEdit();
    weight_pq_w->setValidator( new QIntValidator(0, 1000, this) );
    layout->addWidget(weight_pq_w,3,1,1,1);
    registerField("weight_pq_w", weight_pq_w);
    layout->addWidget(new QLabel(tr("weight_pq_mw")),3,2,1,1);
    weight_pq_mw = new QLineEdit();
    weight_pq_mw->setValidator( new QIntValidator(0, 1000, this) );
    layout->addWidget(weight_pq_mw,3,3,1,1);
    registerField("weight_pq_mw", weight_pq_mw);

    layout->addWidget(new QLabel(tr("weight_pq_b")),4,0,1,1);
    weight_pq_b = new QLineEdit();
    weight_pq_b->setValidator( new QIntValidator(0, 1000, this) );
    layout->addWidget(weight_pq_b,4,1,1,1);
    registerField("weight_pq_b", weight_pq_b);
    layout->addWidget(new QLabel(tr("weight_pq_mb")),4,2,1,1);
    weight_pq_mb = new QLineEdit();
    weight_pq_mb->setValidator( new QIntValidator(0, 1000, this) );
    layout->addWidget(weight_pq_mb,4,3,1,1);
    registerField("weight_pq_mb", weight_pq_mb);

    layout->addWidget(new QLabel(tr("weight_pq_c")),5,0,1,1);
    weight_pq_c = new QLineEdit();
    weight_pq_c->setValidator( new QIntValidator(0, 1000, this) );
    layout->addWidget(weight_pq_c,5,1,1,1);
    registerField("weight_pq_c", weight_pq_c);
    layout->addWidget(new QLabel(tr("weight_pq_mc")),5,2,1,1);
    weight_pq_mc = new QLineEdit();
    weight_pq_mc->setValidator( new QIntValidator(0, 1000, this) );
    layout->addWidget(weight_pq_mc,5,3,1,1);
    registerField("weight_pq_mc", weight_pq_mc);

    layout->addWidget(new QLabel(tr("weight_pq_s")),6,0,1,1);
    weight_pq_s = new QLineEdit();
    weight_pq_s->setValidator( new QIntValidator(0, 1000, this) );
    layout->addWidget(weight_pq_s,6,1,1,1);
    registerField("weight_pq_s", weight_pq_s);
    layout->addWidget(new QLabel(tr("weight_pq_ms")),6,2,1,1);
    weight_pq_ms = new QLineEdit();
    weight_pq_ms->setValidator( new QIntValidator(0, 1000, this) );
    layout->addWidget(weight_pq_ms,6,3,1,1);
    registerField("weight_pq_ms", weight_pq_ms);

    setLayout(layout);
}

int OptionsPage::nextId() const
{
    return -1;
}

ScriptEditPage::ScriptEditPage(QWidget *parent)
    : QWizardPage(parent)
{
    setTitle(tr("Edit script"));

    text = new QTextEdit();
    text->show();
    registerField("script", this, "theScript");
    QVBoxLayout *layout = new QVBoxLayout;
    layout->addWidget(text);
    setLayout(layout);
}

void ScriptEditPage::initializePage()
{
    QStringList fileList = field("theFileList").toStringList();
    text->clear();
    for (auto item : fileList)
        text->append(QString("read_verilog ") + item);
    text->append("");
}

QString ScriptEditPage::theScript() const
{
    return text->toPlainText();
}

int ScriptEditPage::nextId() const
{
    return CreateWizard::Page_Options;
}