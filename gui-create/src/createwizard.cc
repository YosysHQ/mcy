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
#include <QProcess>
#include "createwizard.h"
#include "intropage.h"
#include "selectdir.h"
#include "designsetup.h"
#include "testsetup.h"
#include "addtest.h"

CreateWizard::CreateWizard(QWidget *parent)
    : QWizard(parent)
{
    setPage(Page_Intro, new IntroPage);
    setPage(Page_SelectDirectory, new SelectDirectoryPage);
    setPage(Page_DesignSetup, new DesignSetupPage);
    testSetupPage = new TestSetupPage;
    setPage(Page_TestSetup, testSetupPage);

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
    for(int i=0;i<testSetupPage->tests()->topLevelItemCount();i++){
        TestFile tf = testSetupPage->tests()->topLevelItem(i)->data(0, Qt::UserRole).value<TestFile>();            
        QProcess process;
        QStringList args;
        args << "generate";
        args << tf.type;
        args << "--tb";
        args << tf.filename;
        args << "--output";
        args << tf.name + ".sh";
        
        process.setProgram("mcy");
        process.setArguments(args);
        printf("%s\n",args.join(' ').toStdString().c_str());
        QProcessEnvironment env = QProcessEnvironment::systemEnvironment();
        env.insert("PYTHONUNBUFFERED","1");
        process.setProcessEnvironment(env);    
        process.setWorkingDirectory(QDir(field("directory").toString()).canonicalPath());
        process.setProcessChannelMode(QProcess::MergedChannels);       
        process.start();   
        process.waitForFinished();
        //return process.readAllStandardOutput();
    }

    QByteArray content;
    content += "[options]\n";
    content += QString("size ") + field("mutations_size").toString(); content += "\n";
    content += "tags COVERED UNCOVERED PROBE GAP NOC\n";
    content += "\n";
    
    content += "[script]\n";
    content += field("script").toString();
    content += "\n";
    content += "\n";

    content += "[files]"; content += "\n";
    QStringList fileList = field("theFileList").toStringList();
    for (auto item : fileList) {
        content += item;
        content += "\n";
    }
    content += "\n";

    content += "[logic]\n";
    for(int i=0;i<testSetupPage->tests()->topLevelItemCount();i++){
        TestFile tf = testSetupPage->tests()->topLevelItem(i)->data(0, Qt::UserRole).value<TestFile>();    
        content += "if result(\"" + tf.name + "\") == \"FAIL\":\n";
        content += "    tag(\"COVERED\")\n";
        content += "    return\n";
    }
    content += "tag(\"NOC\")\n";
    
    content += "\n";
    content += "[report]\n";
    content += "if tags(\"!NOC\"):\n";
    content += "    print(\"Coverage: %.2f%%\" % (100.0*tags(\"COVERED\")/tags(\"!NOC\")))\n";
    content += "if tags():\n";
    content += "    print(\"Noc percentage: %.2f%%\" % (100.0*tags(\"NOC\")/tags()))\n";
    content += "if tags(\"PROBE\"):\n";
    content += "    print(\"Gap percentage: %.2f%%\" % (100.0*tags(\"GAP\")/tags(\"PROBE\")))\n";

    content += "\n";
    for(int i=0;i<testSetupPage->tests()->topLevelItemCount();i++){
        TestFile tf = testSetupPage->tests()->topLevelItem(i)->data(0, Qt::UserRole).value<TestFile>();
        content += "[test " + tf.name + "]\n";
        content += "maxbatchsize 10\n";
        content += "expect PASS FAIL\n";
        content += "run bash $PRJDIR/" + tf.name + ".sh\n";
    }

    QFile headerFile(QDir::cleanPath(field("directory").toString() + QDir::separator() + "config.mcy"));
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
