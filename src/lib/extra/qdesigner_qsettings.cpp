/***********************************************************************
*
* Copyright (c) 2021-2022 Barbara Geller
* Copyright (c) 2021-2022 Ansel Sermersheim
*
* Copyright (c) 2015 The Qt Company Ltd.
*
* Designer is free software. You can redistribute it and/or
* modify it under the terms of the GNU Lesser General Public License
* version 2.1 as published by the Free Software Foundation.
*
* Designer is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
*
* https://www.gnu.org/licenses/
*
***********************************************************************/

#include <qdesigner_qsettings_p.h>

#include <QSettings>
#include <QCoreApplication>
#include <QTextStream>
#include <QString>
#include <QDebug>

QDesignerQSettings::QDesignerQSettings() :
   m_settings(qApp->organizationName(), settingsApplicationName())
{
}

QString QDesignerQSettings::settingsApplicationName()
{
   return qApp->applicationName();
}

void QDesignerQSettings::beginGroup(const QString &prefix)
{
   m_settings.beginGroup(prefix);
}

void QDesignerQSettings::endGroup()
{
   m_settings.endGroup();
}

bool QDesignerQSettings::contains(const QString &key) const
{
   return m_settings.contains(key);
}

void QDesignerQSettings::setValue(const QString &key, const QVariant &value)
{
   m_settings.setValue(key, value);
}

QVariant QDesignerQSettings::value(const QString &key, const QVariant &defaultValue) const
{
   return m_settings.value(key, defaultValue);
}

void QDesignerQSettings::remove(const QString &key)
{
   m_settings.remove(key);
}
