#pragma once
#include <QtGlobal>

#if QT_VERSION_MAJOR >= 6
#include <QMetaType>
#include <QRegularExpression>
#include <QRegularExpressionValidator>
#include <QColorDialog>
#include <QStyleOption>
#include <QFont>
#include <QTreeView>

// QRegExp / QRegExpValidator aliases
using QRegExp = QRegularExpression;
using QRegExpValidator = QRegularExpressionValidator;

// qVariantSetValue replacement
#define qVariantSetValue(v, val) v = QVariant::fromValue(val)

// QStyleOption::init replacement
#define INIT_STYLE_OPTION(opt, widget) \
    opt.rect = widget->rect(); \
    opt.state = widget->isEnabled() ? QStyle::State_Enabled : QStyle::State_None; \
    opt.palette = widget->palette(); \
    opt.fontMetrics = widget->fontMetrics();

// QStyleOptionViewItemV2 alias
#define QStyleOptionViewItemV2 QStyleOptionViewItem

#else

#include <QRegExp>
#include <QRegExpValidator>
#define INIT_STYLE_OPTION(opt, widget) opt.init(widget)

#endif
