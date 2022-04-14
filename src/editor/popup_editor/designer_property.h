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

#ifndef DESIGNER_PROPERTY_H
#define DESIGNER_PROPERTY_H

#include <brush_property.h>
#include <font_property.h>
#include <edit_variant_property.h>

#include <designer_utils.h>
#include <shared_enums_p.h>

#include <QUrl>
#include <QMap>
#include <QFont>
#include <QIcon>

typedef QPair<QString, uint> DesignerIntPair;
typedef QList<DesignerIntPair> DesignerFlagList;

class QDesignerFormEditorInterface;
class QLineEdit;
class QUrl;
class QKeySequenceEdit;

namespace qdesigner_internal {

class ResetWidget;

class TextEditor;
class PaletteEditorButton;
class PixmapEditor;
class StringListEditorButton;
class FormWindowBase;

class ResetDecorator : public QObject
{
   CS_OBJECT(ResetDecorator)
 public:
   explicit ResetDecorator(const QDesignerFormEditorInterface *core, QObject *parent = nullptr);
   ~ResetDecorator();

   void connectPropertyManager(QtAbstractPropertyManager *manager);
   QWidget *editor(QWidget *subEditor, bool resettable, QtAbstractPropertyManager *manager, QtProperty *property,
      QWidget *parent);
   void disconnectPropertyManager(QtAbstractPropertyManager *manager);
   void setSpacing(int spacing);
 public:
   CS_SIGNAL_1(Public, void resetProperty(QtProperty *property))
   CS_SIGNAL_2(resetProperty, property)
 private:
   CS_SLOT_1(Private, void slotPropertyChanged(QtProperty *property))
   CS_SLOT_2(slotPropertyChanged)
   CS_SLOT_1(Private, void slotEditorDestroyed(QObject *object))
   CS_SLOT_2(slotEditorDestroyed)
 private:
   QMap<QtProperty *, QList<ResetWidget *>> m_createdResetWidgets;
   QMap<ResetWidget *, QtProperty *> m_resetWidgetToProperty;
   int m_spacing;
   const QDesignerFormEditorInterface *m_core;
};

// Helper for handling sub-properties of properties inheriting PropertySheetTranslatableData
// (translatable, disambiguation, comment).
template <class PropertySheetValue>
class TranslatablePropertyManager
{
 public:
   void initialize(QtVariantPropertyManager *m, QtProperty *property, const PropertySheetValue &value);
   bool uninitialize(QtProperty *property);
   bool destroy(QtProperty *subProperty);

   bool value(const QtProperty *property, QVariant *rc) const;
   int valueChanged(QtVariantPropertyManager *m, QtProperty *property,
      const QVariant &value);

   int setValue(QtVariantPropertyManager *m, QtProperty *property,
      uint expectedTypeId, const QVariant &value);

 private:
   QMap<QtProperty *, PropertySheetValue> m_values;
   QMap<QtProperty *, QtProperty *> m_valueToComment;
   QMap<QtProperty *, QtProperty *> m_valueToTranslatable;
   QMap<QtProperty *, QtProperty *> m_valueToDisambiguation;

   QMap<QtProperty *, QtProperty *> m_commentToValue;
   QMap<QtProperty *, QtProperty *> m_translatableToValue;
   QMap<QtProperty *, QtProperty *> m_disambiguationToValue;
};

class DesignerPropertyManager : public QtVariantPropertyManager
{
   CS_OBJECT(DesignerPropertyManager)
 public:
   enum ValueChangedResult { NoMatch, Unchanged, Changed };

   explicit DesignerPropertyManager(QDesignerFormEditorInterface *core, QObject *parent = nullptr);
   ~DesignerPropertyManager();

   QStringList attributes(uint propertyType) const override;
   uint attributeType(uint propertyType, const QString &attribute) const override;

   QVariant attributeValue(const QtProperty *property, const QString &attribute) const override;
   bool isPropertyTypeSupported(uint propertyType) const override;
   QVariant value(const QtProperty *property) const override;
   uint valueType(uint propertyType) const override;
   QString valueText(const QtProperty *property) const override;
   QIcon valueIcon(const QtProperty *property) const override;

   bool resetFontSubProperty(QtProperty *property);
   bool resetIconSubProperty(QtProperty *subProperty);

   void reloadResourceProperties();

   static uint designerFlagTypeId();
   static uint designerFlagListTypeId();
   static uint designerAlignmentTypeId();
   static uint designerPixmapTypeId();
   static uint designerIconTypeId();
   static uint designerStringTypeId();
   static uint designerStringListTypeId();
   static uint designerKeySequenceTypeId();

   void setObject(QObject *object) {
      m_object = object;
   }

 public :
   CS_SLOT_1(Public, void setAttribute(QtProperty *property, const QString &attribute, const QVariant &value)override)
   CS_SLOT_2(setAttribute)
   CS_SLOT_1(Public, void setValue(QtProperty *property, const QVariant &value)override)
   CS_SLOT_2(setValue)
 public:
   // sourceOfChange - a subproperty (or just property) which caused a change
   //void valueChanged(QtProperty *property, const QVariant &value, QtProperty *sourceOfChange);
   CS_SIGNAL_1(Public, void valueChanged(QtProperty *property, const QVariant &value, bool enableSubPropertyHandling))
   CS_SIGNAL_2(valueChanged, property, value, enableSubPropertyHandling)
 protected:
   void initializeProperty(QtProperty *property) override;
   void uninitializeProperty(QtProperty *property) override;
 private:
   CS_SLOT_1(Private, void slotValueChanged(QtProperty *property, const QVariant &value))
   CS_SLOT_2(slotValueChanged)
   CS_SLOT_1(Private, void slotPropertyDestroyed(QtProperty *property))
   CS_SLOT_2(slotPropertyDestroyed)
 private:
   void createIconSubProperty(QtProperty *iconProperty, QIcon::Mode mode, QIcon::State state, const QString &subName);

   typedef QMap<QtProperty *, bool> PropertyBoolMap;
   PropertyBoolMap m_resetMap;

   int bitCount(int mask) const;
   struct FlagData {
      FlagData() : val(0) {}
      uint val;
      DesignerFlagList flags;
      QList<uint> values;
   };
   typedef QMap<QtProperty *, FlagData> PropertyFlagDataMap;
   PropertyFlagDataMap m_flagValues;
   typedef  QMap<QtProperty *, QList<QtProperty *>> PropertyToPropertyListMap;
   PropertyToPropertyListMap m_propertyToFlags;
   QMap<QtProperty *, QtProperty *> m_flagToProperty;

   int alignToIndexH(uint align) const;
   int alignToIndexV(uint align) const;
   uint indexHToAlign(int idx) const;
   uint indexVToAlign(int idx) const;
   QString indexHToString(int idx) const;
   QString indexVToString(int idx) const;
   QMap<QtProperty *, uint> m_alignValues;
   typedef QMap<QtProperty *, QtProperty *> PropertyToPropertyMap;
   PropertyToPropertyMap m_propertyToAlignH;
   PropertyToPropertyMap m_propertyToAlignV;
   PropertyToPropertyMap m_alignHToProperty;
   PropertyToPropertyMap m_alignVToProperty;

   QMap<QtProperty *, QMap<QPair<QIcon::Mode, QIcon::State>, QtProperty *>> m_propertyToIconSubProperties;
   QMap<QtProperty *, QPair<QIcon::Mode, QIcon::State>> m_iconSubPropertyToState;
   PropertyToPropertyMap m_iconSubPropertyToProperty;
   PropertyToPropertyMap m_propertyToTheme;

   TranslatablePropertyManager<PropertySheetStringValue> m_stringManager;
   TranslatablePropertyManager<PropertySheetKeySequenceValue> m_keySequenceManager;
   TranslatablePropertyManager<PropertySheetStringListValue> m_stringListManager;

   struct PaletteData {
      QPalette val;
      QPalette superPalette;
   };
   typedef QMap<QtProperty *, PaletteData>  PropertyPaletteDataMap;
   PropertyPaletteDataMap m_paletteValues;

   QMap<QtProperty *, qdesigner_internal::PropertySheetPixmapValue> m_pixmapValues;
   QMap<QtProperty *, qdesigner_internal::PropertySheetIconValue> m_iconValues;

   QMap<QtProperty *, uint> m_uintValues;
   QMap<QtProperty *, qint64> m_longLongValues;
   QMap<QtProperty *, quint64> m_uLongLongValues;
   QMap<QtProperty *, QUrl> m_urlValues;
   QMap<QtProperty *, QByteArray> m_byteArrayValues;

   typedef QMap<QtProperty *, int>  PropertyIntMap;
   PropertyIntMap m_stringAttributes;

   typedef QMap<QtProperty *, QFont>  PropertyFontMap;
   PropertyFontMap m_stringFontAttributes;
   PropertyBoolMap m_stringThemeAttributes;

   BrushPropertyManager m_brushManager;
   FontPropertyManager m_fontManager;

   QMap<QtProperty *, QPixmap> m_defaultPixmaps;
   QMap<QtProperty *, QIcon> m_defaultIcons;

   bool m_changingSubValue;
   QDesignerFormEditorInterface *m_core;

   QObject *m_object;

   QtProperty *m_sourceOfChange;
};

class DesignerEditorFactory : public QtVariantEditorFactory
{
   CS_OBJECT(DesignerEditorFactory)

 public:
   explicit DesignerEditorFactory(QDesignerFormEditorInterface *core, QObject *parent = nullptr);
   ~DesignerEditorFactory();
   void setSpacing(int spacing);
   void setFormWindowBase(FormWindowBase *fwb);

   CS_SIGNAL_1(Public, void resetProperty(QtProperty *property))
   CS_SIGNAL_2(resetProperty, property)

 protected:
   void connectPropertyManager(QtVariantPropertyManager *manager) override;
   QWidget *createEditor(QtVariantPropertyManager *manager, QtProperty *property, QWidget *parent) override;
   void disconnectPropertyManager(QtVariantPropertyManager *manager) override;

 private:
   CS_SLOT_1(Private, void slotEditorDestroyed(QObject *object))
   CS_SLOT_2(slotEditorDestroyed)
   CS_SLOT_1(Private, void slotAttributeChanged(QtProperty *property, const QString &attribute, const QVariant &value))
   CS_SLOT_2(slotAttributeChanged)
   CS_SLOT_1(Private, void slotPropertyChanged(QtProperty *property))
   CS_SLOT_2(slotPropertyChanged)
   CS_SLOT_1(Private, void slotValueChanged(QtProperty *property, const QVariant &value))
   CS_SLOT_2(slotValueChanged)
   CS_SLOT_1(Private, void slotStringTextChanged(const QString &value))
   CS_SLOT_2(slotStringTextChanged)
   CS_SLOT_1(Private, void slotKeySequenceChanged(const QKeySequence &value))
   CS_SLOT_2(slotKeySequenceChanged)
   CS_SLOT_1(Private, void slotPaletteChanged(const QPalette &value))
   CS_SLOT_2(slotPaletteChanged)
   CS_SLOT_1(Private, void slotPixmapChanged(const QString &value))
   CS_SLOT_2(slotPixmapChanged)
   CS_SLOT_1(Private, void slotIconChanged(const QString &value))
   CS_SLOT_2(slotIconChanged)
   CS_SLOT_1(Private, void slotIconThemeChanged(const QString &value))
   CS_SLOT_2(slotIconThemeChanged)
   CS_SLOT_1(Private, void slotUintChanged(const QString &value))
   CS_SLOT_2(slotUintChanged)
   CS_SLOT_1(Private, void slotLongLongChanged(const QString &value))
   CS_SLOT_2(slotLongLongChanged)
   CS_SLOT_1(Private, void slotULongLongChanged(const QString &value))
   CS_SLOT_2(slotULongLongChanged)
   CS_SLOT_1(Private, void slotUrlChanged(const QString &value))
   CS_SLOT_2(slotUrlChanged)
   CS_SLOT_1(Private, void slotByteArrayChanged(const QString &value))
   CS_SLOT_2(slotByteArrayChanged)
   CS_SLOT_1(Private, void slotStringListChanged(const QStringList &value))
   CS_SLOT_2(slotStringListChanged)

   TextEditor *createTextEditor(QWidget *parent, TextPropertyValidationMode vm, const QString &value);

   ResetDecorator *m_resetDecorator;
   bool m_changingPropertyValue;
   QDesignerFormEditorInterface *m_core;
   FormWindowBase *m_fwb;

   int m_spacing;

   QMap<QtProperty *, QList<TextEditor *>>                m_stringPropertyToEditors;
   QMap<TextEditor *, QtProperty *>                        m_editorToStringProperty;
   QMap<QtProperty *, QList<QKeySequenceEdit *>>         m_keySequencePropertyToEditors;
   QMap<QKeySequenceEdit *, QtProperty *>                 m_editorToKeySequenceProperty;
   QMap<QtProperty *, QList<PaletteEditorButton *>>       m_palettePropertyToEditors;
   QMap<PaletteEditorButton *, QtProperty *>               m_editorToPaletteProperty;
   QMap<QtProperty *, QList<PixmapEditor *>>              m_pixmapPropertyToEditors;
   QMap<PixmapEditor *, QtProperty *>                      m_editorToPixmapProperty;
   QMap<QtProperty *, QList<PixmapEditor *>>              m_iconPropertyToEditors;
   QMap<PixmapEditor *, QtProperty *>                      m_editorToIconProperty;
   QMap<QtProperty *, QList<QLineEdit *>>                 m_uintPropertyToEditors;
   QMap<QLineEdit *, QtProperty *>                         m_editorToUintProperty;
   QMap<QtProperty *, QList<QLineEdit *>>                 m_longLongPropertyToEditors;
   QMap<QLineEdit *, QtProperty *>                         m_editorToLongLongProperty;
   QMap<QtProperty *, QList<QLineEdit *>>                 m_uLongLongPropertyToEditors;
   QMap<QLineEdit *, QtProperty *>                         m_editorToULongLongProperty;
   QMap<QtProperty *, QList<TextEditor *>>                m_urlPropertyToEditors;
   QMap<TextEditor *, QtProperty *>                        m_editorToUrlProperty;
   QMap<QtProperty *, QList<TextEditor *>>                m_byteArrayPropertyToEditors;
   QMap<TextEditor *, QtProperty *>                        m_editorToByteArrayProperty;
   QMap<QtProperty *, QList<StringListEditorButton *>>    m_stringListPropertyToEditors;
   QMap<StringListEditorButton *, QtProperty *>            m_editorToStringListProperty;
};

} // namespace qdesigner_internal

CS_DECLARE_METATYPE(DesignerIntPair)
CS_DECLARE_METATYPE(DesignerFlagList)

#endif

