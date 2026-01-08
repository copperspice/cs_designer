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

#ifndef WIDGETSELECTION_H
#define WIDGETSELECTION_H

#include <invisible_widget_p.h>

#include <QHash>
#include <QPointer>

class QDesignerFormEditorInterface;
class QMouseEvent;
class QPaintEvent;

namespace qdesigner_internal {

class FormWindow;
class WidgetSelection;

class WidgetHandle: public InvisibleWidget
{
   CS_OBJECT(WidgetHandle)
 public:
   enum Type {
      LeftTop,
      Top,
      RightTop,
      Right,
      RightBottom,
      Bottom,
      LeftBottom,
      Left,

      TypeCount
   };

   WidgetHandle(FormWindow *parent, Type t, WidgetSelection *s);
   void setWidget(QWidget *w);
   void setActive(bool a);
   void updateCursor();

   void setEnabled(bool) {}

   QDesignerFormEditorInterface *core() const;

 protected:
   void paintEvent(QPaintEvent *e) override;
   void mousePressEvent(QMouseEvent *e) override;
   void mouseMoveEvent(QMouseEvent *e) override;
   void mouseReleaseEvent(QMouseEvent *e) override;

 private:
   void changeGridLayoutItemSpan();
   void changeFormLayoutItemSpan();
   void trySetGeometry(QWidget *w, int x, int y, int width, int height);
   void tryResize(QWidget *w, int width, int height);

   QWidget *m_widget;
   const Type m_type;
   QPoint m_origPressPos;
   FormWindow *m_formWindow;
   WidgetSelection *m_sel;
   QRect m_geom, m_origGeom;
   bool m_active;
};

class WidgetSelection: public QObject
{
   CS_OBJECT(WidgetSelection)
 public:
   WidgetSelection(FormWindow *parent);

   void setWidget(QWidget *w);
   bool isUsed() const;

   void updateActive();
   void updateGeometry();
   void hide();
   void show();
   void update();

   QWidget *widget() const;

   QDesignerFormEditorInterface *core() const;

   bool eventFilter(QObject *object, QEvent *event) override;

   enum  WidgetState { UnlaidOut, LaidOut, ManagedGridLayout, ManagedFormLayout };
   static WidgetState widgetState(const QDesignerFormEditorInterface *core, QWidget *w);

 private:
   WidgetHandle *m_handles[WidgetHandle::TypeCount];
   QPointer<QWidget> m_widget;
   FormWindow *m_formWindow;
};

}  // namespace qdesigner_internal

#endif // WIDGETSELECTION_H
