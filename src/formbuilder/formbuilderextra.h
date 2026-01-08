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

#ifndef ABSTRACTFORMBUILDERPRIVATE_H
#define ABSTRACTFORMBUILDERPRIVATE_H

#include <QDir>
#include <QHash>
#include <QMap>
#include <QPointer>
#include <QStringList>

class DomButtonGroup;
class DomButtonGroups;
class DomCustomWidget;
class QAbstractFormBuilder;
class QDesignerCustomWidgetInterface;
class QResourceBuilder;
class QTextBuilder;

class QAction;
class QActionGroup;
class QBoxLayout;
class QButtonGroup;
class QGridLayout;
class QLabel;
class QObject;
class QObject;
class QVariant;
class QWidget;

class QFormBuilderExtra
{
 public:
   QFormBuilderExtra();
   ~QFormBuilderExtra();

   struct CustomWidgetData {
      CustomWidgetData();
      explicit CustomWidgetData(const DomCustomWidget *dc);

      QString addPageMethod;
      QString script;
      QString baseClass;
      bool isContainer;
   };

   void clear();

   bool applyPropertyInternally(QObject *o, const QString &propertyName, const QVariant &value);

   enum BuddyMode { BuddyApplyAll, BuddyApplyVisibleOnly };

   void applyInternalProperties() const;
   static bool applyBuddy(const QString &buddyName, BuddyMode applyMode, QLabel *label);

   const QPointer<QWidget> &parentWidget() const;
   bool parentWidgetIsSet() const;
   void setParentWidget(const QPointer<QWidget> &w);

   void setProcessingLayoutWidget(bool processing);
   bool processingLayoutWidget() const;

   void setResourceBuilder(QResourceBuilder *builder);
   QResourceBuilder *resourceBuilder() const;

   void setTextBuilder(QTextBuilder *builder);
   QTextBuilder *textBuilder() const;

   void storeCustomWidgetData(const QString &className, const DomCustomWidget *d);
   QString customWidgetAddPageMethod(const QString &className) const;
   QString customWidgetBaseClass(const QString &className) const;
   bool isCustomWidgetContainer(const QString &className) const;

   // --- Hash used in creating button groups on demand. Store a map of name and pair of dom group and real group
   void registerButtonGroups(const DomButtonGroups *groups);

   typedef QPair<DomButtonGroup *, QButtonGroup *> ButtonGroupEntry;
   typedef QHash<QString, ButtonGroupEntry> ButtonGroupHash;
   const ButtonGroupHash &buttonGroups() const {
      return m_buttonGroups;
   }
   ButtonGroupHash &buttonGroups()  {
      return m_buttonGroups;
   }

   // return stretch as a comma-separated list
   static QString boxLayoutStretch(const QBoxLayout *);
   // apply stretch
   static bool setBoxLayoutStretch(const QString &, QBoxLayout *);
   static void clearBoxLayoutStretch(QBoxLayout *);

   static QString gridLayoutRowStretch(const QGridLayout *);
   static bool setGridLayoutRowStretch(const QString &, QGridLayout *);
   static void clearGridLayoutRowStretch(QGridLayout *);

   static QString gridLayoutColumnStretch(const QGridLayout *);
   static bool setGridLayoutColumnStretch(const QString &, QGridLayout *);
   static void clearGridLayoutColumnStretch(QGridLayout *);

   // return the row/column sizes as  comma-separated lists
   static QString gridLayoutRowMinimumHeight(const QGridLayout *);
   static bool setGridLayoutRowMinimumHeight(const QString &, QGridLayout *);
   static void clearGridLayoutRowMinimumHeight(QGridLayout *);

   static QString gridLayoutColumnMinimumWidth(const QGridLayout *);
   static bool setGridLayoutColumnMinimumWidth(const QString &, QGridLayout *);
   static void clearGridLayoutColumnMinimumWidth(QGridLayout *);

   QStringList m_pluginPaths;
   QMap<QString, QDesignerCustomWidgetInterface *> m_customWidgets;

   QHash<QObject *, bool> m_laidout;
   QHash<QString, QAction *> m_actions;
   QHash<QString, QActionGroup *> m_actionGroups;
   int m_defaultMargin;
   int m_defaultSpacing;
   QDir m_workingDirectory;
   QString m_errorString;
   QString m_language;

 private:
   void clearResourceBuilder();
   void clearTextBuilder();

   typedef QHash<QLabel *, QString> BuddyHash;
   BuddyHash m_buddies;

   QHash<QString, CustomWidgetData> m_customWidgetDataHash;

   ButtonGroupHash m_buttonGroups;

   bool m_layoutWidget;
   QResourceBuilder *m_resourceBuilder;
   QTextBuilder *m_textBuilder;

   QPointer<QWidget> m_parentWidget;
   bool m_parentWidgetIsSet;
};

// Struct with static accessor that provides most strings used in the form builder.
struct QFormBuilderStrings {
   QFormBuilderStrings();

   static const QFormBuilderStrings &instance();

   const QString buddyProperty;
   const QString cursorProperty;
   const QString objectNameProperty;
   const QString trueValue;
   const QString falseValue;
   const QString horizontalPostFix;
   const QString separator;
   const QString defaultTitle;
   const QString titleAttribute;
   const QString labelAttribute;
   const QString toolTipAttribute;
   const QString whatsThisAttribute;
   const QString flagsAttribute;
   const QString iconAttribute;
   const QString pixmapAttribute;
   const QString textAttribute;
   const QString currentIndexProperty;
   const QString toolBarAreaAttribute;
   const QString toolBarBreakAttribute;
   const QString dockWidgetAreaAttribute;
   const QString marginProperty;
   const QString spacingProperty;
   const QString leftMarginProperty;
   const QString topMarginProperty;
   const QString rightMarginProperty;
   const QString bottomMarginProperty;
   const QString horizontalSpacingProperty;
   const QString verticalSpacingProperty;
   const QString sizeHintProperty;
   const QString sizeTypeProperty;
   const QString orientationProperty;
   const QString styleSheetProperty;
   const QString qtHorizontal;
   const QString qtVertical;
   const QString currentRowProperty;
   const QString tabSpacingProperty;
   const QString qWidgetClass;
   const QString lineClass;
   const QString geometryProperty;
   const QString scriptWidgetVariable;
   const QString scriptChildWidgetsVariable;

   typedef QPair<Qt::ItemDataRole, QString> RoleNName;
   QList<RoleNName> itemRoles;
   QHash<QString, Qt::ItemDataRole> treeItemRoleHash;

   // first.first is primary role, first.second is shadow role.
   // Shadow is used for either the translation source or the designer
   // representation of the string value.
   typedef QPair<QPair<Qt::ItemDataRole, Qt::ItemDataRole>, QString> TextRoleNName;
   QList<TextRoleNName> itemTextRoles;
   QHash<QString, QPair<Qt::ItemDataRole, Qt::ItemDataRole>> treeItemTextRoleHash;
};

#endif
