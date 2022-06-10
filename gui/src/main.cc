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

#include <QApplication>
#include <QCommandLineParser>
#include "mainwindow.h"
#define BOOST_FILESYSTEM_NO_DEPRECATED
#include <boost/filesystem.hpp>

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    QCoreApplication::setApplicationName("MCY Gui");
    QCoreApplication::setApplicationVersion("1.0");
    QCommandLineParser parser;
    parser.addPositionalArgument("project", "Project folder/directory to use");
    QCommandLineOption sourceDirectoryOption(QStringList() << "src"
                                                           << "source-directory",
                                             "Copy all source files into <directory>.", "directory");
    parser.addOption(sourceDirectoryOption);
    parser.addHelpOption();
    parser.addVersionOption();
    parser.process(app);
    const QStringList positionalArguments = parser.positionalArguments();
    if (positionalArguments.size() > 1) {
        printf("Several project folders/directories have been specified.\n");
        return -1;
    }
    QString location = ".";
    if (positionalArguments.size() == 1) {
        location = positionalArguments[0];
    }
    if (boost::filesystem::exists(location.toStdString())) {
        if (boost::filesystem::is_directory(location.toStdString())) {
            boost::filesystem::path path = boost::filesystem::path(location.toStdString()) / "database";
            if (!boost::filesystem::exists(path)) {
                printf("Database directory does not exists.\n");
                return -1;
            }
            path /= "db.sqlite3";
            if (!boost::filesystem::exists(path)) {
                printf("Database file does not exists.\n");
                return -1;
            }
            location = path.string().c_str();
        }
    } else {
        printf("File location does not exists.\n");
        return -1;
    }
    QString srcDir = parser.value(sourceDirectoryOption);
    if (!srcDir.isEmpty()) {
        if (boost::filesystem::exists(srcDir.toStdString())) {
            if (!boost::filesystem::is_directory(srcDir.toStdString())) {
                printf("Source file location is not directory.\n");
                return -1;
            }
        } else {
            printf("Source directory does not exists.\n");
            return -1;
        }
    }

    MainWindow win(location, srcDir);
    win.show();

    return app.exec();
}
