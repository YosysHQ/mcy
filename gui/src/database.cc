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
    QSqlQuery query("SELECT srctag FROM sources ORDER BY SUBSTR(srctag,0,INSTR(srctag,':')),CAST(SUBSTR(srctag,INSTR(srctag,':')+1) AS INTEGER)");
    while (query.next()) {
        sources << query.value(0).toString();
    }
    return sources;
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

QString DbManager::getFileContent(QString filename)
{
    QSqlQuery query("SELECT data FROM files WHERE filename='" + filename + "'");
    if (query.next()) {
        return query.value(0).toString();
    }
    return "";
}

QMap<int, QPair<int, int>> DbManager::getCoverage(QString filename)
{
    QMap<int, QPair<int, int>> retVal;
    QString str = "SELECT REPLACE(opt_value,'" + filename + ":',''),"
                                                            "       COUNT(CASE WHEN tag =   'COVERED' THEN 1 END),"
                                                            "       COUNT(CASE WHEN tag = 'UNCOVERED' THEN 1 END)"
                                                            "   FROM options"
                                                            "   JOIN tags ON (options.mutation_id = tags.mutation_id)"
                                                            "   WHERE opt_type = 'src'"
                                                            "       AND opt_value LIKE '" +
                  filename + ":%' "
                             "   GROUP BY opt_value ";
    QSqlQuery query(str);
    while (query.next()) {
        retVal.insert(query.value(0).toInt(), QPair<int, int>(query.value(1).toInt(), query.value(2).toInt()));
    }
    return retVal;
}

QList<int> DbManager::getMutationsForSource(QString source)
{
    QList<int> retVal;
    QSqlQuery query("SELECT mutation_id FROM options WHERE opt_type = 'src' AND opt_value='" + source + "'");
    while (query.next()) {
        retVal.append(query.value(0).toInt());
    }
    return retVal;
}

QList<QPair<QString, QString>> DbManager::getMutationOption(int mutationId)
{
    QList<QPair<QString, QString>> retVal;
    QSqlQuery query("SELECT opt_type, opt_value FROM options WHERE mutation_id = " + QString::number(mutationId));
    while (query.next()) {
        retVal.append(QPair<QString, QString>(query.value(0).toString(), query.value(1).toString()));
    }
    return retVal;
}

QList<QPair<QString, QString>> DbManager::getMutationResults(int mutationId)
{
    QList<QPair<QString, QString>> retVal;
    QSqlQuery query("SELECT test, result FROM results WHERE mutation_id = " + QString::number(mutationId));
    while (query.next()) {
        retVal.append(QPair<QString, QString>(query.value(0).toString(), query.value(1).toString()));
    }
    return retVal;
}

QStringList DbManager::getTagsForMutation(int mutationId)
{
    QStringList tags;
    QSqlQuery query("SELECT tag FROM tags WHERE mutation_id = " + QString::number(mutationId));
    while (query.next()) {
        tags << query.value(0).toString();
    }
    return tags;
}
