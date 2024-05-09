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

#ifndef FORMWINDOWCURSOR_H
#define FORMWINDOWCURSOR_H

#include <abstract_formwindowcursor.h>
#include <formwindow.h>

#include <QObject>

namespace qdesigner_internal {

class FormWindowCursor: public QObject, public QDesignerFormWindowCursorInterface
{
   CS_OBJECT(FormWindowCursor)

 public:
   explicit FormWindowCursor(FormWindow *fw, QObject *parent = nullptr);
   virtual ~FormWindowCursor();

   QDesignerFormWindowInterface *formWindow() const override;

   bool movePosition(MoveOperation op, MoveMode mode) override;

   int position() const override;
   void setPosition(int pos, MoveMode mode) override;

   QWidget *current() const override;

   int widgetCount() const override;
   QWidget *widget(int index) const override;

   bool hasSelection() const override;
   int selectedWidgetCount() const override;
   QWidget *selectedWidget(int index) const override;

   void setProperty(const QString &name, const QVariant &value) override;
   void setWidgetProperty(QWidget *widget, const QString &name, const QVariant &value) override;
   void resetWidgetProperty(QWidget *widget, const QString &name) override;

   CS_SLOT_1(Public, void update())
   CS_SLOT_2(update)

 private:
   FormWindow *m_formWindow;
};

}  // namespace qdesigner_internal


#endif // FORMWINDOWCURSOR_H
