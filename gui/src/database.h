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

#ifndef DATABASE_H
#define DATABASE_H

#include <QList>
#include <QMap>
#include <QSqlDatabase>

class DbManager
{
  public:
    DbManager(const QString &path);

    int getMutationsCount();
    QStringList getSources();
    QStringList getSourcesLines(QString filename);
    QList<int> getMutations();
    QStringList getFileList();
    QString getFileContent(QString filename);
    QMap<QString, QPair<int, int>> getCoverage(QString filename);
    QList<QString> getLinesYetToCover(QString filename);
    QList<int> getMutationsForSource(QString source);
    QStringList getSourcesForMutation(int mutationId);
    QList<QPair<QString, QString>> getMutationOption(int mutationId);
    QList<QPair<QString, QString>> getMutationResults(int mutationId);
    QStringList getTagsForMutation(int mutationId);
    QStringList getUniqueTags(bool addAllTags);
    QList<int> getMutationsForTag(QString tag);
    QList<int> getMutationsNoTags();

    static const QString ALL_TAGS;
    static const QString NO_TAGS;

  private:
    QSqlDatabase db;
};

#endif // DATABASE_H
