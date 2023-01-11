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

#include <abstract_formwindow.h>
#include <abstract_formeditor.h>
#include <abstract_formwindow.h>
#include <abstract_formwindowcursor.h>
#include <abstract_language.h>
#include <inplace_editor.h>
#include <extension.h>

#include <designer_propertysheet.h>

#include <QVariant>

namespace qdesigner_internal {

InPlaceEditor::InPlaceEditor(QWidget *widget, TextPropertyValidationMode validationMode,
      QDesignerFormWindowInterface *fw, const QString &text, const QRect &r)
   : TextPropertyEditor(widget, EmbeddingInPlace, validationMode), m_InPlaceWidgetHelper(this, widget, fw)
{
   setAlignment(m_InPlaceWidgetHelper.alignment());
   setObjectName(QString("__qt__passive_m_editor"));

   setText(text);
   selectAll();

   setGeometry(QRect(widget->mapTo(widget->window(), r.topLeft()), r.size()));
   setFocus();
   show();

   connect(this, &TextPropertyEditor::editingFinished, this, &QWidget::close);
}


// -------------- TaskMenuInlineEditor

TaskMenuInlineEditor::TaskMenuInlineEditor(QWidget *w, TextPropertyValidationMode vm,
   const QString &property, QObject *parent) :
   QObject(parent),
   m_vm(vm),
   m_property(property),
   m_widget(w),
   m_managed(true)
{
}

void TaskMenuInlineEditor::editText()
{
   m_formWindow = QDesignerFormWindowInterface::findFormWindow(m_widget);
   if (m_formWindow.isNull()) {
      return;
   }
   m_managed = m_formWindow->isManaged(m_widget);

   // Close as soon as a different widget is selected
   connect(m_formWindow.data(), &QDesignerFormWindowInterface::selectionChanged,
      this, &TaskMenuInlineEditor::updateSelection);

   // get old value
   QDesignerFormEditorInterface *core = m_formWindow->core();
   const QDesignerPropertySheetExtension *sheet = qt_extension<QDesignerPropertySheetExtension *>(core->extensionManager(), m_widget);
   const int index = sheet->indexOf(m_property);

   if (index == -1) {
      return;
   }

   QVariant data = sheet->property(index);
   m_value = data.value<PropertySheetStringValue>();
   const QString oldValue = m_value.value();

   m_editor = new InPlaceEditor(m_widget, m_vm, m_formWindow, oldValue, editRectangle());
   connect(m_editor.data(), &InPlaceEditor::textChanged, this, &TaskMenuInlineEditor::updateText);
}

void TaskMenuInlineEditor::updateText(const QString &text)
{
   // In the [rare] event we are invoked on an unmanaged widget,
   // do not use the cursor selection
   m_value.setValue(text);

   if (m_managed) {
      m_formWindow->cursor()->setProperty(m_property, QVariant::fromValue(m_value));
   } else {
      m_formWindow->cursor()->setWidgetProperty(m_widget, m_property, QVariant::fromValue(m_value));
   }
}

void TaskMenuInlineEditor::updateSelection()
{
   if (m_editor) {
      m_editor->deleteLater();
   }
}

}
