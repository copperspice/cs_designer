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

#ifndef WIDGETBOX_DNDITEM_H
#define WIDGETBOX_DNDITEM_H

#include <designer_dnditem.h>

class QDesignerFormEditorInterface;
class DomUI;

namespace qdesigner_internal {

class WidgetBoxDnDItem : public QDesignerDnDItem
{
 public:
   WidgetBoxDnDItem(QDesignerFormEditorInterface *core,
      DomUI *dom_ui,
      const QPoint &global_mouse_pos);
};

}  // namespace qdesigner_internal



#endif // WIDGETBOX_DNDITEM_H
