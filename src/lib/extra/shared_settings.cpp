/***********************************************************************
*
* Copyright (c) 2021-2025 Barbara Geller
* Copyright (c) 2021-2025 Ansel Sermersheim
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
#include <designer_settings.h>
#include <designer_utils.h>
#include <preview_manager.h>

#include <grid_p.h>
#include <shared_settings_p.h>

#include <QCoreApplication>
#include <QDir>
#include <QSize>
#include <QStringList>
#include <QVariant>

static const QString defaultGridKey        = "defaultGrid";
static const QString designerPath          = "/.designer";
static const QString deviceProfileIndexKey = "DeviceProfileIndex";
static const QString deviceProfilesKey     = "DeviceProfiles";
static const QString enabledKey            = "Enabled";
static const QString formTemplateKey       = "FormTemplate";
static const QString formTemplatePathsKey  = "FormTemplatePaths";
static const QString newFormSizeKey        = "NewFormSize";
static const QString previewKey            = "Preview";
static const QString zoomEnabledKey        = "zoomEnabled";
static const QString zoomKey               = "zoom";

//   static const char *userDeviceSkinsKey= "UserDeviceSkins"

using namespace qdesigner_internal;

static bool checkTemplatePath(const QString &path, bool create)
{
   QDir current(QDir::current());
   if (current.exists(path)) {
      return true;
   }

   if (!create) {
      return false;
   }

   if (current.mkpath(path)) {
      return true;
   }

   csWarning(QCoreApplication::translate("QDesignerSharedSettings",
         "The template path %1 could not be created.").formatArg(path));
   return false;
}

namespace qdesigner_internal {

QDesignerSharedSettings::QDesignerSharedSettings(QDesignerFormEditorInterface *core)
   : m_settings(core->settingsManager())
{
}

Grid QDesignerSharedSettings::defaultGrid() const
{
   Grid grid;
   const QVariantMap defaultGridMap
      = m_settings->value(defaultGridKey, QVariantMap()).toMap();
   if (!defaultGridMap.empty()) {
      grid.fromVariantMap(defaultGridMap);
   }
   return grid;
}

void QDesignerSharedSettings::setDefaultGrid(const Grid &grid)
{
   m_settings->setValue(defaultGridKey, grid.toVariantMap());
}

const QStringList &QDesignerSharedSettings::defaultFormTemplatePaths()
{
   static QStringList rc;
   if (rc.empty()) {
      // Ensure default form template paths
      const QString templatePath = QString("/templates");
      // home
      QString path = QDir::homePath();
      path += designerPath;
      path += templatePath;

      if (checkTemplatePath(path, true)) {
         rc += path;
      }

      // designer/bin: Might be owned by root in some installations, do not force it.
      path = qApp->applicationDirPath();
      path += templatePath;
      if (checkTemplatePath(path, false)) {
         rc += path;
      }
   }
   return rc;
}

QStringList QDesignerSharedSettings::formTemplatePaths() const
{
   return m_settings->value(formTemplatePathsKey, defaultFormTemplatePaths()).toStringList();
}

void QDesignerSharedSettings::setFormTemplatePaths(const QStringList &paths)
{
   m_settings->setValue(formTemplatePathsKey, paths);
}

QString QDesignerSharedSettings::formTemplate() const
{
   return m_settings->value(formTemplateKey).toString();
}

void QDesignerSharedSettings::setFormTemplate(const QString &t)
{
   m_settings->setValue(formTemplateKey, t);
}

void QDesignerSharedSettings::setAdditionalFormTemplatePaths(const QStringList &additionalPaths)
{
   // merge template paths
   QStringList templatePaths = defaultFormTemplatePaths();
   templatePaths += additionalPaths;
   setFormTemplatePaths(templatePaths);
}

QStringList QDesignerSharedSettings::additionalFormTemplatePaths() const
{
   // get template paths excluding internal ones
   QStringList rc = formTemplatePaths();

   for (const QString &internalTemplatePath : defaultFormTemplatePaths()) {
      const int index = rc.indexOf(internalTemplatePath);
      if (index != -1) {
         rc.removeAt(index);
      }
   }
   return rc;
}

QSize QDesignerSharedSettings::newFormSize() const
{
   return m_settings->value(newFormSizeKey, QSize(0, 0)).toSize();
}

void  QDesignerSharedSettings::setNewFormSize(const QSize &s)
{
   if (s.isNull()) {
      m_settings->remove(newFormSizeKey);
   } else {
      m_settings->setValue(newFormSizeKey, s);
   }
}


PreviewConfiguration QDesignerSharedSettings::customPreviewConfiguration() const
{
   PreviewConfiguration configuration;
   configuration.fromSettings(previewKey, m_settings);

   return configuration;
}

void QDesignerSharedSettings::setCustomPreviewConfiguration(const PreviewConfiguration &configuration)
{
   configuration.toSettings(previewKey, m_settings);
}

bool QDesignerSharedSettings::isCustomPreviewConfigurationEnabled() const
{
   m_settings->beginGroup(previewKey);
   bool isEnabled = m_settings->value(enabledKey, false).toBool();
   m_settings->endGroup();
   return isEnabled;
}

void QDesignerSharedSettings::setCustomPreviewConfigurationEnabled(bool enabled)
{
   m_settings->beginGroup(previewKey);
   m_settings->setValue(enabledKey, enabled);
   m_settings->endGroup();
}

/*

QStringList QDesignerSharedSettings::userDeviceSkins() const
{
    m_settings->beginGroup(previewKey);
    QStringList userDeviceSkins
            = m_settings->value(userDeviceSkinsKey, QStringList()).toStringList();
    m_settings->endGroup();
    return userDeviceSkins;
}

void QDesignerSharedSettings::setUserDeviceSkins(const QStringList &userDeviceSkins)
{
    m_settings->beginGroup(previewKey);
    m_settings->setValue(userDeviceSkinsKey, userDeviceSkins);
    m_settings->endGroup();
}

*/

int QDesignerSharedSettings::zoom() const
{
   return m_settings->value(zoomKey, 100).toInt();
}

void QDesignerSharedSettings::setZoom(int z)
{
   m_settings->setValue(zoomKey, QVariant(z));
}

bool QDesignerSharedSettings::zoomEnabled() const
{
   return m_settings->value(zoomEnabledKey, false).toBool();
}

void QDesignerSharedSettings::setZoomEnabled(bool v)
{
   m_settings->setValue(zoomEnabledKey, v);
}

DeviceProfile QDesignerSharedSettings::currentDeviceProfile() const
{
   return deviceProfileAt(currentDeviceProfileIndex());
}

void QDesignerSharedSettings::setCurrentDeviceProfileIndex(int i)
{
   m_settings->setValue(deviceProfileIndexKey, i);
}

int QDesignerSharedSettings::currentDeviceProfileIndex() const
{
   return m_settings->value(deviceProfileIndexKey, -1).toInt();
}

static inline QString msgWarnDeviceProfileXml(const QString &msg)
{
   return QCoreApplication::translate("QDesignerSharedSettings",
         "An error has been encountered while parsing device profile XML: %1").formatArg(msg);
}

DeviceProfile QDesignerSharedSettings::deviceProfileAt(int idx) const
{
   DeviceProfile rc;
   if (idx < 0) {
      return rc;
   }
   const QStringList xmls = deviceProfileXml();
   if (idx >= xmls.size()) {
      return rc;
   }
   QString errorMessage;
   if (!rc.fromXml(xmls.at(idx), &errorMessage)) {
      rc.clear();
      csWarning(msgWarnDeviceProfileXml(errorMessage));
   }
   return rc;
}

QStringList QDesignerSharedSettings::deviceProfileXml() const
{
   return m_settings->value(deviceProfilesKey, QStringList()).toStringList();
}

QList<DeviceProfile> QDesignerSharedSettings::deviceProfiles() const
{
   QList<DeviceProfile> retval;

   const QStringList xmls = deviceProfileXml();

   if (xmls.empty()) {
      return retval;
   }

   // De-serialize
   QString errorMessage;
   DeviceProfile dp;

   for (QStringList::const_iterator iter = xmls.constBegin(); iter != xmls.constEnd(); ++iter) {
      if (dp.fromXml(*iter, &errorMessage)) {
         retval.push_back(dp);
      } else {
         csWarning(msgWarnDeviceProfileXml(errorMessage));
      }
   }

   return retval;
}

void QDesignerSharedSettings::setDeviceProfiles(const QList<DeviceProfile> &dp)
{
   QStringList list;

   for (auto iter = dp.constBegin(); iter != dp.constEnd(); ++iter) {
      list.push_back(iter->toXml());
   }

   m_settings->setValue(deviceProfilesKey, list);
}

}
