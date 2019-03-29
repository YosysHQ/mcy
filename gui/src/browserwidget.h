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

#ifndef BROWSERWIDGET_H
#define BROWSERWIDGET_H

#include <QListWidget>
#include <QMouseEvent>
#include <QTabWidget>
#include <QTreeView>
#include <QVariant>
#include "database.h"
#include "qtpropertymanager.h"
#include "qttreepropertybrowser.h"
#include "qtvariantproperty.h"

class BrowserWidget : public QWidget
{
    Q_OBJECT

  public:
    explicit BrowserWidget(DbManager *database, QWidget *parent = 0);
    ~BrowserWidget();

  private:
    // property handling
    void clearProperties();
    void addProperty(QtProperty *topItem, int propertyType, const QString &name, QVariant value);
    QtProperty *addTopLevelProperty(const QString &id);
    QtProperty *addSubGroup(QtProperty *topItem, const QString &name);
    // history handling
    void updateButtons();
    void addToHistory(QListWidgetItem *item);

  private Q_SLOTS:
    // source list slots
    void prepareMenuSourceList(const QPoint &pos);
    void onSourceDoubleClicked(QListWidgetItem *item);
    void onSourceSelectionChanged();
    // property view slots
    void prepareMenuProperty(const QPoint &pos);
    void onPropertyDoubleClicked(QTreeWidgetItem *item, int column);
    // search slots
    void onSearchInserted();

  public Q_SLOTS:
    void selectSource(QString source);

  Q_SIGNALS:
    void selectLine(QString filename, int line);

  private:
    // database
    DbManager *database;

    // source list
    QListWidget *sourceList;

    // search
    QLineEdit *searchEdit;
    QString currentSearch;

    // property
    QtVariantPropertyManager *variantManager;
    QtVariantPropertyManager *readOnlyManager;
    QtGroupPropertyManager *groupManager;
    QtVariantEditorFactory *variantFactory;
    QtTreePropertyBrowser *propertyEditor;

    QMap<QtProperty *, QString> propertyToId;
    QMap<QString, QtProperty *> idToProperty;

    // history
    std::vector<QListWidgetItem *> history;
    int history_index;
    bool history_ignore;

    // history actions
    QAction *actionFirst;
    QAction *actionPrev;
    QAction *actionNext;
    QAction *actionLast;
    QAction *actionClear;
};

#endif // BROWSERWIDGET_H
