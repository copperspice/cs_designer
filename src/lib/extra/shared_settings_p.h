/***********************************************************************
*
* Copyright (c) 2021-2023 Barbara Geller
* Copyright (c) 2021-2023 Ansel Sermersheim
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

#ifndef SHARED_SETTINGS_H
#define SHARED_SETTINGS_H

#include <abstract_formeditor.h>
#include <abstract_settings.h>

#include <deviceprofile_p.h>

#include <qglobal.h>
#include <QList>
#include <QStringList>

class QSize;

namespace qdesigner_internal {

class Grid;
class PreviewConfiguration;

}

namespace qdesigner_internal {

class QDesignerSharedSettings
{
 public:
   typedef QList<DeviceProfile> DeviceProfileList;

   explicit QDesignerSharedSettings(QDesignerFormEditorInterface *core);

   Grid defaultGrid() const;
   void setDefaultGrid(const Grid &grid);

   QStringList formTemplatePaths() const;
   void setFormTemplatePaths(const QStringList &paths);

   void setAdditionalFormTemplatePaths(const QStringList &additionalPaths);
   QStringList additionalFormTemplatePaths() const;

   QString formTemplate() const;
   void setFormTemplate(const QString &t);

   QSize newFormSize() const;
   void setNewFormSize(const QSize &s);

   // Check with isCustomPreviewConfigurationEnabled if custom or default
   // configuration should be used.
   PreviewConfiguration customPreviewConfiguration() const;
   void setCustomPreviewConfiguration(const PreviewConfiguration &configuration);

   bool isCustomPreviewConfigurationEnabled() const;
   void setCustomPreviewConfigurationEnabled(bool enabled);

   QStringList userDeviceSkins() const;
   void setUserDeviceSkins(const QStringList &userDeviceSkins);

   bool zoomEnabled() const;
   void setZoomEnabled(bool v);

   // Zoom in percent
   int zoom() const;
   void setZoom(int z);

   // Embedded Design
   DeviceProfile currentDeviceProfile() const;
   void setCurrentDeviceProfileIndex(int i);
   int currentDeviceProfileIndex() const;

   DeviceProfile deviceProfileAt(int idx) const;
   DeviceProfileList deviceProfiles() const;
   void setDeviceProfiles(const DeviceProfileList &dp);

   static const QStringList &defaultFormTemplatePaths();

 protected:
   QDesignerSettingsInterface *settings() const {
      return m_settings;
   }

 private:
   QStringList deviceProfileXml() const;
   QDesignerSettingsInterface *m_settings;
};

} // namespace qdesigner_internal

#endif // SHARED_SETTINGS_H
