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

#ifndef PREVIEW_CONFIG_WIDGET_H
#define PREVIEW_CONFIG_WIDGET_H

#include <abstract_settings.h>
#include <abstract_formeditor.h>

#include <QGroupBox>
#include <QSharedDataPointer>

namespace qdesigner_internal {

// ----------- PreviewConfigurationWidget: Widget to edit the preview configuration.

class PreviewConfigurationWidget : public QGroupBox
{
   CS_OBJECT(PreviewConfigurationWidget)

 public:
   explicit PreviewConfigurationWidget(QDesignerFormEditorInterface *core,
      QWidget *parent = nullptr);
   virtual ~PreviewConfigurationWidget();
   void saveState();

 private:
   CS_SLOT_1(Private, void slotEditAppStyleSheet())
   CS_SLOT_2(slotEditAppStyleSheet)

   /*
       CS_SLOT_1(Private, void slotDeleteSkinEntry())
       CS_SLOT_2(slotDeleteSkinEntry)

       CS_SLOT_1(Private, void slotSkinChanged(int un_named_arg1))
       CS_SLOT_2(slotSkinChanged)
   */

   class PreviewConfigurationWidgetPrivate;
   PreviewConfigurationWidgetPrivate *m_impl;

   PreviewConfigurationWidget(const PreviewConfigurationWidget &other);
   PreviewConfigurationWidget &operator =(const PreviewConfigurationWidget &other);
};

} // namespace qdesigner_internal

#endif
