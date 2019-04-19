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
#include <QTimer>
#include <QToolBar>
#include <QTreeWidgetItem>

BrowserWidget::BrowserWidget(DbManager *database, QWidget *parent) : QWidget(parent), database(database)
{
    tabWidget = new QTabWidget();

    sourceList = new QTreeWidget();
    sourceList->setHeaderHidden(true);    
    for(QString name : database->getSources()) 
    {
        QTreeWidgetItem *treeItem = new QTreeWidgetItem(sourceList);
        treeItem->setText(0, name);
        for (int mutation : database->getMutationsForSource(name))
        {
            QTreeWidgetItem *subItem = new QTreeWidgetItem(treeItem);
            subItem->setText(0, QString::number(mutation));        
            treeItem->addChild(subItem);
        }
        sourceList->addTopLevelItem(treeItem);
    }
    sourceList->setContextMenuPolicy(Qt::CustomContextMenu);
    sourceList->installEventFilter(this);

    mutationsList = new QTreeWidget();
    mutationsList->setHeaderHidden(true);
    for(int mutation : database->getMutations()) 
    {
        QTreeWidgetItem *treeItem = new QTreeWidgetItem(mutationsList);
        treeItem->setText(0, QString::number(mutation));
        for (QString name: database->getSourcesForMutation(mutation))
        {
            QTreeWidgetItem *subItem = new QTreeWidgetItem(treeItem);
            subItem->setText(0, name);        
            treeItem->addChild(subItem);
        }
        mutationsList->addTopLevelItem(treeItem);
    }
    mutationsList->setContextMenuPolicy(Qt::CustomContextMenu);
    mutationsList->installEventFilter(this);

    tabWidget->addTab(sourceList, "Sources");
    tabWidget->addTab(mutationsList, "Mutations");
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
        //sourceList->setCurrentItem(history.at(history_index));
        //sourceList->currentItem()->setHidden(false);
        updateButtons();
    });

    actionPrev = new QAction("", this);
    actionPrev->setIcon(QIcon(":/icons/resources/resultset_previous.png"));
    actionPrev->setEnabled(false);
    connect(actionPrev, &QAction::triggered, this, [this] {
        history_ignore = true;
        history_index--;
        //sourceList->setCurrentItem(history.at(history_index));
        //sourceList->currentItem()->setHidden(false);
        updateButtons();
    });

    actionNext = new QAction("", this);
    actionNext->setIcon(QIcon(":/icons/resources/resultset_next.png"));
    actionNext->setEnabled(false);
    connect(actionNext, &QAction::triggered, this, [this] {
        history_ignore = true;
        history_index++;
        //sourceList->setCurrentItem(history.at(history_index));
        //sourceList->currentItem()->setHidden(false);
        updateButtons();
    });

    actionLast = new QAction("", this);
    actionLast->setIcon(QIcon(":/icons/resources/resultset_last.png"));
    actionLast->setEnabled(false);
    connect(actionLast, &QAction::triggered, this, [this] {
        history_ignore = true;
        history_index = int(history.size() - 1);
        //sourceList->setCurrentItem(history.at(history_index));
        //sourceList->currentItem()->setHidden(false);
        updateButtons();
    });

    actionClear = new QAction("", this);
    actionClear->setIcon(QIcon(":/icons/resources/cross.png"));
    actionClear->setEnabled(true);
    connect(actionClear, &QAction::triggered, this, [this] {
        history_index = -1;
        history.clear();
        //addToHistory(sourceList->currentItem());
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
    vbox1->addWidget(tabWidget);

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

    connect(sourceList, &QTreeWidget::itemDoubleClicked, this, &BrowserWidget::onSourceDoubleClicked);
    connect(sourceList, &QTreeWidget::currentItemChanged, this, &BrowserWidget::onSourceCurrentItemChanged);

    connect(mutationsList, &QTreeWidget::itemDoubleClicked, this, &BrowserWidget::onMutationDoubleClicked);
    connect(mutationsList, &QTreeWidget::currentItemChanged, this, &BrowserWidget::onMutationCurrentItemChanged);

    history_index = -1;
    history_ignore = false;

    sourceList->setCurrentItem(sourceList->topLevelItem(0), 0, QItemSelectionModel::Select);
    QTimer::singleShot(0, sourceList, SLOT(setFocus()));
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

void BrowserWidget::addProperty(QtProperty *topItem, int propertyType, const QString &name, QVariant value,
                                QString type)
{
    QtVariantProperty *item = readOnlyManager->addProperty(propertyType, name);
    item->setValue(value);
    item->setSelectable(false);
    item->setPropertyId(type);
    topItem->addSubProperty(item);
}

QtProperty *BrowserWidget::addSubGroup(QtProperty *topItem, const QString &name)
{
    QtProperty *item = groupManager->addProperty(name);
    item->setSelectable(false);
    topItem->addSubProperty(item);
    return item;
}

void BrowserWidget::onSourceCurrentItemChanged(QTreeWidgetItem *current, QTreeWidgetItem *previous)
{
    if (current == nullptr)
        return;
    QTreeWidgetItem *item = current;
    QString source = item->text(0);
    int mutationId = -1;
    if (item->parent()!=nullptr) {
        source = item->parent()->text(0);
        mutationId = item->text(0).toInt();
    }

//    addToHistory(item);

    clearProperties();

    QtProperty *topItem = addTopLevelProperty("Source");
    addProperty(topItem, QVariant::String, "Name", source);

    if (mutationId==-1) {
        QtProperty *mutItem = addTopLevelProperty("Mutations");
        QList<int> mutations = database->getMutationsForSource(source);
        for (auto mutationId : mutations) {
            QtProperty *mItem = addSubGroup(mutItem, "Mutation " + QString::number(mutationId));
            for (auto option : database->getMutationOption(mutationId)) {
                addProperty(mItem, QVariant::String, option.first, option.second, option.first);
            }
            QtProperty *tagsItem = addSubGroup(mItem, "Tags");
            for (auto tag : database->getTagsForMutation(mutationId)) {
                addProperty(tagsItem, QVariant::String, "", tag);
            }
            QtProperty *resItem = addSubGroup(mItem, "Results");
            for (auto result : database->getMutationResults(mutationId)) {
                addProperty(resItem, QVariant::String, result.first, result.second);
            }
        }
    } else {        
        QtProperty *mItem = addTopLevelProperty("Mutation " + QString::number(mutationId));
        for (auto option : database->getMutationOption(mutationId)) {
            addProperty(mItem, QVariant::String, option.first, option.second, option.first);
        }
        QtProperty *tagsItem = addSubGroup(mItem, "Tags");
        for (auto tag : database->getTagsForMutation(mutationId)) {
            addProperty(tagsItem, QVariant::String, "", tag);
        }
        QtProperty *resItem = addSubGroup(mItem, "Results");
        for (auto result : database->getMutationResults(mutationId)) {
            addProperty(resItem, QVariant::String, result.first, result.second);
        }
    }
}

void BrowserWidget::onMutationCurrentItemChanged(QTreeWidgetItem *current, QTreeWidgetItem *previous)
{
    if (current == nullptr)
        return;
    QTreeWidgetItem *item = current;

    QString source = "";
    int mutationId = -1;
    if (item->parent()==nullptr) {
        mutationId = item->text(0).toInt();
    } else {
        source = item->text(0);
        mutationId = item->parent()->text(0).toInt();
    }

//    addToHistory(item);

    clearProperties();

    if (source=="") {
        QtProperty *mItem = addTopLevelProperty("Mutation " + QString::number(mutationId));
        for (auto option : database->getMutationOption(mutationId)) {
            addProperty(mItem, QVariant::String, option.first, option.second, option.first);
        }
        QtProperty *tagsItem = addSubGroup(mItem, "Tags");
        for (auto tag : database->getTagsForMutation(mutationId)) {
            addProperty(tagsItem, QVariant::String, "", tag);
        }
        QtProperty *resItem = addSubGroup(mItem, "Results");
        for (auto result : database->getMutationResults(mutationId)) {
            addProperty(resItem, QVariant::String, result.first, result.second);
        }
    } else {        
        QtProperty *topItem = addTopLevelProperty("Source");
        addProperty(topItem, QVariant::String, "Name", source);

        QtProperty *mItem = addTopLevelProperty("Mutation " + QString::number(mutationId));
        for (auto option : database->getMutationOption(mutationId)) {
            addProperty(mItem, QVariant::String, option.first, option.second, option.first);
        }
        QtProperty *tagsItem = addSubGroup(mItem, "Tags");
        for (auto tag : database->getTagsForMutation(mutationId)) {
            addProperty(tagsItem, QVariant::String, "", tag);
        }
        QtProperty *resItem = addSubGroup(mItem, "Results");
        for (auto result : database->getMutationResults(mutationId)) {
            addProperty(resItem, QVariant::String, result.first, result.second);
        }
    }

}

void BrowserWidget::onSourceDoubleClicked(QTreeWidgetItem *item, int column)
{
    bool ok;
    QString source = item->text(0);
    QString mut = "";
    if (item->parent()!=nullptr) {
        source = item->parent()->text(0);
        mut = item->text(0);
    }
    QStringList param = source.split(':');
    int line = param.at(1).toInt(&ok);
    if (ok)
        Q_EMIT selectLine(param.at(0), line);
}

void BrowserWidget::onMutationDoubleClicked(QTreeWidgetItem *item, int column)
{
    if (item->parent()==nullptr)
        return;

    bool ok;
    QStringList param = item->text(0).split(':');
    int line = param.at(1).toInt(&ok);
    if (ok)
        Q_EMIT selectLine(param.at(0), line);
}

void BrowserWidget::prepareMenuProperty(const QPoint &pos) {}

void BrowserWidget::onPropertyDoubleClicked(QTreeWidgetItem *item, int column)
{
    QtProperty *selectedProperty = propertyEditor->itemToBrowserItem(item)->property();
    QString type = selectedProperty->propertyId();
    if (type != "src")
        return;
    selectSource(selectedProperty->valueText());
}

void BrowserWidget::onSearchInserted()
{
    /*for (int i = 0; i < sourceList->count(); i++)
        sourceList->item(i)->setHidden(true);

    QList<QListWidgetItem *> matches(sourceList->findItems(searchEdit->text(), Qt::MatchFlag::MatchContains));
    for (QListWidgetItem *item : matches)
        item->setHidden(false);*/
}

void BrowserWidget::selectSource(QString source)
{
    QList<QTreeWidgetItem *> items = sourceList->findItems(source, Qt::MatchExactly);
    if (items.size() > 0) {
        sourceList->setCurrentItem(items.at(0), 0, QItemSelectionModel::Select);
    }
}

bool BrowserWidget::eventFilter(QObject *obj, QEvent *event)
{
    if (obj == sourceList) {
        if (event->type() == QEvent::KeyPress) {
            QKeyEvent *key = static_cast<QKeyEvent *>(event);
            if ((key->key() == Qt::Key_Enter) || (key->key() == Qt::Key_Return)) {
                onSourceDoubleClicked(sourceList->currentItem(),0);
            } else {
                return QObject::eventFilter(obj, event);
            }
            return true;
        } else {
            return false;
        }
    } else if (obj == mutationsList) {
        if (event->type() == QEvent::KeyPress) {
            QKeyEvent *key = static_cast<QKeyEvent *>(event);
            if ((key->key() == Qt::Key_Enter) || (key->key() == Qt::Key_Return)) {
                onMutationDoubleClicked(mutationsList->currentItem(),0);
            } else {
                return QObject::eventFilter(obj, event);
            }
            return true;
        } else {
            return false;
        }
    } else {
        return QObject::eventFilter(obj, event);
    }
}
