/***********************************************************************
*
* Copyright (c) 2021-2024 Barbara Geller
* Copyright (c) 2021-2024 Ansel Sermersheim
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

#ifndef PREVIEWACTIONGROUP_H
#define PREVIEWACTIONGROUP_H

#include <QActionGroup>

class QDesignerFormEditorInterface;

namespace qdesigner_internal {

// used as a submenu for 'Preview in...', offers a menu of styles and device profiles.

class PreviewActionGroup : public QActionGroup
{
   CS_OBJECT(PreviewActionGroup)

 public:
   explicit PreviewActionGroup(QDesignerFormEditorInterface *core, QObject *parent = nullptr);

   CS_SIGNAL_1(Public, void preview(const QString &style, int deviceProfileIndex))
   CS_SIGNAL_2(preview, style, deviceProfileIndex)

   CS_SLOT_1(Public, void updateDeviceProfiles())
   CS_SLOT_2(updateDeviceProfiles)

 private:
   PreviewActionGroup(const PreviewActionGroup &) = delete;
   PreviewActionGroup &operator=(const PreviewActionGroup &) = delete;

   CS_SLOT_1(Private, void slotTriggered(QAction *un_named_arg1))
   CS_SLOT_2(slotTriggered)

   QDesignerFormEditorInterface *m_core;
};
}

#endif
