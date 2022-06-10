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

#ifndef CODEVIEW_H
#define CODEVIEW_H

#include "ScintillaEdit.h"
#include "database.h"

class CodeView : public ScintillaEdit
{
    Q_OBJECT

  public:
    explicit CodeView(QString filename, QWidget *parent = 0);
    ~CodeView();

    void loadContent(const char *content);
    void setCoverage(QMap<QString, QPair<int, int>> coverage, QList<QString> yetToCover);
    QString &getFilename() { return filename; }
    void selectLine(QString line);
    void unselectLine();
    void find(QString text, bool forward);

  private:
    QString filename;
};

#endif // CODEVIEW_H
