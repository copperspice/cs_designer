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

#ifndef QDESIGNER_PROPERTYCOMMAND_H
#define QDESIGNER_PROPERTYCOMMAND_H

#include <designer_formwindow_command.h>

#include <QList>
#include <QPair>
#include <QSharedPointer>
#include <QVariant>

class QDesignerFormWindowInterface;
class QDesignerPropertySheetExtension;
class QDesignerIntegration;

namespace qdesigner_internal {

enum SpecialProperty {
   SP_None, SP_ObjectName, SP_LayoutName, SP_SpacerName, SP_WindowTitle,
   SP_MinimumSize, SP_MaximumSize, SP_Geometry, SP_Icon, SP_CurrentTabName, SP_CurrentItemName, SP_CurrentPageName,
   SP_AutoDefault, SP_Alignment, SP_Shortcut, SP_Orientation
};

// Determine special property
enum SpecialProperty getSpecialProperty(const QString &propertyName);

// A helper class for applying properties to objects.
// Can be used for Set commands (setValue(), restoreOldValue()) or
// Reset Commands     restoreDefaultValue(), restoreOldValue()).
//
class PropertyHelper
{
   Q_DISABLE_COPY(PropertyHelper)

 public:
   // A pair of Value and changed flag
   typedef QPair<QVariant, bool> Value;

   enum ObjectType {OT_Object, OT_FreeAction, OT_AssociatedAction, OT_Widget};

   PropertyHelper(QObject *object, SpecialProperty specialProperty,
      QDesignerPropertySheetExtension *sheet, int index);

   virtual ~PropertyHelper() {}

   QObject *object() const {
      return m_object;
   }

   SpecialProperty specialProperty() const {
      return m_specialProperty;
   }

   // set a new value. Can be overwritten to perform a transformation (see
   // handling of Arrow key move in FormWindow class).
   virtual Value setValue(QDesignerFormWindowInterface *fw, const QVariant &value, bool changed, unsigned subPropertyMask);

   // restore old value
   Value restoreOldValue(QDesignerFormWindowInterface *fw);
   // set default value
   Value restoreDefaultValue(QDesignerFormWindowInterface *fw);

   inline QVariant oldValue() const {
      return m_oldValue.first;
   }

   inline void setOldValue(const QVariant &oldValue) {
      m_oldValue.first = oldValue;
   }

   // required updates for this property (bit mask)
   enum UpdateMask {
      UpdatePropertyEditor  = 1,
      UpdateObjectInspector = 2
   };

   unsigned updateMask() const;

   // can be merged into one command (that is, object and name match)
   bool canMerge(const PropertyHelper &other) const;
   QDesignerIntegration *integration(QDesignerFormWindowInterface *fw) const;

   static void triggerActionChanged(QAction *a);

 private:
   // Apply the value and update. Returns corrected value
   Value applyValue(QDesignerFormWindowInterface *fw, const QVariant &oldValue, Value newValue);

   static void checkApplyWidgetValue(QDesignerFormWindowInterface *fw, QWidget *w,
      SpecialProperty specialProperty, QVariant &v);

   void updateObject(QDesignerFormWindowInterface *fw, const QVariant &oldValue, const QVariant &newValue);
   QVariant findDefaultValue(QDesignerFormWindowInterface *fw) const;
   void ensureUniqueObjectName(QDesignerFormWindowInterface *fw, QObject *object) const;
   SpecialProperty m_specialProperty;

   QPointer<QObject> m_object;
   ObjectType m_objectType;
   QPointer<QWidget> m_parentWidget;

   QDesignerPropertySheetExtension *m_propertySheet;
   int m_index;

   Value m_oldValue;
};

// Base class for commands that can be applied to several widgets

class PropertyListCommand : public QDesignerFormWindowCommand
{
 public:
   typedef QList<QObject *> ObjectList;

   explicit PropertyListCommand(QDesignerFormWindowInterface *formWindow, QUndoCommand *parent = nullptr);

   QObject *object(int index = 0) const;

   QVariant oldValue(int index = 0) const;

   void setOldValue(const QVariant &oldValue, int index = 0);

   // Calls restoreDefaultValue() and update()
   void undo() override;

 protected:
   typedef QSharedPointer<PropertyHelper> PropertyHelperPtr;
   typedef QList<PropertyHelperPtr> PropertyHelperList;

   // add an object
   bool add(QObject *object, const QString &propertyName);

   // Init from a list and make sure referenceObject is added first to obtain the right property group
   bool initList(const ObjectList &list, const QString &apropertyName, QObject *referenceObject = nullptr);

   // set a new value, return update mask
   unsigned setValue(QVariant value, bool changed, unsigned subPropertyMask);

   // restore old value,  return update mask
   unsigned  restoreOldValue();
   // set default value,  return update mask
   unsigned  restoreDefaultValue();

   // update designer
   void update(unsigned updateMask);

   // check if lists are aequivalent for command merging (same widgets and props)
   bool canMergeLists(const PropertyHelperList &other) const;

   PropertyHelperList &propertyHelperList() {
      return m_propertyHelperList;
   }
   const PropertyHelperList &propertyHelperList() const {
      return m_propertyHelperList;
   }

   const QString propertyName() const;
   SpecialProperty specialProperty() const;

   // Helper struct describing a property used for checking whether
   // properties of different widgets are equivalent
   struct PropertyDescription {
    public:
      PropertyDescription();
      PropertyDescription(const QString &propertyName, QDesignerPropertySheetExtension *propertySheet, int index);
      bool equals(const PropertyDescription &p) const;
      void debug() const;

      QString m_propertyName;
      QString m_propertyGroup;
      QVariant::Type m_propertyType;
      SpecialProperty m_specialProperty;
   };
   const PropertyDescription &propertyDescription() const {
      return  m_propertyDescription;
   }

 protected:
   virtual PropertyHelper *createPropertyHelper(QObject *o, SpecialProperty sp,
      QDesignerPropertySheetExtension *sheet, int sheetIndex) const;

 private:
   PropertyDescription m_propertyDescription;
   PropertyHelperList m_propertyHelperList;
};

class SetPropertyCommand: public PropertyListCommand
{

 public:
   typedef QList<QObject *> ObjectList;

   explicit SetPropertyCommand(QDesignerFormWindowInterface *formWindow, QUndoCommand *parent = nullptr);

   bool init(QObject *object, const QString &propertyName, const QVariant &newValue);
   bool init(const ObjectList &list, const QString &propertyName, const QVariant &newValue,
      QObject *referenceObject = nullptr, bool enableSubPropertyHandling = true);


   inline QVariant newValue() const {
      return m_newValue;
   }

   inline void setNewValue(const QVariant &newValue) {
      m_newValue = newValue;
   }

   int id() const override;
   bool mergeWith(const QUndoCommand *other) override;

   void redo() override;

 protected:
   virtual QVariant mergeValue(const QVariant &newValue);

 private:
   unsigned subPropertyMask(const QVariant &newValue, QObject *referenceObject);
   void setDescription();
   QVariant m_newValue;
   unsigned m_subPropertyMask;
};

class ResetPropertyCommand: public PropertyListCommand
{

 public:
   typedef QList<QObject *> ObjectList;

   explicit ResetPropertyCommand(QDesignerFormWindowInterface *formWindow);

   bool init(QObject *object, const QString &propertyName);
   bool init(const ObjectList &list, const QString &propertyName, QObject *referenceObject = nullptr);

   void redo() override;

 protected:
   bool mergeWith(const QUndoCommand *) override {
      return false;
   }

 private:
   void setDescription();
   QString m_propertyName;
};


class AddDynamicPropertyCommand: public QDesignerFormWindowCommand
{

 public:
   explicit AddDynamicPropertyCommand(QDesignerFormWindowInterface *formWindow);

   bool init(const QList<QObject *> &selection, QObject *current, const QString &propertyName, const QVariant &value);

   void redo() override;
   void undo() override;
 private:
   void setDescription();
   QString m_propertyName;
   QList<QObject *> m_selection;
   QVariant m_value;
};

class RemoveDynamicPropertyCommand: public QDesignerFormWindowCommand
{

 public:
   explicit RemoveDynamicPropertyCommand(QDesignerFormWindowInterface *formWindow);

   bool init(const QList<QObject *> &selection, QObject *current, const QString &propertyName);

   void redo() override;
   void undo() override;
 private:
   void setDescription();
   QString m_propertyName;
   QMap<QObject *, QPair<QVariant, bool>> m_objectToValueAndChanged;
};

} // namespace qdesigner_internal

#endif // QDESIGNER_PROPERTYCOMMAND_H
