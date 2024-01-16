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

#ifndef PROMOTIONTASKMENU_H
#define PROMOTIONTASKMENU_H

class QDesignerFormWindowInterface;
class QDesignerFormEditorInterface;

#include <QList>
#include <QObject>
#include <QPointer>

class QAction;
class QMenu;
class QWidget;
class QSignalMapper;

namespace qdesigner_internal {

// A helper class for creating promotion context menus and handling promotion actions.

class PromotionTaskMenu: public QObject
{
   CS_OBJECT(PromotionTaskMenu)

 public:
   enum Mode {
      ModeSingleWidget,
      ModeManagedMultiSelection,
      ModeUnmanagedMultiSelection
   };

   explicit PromotionTaskMenu(QWidget *widget, Mode mode = ModeManagedMultiSelection, QObject *parent = nullptr);

   Mode mode() const;
   void setMode(Mode m);

   void setWidget(QWidget *widget);

   // Set menu labels
   void setPromoteLabel(const QString &promoteLabel);
   void setEditPromoteToLabel(const QString &promoteEditLabel);

   // Defaults to "Demote to %1".formatArg(class).
   void setDemoteLabel(const QString &demoteLabel);

   typedef QList<QAction *> ActionList;

   enum AddFlags { LeadingSeparator = 1, TrailingSeparator = 2, SuppressGlobalEdit = 4};

   // Adds a list of promotion actions according to the current promotion state of the widget.
   void addActions(QDesignerFormWindowInterface *fw, unsigned flags, ActionList &actionList);
   // Convenience that finds the form window.
   void addActions(unsigned flags, ActionList &actionList);

   void addActions(QDesignerFormWindowInterface *fw, unsigned flags, QMenu *menu);
   void addActions(unsigned flags, QMenu *menu);

   // Pop up the editor in a global context.
   static void editPromotedWidgets(QDesignerFormEditorInterface *core, QWidget *parent);

 private:
   CS_SLOT_1(Private, void slotPromoteToCustomWidget(const QString &customClassName))
   CS_SLOT_2(slotPromoteToCustomWidget)
   CS_SLOT_1(Private, void slotDemoteFromCustomWidget())
   CS_SLOT_2(slotDemoteFromCustomWidget)
   CS_SLOT_1(Private, void slotEditPromotedWidgets())
   CS_SLOT_2(slotEditPromotedWidgets)
   CS_SLOT_1(Private, void slotEditPromoteTo())
   CS_SLOT_2(slotEditPromoteTo)
   CS_SLOT_1(Private, void slotEditSignalsSlots())
   CS_SLOT_2(slotEditSignalsSlots)

 private:
   enum PromotionState {
      NotApplicable,
      NoHomogenousSelection,
      CanPromote,
      CanDemote
   };

   void promoteTo(QDesignerFormWindowInterface *fw, const QString &customClassName);

   PromotionState createPromotionActions(QDesignerFormWindowInterface *formWindow);
   QDesignerFormWindowInterface *formWindow() const;

   typedef QList<QPointer<QWidget>> PromotionSelectionList;
   PromotionSelectionList promotionSelectionList(QDesignerFormWindowInterface *formWindow) const;

   Mode m_mode;

   QPointer<QWidget> m_widget;

   QSignalMapper *m_promotionMapper;
   // Per-Widget actions
   QList<QAction *> m_promotionActions;

   QAction *m_globalEditAction;
   QAction *m_EditPromoteToAction;
   QAction *m_EditSignalsSlotsAction;

   QString m_promoteLabel;
   QString m_demoteLabel;
};

} // namespace qdesigner_internal



#endif // PROMOTIONTASKMENU_H
