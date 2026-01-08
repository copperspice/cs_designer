/***********************************************************************
*
* Copyright (c) 2021-2026 Barbara Geller
* Copyright (c) 2021-2026 Ansel Sermersheim
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

#ifndef LAYOUTINFO_H
#define LAYOUTINFO_H

#include <QString>

class QDesignerFormEditorInterface;
class QFormLayout;

class QLayout;
class QLayoutItem;
class QRect;
class QWidget;

namespace qdesigner_internal {

class LayoutInfo
{
 public:
   enum Type {
      NoLayout,
      HSplitter,
      VSplitter,
      HBox,
      VBox,
      Grid,
      Form,
      UnknownLayout // QDockWindow inside QMainWindow is inside QMainWindowLayout - it doesn't mean there is no layout
   };

   static void deleteLayout(const QDesignerFormEditorInterface *core, QWidget *widget);

   // Examines the immediate layout of the widget.
   static Type layoutType(const QDesignerFormEditorInterface *core, const QWidget *w);

   // Examines the managed layout of the widget
   static Type managedLayoutType(const QDesignerFormEditorInterface *core, const QWidget *w, QLayout **layout = nullptr);
   static Type layoutType(const QDesignerFormEditorInterface *core, const QLayout *layout);
   static Type layoutType(const QString &typeName);
   static QString layoutName(Type t);

   static QWidget *layoutParent(const QDesignerFormEditorInterface *core, QLayout *layout);

   static Type laidoutWidgetType(const QDesignerFormEditorInterface *core, QWidget *widget,
         bool *isManaged = nullptr, QLayout **layout = nullptr);

   static bool inline isWidgetLaidout(const QDesignerFormEditorInterface *core, QWidget *widget) {
      return laidoutWidgetType(core, widget) != NoLayout;
   }

   static QLayout *managedLayout(const QDesignerFormEditorInterface *core, const QWidget *widget);
   static QLayout *managedLayout(const QDesignerFormEditorInterface *core, QLayout *layout);
   static QLayout *internalLayout(const QWidget *widget);

   // Is it a dummy grid placeholder created by Designer
   static bool isEmptyItem(QLayoutItem *item);
};

void getFormLayoutItemPosition(const QFormLayout *formLayout, int index, int *rowPtr,
   int *columnPtr = nullptr, int *rowspanPtr = nullptr, int *colspanPtr = nullptr);

void formLayoutAddWidget(QFormLayout *formLayout, QWidget *w, const QRect &r, bool insert);

} // namespace qdesigner_internal


#endif // LAYOUTINFO_H
