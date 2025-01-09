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

#include <previewactiongroup.h>

#include <deviceprofile_p.h>
#include <shared_settings_p.h>

#include <QStyleFactory>
#include <QVariant>

constexpr const int MAX_DEVICE_ACTIONS = 20;

namespace qdesigner_internal {

PreviewActionGroup::PreviewActionGroup(QDesignerFormEditorInterface *core, QObject *parent)
   : QActionGroup(parent), m_core(core)
{
   /* Create a list of up to MAX_DEVICE_ACTIONS invisible actions to be
    * populated with device profiles (actiondata: index) followed by the
    * standard style actions (actiondata: style name). */
   connect(this, &PreviewActionGroup::triggered, this, &PreviewActionGroup::slotTriggered);
   setExclusive(true);

   const QString objNamePostfix = QString("_action");
   // Create invisible actions for devices. Set index as action data.
   QString objNamePrefix = QString("__qt_designer_device_");
   for (int i = 0; i < MAX_DEVICE_ACTIONS; i++) {
      QAction *a = new QAction(this);
      QString objName = objNamePrefix;
      objName += QString::number(i);
      objName += objNamePostfix;
      a->setObjectName(objName);
      a->setVisible(false);
      a->setData(i);
      addAction(a);
   }

   // Create separator at index MAX_DEVICE_ACTIONS
   QAction *sep = new QAction(this);
   sep->setObjectName(QString("__qt_designer_deviceseparator"));
   sep->setSeparator(true);
   sep->setVisible(false);
   addAction(sep);

   // Populate devices
   updateDeviceProfiles();

   // Add style actions
   const QStringList styles = QStyleFactory::keys();
   const QStringList::const_iterator cend = styles.constEnd();
   // Make sure ObjectName  is unique in case toolbar solution is used.
   objNamePrefix = QString("__qt_designer_style_");
   // Create styles. Set style name string as action data.
   for (QStringList::const_iterator it = styles.constBegin(); it !=  cend ; ++it) {
      QAction *a = new QAction(tr("%1 Style").formatArg(*it), this);
      QString objName = objNamePrefix;
      objName += *it;
      objName += objNamePostfix;
      a->setObjectName(objName);
      a->setData(*it);
      addAction(a);
   }
}

void PreviewActionGroup::updateDeviceProfiles()
{
   const QDesignerSharedSettings settings(m_core);
   const QList<DeviceProfile> profiles = settings.deviceProfiles();
   const QList<QAction *> al = actions();

   // Separator?
   const bool hasProfiles = !profiles.empty();
   al.at(MAX_DEVICE_ACTIONS)->setVisible(hasProfiles);
   int index = 0;

   if (hasProfiles) {
      // Make actions visible
      const int maxIndex = qMin(static_cast<int>(MAX_DEVICE_ACTIONS), profiles.size());

      for (; index < maxIndex; index++) {
         const QString name = profiles.at(index).name();
         al.at(index)->setText(name);
         al.at(index)->setVisible(true);
      }
   }

   // Hide rest
   for ( ; index < MAX_DEVICE_ACTIONS; index++) {
      al.at(index)->setVisible(false);
   }
}

void PreviewActionGroup::slotTriggered(QAction *a)
{
   // Device or style according to data.
   const QVariant data = a->data();

   switch (data.type()) {
      case QVariant::String:
         emit preview(data.toString(), -1);
         break;

      case QVariant::Int:
         emit preview(QString(), data.toInt());
         break;

      default:
         break;
   }
}

}   // end namespace
