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

#include <abstract_formeditor.h>
#include <designer.h>
#include <designer_settings.h>
#include <designer_settings.h>
#include <designer_toolwindow.h>
#include <designer_workbench.h>
#include <preview_manager.h>

#include <designer_utils.h>

#include <QDebug>
#include <QDesktopWidget>
#include <QDir>
#include <QListView>
#include <QStyle>
#include <QVariant>

constexpr const int DEBUG_SETTINGS = 0;

static const QString newFormShowKey     = "newFormDialog/ShowOnStartup";
static const QString mainWindowStateKey = "MainWindowState45";
static const QString toolBarsStateKey   = "ToolBarsState45";
static const QString backupOrgListKey   = "backup/fileListOrg";
static const QString backupBakListKey   = "backup/fileListBak";
static const QString recentFilesListKey = "recentFilesList";

QDesignerSettings::QDesignerSettings(QDesignerFormEditorInterface *core)
   : qdesigner_internal::QDesignerSharedSettings(core)
{
}

void QDesignerSettings::setValue(const QString &key, const QVariant &value)
{
   settings()->setValue(key, value);
}

QVariant QDesignerSettings::value(const QString &key, const QVariant &defaultValue) const
{
   return settings()->value(key, defaultValue);
}

static inline QChar modeChar(UIMode mode)
{
   switch (mode) {
      case UIMode::NeutralMode:
         return QChar('0');

      case UIMode::TopLevelMode:
         return QChar('1');

      case UIMode::DockedMode:
         return QChar('2');

      default:
         return QChar();
   }
}

void QDesignerSettings::saveGeometryFor(const QWidget *w)
{
   Q_ASSERT(w && ! w->objectName().isEmpty());

   QDesignerSettingsInterface *s = settings();

   const bool visible = w->isVisible();

   if (DEBUG_SETTINGS) {
      qDebug() << "saveGeometryFor() " << w << "visible = " << visible;
   }

   s->beginGroup(w->objectName());
   s->setValue("visible", visible);
   s->setValue("geometry", w->saveGeometry());
   s->endGroup();
}

void QDesignerSettings::restoreGeometry(QWidget *w, QRect fallBack) const
{
   Q_ASSERT(w && ! w->objectName().isEmpty());

   const QString key = w->objectName();

   const QByteArray data = settings()->value(key + "/geometry").toByteArray();
   const bool visible    = settings()->value(key + "/visible", true).toBool();

   if (data.isEmpty()) {
      /// Apply default geometry, check for null and maximal size

      if (fallBack.isNull()) {
         fallBack = QRect(QPoint(0, 0), w->sizeHint());
      }

      if (fallBack.size() == QSize(QWIDGETSIZE_MAX, QWIDGETSIZE_MAX)) {
         w->setWindowState(w->windowState() | Qt::WindowMaximized);

      } else {
         w->move(fallBack.topLeft());
         w->resize(fallBack.size());
      }

   } else {
      w->restoreGeometry(data);

   }

   if (visible) {
      w->show();
   }
}

QStringList QDesignerSettings::recentFilesList() const
{
   return settings()->value(recentFilesListKey).toStringList();
}

void QDesignerSettings::setRecentFilesList(const QStringList &sl)
{
   settings()->setValue(recentFilesListKey, sl);
}

void QDesignerSettings::setShowNewFormOnStartup(bool showIt)
{
   settings()->setValue(QLatin1String(newFormShowKey), showIt);
}

bool QDesignerSettings::showNewFormOnStartup() const
{
   return settings()->value(QLatin1String(newFormShowKey), true).toBool();
}

QByteArray QDesignerSettings::mainWindowState(UIMode mode) const
{
   return settings()->value(QString(mainWindowStateKey) + modeChar(mode)).toByteArray();
}

void QDesignerSettings::setMainWindowState(UIMode mode, const QByteArray &mainWindowState)
{
   settings()->setValue(QString(mainWindowStateKey) + modeChar(mode), mainWindowState);
}

QByteArray QDesignerSettings::toolBarsState(UIMode mode) const
{
   QString key = toolBarsStateKey;
   key += modeChar(mode);




   return settings()->value(key).toByteArray();
}

void QDesignerSettings::setToolBarsState(UIMode mode, const QByteArray &toolBarsState)
{
   QString key = toolBarsStateKey;
   key += modeChar(mode);
   settings()->setValue(key, toolBarsState);
}

void QDesignerSettings::clearBackup()
{
   QDesignerSettingsInterface *s = settings();

   s->remove(QLatin1String(backupOrgListKey));
   s->remove(QLatin1String(backupBakListKey));
}

void QDesignerSettings::setBackup(const QMap<QString, QString> &map)
{
   const QStringList org = map.keys();
   const QStringList bak = map.values();

   QDesignerSettingsInterface *s = settings();
   s->setValue(QLatin1String(backupOrgListKey), org);
   s->setValue(QLatin1String(backupBakListKey), bak);
}

QMap<QString, QString> QDesignerSettings::backup() const
{
   const QStringList org = settings()->value(QLatin1String(backupOrgListKey), QStringList()).toStringList();
   const QStringList bak = settings()->value(QLatin1String(backupBakListKey), QStringList()).toStringList();

   QMap<QString, QString> map;
   const int orgCount = org.count();
   for (int i = 0; i < orgCount; ++i) {
      map.insert(org.at(i), bak.at(i));
   }

   return map;
}

void QDesignerSettings::setUiMode(UIMode mode)
{
   QDesignerSettingsInterface *s = settings();
   s->beginGroup(QString("UI"));
   s->setValue(QString("currentMode"), mode);
   s->endGroup();
}

UIMode QDesignerSettings::uiMode() const
{
#ifdef Q_OS_DARWIN
   const UIMode defaultMode = TopLevelMode;
#else
   const UIMode defaultMode = DockedMode;
#endif

   UIMode uiMode = static_cast<UIMode>(value(QString("UI/currentMode"), defaultMode).toInt());

   return uiMode;
}

void QDesignerSettings::setToolWindowFont(const ToolWindowFontSettings &fontSettings)
{
   QDesignerSettingsInterface *s = settings();
   s->beginGroup(QString("UI"));

   s->setValue(QString("font"), fontSettings.m_font);
   s->setValue(QString("useFont"), fontSettings.m_useFont);
   s->setValue(QString("writingSystem"), fontSettings.m_writingSystem);
   s->endGroup();
}

ToolWindowFontSettings QDesignerSettings::toolWindowFont() const
{
   ToolWindowFontSettings fontSettings;

   fontSettings.m_writingSystem = static_cast<QFontDatabase::WritingSystem>(value("UI/writingSystem", QFontDatabase::Any).toInt());
   fontSettings.m_font    = value("UI/font").value<QFont>();
   fontSettings.m_useFont = settings()->value("UI/useFont", QVariant(false)).toBool();

   return fontSettings;
}

