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

#include "database.h"
#include <QSqlQuery>
#include <QVariant>

DbManager::DbManager(const QString &path)
{
    db = QSqlDatabase::addDatabase("QSQLITE");
    db.setDatabaseName(path);

    if (!db.open()) {
        printf("Error: connection with database fail\n");
    }
}

int DbManager::getMutationsCount()
{
    int count = 0;
    QSqlQuery query("SELECT COUNT(*) FROM mutations");
    if (query.next()) {
        count = query.value(0).toInt();
    }
    return count;
}

QStringList DbManager::getSources()
{
    QStringList sources;
    QSqlQuery query("SELECT srctag FROM sources");
    while (query.next()) {
        sources << query.value(0).toString();
    }
    return sources;
}

std::vector<int> DbManager::getSourceLines(std::string filename)
{
    std::vector<int> lines;
    std::string request = "SELECT srctag FROM sources WHERE srctag LIKE '" + filename + ":%'";
    QSqlQuery query(request.c_str());
    bool ok;
    while (query.next()) {
        QString data = query.value(0).toString().replace((filename + ":").c_str(), "");
        lines.push_back(data.toInt(&ok));
    }
    return lines;
}

QStringList DbManager::getFileList()
{
    QStringList files;
    QSqlQuery query("SELECT filename FROM files");
    while (query.next()) {
        files << query.value(0).toString();
    }
    return files;
}

std::vector<int> DbManager::getMutationsForSourceLine(std::string source)
{
    std::vector<int> mutations;
    QSqlQuery query;
    query.prepare("SELECT DISTINCT mutation_id FROM options WHERE opt_type='src' AND opt_value = :source");
    query.bindValue(":source", source.c_str());
    if (query.exec()) {
        while (query.next()) {
            mutations.push_back(query.value(0).toInt());
        }
    }
    return mutations;
}