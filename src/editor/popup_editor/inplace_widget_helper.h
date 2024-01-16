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

#ifndef INPLACE_WIDGETHELPER_H
#define INPLACE_WIDGETHELPER_H

#include <QObject>
#include <QPoint>
#include <QSize>
#include <QPointer>
#include <qglobal.h>

class QDesignerFormWindowInterface;

namespace qdesigner_internal {

// A helper class to make an editor widget suitable for form inline
// editing. Derive from the editor widget class and  make InPlaceWidgetHelper  a member.
//
// Sets "destructive close" on the editor widget and
// wires "ESC" to it.
// Installs an event filter on the parent to listen for
// resize events and passes them on to the child.
// You might want to connect editingFinished() to close() of the editor widget.
class InPlaceWidgetHelper: public QObject
{
   CS_OBJECT(InPlaceWidgetHelper)

 public:
   InPlaceWidgetHelper(QWidget *editorWidget, QWidget *parentWidget, QDesignerFormWindowInterface *fw);
   virtual ~InPlaceWidgetHelper();

   bool eventFilter(QObject *object, QEvent *event) override;

   // returns a recommended alignment for the editor widget determined from the parent.
   Qt::Alignment alignment() const;

 private:
   QWidget *m_editorWidget;
   QPointer<QWidget> m_parentWidget;
   const bool m_noChildEvent;
   QPoint m_posOffset;
   QSize m_sizeOffset;
};

}  // namespace qdesigner_internal

#endif
