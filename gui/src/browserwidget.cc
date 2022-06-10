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

#include "browserwidget.h"
#include <QAction>
#include <QApplication>
#include <QClipboard>
#include <QGridLayout>
#include <QLineEdit>
#include <QMenu>
#include <QSplitter>
#include <QTimer>
#include <QToolBar>
#include <QTreeWidgetItem>
#include <QFileInfo>

BrowserWidget::BrowserWidget(DbManager *database, QWidget *parent) : QWidget(parent), database(database)
{
    tabWidget = new QTabWidget();

    sourceList = new QTreeWidget();
    sourceList->setHeaderHidden(true);
    for (QString name : database->getSources()) {
        QTreeWidgetItem *treeItem = new QTreeWidgetItem(sourceList);
        QFileInfo fi = QFileInfo(name);
        treeItem->setText(0, fi.completeBaseName() + "." + fi.completeSuffix());
        treeItem->setData(0, Qt::UserRole, name);
        for (QString line : database->getSourcesLines(name)) {
            QTreeWidgetItem *lineItem = new QTreeWidgetItem(treeItem);
            lineItem->setText(0, "Line " + line);
            lineItem->setData(0, Qt::UserRole, name + ":" + line);
            for (int mutation : database->getMutationsForSource(name + ":" + line)) {
                QTreeWidgetItem *subItem = new QTreeWidgetItem(lineItem);
                subItem->setText(0, "Mutation " + QString::number(mutation));
                subItem->setData(0, Qt::UserRole, QString::number(mutation));
                lineItem->addChild(subItem);
            }
            treeItem->addChild(lineItem);
        }
        sourceList->addTopLevelItem(treeItem);
    }
    sourceList->setContextMenuPolicy(Qt::CustomContextMenu);
    sourceList->installEventFilter(this);

    mutationsList = new QTreeWidget();
    mutationsList->setHeaderHidden(true);
    for (int mutation : database->getMutations()) {
        QTreeWidgetItem *treeItem = new QTreeWidgetItem(mutationsList);
        treeItem->setText(0, "Mutation " + QString::number(mutation));
        treeItem->setData(0, Qt::UserRole, QString::number(mutation));
        for (QString name : database->getSourcesForMutation(mutation)) {
            QTreeWidgetItem *subItem = new QTreeWidgetItem(treeItem);
            QStringList parts = name.split(':');
            QString realname = parts.at(0);
            QFileInfo fi = QFileInfo(realname);
            subItem->setText(0, fi.completeBaseName() + "." + fi.completeSuffix() + ":" + parts.at(1));
            subItem->setData(0, Qt::UserRole, name);
            treeItem->addChild(subItem);
        }
        mutationsList->addTopLevelItem(treeItem);
    }
    mutationsList->setContextMenuPolicy(Qt::CustomContextMenu);
    mutationsList->installEventFilter(this);

    tagList = new QTreeWidget();
    tagList->setHeaderHidden(true);
    for (QString name : database->getUniqueTags(false)) {
        QTreeWidgetItem *treeItem = new QTreeWidgetItem(tagList);
        QFileInfo fi = QFileInfo(name);
        treeItem->setText(0, name);
        treeItem->setData(0, Qt::UserRole, name);
        for (int mutation : database->getMutationsForTag(name)) {
            QTreeWidgetItem *subItem = new QTreeWidgetItem(treeItem);
            subItem->setText(0, "Mutation " + QString::number(mutation));
            subItem->setData(0, Qt::UserRole, QString::number(mutation));
            for (QString name : database->getSourcesForMutation(mutation)) {
                QTreeWidgetItem *lineItem = new QTreeWidgetItem(subItem);
                QStringList parts = name.split(':');
                QString realname = parts.at(0);
                QFileInfo fi = QFileInfo(realname);
                lineItem->setText(0, fi.completeBaseName() + "." + fi.completeSuffix() + ":" + parts.at(1));
                lineItem->setData(0, Qt::UserRole, name);
                subItem->addChild(lineItem);
            }
            treeItem->addChild(subItem);
        }
        tagList->addTopLevelItem(treeItem);
    }
    tagList->setContextMenuPolicy(Qt::CustomContextMenu);
    tagList->installEventFilter(this);

    tabWidget->addTab(sourceList, "Sources");
    tabWidget->addTab(mutationsList, "Mutations");
    tabWidget->addTab(tagList, "Tags");
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
    connect(propertyEditor->treeWidget(), &QTreeWidget::customContextMenuRequested, this, &BrowserWidget::prepareContextMenu);

    tagFilter = new QComboBox();
    tagFilter->addItems(database->getUniqueTags(true));
    connect(tagFilter, &QComboBox::currentTextChanged, this, &BrowserWidget::onTagFilterChange);

    actionFirst = new QAction("", this);
    actionFirst->setIcon(QIcon(":/icons/resources/resultset_first.png"));
    actionFirst->setEnabled(false);
    connect(actionFirst, &QAction::triggered, this, [this] {
        history_ignore = true;
        history_index = 0;
        auto curr = history.at(history_index);
        if (tabWidget->currentWidget() != curr.first) {
            ((QTreeWidget *)tabWidget->currentWidget())->selectionModel()->clearSelection();
            tabWidget->setCurrentWidget(curr.first);
            curr.first->setCurrentItem(curr.second, 0, QItemSelectionModel::Select);
        } else
            curr.first->setCurrentItem(curr.second, 0, QItemSelectionModel::ClearAndSelect);
        curr.first->currentItem()->setHidden(false);
        updateButtons();
    });

    actionPrev = new QAction("", this);
    actionPrev->setIcon(QIcon(":/icons/resources/resultset_previous.png"));
    actionPrev->setEnabled(false);
    connect(actionPrev, &QAction::triggered, this, [this] {
        history_ignore = true;
        history_index--;
        auto curr = history.at(history_index);
        if (tabWidget->currentWidget() != curr.first) {
            ((QTreeWidget *)tabWidget->currentWidget())->selectionModel()->clearSelection();
            tabWidget->setCurrentWidget(curr.first);
            curr.first->setCurrentItem(curr.second, 0, QItemSelectionModel::Select);
        } else
            curr.first->setCurrentItem(curr.second, 0, QItemSelectionModel::ClearAndSelect);
        curr.first->currentItem()->setHidden(false);
        updateButtons();
    });

    actionNext = new QAction("", this);
    actionNext->setIcon(QIcon(":/icons/resources/resultset_next.png"));
    actionNext->setEnabled(false);
    connect(actionNext, &QAction::triggered, this, [this] {
        history_ignore = true;
        history_index++;
        auto curr = history.at(history_index);
        if (tabWidget->currentWidget() != curr.first) {
            ((QTreeWidget *)tabWidget->currentWidget())->selectionModel()->clearSelection();
            tabWidget->setCurrentWidget(curr.first);
            curr.first->setCurrentItem(curr.second, 0, QItemSelectionModel::Select);
        } else
            curr.first->setCurrentItem(curr.second, 0, QItemSelectionModel::ClearAndSelect);
        curr.first->currentItem()->setHidden(false);
        updateButtons();
    });

    actionLast = new QAction("", this);
    actionLast->setIcon(QIcon(":/icons/resources/resultset_last.png"));
    actionLast->setEnabled(false);
    connect(actionLast, &QAction::triggered, this, [this] {
        history_ignore = true;
        history_index = int(history.size() - 1);
        auto curr = history.at(history_index);
        if (tabWidget->currentWidget() != curr.first) {
            ((QTreeWidget *)tabWidget->currentWidget())->selectionModel()->clearSelection();
            tabWidget->setCurrentWidget(curr.first);
            curr.first->setCurrentItem(curr.second, 0, QItemSelectionModel::Select);
        } else
            curr.first->setCurrentItem(curr.second, 0, QItemSelectionModel::ClearAndSelect);
        curr.first->currentItem()->setHidden(false);
        updateButtons();
    });

    actionClear = new QAction("", this);
    actionClear->setIcon(QIcon(":/icons/resources/cross.png"));
    actionClear->setEnabled(true);
    connect(actionClear, &QAction::triggered, this, [this] {
        history_index = -1;
        history.clear();
        addToHistory((QTreeWidget *)tabWidget->currentWidget(),
                     ((QTreeWidget *)tabWidget->currentWidget())->currentItem());
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
    vbox1->addWidget(tagFilter);
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
    connect(propertyEditor->treeWidget(), &QTreeWidget::itemDoubleClicked, this,
            &BrowserWidget::onPropertyDoubleClicked);

    connect(sourceList, &QTreeWidget::itemDoubleClicked, this, &BrowserWidget::onSourceDoubleClicked);
    connect(sourceList->selectionModel(), &QItemSelectionModel::selectionChanged, this,
            &BrowserWidget::onSourceSelectionChanged);

    connect(mutationsList, &QTreeWidget::itemDoubleClicked, this, &BrowserWidget::onMutationDoubleClicked);
    connect(mutationsList->selectionModel(), &QItemSelectionModel::selectionChanged, this,
            &BrowserWidget::onMutationSelectionChanged);

    connect(tagList, &QTreeWidget::itemDoubleClicked, this, &BrowserWidget::onTagMutationDoubleClicked);
    connect(tagList->selectionModel(), &QItemSelectionModel::selectionChanged, this,
            &BrowserWidget::onTagMutationSelectionChanged);

    connect(tabWidget, &QTabWidget::currentChanged, this, &BrowserWidget::onCurrentTabChanged);

    history_index = -1;
    history_ignore = false;

    sourceList->setCurrentItem(sourceList->topLevelItem(0), 0, QItemSelectionModel::ClearAndSelect);
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

void BrowserWidget::addToHistory(QTreeWidget *tree, QTreeWidgetItem *item)
{
    if (!history_ignore) {
        int count = int(history.size());
        for (int i = count - 1; i > history_index; i--)
            history.pop_back();
        history.push_back(std::make_pair(tree, item));
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

void BrowserWidget::onSourceSelectionChanged(const QItemSelection &selected, const QItemSelection &deselected)
{
    QTreeWidgetItem *item = sourceList->currentItem();
    if (selected.size() == 0 || item == nullptr || item->parent() == nullptr) {
        clearProperties();
        Q_EMIT unselectLine();
        return;
    }

    QString source = item->data(0, Qt::UserRole).toString();
    int mutationId = -1;
    if (item->parent()->parent() != nullptr) {
        source = item->parent()->data(0, Qt::UserRole).toString();
        mutationId = item->data(0, Qt::UserRole).toInt();
    }

    addToHistory(sourceList, item);

    clearProperties();

    if (mutationId != -1) {
        QString msg = getMutationMessage(mutationId);
        QtProperty *descItem = addTopLevelProperty("Description:\n" + msg);
        propertyEditor->setBackgroundColor(propertyEditor->itemToBrowserItem(propertyEditor->treeWidget()->topLevelItem(0)), QColor(0, 178, 0, 127));

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
    } else {
        QtProperty *topItem = addTopLevelProperty("Source");
        addProperty(topItem, QVariant::String, "Name", source);

        QStringList param = source.split(':');
        Q_EMIT selectLine(param.at(0), param.at(1));
    }
}

void BrowserWidget::onTagMutationSelectionChanged(const QItemSelection &selected, const QItemSelection &deselected)
{
    QTreeWidgetItem *item = tagList->currentItem();
    if (selected.size() == 0 || item == nullptr) {
        Q_EMIT unselectLine();
        return;
    }

    QString source = "";
    int mutationId = -1;
    if (item->parent() == nullptr) {
        clearProperties();
        Q_EMIT unselectLine();
        return;
    } else {
        mutationId = item->data(0, Qt::UserRole).toInt();
        if (item->parent()->parent() != nullptr) {
            source = item->data(0, Qt::UserRole).toString();
            mutationId =  item->parent()->data(0, Qt::UserRole).toInt();
        }
    }

    addToHistory(tagList, item);
    mutationProperties(source, mutationId);
}

void BrowserWidget::onMutationSelectionChanged(const QItemSelection &selected, const QItemSelection &deselected)
{
    QTreeWidgetItem *item = mutationsList->currentItem();
    if (selected.size() == 0 || item == nullptr) {
        clearProperties();
        Q_EMIT unselectLine();
        return;
    }

    QString source = "";
    int mutationId = -1;
    if (item->parent() == nullptr) {
        mutationId = item->data(0, Qt::UserRole).toInt();
    } else {
        source = item->data(0, Qt::UserRole).toString();
        mutationId = item->parent()->data(0, Qt::UserRole).toInt();
    }

    addToHistory(mutationsList, item);
    mutationProperties(source, mutationId);
}

QString BrowserWidget::getMutationMessage(int mutationId)
{
    auto options = database->getMutationOption(mutationId);
    QString mode, module, cell, port, portbit, ctrlbit;
    for (auto option : options) {
        if (option.first == "mode") mode = option.second;
        else if (option.first ==  "module") module = option.second;
        else if (option.first == "cell") cell = option.second;
        else if (option.first == "port") port = option.second;
        else if (option.first == "portbit") portbit = option.second;
        else if (option.first == "ctrlbit") ctrlbit = option.second;
    }
    QString msg;
    if (mode=="none")
        msg = QString("None");
    else if (mode=="inv")
        msg = QString("In module %1, cell %2:\nInvert bit %4 of port %3.").arg(module).arg(cell).arg(port).arg(portbit);
    else if (mode=="const0")
        msg = QString("In module %1, cell %2:\nDrive bit %4 of port %3 to constant 0.").arg(module).arg(cell).arg(port).arg(portbit);
    else if (mode=="const1")
        msg = QString("In module %1, cell %2:\nDrive bit %4 of port %3 to constant 1.").arg(module).arg(cell).arg(port).arg(portbit);
    else if (mode=="cnot0")
        msg = QString("In module %1, cell %2:\nIf bit %4 of port %3 is 0, invert bit %5.").arg(module).arg(cell).arg(port).arg(portbit).arg(ctrlbit);
    else if (mode=="cnot1")
        msg = QString("In module %1, cell %2:\nIf bit %4 of port %3 is 1, invert bit %5.").arg(module).arg(cell).arg(port).arg(portbit).arg(ctrlbit);
    return msg;
}

void BrowserWidget::mutationProperties(QString source, int mutationId)
{
    clearProperties();

    QString msg = getMutationMessage(mutationId);
    QtProperty *descItem = addTopLevelProperty("Description:\n" + msg);
    propertyEditor->setBackgroundColor(propertyEditor->itemToBrowserItem(propertyEditor->treeWidget()->topLevelItem(0)), QColor(0, 178, 0, 127));

    if (source == "") {
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
        Q_EMIT unselectLine();
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
        bool ok;
        QStringList param = source.split(':');
        Q_EMIT selectLine(param.at(0), param.at(1));
    }
}

void BrowserWidget::onSourceDoubleClicked(QTreeWidgetItem *item, int column)
{
    if (item->parent()->parent() != nullptr) {
        selectMutation(item->data(0, Qt::UserRole).toString());
    }
}

void BrowserWidget::onMutationDoubleClicked(QTreeWidgetItem *item, int column)
{
    if (item->parent() == nullptr)
        return;

    selectSource(item->data(0, Qt::UserRole).toString());
}

void BrowserWidget::onTagMutationDoubleClicked(QTreeWidgetItem *item, int column)
{
    if (item->parent() == nullptr)
        return;
    if (item->parent() == nullptr) {
        selectMutation(item->data(0, Qt::UserRole).toString());
    } else {
        selectSource(item->data(0, Qt::UserRole).toString());
    }
}

void BrowserWidget::onPropertyDoubleClicked(QTreeWidgetItem *item, int column)
{
    QtProperty *selectedProperty = propertyEditor->itemToBrowserItem(item)->property();
    QString type = selectedProperty->propertyId();
    if (type != "src")
        return;
    selectSource(selectedProperty->valueText());
}

QString BrowserWidget::selectSource(QString source)
{
    QTreeWidgetItemIterator it(sourceList);
    while (*it) {
        QString val = (*it)->data(0, Qt::UserRole).toString();
        if (val == source || val.startsWith(source + ".")) {
            if (tabWidget->currentWidget() != sourceList) {
                ((QTreeWidget *)tabWidget->currentWidget())->selectionModel()->clearSelection();
                tabWidget->setCurrentWidget(sourceList);
            }
            sourceList->setCurrentItem(*it, 0, QItemSelectionModel::ClearAndSelect);
            return val;
        }
        ++it;
    }
    return source;
}

void BrowserWidget::selectMutation(QString mutation)
{
    QTreeWidgetItemIterator it(mutationsList);
    while (*it) {
        QString val = (*it)->data(0, Qt::UserRole).toString();
        if (val == mutation) {
            if (tabWidget->currentWidget() != mutationsList) {
                ((QTreeWidget *)tabWidget->currentWidget())->selectionModel()->clearSelection();
                tabWidget->setCurrentWidget(mutationsList);
            }
            mutationsList->setCurrentItem(*it, 0, QItemSelectionModel::ClearAndSelect);
            break;
        }
        ++it;
    }
}
bool BrowserWidget::eventFilter(QObject *obj, QEvent *event)
{
    if (obj == sourceList) {
        if (event->type() == QEvent::KeyPress) {
            QKeyEvent *key = static_cast<QKeyEvent *>(event);
            if ((key->key() == Qt::Key_Enter) || (key->key() == Qt::Key_Return)) {
                onSourceDoubleClicked(sourceList->currentItem(), 0);
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
                onMutationDoubleClicked(mutationsList->currentItem(), 0);
            } else {
                return QObject::eventFilter(obj, event);
            }
            return true;
        } else {
            return false;
        }
    } else if (obj == tagList) {
        if (event->type() == QEvent::KeyPress) {
            QKeyEvent *key = static_cast<QKeyEvent *>(event);
            if ((key->key() == Qt::Key_Enter) || (key->key() == Qt::Key_Return)) {
                onTagMutationDoubleClicked(tagList->currentItem(), 0);
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

void BrowserWidget::onTagFilterChange(const QString &text)
{
    QList<int> mutations = database->getMutationsForTag(text);
    for (int i = 0; i < sourceList->topLevelItemCount(); i++) {
        QTreeWidgetItem *item = sourceList->topLevelItem(i);
        for (int j = 0; j < item->childCount(); j++) {
            QTreeWidgetItem *it = item->child(j);
            for (int k = 0; k < it->childCount(); k++) {
                QTreeWidgetItem *dataItem = it->child(k);
                if (text == DbManager::ALL_TAGS)
                    dataItem->setHidden(false);
                else
                    dataItem->setHidden(mutations.contains(dataItem->data(0, Qt::UserRole).toInt()) == 0);
            }
        }
    }
    for (int i = 0; i < mutationsList->topLevelItemCount(); i++) {
        QTreeWidgetItem *dataItem = mutationsList->topLevelItem(i);
        if (text == DbManager::ALL_TAGS)
            dataItem->setHidden(false);
        else {
            dataItem->setHidden(mutations.contains(dataItem->data(0, Qt::UserRole).toInt()) == 0);
        }
    }
    for (int i = 0; i < tagList->topLevelItemCount(); i++) {
        QTreeWidgetItem *item = tagList->topLevelItem(i);
        for (int j = 0; j < item->childCount(); j++) {
            QTreeWidgetItem *dataItem = item->child(j);
            if (text == DbManager::ALL_TAGS)
                dataItem->setHidden(false);
            else {
                dataItem->setHidden(mutations.contains(dataItem->data(0, Qt::UserRole).toInt()) == 0);
            }
        }
    }
}

void BrowserWidget::onCurrentTabChanged(int index)
{
    switch (index)
    {
        case 0:
            onSourceSelectionChanged(sourceList->selectionModel()->selection(), QItemSelection());
            break;
        case 1:
            onMutationSelectionChanged(mutationsList->selectionModel()->selection(), QItemSelection());
            break;
        case 2:
            onTagMutationSelectionChanged(tagList->selectionModel()->selection(), QItemSelection());
            break;
        default:
            break;
    }
}

void BrowserWidget::prepareContextMenu(const QPoint & pos)
{
    QAction *copyAction = new QAction("&Copy", this);
    connect(copyAction, &QAction::triggered, [=] {
        QTreeWidgetItem *item = propertyEditor->treeWidget()->itemAt(pos);
        QClipboard* clipboard = QApplication::clipboard();
        clipboard->setText(item->text(item->columnCount()-1));
    });
    QPoint pt(pos);
    QMenu menu(this);
    menu.addAction(copyAction);
    menu.exec(propertyEditor->treeWidget()->viewport()->mapToGlobal(pos));
}
