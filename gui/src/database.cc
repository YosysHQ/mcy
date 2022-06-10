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

#include "database.h"
#include <QSqlQuery>
#include <QVariant>

const QString DbManager::ALL_TAGS = "All tags";
const QString DbManager::NO_TAGS = "No tags";

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
    QSqlQuery query("SELECT DISTINCT SUBSTR(srctag,0,INSTR(srctag,':')) FROM sources ORDER BY "
                    "SUBSTR(srctag,0,INSTR(srctag,':'))");
    while (query.next()) {
        sources << query.value(0).toString();
    }
    return sources;
}

QStringList DbManager::getSourcesLines(QString filename)
{
    QStringList sources;
    QSqlQuery query("SELECT SUBSTR(srctag,INSTR(srctag,':')+1) FROM sources WHERE "
                    "SUBSTR(srctag,0,INSTR(srctag,':')) = \"" +
                    filename + "\" ORDER BY CAST(SUBSTR(srctag,INSTR(srctag,':')+1) AS INTEGER)");
    while (query.next()) {
        sources << query.value(0).toString();
    }
    return sources;
}

QList<int> DbManager::getMutations()
{
    QList<int> retVal;
    QSqlQuery query("SELECT mutation_id FROM mutations ORDER BY mutation_id");
    while (query.next()) {
        retVal.append(query.value(0).toInt());
    }
    return retVal;
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

QMap<QString, QPair<int, int>> DbManager::getCoverage(QString filename)
{
    QMap<QString, QPair<int, int>> retVal;
    QString str = "SELECT REPLACE(opt_value,'" + filename +
                  ":',''),"
                  "       COUNT(CASE WHEN tag =   'COVERED' THEN 1 END),"
                  "       COUNT(CASE WHEN tag = 'UNCOVERED' THEN 1 END)"
                  "   FROM options"
                  "   JOIN tags ON (options.mutation_id = tags.mutation_id)"
                  "   WHERE opt_type = 'src'"
                  "       AND opt_value LIKE '" +
                  filename +
                  ":%' "
                  "   GROUP BY opt_value ";
    QSqlQuery query(str);
    while (query.next()) {
        retVal.insert(query.value(0).toString(), QPair<int, int>(query.value(1).toInt(), query.value(2).toInt()));
    }
    return retVal;
}

QList<QString> DbManager::getLinesYetToCover(QString filename)
{
    QList<QString> retVal;
    QString str = "SELECT REPLACE(opt_value,'" + filename +
                  ":','') "
                  "   FROM options"
                  "   WHERE opt_type = 'src'"
                  "       AND mutation_id NOT IN (SELECT mutation_id FROM tags)"
                  "       AND opt_value LIKE '" +
                  filename +
                  ":%' "
                  "   GROUP BY opt_value ";
    QSqlQuery query(str);
    while (query.next()) {
        retVal.append(query.value(0).toString());
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

QStringList DbManager::getSourcesForMutation(int mutationId)
{
    QStringList retVal;
    QSqlQuery query("SELECT opt_value FROM options WHERE opt_type = 'src' AND mutation_id = " +
                    QString::number(mutationId));
    while (query.next()) {
        retVal << query.value(0).toString();
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

QStringList DbManager::getUniqueTags(bool addAllTags)
{
    QStringList tags;
    QSqlQuery query("SELECT tag FROM tags GROUP BY tag");
    if (addAllTags) 
        tags << DbManager::ALL_TAGS;
    while (query.next()) {
        tags << query.value(0).toString();
    }
    tags << DbManager::NO_TAGS;
    return tags;
}

QList<int> DbManager::getMutationsNoTags()
{
    QList<int> retVal;
    QSqlQuery query("SELECT mutation_id FROM mutations where mutation_id not in (SELECT mutation_id FROM tags)");
    while (query.next()) {
        retVal.append(query.value(0).toInt());
    }
    return retVal;
}

QList<int> DbManager::getMutationsForTag(QString tag)
{
    if (tag == DbManager::NO_TAGS) return getMutationsNoTags();

    QList<int> retVal;
    QSqlQuery query("SELECT mutation_id FROM tags WHERE tag = '" + tag + "'");
    while (query.next()) {
        retVal.append(query.value(0).toInt());
    }
    return retVal;
}