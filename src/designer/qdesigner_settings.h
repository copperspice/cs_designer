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

#ifndef QDESIGNER_SETTINGS_H
#define QDESIGNER_SETTINGS_H

#include <abstract_settings.h>
#include <abstract_formeditor.h>
#include <designer_enums.h>

#include <shared_settings_p.h>

#include <QMap>
#include <QRect>
#include <QStringList>
#include <QVariant>

struct ToolWindowFontSettings;

class QDesignerSettings : public qdesigner_internal::QDesignerSharedSettings
{
 public:
   QDesignerSettings(QDesignerFormEditorInterface *core);

   void setValue(const QString &key, const QVariant &value);
   QVariant value(const QString &key, const QVariant &defaultValue = QVariant()) const;

   void restoreGeometry(QWidget *w, QRect fallBack = QRect()) const;
   void saveGeometryFor(const QWidget *w);

   QStringList recentFilesList() const;
   void setRecentFilesList(const QStringList &list);

   void setShowNewFormOnStartup(bool showIt);
   bool showNewFormOnStartup() const;

   void setUiMode(UIMode mode);
   UIMode uiMode() const;

   void setToolWindowFont(const ToolWindowFontSettings &fontSettings);
   ToolWindowFontSettings toolWindowFont() const;

   QByteArray mainWindowState(UIMode mode) const;
   void setMainWindowState(UIMode mode, const QByteArray &mainWindowState);

   QByteArray toolBarsState(UIMode mode) const;
   void setToolBarsState(UIMode mode, const QByteArray &mainWindowState);

   void clearBackup();
   void setBackup(const QMap<QString, QString> &map);
   QMap<QString, QString> backup() const;
};

#endif