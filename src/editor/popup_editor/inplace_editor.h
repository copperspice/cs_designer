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

#ifndef INPLACE_EDITOR_H
#define INPLACE_EDITOR_H

#include <inplace_widget_helper.h>
#include <designer_utils.h>
#include <textproperty_editor.h>

#include <shared_enums_p.h>

#include <QPointer>

class QDesignerFormWindowInterface;

namespace qdesigner_internal {

class InPlaceEditor: public TextPropertyEditor
{
   CS_OBJECT(InPlaceEditor)

 public:
   InPlaceEditor(QWidget *widget, TextPropertyValidationMode validationMode,
      QDesignerFormWindowInterface *fw, const QString &text, const QRect &r);

 private:
   InPlaceWidgetHelper m_InPlaceWidgetHelper;
};

// Base class for inline editor helpers to be embedded into a task menu.
// Inline-edits a property on a multi-selection.
// To use it for a particular widget/property, overwrite the method
// returning the edit area.

class TaskMenuInlineEditor : public QObject
{
   CS_OBJECT(TaskMenuInlineEditor)

 public:
   CS_SLOT_1(Public, void editText())
   CS_SLOT_2(editText)

 protected:
   TaskMenuInlineEditor(QWidget *w, TextPropertyValidationMode vm, const QString &property, QObject *parent);
   // Overwrite to return the area for the inline editor.
   virtual QRect editRectangle() const = 0;
   QWidget *widget() const {
      return m_widget;
   }

 private:
   CS_SLOT_1(Private, void updateText(const QString &text))
   CS_SLOT_2(updateText)

   CS_SLOT_1(Private, void updateSelection())
   CS_SLOT_2(updateSelection)

   TaskMenuInlineEditor(const TaskMenuInlineEditor &);
   TaskMenuInlineEditor &operator=(const TaskMenuInlineEditor &);

   const TextPropertyValidationMode m_vm;
   const QString m_property;

   QWidget *m_widget;
   QPointer<QDesignerFormWindowInterface> m_formWindow;
   QPointer<InPlaceEditor> m_editor;

   bool m_managed;
   qdesigner_internal::PropertySheetStringValue m_value;
};

}  // namespace qdesigner_internal

#endif
