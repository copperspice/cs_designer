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

#ifndef QDESIGNER_DNDITEM_H
#define QDESIGNER_DNDITEM_H

#include <abstract_dnditem.h>

#include <QPoint>
#include <QList>
#include <QMimeData>

class QDrag;
class QImage;
class QDropEvent;

namespace qdesigner_internal {

class QDesignerDnDItem: public QDesignerDnDItemInterface
{
 public:
   explicit QDesignerDnDItem(DropType type, QWidget *source = nullptr);
   virtual ~QDesignerDnDItem();

   DomUI *domUi() const override;
   QWidget *decoration() const override;
   QWidget *widget() const override;
   QPoint hotSpot() const override;
   QWidget *source() const override;

   DropType type() const override;

 protected:
   void setDomUi(DomUI *dom_ui);
   void init(DomUI *ui, QWidget *widget, QWidget *decoration, const QPoint &global_mouse_pos);

 private:
   QWidget *m_source;
   const DropType m_type;
   const QPoint m_globalStartPos;
   DomUI *m_dom_ui;
   QWidget *m_widget;
   QWidget *m_decoration;
   QPoint m_hot_spot;

   Q_DISABLE_COPY(QDesignerDnDItem)
};

// Mime data for use with designer drag and drop operations.

class  QDesignerMimeData : public QMimeData
{
   CS_OBJECT(QDesignerMimeData)

 public:
   typedef QList<QDesignerDnDItemInterface *> QDesignerDnDItems;

   virtual ~QDesignerMimeData();

   const QDesignerDnDItems &items() const {
      return m_items;
   }

   // Execute a drag and drop operation.
   static Qt::DropAction execDrag(const QDesignerDnDItems &items, QWidget *dragSource);

   QPoint hotSpot() const {
      return m_hotSpot;
   }

   // Move the decoration. Required for drops over form windows as the position
   // is derived from the decoration position.
   void moveDecoration(const QPoint &globalPos) const;

   // For a move operation, create the undo command sequence to remove
   // the widgets from the source form.
   static void removeMovedWidgetsFromSourceForm(const QDesignerDnDItems &items);

   // Accept an event with the proper action.
   void acceptEvent(QDropEvent *e) const;

   // Helper to accept an event with the desired action.
   static void acceptEventWithAction(Qt::DropAction desiredAction, QDropEvent *e);

 private:
   QDesignerMimeData(const QDesignerDnDItems &items, QDrag *drag);
   Qt::DropAction proposedDropAction() const;

   static void setImageTransparency(QImage &image, int alpha);

   const QDesignerDnDItems m_items;
   QPoint m_globalStartPos;
   QPoint m_hotSpot;
};

} // namespace qdesigner_internal



#endif // QDESIGNER_DNDITEM_H
