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

#include <abstract_formwindow.h>
#include <abstract_formwindowcursor.h>
#include <abstract_language.h>
#include <abstract_formeditor.h>
#include <abstract_introspection.h>
#include <designer_promotion_dialog.h>
#include <designer_object_inspector.h>
#include <designer_command.h>
#include <extension.h>
#include <extension_manager.h>
#include <promotion_taskmenu.h>
#include <signalslot_dialog.h>
#include <widgetfactory.h>

#include <metadatabase_p.h>
#include <widgetdatabase_p.h>

#include <QAction>
#include <QWidget>
#include <QMenu>
#include <QSignalMapper>
#include <QDebug>
#include <qalgorithms.h>

static QAction *separatorAction(QObject *parent)
{
   QAction *rc = new  QAction(parent);
   rc->setSeparator(true);
   return rc;
}

static inline QDesignerLanguageExtension *languageExtension(QDesignerFormEditorInterface *core)
{
   return qt_extension<QDesignerLanguageExtension *>(core->extensionManager(), core);
}

namespace qdesigner_internal {

PromotionTaskMenu::PromotionTaskMenu(QWidget *widget, Mode mode, QObject *parent) :
   QObject(parent),
   m_mode(mode),
   m_widget(widget),
   m_promotionMapper(0),
   m_globalEditAction(new QAction(tr("Promoted widgets..."), this)),
   m_EditPromoteToAction(new QAction(tr("Promote to ..."), this)),
   m_EditSignalsSlotsAction(new QAction(tr("Change signals/slots..."), this)),
   m_promoteLabel(tr("Promote to")),
   m_demoteLabel(tr("Demote to %1"))
{
   connect(m_globalEditAction, &QAction::triggered, this, &PromotionTaskMenu::slotEditPromotedWidgets);
   connect(m_EditPromoteToAction, &QAction::triggered, this, &PromotionTaskMenu::slotEditPromoteTo);
   connect(m_EditSignalsSlotsAction, &QAction::triggered, this, &PromotionTaskMenu::slotEditSignalsSlots);
}

PromotionTaskMenu::Mode PromotionTaskMenu::mode() const
{
   return m_mode;
}

void PromotionTaskMenu::setMode(Mode m)
{
   m_mode = m;
}

void PromotionTaskMenu::setWidget(QWidget *widget)
{
   m_widget = widget;
}

void PromotionTaskMenu::setPromoteLabel(const QString &promoteLabel)
{
   m_promoteLabel = promoteLabel;
}

void PromotionTaskMenu::setEditPromoteToLabel(const QString &promoteEditLabel)
{
   m_EditPromoteToAction->setText(promoteEditLabel);
}

void PromotionTaskMenu::setDemoteLabel(const QString &demoteLabel)
{
   m_demoteLabel = demoteLabel;
}

PromotionTaskMenu::PromotionState  PromotionTaskMenu::createPromotionActions(QDesignerFormWindowInterface *formWindow)
{
   typedef void (QSignalMapper::*MapperVoidSlot)();
   typedef void (QSignalMapper::*MapperStringSignal)(const QString &);

   // clear out old
   if (!m_promotionActions.empty()) {
      qDeleteAll(m_promotionActions);
      m_promotionActions.clear();
   }
   // No promotion of main container
   if (formWindow->mainContainer() == m_widget) {
      return NotApplicable;
   }

   // Check for a homogenous selection
   const PromotionSelectionList promotionSelection = promotionSelectionList(formWindow);

   if (promotionSelection.empty()) {
      return NoHomogenousSelection;
   }

   QDesignerFormEditorInterface *core = formWindow->core();

   // if it is promoted: demote only.
   if (isPromoted(formWindow->core(), m_widget)) {
      const QString label = m_demoteLabel.formatArg( promotedExtends(core, m_widget));

      QAction *demoteAction = new QAction(label, this);
      connect(demoteAction, &QAction::triggered, this, &PromotionTaskMenu::slotDemoteFromCustomWidget);
      m_promotionActions.push_back(demoteAction);
      return CanDemote;
   }
   // figure out candidates
   const QString baseClassName = WidgetFactory::classNameOf(core,  m_widget);
   const WidgetDataBaseItemList candidates = promotionCandidates(core->widgetDataBase(), baseClassName );
   if (candidates.empty()) {
      // Is this thing promotable at all?
      return QDesignerPromotionDialog::baseClassNames(core->promotion()).contains(baseClassName) ?  CanPromote : NotApplicable;
   }
   // Set up a signal mapper to associate class names
   if (!m_promotionMapper) {
      m_promotionMapper = new QSignalMapper(this);
      connect(m_promotionMapper, static_cast<MapperStringSignal>(&QSignalMapper::mapped),
         this, &PromotionTaskMenu::slotPromoteToCustomWidget);
   }

   QMenu *candidatesMenu = new QMenu();
   // Create a sub menu
   const WidgetDataBaseItemList::const_iterator cend = candidates.constEnd();
   // Set up actions and map class names
   for (WidgetDataBaseItemList::const_iterator it = candidates.constBegin(); it != cend; ++it) {
      const QString customClassName = (*it)->name();
      QAction *action = new QAction((*it)->name(), this);
      connect(action, &QAction::triggered,
         m_promotionMapper, static_cast<MapperVoidSlot>(&QSignalMapper::map));
      m_promotionMapper->setMapping(action, customClassName);
      candidatesMenu->addAction(action);
   }
   // Sub menu action
   QAction *subMenuAction = new QAction(m_promoteLabel, this);
   subMenuAction->setMenu(candidatesMenu);
   m_promotionActions.push_back(subMenuAction);
   return CanPromote;
}

void PromotionTaskMenu::addActions(unsigned separatorFlags, ActionList &actionList)
{
   addActions(formWindow(), separatorFlags, actionList);
}

void PromotionTaskMenu::addActions(QDesignerFormWindowInterface *fw, unsigned flags,
   ActionList &actionList)
{
   Q_ASSERT(m_widget);
   const int previousSize = actionList.size();
   const PromotionState promotionState = createPromotionActions(fw);

   // Promotion candidates/demote
   actionList += m_promotionActions;

   // Edit action depending on context
   switch (promotionState) {
      case  CanPromote:
         actionList += m_EditPromoteToAction;
         break;
      case CanDemote:
         if (!(flags & SuppressGlobalEdit)) {
            actionList += m_globalEditAction;
         }
         if (!languageExtension(fw->core())) {
            actionList += separatorAction(this);
            actionList += m_EditSignalsSlotsAction;
         }
         break;
      default:
         if (!(flags & SuppressGlobalEdit)) {
            actionList += m_globalEditAction;
         }
         break;
   }
   // Add separators if required
   if (actionList.size() > previousSize) {
      if (flags &  LeadingSeparator) {
         actionList.insert(previousSize, separatorAction(this));
      }
      if (flags & TrailingSeparator) {
         actionList += separatorAction(this);
      }
   }
}

void  PromotionTaskMenu::addActions(QDesignerFormWindowInterface *fw, unsigned flags, QMenu *menu)
{
   ActionList actionList;
   addActions(fw, flags, actionList);
   menu->addActions(actionList);
}

void  PromotionTaskMenu::addActions(unsigned flags, QMenu *menu)
{
   addActions(formWindow(), flags, menu);
}

void PromotionTaskMenu::promoteTo(QDesignerFormWindowInterface *fw, const QString &customClassName)
{
   Q_ASSERT(m_widget);
   PromoteToCustomWidgetCommand *cmd = new PromoteToCustomWidgetCommand(fw);
   cmd->init(promotionSelectionList(fw), customClassName);
   fw->commandHistory()->push(cmd);
}


void  PromotionTaskMenu::slotPromoteToCustomWidget(const QString &customClassName)
{
   promoteTo(formWindow(), customClassName);
}

void PromotionTaskMenu::slotDemoteFromCustomWidget()
{
   QDesignerFormWindowInterface *fw = formWindow();
   const PromotionSelectionList promotedWidgets = promotionSelectionList(fw);
   Q_ASSERT(!promotedWidgets.empty() && isPromoted(fw->core(), promotedWidgets.front()));

   // ### use the undo stack
   DemoteFromCustomWidgetCommand *cmd = new DemoteFromCustomWidgetCommand(fw);
   cmd->init(promotedWidgets);
   fw->commandHistory()->push(cmd);
}

void PromotionTaskMenu::slotEditPromoteTo()
{
   Q_ASSERT(m_widget);
   // Check whether invoked over a promotable widget
   QDesignerFormWindowInterface *fw = formWindow();
   QDesignerFormEditorInterface *core = fw->core();
   const QString base_class_name = WidgetFactory::classNameOf(core, m_widget);
   Q_ASSERT(QDesignerPromotionDialog::baseClassNames(core->promotion()).contains(base_class_name));
   // Show over promotable widget
   QString promoteToClassName;
   QDialog *promotionEditor = 0;
   if (QDesignerLanguageExtension *lang = languageExtension(core)) {
      promotionEditor = lang->createPromotionDialog(core, base_class_name, &promoteToClassName, fw);
   }
   if (!promotionEditor) {
      promotionEditor = new QDesignerPromotionDialog(core, fw, base_class_name, &promoteToClassName);
   }
   if (promotionEditor->exec() == QDialog::Accepted && !promoteToClassName.isEmpty()) {
      promoteTo(fw, promoteToClassName);
   }
   delete promotionEditor;
}

void PromotionTaskMenu::slotEditPromotedWidgets()
{
   // Global context, show over non-promotable widget
   QDesignerFormWindowInterface *fw = formWindow();
   if (!fw) {
      return;
   }
   editPromotedWidgets(fw->core(), fw);
}

PromotionTaskMenu::PromotionSelectionList PromotionTaskMenu::promotionSelectionList(QDesignerFormWindowInterface *formWindow) const
{
   // In multi selection mode, check for a homogenous selection (same class, same promotion state)
   // and return the list if this is the case. Also make sure m_widget
   // is the last widget in the list so that it is re-selected as the last
   // widget by the promotion commands.

   PromotionSelectionList rc;

   if (m_mode != ModeSingleWidget) {
      QDesignerFormEditorInterface *core = formWindow->core();
      const QDesignerIntrospectionInterface *intro = core->introspection();
      const QString className = intro->metaObject(m_widget)->className();
      const bool promoted = isPromoted(formWindow->core(), m_widget);
      // Just in case someone plugged an old-style Object Inspector
      if (QDesignerObjectInspector *designerObjectInspector = dynamic_cast<QDesignerObjectInspector *>(core->objectInspector())) {
         Selection s;
         designerObjectInspector->getSelection(s);
         // Find objects of similar state
         const QWidgetList &source = m_mode == ModeManagedMultiSelection ? s.managed : s.unmanaged;
         const QWidgetList::const_iterator cend = source.constEnd();
         for (QWidgetList::const_iterator it = source.constBegin(); it != cend; ++it) {
            QWidget *w = *it;
            if (w != m_widget) {
               // Selection state mismatch
               if (intro->metaObject(w)->className() != className || isPromoted(core, w) !=  promoted) {
                  return PromotionSelectionList();
               }
               rc.push_back(w);
            }
         }
      }
   }

   rc.push_back(m_widget);
   return rc;
}

QDesignerFormWindowInterface *PromotionTaskMenu::formWindow() const
{
   // Use the QObject overload of  QDesignerFormWindowInterface::findFormWindow since that works
   // for QDesignerMenus also.
   QObject *o = m_widget;
   QDesignerFormWindowInterface *result = QDesignerFormWindowInterface::findFormWindow(o);
   Q_ASSERT(result != 0);
   return result;
}

void PromotionTaskMenu::editPromotedWidgets(QDesignerFormEditorInterface *core, QWidget *parent)
{
   QDesignerLanguageExtension *lang = languageExtension(core);
   // Show over non-promotable widget
   QDialog *promotionEditor =  0;
   if (lang) {
      lang->createPromotionDialog(core, parent);
   }
   if (!promotionEditor) {
      promotionEditor = new QDesignerPromotionDialog(core, parent);
   }
   promotionEditor->exec();
   delete promotionEditor;
}

void PromotionTaskMenu::slotEditSignalsSlots()
{
   QDesignerFormWindowInterface *fw = formWindow();
   if (!fw) {
      return;
   }
   SignalSlotDialog::editPromotedClass(fw->core(), m_widget, fw);
}
} // namespace qdesigner_internal


