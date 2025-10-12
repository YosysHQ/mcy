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

#include "codeview.h"
#include <QFontDatabase>
#include <QMessageBox>
#include "Qsci/qscilexerverilog.h"
#include "Qsci/qsciscintilla.h"
#include "SciLexer.h"

CodeView::CodeView(QString filename, QWidget *parent) : filename(filename), QsciScintilla(parent) {}

CodeView::~CodeView() {}

static const char *MonospaceFont()
{
    static char fontNameDefault[200] = "";
    if (!fontNameDefault[0]) {
        const QFont font = QFontDatabase::systemFont(QFontDatabase::FixedFont);
        strcpy(fontNameDefault, font.family().toUtf8());
    }
    return fontNameDefault;
}
void CodeView::loadContent(const char *content)
{
    QFont monospaceFont(MonospaceFont());
    monospaceFont.setPointSize(10);
    setScrollWidth(200);
    setScrollWidthTracking(true);
    SendScintilla(QsciScintilla::SCI_SETUNDOCOLLECTION, 0);
    // Line number
    setMarginWidth(0, 40);

    QsciLexerVerilog *verilogLexer = new QsciLexerVerilog(this);
    verilogLexer->setDefaultFont(monospaceFont);
    setFont(monospaceFont);
    for (int style = 0; style <= verilogLexer->styleBitsNeeded(); ++style) {
        verilogLexer->setFont(monospaceFont, style);
        verilogLexer->setColor(verilogLexer->defaultColor(style), style);
    }
    setLexer(verilogLexer);

    setText(content);
    SendScintilla(QsciScintilla::SCI_GOTOLINE, 0);
    setReadOnly(true);

    // Caret
    setCaretLineVisible(true);
    SendScintilla(QsciScintilla::SCI_SETCARETLINEVISIBLEALWAYS, 1);
    setCaretLineBackgroundColor(QColor("#e0e0e0"));

    // Icon margin
    setMarginWidth(1, 30);
    setMarginType(1, MarginType::SymbolMargin);
    markerDefine(MarkerSymbol::Circle, 0);
    setMarkerForegroundColor(QColor("#ff0000"), 0);
    setMarkerBackgroundColor(QColor("#ff0000"), 0);
    markerDefine(MarkerSymbol::Background, 1);
    setMarkerBackgroundColor(QColor("#e00000"), 1);

    // Counter margin
    setMarginWidth(2, 20);
    setMarginType(2, MarginType::TextMarginRightJustified);
    setMarginSensitivity(0, true);
    setMarginSensitivity(1, true);
    setMarginSensitivity(2, true);

    indicatorDefine(QsciScintilla::FullBoxIndicator, 0);
    SendScintilla(SCI_SETINDICATORCURRENT, 0);
}

static int extractLineNumber(QString line)
{
    if (line.contains('.')) {
        line = line.left(line.indexOf('.'));
    }
    return line.toInt();
}

void CodeView::selectLine(QString line)
{
    setCaretLineVisible(true);
    int ln = extractLineNumber(line) - 1;
    setCursorPosition(ln, 0);
    ensureLineVisible(ln);
    SendScintilla(SCI_SETINDICATORCURRENT, 0);
    SendScintilla(SCI_INDICATORCLEARRANGE, 0, length());
    if (line.contains('-')) {
        QStringList parts = line.split('-');
        if (parts.at(0).contains('.') && parts.at(1).contains('.')) {
            QString part = parts.at(0);
            int ln = part.left(part.indexOf('.')).toInt()-1;
            int pos = part.mid(part.indexOf('.')+1).toInt();
            int start = positionFromLineIndex(ln, pos - 1);
            part = parts.at(1);
            ln = part.left(part.indexOf('.')).toInt()-1;
            pos = part.mid(part.indexOf('.')+1).toInt();
            int end = positionFromLineIndex(ln, pos -1);
            SendScintilla(SCI_SETINDICATORCURRENT, 0);
            SendScintilla(SCI_INDICATORFILLRANGE, start, end - start);
        }
    }
}
void CodeView::unselectLine()
{
    setCaretLineVisible(false);
    SendScintilla(SCI_SETINDICATORCURRENT, 0);
    SendScintilla(SCI_INDICATORCLEARRANGE, 0, length());
}

void CodeView::setCoverage(QMap<QString, QPair<int, int>> coverage, QList<QString> yetToCover)
{
    for (int i = 0; i < yetToCover.count(); ++i) {
        int line = extractLineNumber(yetToCover[i]) - 1;
        if (line < 0)
            continue;
        SendScintilla(SCI_MARGINSETTEXT, line, "?");
        SendScintilla(SCI_MARGINSETSTYLE, line, STYLE_LINENUMBER);
    }

    QMap<int, QPair<int, int>> cov;
    QMap<QString, QPair<int, int>>::const_iterator cit = coverage.constBegin();
    while (cit != coverage.constEnd()) {
        int line = extractLineNumber(cit.key());
        if (cov.contains(line)) {
            QPair<int, int> &p = cov[line];
            p.first += cit.value().first;
            p.second += cit.value().second;
        } else
            cov[line] = cit.value();
        ++cit;
    }

    QMap<int, QPair<int, int>>::const_iterator it = cov.constBegin();
    while (it != cov.constEnd()) {
        int line = it.key() - 1;
        if (line >= 0) {
            auto val = it.value();
            if (val.second > 0) {
                markerAdd(line, 0);
                // markerAdd(line,1);
                SendScintilla(SCI_MARGINSETTEXT, line, std::to_string(-val.second).c_str());
                SendScintilla(SCI_MARGINSETSTYLE, line, STYLE_LINENUMBER);
            } else {
                SendScintilla(SCI_MARGINSETTEXT, line, std::to_string(val.first).c_str());
                SendScintilla(SCI_MARGINSETSTYLE, line, STYLE_LINENUMBER);
            }
        }
        ++it;
    }
}

void CodeView::find(QString text, bool forward)
{
    if (text.isEmpty())
        return;
    bool found = findFirst(
        text,
        false,       // regexp
        false,       // case sensitive
        false,       // whole words
        true,        // wrap around
        forward,     // search forward
        -1,          // line (current line)
        -1,          // index (current index)
        true         // show selection
    );

    if (!found) {
        QMessageBox::warning(this, "Not found", "Text not found.");
    }
}
