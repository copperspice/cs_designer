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

#ifndef DESIGNERPROPERTYEDITOR_H
#define DESIGNERPROPERTYEDITOR_H

#include <abstract_propertyeditor.h>

#include <shared_enums_p.h>

namespace qdesigner_internal {

// Extends the QDesignerPropertyEditorInterface by property comment handling and
// a signal for resetproperty.

class QDesignerPropertyEditor: public QDesignerPropertyEditorInterface
{
   CS_OBJECT(QDesignerPropertyEditor)

 public:
   explicit QDesignerPropertyEditor(QWidget *parent = nullptr, Qt::WindowFlags flags = Qt::EmptyFlag);

   // A pair <ValidationMode, bool isTranslatable>.
   using TextValidPair = QPair<TextPropertyValidationMode, bool>;

   // Return a pair of validation mode and flag indicating whether property is translatable
   // for textual properties.
   static TextValidPair textPropertyValidationMode(QDesignerFormEditorInterface *core,
      const QObject *object, const QString &propertyName, bool isMainContainer);

   CS_SIGNAL_1(Public, void propertyValueChanged(const QString &name, const QVariant &value, bool enableSubPropertyHandling))
   CS_SIGNAL_2(propertyValueChanged, name, value, enableSubPropertyHandling)

   CS_SIGNAL_1(Public, void resetProperty(const QString &name))
   CS_SIGNAL_2(resetProperty, name)

   CS_SIGNAL_1(Public, void addDynamicProperty(const QString &name, const QVariant &value))
   CS_SIGNAL_2(addDynamicProperty, name, value)

   CS_SIGNAL_1(Public, void removeDynamicProperty(const QString &name))
   CS_SIGNAL_2(removeDynamicProperty, name)

   CS_SIGNAL_1(Public, void editorOpened())
   CS_SIGNAL_2(editorOpened)

   CS_SIGNAL_1(Public, void editorClosed())
   CS_SIGNAL_2(editorClosed)

   virtual void updatePropertySheet() = 0;
   virtual void reloadResourceProperties() = 0;

 protected:
   void emitPropertyValueChanged(const QString &name, const QVariant &value, bool enableSubPropertyHandling);

 private:
   CS_SLOT_1(Private, void slotPropertyChanged(const QString &name, const QVariant &value))
   CS_SLOT_2(slotPropertyChanged)

   bool m_propertyChangedForwardingBlocked;
};

}  // namespace qdesigner_internal

#endif
