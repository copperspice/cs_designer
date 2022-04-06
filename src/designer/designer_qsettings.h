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

#ifndef QDESIGNER_QSETTINGS_H
#define QDESIGNER_QSETTINGS_H

#include <abstract_settings.h>
#include <designer_settings.h>

#include <QSettings>

//  Implements QDesignerSettingsInterface by calls to QSettings
class QDesignerQSettings : public QDesignerSettingsInterface
{
 public:
   QDesignerQSettings();

   void beginGroup(const QString &prefix) override;
   void endGroup() override;

   bool contains(const QString &key) const override;
   void setValue(const QString &key, const QVariant &value) override;
   QVariant value(const QString &key, const QVariant &defaultValue = QVariant()) const override;
   void remove(const QString &key) override;

   // The application name to be used for settings. Allows for including
   // the version to prevent settings of different versions from interfering.
   static QString settingsApplicationName();

 private:
   QSettings m_settings;
};

#endif
