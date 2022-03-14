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

#ifndef FORMWINDOW_WIDGETSTACK_H
#define FORMWINDOW_WIDGETSTACK_H

#include <abstract_formwindowtool.h>

#include <QWidget>

class QStackedLayout;

namespace qdesigner_internal {

class FormWindowWidgetStack: public QObject
{
   CS_OBJECT(FormWindowWidgetStack)

 public:
   FormWindowWidgetStack(QObject *parent = nullptr);
   virtual ~FormWindowWidgetStack();

   QLayout *layout() const;

   int count() const;
   QDesignerFormWindowToolInterface *tool(int index) const;
   QDesignerFormWindowToolInterface *currentTool() const;
   int currentIndex() const;
   int indexOf(QDesignerFormWindowToolInterface *tool) const;

   void setMainContainer(QWidget *w = 0);

   // Return the widget containing the form which can be used to apply embedded design settings to.
   // These settings should not affect the other editing tools.
   QWidget *formContainer() const {
      return m_formContainer;
   }

   CS_SIGNAL_1(Public, void currentToolChanged(int index))
   CS_SIGNAL_2(currentToolChanged, index)

   CS_SLOT_1(Public, void addTool(QDesignerFormWindowToolInterface *tool))
   CS_SLOT_2(addTool)

   CS_SLOT_1(Public, void setCurrentTool(QDesignerFormWindowToolInterface *tool))
   CS_SLOT_OVERLOAD(setCurrentTool, (QDesignerFormWindowToolInterface *))

   CS_SLOT_1(Public, void setCurrentTool(int index))
   CS_SLOT_OVERLOAD(setCurrentTool, (int))

   CS_SLOT_1(Public, void setSenderAsCurrentTool())
   CS_SLOT_2(setSenderAsCurrentTool)

 protected:
   QWidget *defaultEditor() const;

 private:
   QList<QDesignerFormWindowToolInterface *> m_tools;
   QWidget *m_formContainer;
   QStackedLayout *m_formContainerLayout;
   QStackedLayout *m_layout;
};

}  // namespace qdesigner_internal

#endif
