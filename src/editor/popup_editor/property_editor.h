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

#ifndef PROPERTY_EDITOR_H
#define PROPERTY_EDITOR_H

#include <designer_property_editor.h>

class DomProperty;
class QDesignerMetaDataBaseItemInterface;
class QDesignerPropertySheetExtension;
class QtAbstractPropertyBrowser;
class QtButtonPropertyBrowser;
class QtTreePropertyBrowser;
class QtProperty;
class QtVariantProperty;
class QtBrowserItem;

#include <QPointer>
#include <QMap>
#include <QVector>
#include <QSet>

class QLineEdit;
class QStackedWidget;

namespace qdesigner_internal {

class StringProperty;
class DesignerPropertyManager;
class DesignerEditorFactory;
class ElidingLabel;

class PropertyEditor: public QDesignerPropertyEditor
{
   CS_OBJECT(PropertyEditor)

 public:
   explicit PropertyEditor(QDesignerFormEditorInterface *core, QWidget *parent = nullptr, Qt::WindowFlags flags = Qt::EmptyFlag);
   virtual ~PropertyEditor();

   QDesignerFormEditorInterface *core() const override;

   bool isReadOnly() const override;
   void setReadOnly(bool readOnly) override;
   void setPropertyValue(const QString &name, const QVariant &value, bool changed = true) override;
   void updatePropertySheet() override;

   void setObject(QObject *object) override;

   void reloadResourceProperties() override;

   QObject *object() const override {
      return m_object;
   }

   QString currentPropertyName() const override;

 protected:
   bool event(QEvent *event) override;

 private:
   CS_SLOT_1(Private, void slotResetProperty(QtProperty *property))
   CS_SLOT_2(slotResetProperty)

   CS_SLOT_1(Private, void slotValueChanged(QtProperty *property, const QVariant &value, bool enableSubPropertyHandling))
   CS_SLOT_2(slotValueChanged)

   CS_SLOT_1(Private, void slotViewTriggered(QAction *action))
   CS_SLOT_2(slotViewTriggered)

   CS_SLOT_1(Private, void slotAddDynamicProperty(QAction *action))
   CS_SLOT_2(slotAddDynamicProperty)

   CS_SLOT_1(Private, void slotRemoveDynamicProperty())
   CS_SLOT_2(slotRemoveDynamicProperty)

   CS_SLOT_1(Private, void slotSorting(bool sort))
   CS_SLOT_2(slotSorting)

   CS_SLOT_1(Private, void slotColoring(bool color))
   CS_SLOT_2(slotColoring)

   CS_SLOT_1(Private, void slotCurrentItemChanged(QtBrowserItem *un_named_arg1))
   CS_SLOT_2(slotCurrentItemChanged)

   CS_SLOT_1(Private, void setFilter(const QString &pattern))
   CS_SLOT_2(setFilter)

   void updateBrowserValue(QtVariantProperty *property, const QVariant &value);
   void updateToolBarLabel();
   uint toBrowserType(const QVariant &value, const QString &propertyName) const;
   QString removeScope(const QString &value) const;

   QDesignerMetaDataBaseItemInterface *metaDataBaseItem() const;

   void setupStringProperty(QtVariantProperty *property, bool isMainContainer);
   void setupPaletteProperty(QtVariantProperty *property);

   QString realClassName(QObject *object) const;
   void storeExpansionState();
   void applyExpansionState();
   void storePropertiesExpansionState(const QList<QtBrowserItem *> &items);
   void applyPropertiesExpansionState(const QList<QtBrowserItem *> &items);
   void applyFilter();
   int applyPropertiesFilter(const QList<QtBrowserItem *> &items);

   void setExpanded(QtBrowserItem *item, bool expanded);
   bool isExpanded(QtBrowserItem *item) const;
   void setItemVisible(QtBrowserItem *item, bool visible);
   bool isItemVisible(QtBrowserItem *item) const;
   void collapseAll();
   void clearView();
   void fillView();
   bool isLayoutGroup(QtProperty *group) const;
   void updateColors();
   void updateForegroundBrightness();
   QColor propertyColor(QtProperty *property) const;
   void updateActionsState();
   QtBrowserItem *nonFakePropertyBrowserItem(QtBrowserItem *item) const;
   void saveSettings() const;
   void editProperty(const QString &name);
   bool isDynamicProperty(const QtBrowserItem *item) const;

   struct Strings {
      Strings();
      QSet<QString> m_alignmentProperties;
      const QString m_fontProperty;
      const QString m_qLayoutWidget;
      const QString m_designerPrefix;
      const QString m_layout;
      const QString m_validationModeAttribute;
      const QString m_fontAttribute;
      const QString m_superPaletteAttribute;
      const QString m_enumNamesAttribute;
      const QString m_resettableAttribute;
      const QString m_flagsAttribute;
   };

   const Strings m_strings;
   QDesignerFormEditorInterface *m_core;
   QDesignerPropertySheetExtension *m_propertySheet;
   QtAbstractPropertyBrowser *m_currentBrowser;
   QtButtonPropertyBrowser *m_buttonBrowser;
   QtTreePropertyBrowser *m_treeBrowser;
   DesignerPropertyManager *m_propertyManager;
   DesignerEditorFactory *m_treeFactory;
   DesignerEditorFactory *m_groupFactory;

   QPointer<QObject> m_object;
   QMap<QString, QtVariantProperty *> m_nameToProperty;
   QMap<QtProperty *, QString> m_propertyToGroup;
   QMap<QString, QtVariantProperty *> m_nameToGroup;
   QList<QtProperty *> m_groups;

   QtProperty *m_dynamicGroup;
   QString m_recentlyAddedDynamicProperty;
   bool m_updatingBrowser;

   QStackedWidget *m_stackedWidget;
   QLineEdit *m_filterWidget;
   int m_buttonIndex;
   int m_treeIndex;
   QAction *m_addDynamicAction;
   QAction *m_removeDynamicAction;
   QAction *m_sortingAction;
   QAction *m_coloringAction;
   QAction *m_treeAction;
   QAction *m_buttonAction;
   ElidingLabel *m_classLabel;

   bool m_sorting;
   bool m_coloring;

   QMap<QString, bool> m_expansionState;

   QString m_filterPattern;
   QVector<QPair<QColor, QColor>> m_colors;
   QPair<QColor, QColor> m_dynamicColor;
   QPair<QColor, QColor> m_layoutColor;

   bool m_brightness;
};

}  // namespace qdesigner_internal

#endif
