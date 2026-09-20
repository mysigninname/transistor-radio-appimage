#include <KColorSchemeManager>
#include <QAbstractItemModel>

/*
SPDX-FileCopyrightText: 2024 Yuri Saurov <dr@i-glu4it.ru>
SPDX-License-Identifier: GPL-3.0-or-later
*/

#include "ColorSchemer.h"

ColorSchemer::ColorSchemer(QObject *parent)
    : QObject(parent)
{
}

ColorSchemer &ColorSchemer::instance()
{
    static ColorSchemer colorSchemer;
    return colorSchemer;
}

QAbstractItemModel *ColorSchemer::model() const
{
    return KColorSchemeManager::instance()->model();
}

void ColorSchemer::apply(int idx)
{
    KColorSchemeManager::instance()->activateScheme(KColorSchemeManager::instance()->model()->index(idx, 0));
}

void ColorSchemer::apply(const QString &name)
{
    KColorSchemeManager::instance()->activateScheme(KColorSchemeManager::instance()->indexForScheme(name));
}

int ColorSchemer::indexForScheme(const QString &name) const
{
    auto index = KColorSchemeManager::instance()->indexForScheme(name).row();
    if (index == -1) {
        index = 0;
    }
    return index;
}

QString ColorSchemer::nameForIndex(int index) const
{
    return KColorSchemeManager::instance()->model()->data(KColorSchemeManager::instance()->model()->index(index, 0), Qt::DisplayRole).toString();
}
