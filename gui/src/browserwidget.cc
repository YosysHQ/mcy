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

#include "browserwidget.h"
#include <QAction>
#include <QApplication>
#include <QGridLayout>
#include <QLineEdit>
#include <QMenu>
#include <QSplitter>
#include <QToolBar>
#include <QTreeWidgetItem>

BrowserWidget::BrowserWidget(DbManager *database, QWidget *parent) : QWidget(parent), database(database)
{
    sourceList = new QListWidget();
    sourceList->addItems(database->getSources());
    sourceList->setContextMenuPolicy(Qt::CustomContextMenu);

    // Add property view
    variantManager = new QtVariantPropertyManager(this);
    readOnlyManager = new QtVariantPropertyManager(this);
    groupManager = new QtGroupPropertyManager(this);
    variantFactory = new QtVariantEditorFactory(this);
    propertyEditor = new QtTreePropertyBrowser(this);
    propertyEditor->setFactoryForManager(variantManager, variantFactory);
    propertyEditor->setPropertiesWithoutValueMarked(true);
    propertyEditor->show();
    propertyEditor->treeWidget()->setContextMenuPolicy(Qt::CustomContextMenu);
    propertyEditor->treeWidget()->setSelectionMode(QAbstractItemView::ExtendedSelection);
    propertyEditor->treeWidget()->viewport()->setMouseTracking(true);

    searchEdit = new QLineEdit();
    searchEdit->setClearButtonEnabled(true);
    searchEdit->addAction(QIcon(":/icons/resources/zoom.png"), QLineEdit::LeadingPosition);
    searchEdit->setPlaceholderText("Search...");
    connect(searchEdit, &QLineEdit::returnPressed, this, &BrowserWidget::onSearchInserted);

    actionFirst = new QAction("", this);
    actionFirst->setIcon(QIcon(":/icons/resources/resultset_first.png"));
    actionFirst->setEnabled(false);
    connect(actionFirst, &QAction::triggered, this, [this] {
        history_ignore = true;
        history_index = 0;
        sourceList->setCurrentItem(history.at(history_index));
        sourceList->currentItem()->setHidden(false);
        updateButtons();
    });

    actionPrev = new QAction("", this);
    actionPrev->setIcon(QIcon(":/icons/resources/resultset_previous.png"));
    actionPrev->setEnabled(false);
    connect(actionPrev, &QAction::triggered, this, [this] {
        history_ignore = true;
        history_index--;
        sourceList->setCurrentItem(history.at(history_index));
        sourceList->currentItem()->setHidden(false);
        updateButtons();
    });

    actionNext = new QAction("", this);
    actionNext->setIcon(QIcon(":/icons/resources/resultset_next.png"));
    actionNext->setEnabled(false);
    connect(actionNext, &QAction::triggered, this, [this] {
        history_ignore = true;
        history_index++;
        sourceList->setCurrentItem(history.at(history_index));
        sourceList->currentItem()->setHidden(false);
        updateButtons();
    });

    actionLast = new QAction("", this);
    actionLast->setIcon(QIcon(":/icons/resources/resultset_last.png"));
    actionLast->setEnabled(false);
    connect(actionLast, &QAction::triggered, this, [this] {
        history_ignore = true;
        history_index = int(history.size() - 1);
        sourceList->setCurrentItem(history.at(history_index));
        sourceList->currentItem()->setHidden(false);
        updateButtons();
    });

    actionClear = new QAction("", this);
    actionClear->setIcon(QIcon(":/icons/resources/cross.png"));
    actionClear->setEnabled(true);
    connect(actionClear, &QAction::triggered, this, [this] {
        history_index = -1;
        history.clear();
        addToHistory(sourceList->currentItem());
    });

    QToolBar *toolbar = new QToolBar();
    toolbar->addAction(actionFirst);
    toolbar->addAction(actionPrev);
    toolbar->addAction(actionNext);
    toolbar->addAction(actionLast);
    toolbar->addAction(actionClear);

    QWidget *topWidget = new QWidget();
    QVBoxLayout *vbox1 = new QVBoxLayout();
    topWidget->setLayout(vbox1);
    vbox1->setSpacing(5);
    vbox1->setContentsMargins(0, 0, 0, 0);
    vbox1->addWidget(searchEdit);
    vbox1->addWidget(sourceList);

    QWidget *toolbarWidget = new QWidget();
    QHBoxLayout *hbox = new QHBoxLayout;
    hbox->setAlignment(Qt::AlignCenter);
    toolbarWidget->setLayout(hbox);
    hbox->setSpacing(0);
    hbox->setContentsMargins(0, 0, 0, 0);
    hbox->addWidget(toolbar);

    QWidget *btmWidget = new QWidget();

    QVBoxLayout *vbox2 = new QVBoxLayout();
    btmWidget->setLayout(vbox2);
    vbox2->setSpacing(0);
    vbox2->setContentsMargins(0, 0, 0, 0);
    vbox2->addWidget(toolbarWidget);
    vbox2->addWidget(propertyEditor);

    QSplitter *splitter = new QSplitter(Qt::Vertical);
    splitter->addWidget(topWidget);
    splitter->addWidget(btmWidget);

    QGridLayout *mainLayout = new QGridLayout();
    mainLayout->setSpacing(0);
    mainLayout->setContentsMargins(0, 0, 0, 0);
    mainLayout->addWidget(splitter);
    setLayout(mainLayout);

    // Connection
    connect(propertyEditor->treeWidget(), &QTreeWidget::customContextMenuRequested, this,
            &BrowserWidget::prepareMenuProperty);
    connect(propertyEditor->treeWidget(), &QTreeWidget::itemDoubleClicked, this,
            &BrowserWidget::onPropertyDoubleClicked);

    connect(sourceList, &QListWidget::customContextMenuRequested, this, &BrowserWidget::prepareMenuSourceList);
    connect(sourceList, &QListWidget::itemDoubleClicked, this, &BrowserWidget::onSourceDoubleClicked);
    connect(sourceList, &QListWidget::itemSelectionChanged, this, &BrowserWidget::onSourceSelectionChanged);

    history_index = -1;
    history_ignore = false;
}

BrowserWidget::~BrowserWidget() {}

void BrowserWidget::updateButtons()
{
    int count = int(history.size());
    actionFirst->setEnabled(history_index > 0);
    actionPrev->setEnabled(history_index > 0);
    actionNext->setEnabled(history_index < (count - 1));
    actionLast->setEnabled(history_index < (count - 1));
}

void BrowserWidget::addToHistory(QListWidgetItem *item)
{
    if (!history_ignore) {
        int count = int(history.size());
        for (int i = count - 1; i > history_index; i--)
            history.pop_back();
        history.push_back(item);
        history_index++;
    }
    history_ignore = false;
    updateButtons();
}

QtProperty *BrowserWidget::addTopLevelProperty(const QString &id)
{
    QtProperty *topItem = groupManager->addProperty(id);
    propertyToId[topItem] = id;
    idToProperty[id] = topItem;
    topItem->setSelectable(false);
    propertyEditor->addProperty(topItem);
    return topItem;
}

void BrowserWidget::clearProperties()
{
    QMap<QtProperty *, QString>::ConstIterator itProp = propertyToId.constBegin();
    while (itProp != propertyToId.constEnd()) {
        delete itProp.key();
        itProp++;
    }
    propertyToId.clear();
    idToProperty.clear();
}

void BrowserWidget::addProperty(QtProperty *topItem, int propertyType, const QString &name, QVariant value)
{
    QtVariantProperty *item = readOnlyManager->addProperty(propertyType, name);
    item->setValue(value);
    item->setSelectable(false);
    topItem->addSubProperty(item);
}

QtProperty *BrowserWidget::addSubGroup(QtProperty *topItem, const QString &name)
{
    QtProperty *item = groupManager->addProperty(name);
    item->setSelectable(false);
    topItem->addSubProperty(item);
    return item;
}

void BrowserWidget::onSourceSelectionChanged()
{
    if (sourceList->selectedItems().size() != 1)
        return;
    QListWidgetItem *item = sourceList->selectedItems()[0];

    addToHistory(item);

    clearProperties();

    QtProperty *topItem = addTopLevelProperty("Source");
    addProperty(topItem, QVariant::String, "Name", item->text());

    QtProperty *mutItem = addTopLevelProperty("Mutations");
    QList<int> mutations = database->getMutationsForSource(item->text());
    for (auto mutationId : mutations) {
        QtProperty *mItem = addSubGroup(mutItem, "Mutation " + QString::number(mutationId));
        QMap<QString, QString> options = database->getMutationOption(mutationId);
        for (QMap<QString, QString>::iterator it = options.begin(); it != options.end(); ++it) {
            addProperty(mItem, QVariant::String, it.key(), it.value());
        }
        QtProperty *tags = addSubGroup(mItem, "Tags");
        for (auto tag : database->getTagsForMutation(mutationId)) {
            addProperty(tags, QVariant::String, "", tag);
        }
    }
}

void BrowserWidget::prepareMenuSourceList(const QPoint &pos)
{
    QMenu menu(this);
    QAction *actionBookmark = new QAction("Bookmark", this);
    menu.addAction(actionBookmark);
    menu.exec(sourceList->mapToGlobal(pos));
}

void BrowserWidget::onSourceDoubleClicked(QListWidgetItem *item)
{
    bool ok;
    QStringList param = item->text().split(':');
    int line = param.at(1).toInt(&ok);
    if (ok)
        Q_EMIT selectLine(param.at(0), line);
}

void BrowserWidget::prepareMenuProperty(const QPoint &pos) {}

void BrowserWidget::onPropertyDoubleClicked(QTreeWidgetItem *item, int column) {}

void BrowserWidget::onSearchInserted()
{
    for (int i = 0; i < sourceList->count(); i++)
        sourceList->item(i)->setHidden(true);

    QList<QListWidgetItem *> matches(sourceList->findItems(searchEdit->text(), Qt::MatchFlag::MatchContains));
    for (QListWidgetItem *item : matches)
        item->setHidden(false);
}
