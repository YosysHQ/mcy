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
    parser.addPositionalArgument("source", "Source folder/directory to use");
    parser.addHelpOption();
    parser.addVersionOption();
    parser.process(app);
    const QStringList positionalArguments = parser.positionalArguments();
    if (positionalArguments.size() > 1) {
        printf("Several source folders/directories have been specified.\n");
        return -1;
    }
    if (positionalArguments.size() == 1) {
        if(boost::filesystem::exists(positionalArguments[0].toStdString())) {
            if (!boost::filesystem::is_directory(positionalArguments[0].toStdString())) {
                printf("File location is not directory.\n");
                return -1;
            }
            boost::filesystem::path path = boost::filesystem::path(positionalArguments[0].toStdString()) / "database";
            if (!boost::filesystem::exists(path)) {
                printf("Database directory does not exists.\n");
                return -1;
            }
            path /= "db.sqlite3";
            if (!boost::filesystem::exists(path)) {
                printf("Database file does not exists.\n");
                return -1;
            }
        } else {
            printf("File location does not exists.\n");
            return -1;
        }
    }
    MainWindow win(positionalArguments.size() ? positionalArguments[0] : ".");
    win.show();

    return app.exec();
}
