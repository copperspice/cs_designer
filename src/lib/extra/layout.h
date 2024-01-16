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

#ifndef LAYOUT_H
#define LAYOUT_H

#include <layout_info.h>

#include <QGridLayout>
#include <QHash>
#include <QLayout>
#include <QMap>
#include <QObject>
#include <QPointer>
#include <QWidget>

class QDesignerFormWindowInterface;

namespace qdesigner_internal {

class Layout : public QObject
{
   CS_OBJECT(Layout)
   Q_DISABLE_COPY(Layout)

 protected:
   Layout(const QWidgetList &wl, QWidget *p, QDesignerFormWindowInterface *fw, QWidget *lb, LayoutInfo::Type layoutType);

 public:
   static  Layout *createLayout(const QWidgetList &widgets,  QWidget *parentWidget,
      QDesignerFormWindowInterface *fw,
      QWidget *layoutBase, LayoutInfo::Type layoutType);

   virtual ~Layout();

   virtual void sort() = 0;
   virtual void doLayout() = 0;

   virtual void setup();
   virtual void undoLayout();
   virtual void breakLayout();

   const QWidgetList &widgets() const {
      return m_widgets;
   }
   QWidget *parentWidget() const      {
      return m_parentWidget;
   }
   QWidget *layoutBaseWidget() const  {
      return m_layoutBase;
   }

   /* Determines whether instances of QLayoutWidget are unmanaged/hidden
    * after breaking a layout. Default is true. Can be turned off when
     * morphing */
   bool reparentLayoutWidget() const  {
      return m_reparentLayoutWidget;
   }
   void setReparentLayoutWidget(bool v) {
      m_reparentLayoutWidget = v;
   }

 protected:
   virtual void finishLayout(bool needMove, QLayout *layout = nullptr);
   virtual bool prepareLayout(bool &needMove, bool &needReparent);

   void setWidgets(const  QWidgetList &widgets) {
      m_widgets = widgets;
   }
   QLayout *createLayout(int type);
   void reparentToLayoutBase(QWidget *w);

 private:
   CS_SLOT_1(Private, void widgetDestroyed())
   CS_SLOT_2(widgetDestroyed)

 private:
   QWidgetList m_widgets;
   QWidget *m_parentWidget;
   typedef QHash<QWidget *, QRect> WidgetGeometryHash;
   WidgetGeometryHash m_geometries;
   QWidget *m_layoutBase;
   QDesignerFormWindowInterface *m_formWindow;
   const LayoutInfo::Type m_layoutType;
   QPoint m_startPoint;
   QRect m_oldGeometry;

   bool m_reparentLayoutWidget;
   const bool m_isBreak;
};

namespace Utils {

inline int indexOfWidget(QLayout *layout, QWidget *widget)
{
   int index = 0;
   while (QLayoutItem *item = layout->itemAt(index)) {
      if (item->widget() == widget) {
         return index;
      }

      ++index;
   }

   return -1;
}

} // namespace Utils

} // namespace qdesigner_internal

#endif
