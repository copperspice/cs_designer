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

#include <toolbar_dialog.h>
#include <ui_designer_toolbar.h>

#include <QAction>
#include <QEvent>
#include <QHeaderView>
#include <QMainWindow>
#include <QPushButton>
#include <QSet>
#include <QToolBar>

class QtFullToolBarManagerPrivate;

class QtFullToolBarManager : public QObject
{
   CS_OBJECT(QtFullToolBarManager)

 public:
   QtFullToolBarManager(QObject *parent);
   ~QtFullToolBarManager();

   void setMainWindow(QMainWindow *mainWindow);
   QMainWindow *mainWindow() const;

   void addCategory(const QString &category);
   bool hasCategory(const QString &category) const;
   QStringList categories() const;
   QList<QAction *> categoryActions(const QString &category) const;
   QString actionCategory(QAction *action) const;

   // only non-separator
   void addAction(QAction *action, const QString &category);

   void removeAction(QAction *action);

   QSet<QAction *> actions() const;
   bool isWidgetAction(QAction *action) const;

   /*
   Adds (registers) toolBar. Adds (registers) actions that already exists in toolBar.
   Remembers toolbar and its actions as a default.
   */
   void addDefaultToolBar(QToolBar *toolBar, const QString &category);

   void removeDefaultToolBar(QToolBar *toolBar);
   // NULL on action list means separator.
   QMap<QToolBar *, QList<QAction *>> defaultToolBars() const;
   bool isDefaultToolBar(QToolBar *toolBar) const;

   QToolBar *createToolBar(const QString &toolBarName);
   void deleteToolBar(QToolBar *toolBar); // only those which were created, not added

   QList<QAction *> actions(QToolBar *toolBar) const;

   void setToolBars(const QMap<QToolBar *, QList<QAction *>> &actions);
   void setToolBar(QToolBar *toolBar, const QList<QAction *> &actions);

   QMap<QToolBar *, QList<QAction *>> toolBarsActions() const;
   QByteArray saveState(int version = 0) const;
   bool restoreState(const QByteArray &state, int version = 0);

   CS_SLOT_1(Public, void resetToolBar(QToolBar *toolBar))
   CS_SLOT_2(resetToolBar)

   CS_SLOT_1(Public, void resetAllToolBars())
   CS_SLOT_2(resetAllToolBars)

   CS_SIGNAL_1(Public, void toolBarCreated(QToolBar *toolBar))
   CS_SIGNAL_2(toolBarCreated, toolBar)

   CS_SIGNAL_1(Public, void toolBarRemoved(QToolBar *toolBar))
   CS_SIGNAL_2(toolBarRemoved, toolBar)

   /*
   If QToolBarWidgetAction was in another tool bar and is inserted into
   this toolBar, toolBarChanged is first emitted for other toolbar - without
   that action. (Another approach may be that user first must call setToolBar
   without that action for old tool bar)
   */

   CS_SIGNAL_1(Public, void toolBarChanged(QToolBar *toolBar, const QList <QAction *> &actions))
   CS_SIGNAL_2(toolBarChanged, toolBar, actions)

 private:
   QScopedPointer<QtFullToolBarManagerPrivate> d_ptr;
   Q_DECLARE_PRIVATE(QtFullToolBarManager)
   Q_DISABLE_COPY(QtFullToolBarManager)
};

class QtFullToolBarManagerPrivate
{
   class QtFullToolBarManager *q_ptr;
   Q_DECLARE_PUBLIC(QtFullToolBarManager)

 public:
   QToolBar *toolBarWidgetAction(QAction *action) const;
   void removeWidgetActions(const QMap<QToolBar *, QList<QAction *>> &actions);

   enum MarkerType {
      VersionMarker       = 0xff,
      ToolBarMarker       = 0xfe,
      CustomToolBarMarker = 0xfd,
   };

   void saveState(QDataStream &stream) const;
   bool restoreState(QDataStream &stream) const;
   QToolBar *findDefaultToolBar(const QString &objectName) const;
   QAction *findAction(const QString &actionName) const;

   QToolBar *toolBarByName(const QString &toolBarName) const;

   QtFullToolBarManagerPrivate();

   QMap<QString, QList<QAction *>> categoryToActions;
   QMap<QAction *, QString> actionToCategory;

   QSet<QAction *> allActions;
   QMap<QAction *, QToolBar *> widgetActions;
   QSet<QAction *> regularActions;
   QMap<QAction *, QList<QToolBar *>> actionToToolBars;

   QMap<QToolBar *, QList<QAction *>> toolBars;
   QMap<QToolBar *, QList<QAction *>> toolBarsWithSeparators;
   QMap<QToolBar *, QList<QAction *>> defaultToolBars;
   QList<QToolBar *> customToolBars;

   QMainWindow *theMainWindow;
};

QtFullToolBarManagerPrivate::QtFullToolBarManagerPrivate()
   : theMainWindow(nullptr)
{
}

QToolBar *QtFullToolBarManagerPrivate::toolBarWidgetAction(QAction *action) const
{
   if (widgetActions.contains(action)) {
      return widgetActions.value(action);
   }

   return nullptr;
}

void QtFullToolBarManagerPrivate::removeWidgetActions(const QMap<QToolBar *, QList<QAction *>>
   &actions)
{
   auto itToolBar = actions.constBegin();

   while (itToolBar != actions.constEnd()) {
      QToolBar *toolBar = itToolBar.key();
      QList<QAction *> newActions = toolBars.value(toolBar);
      QList<QAction *> newActionsWithSeparators = toolBarsWithSeparators.value(toolBar);

      QList<QAction *> removedActions;
      QList<QAction *> actionList = itToolBar.value();
      QListIterator<QAction *> itAction(actionList);

      while (itAction.hasNext()) {
         QAction *action = itAction.next();
         if (newActions.contains(action) && toolBarWidgetAction(action) == toolBar) {
            newActions.removeAll(action);
            newActionsWithSeparators.removeAll(action);
            removedActions.append(action);
         }
      }

      //emit q_ptr->toolBarChanged(toolBar, newActions);

      toolBars.insert(toolBar, newActions);
      toolBarsWithSeparators.insert(toolBar, newActionsWithSeparators);
      QListIterator<QAction *> itRemovedAction(removedActions);

      while (itRemovedAction.hasNext()) {
         QAction *oldAction = itRemovedAction.next();
         widgetActions.insert(oldAction, nullptr);
         actionToToolBars[oldAction].removeAll(toolBar);
      }

      ++itToolBar;
   }
}

void QtFullToolBarManagerPrivate::saveState(QDataStream &stream) const
{
   stream << static_cast<uchar>(QtFullToolBarManagerPrivate::MarkerType::ToolBarMarker);
   stream << defaultToolBars.size();

   auto itToolBar = defaultToolBars.constBegin();

   while (itToolBar != defaultToolBars.constEnd()) {
      QToolBar *tb = itToolBar.key();

      if (tb->objectName().isEmpty()) {
         qWarning("QtToolBarManager::saveState() No objectName was set for QToolBar "
               "%s, using value for windowTitle", csPrintable(tb->windowTitle()));

         stream << tb->windowTitle();
      } else {
         stream << tb->objectName();
      }

      stream << toolBars[tb].size();
      QListIterator<QAction *> itAction(toolBars[tb]);
      while (itAction.hasNext()) {
         QAction *action = itAction.next();

         if (action) {
            if (action->objectName().isEmpty()) {
               qWarning("QtToolBarManager::saveState() No objectName was set for QAction "
                  "%s, using value for text", csPrintable(action->text()));

               stream << action->text();
            } else {
               stream << action->objectName();
            }
         } else {
            stream << QString();
         }
      }
      ++itToolBar;
   }


   stream << static_cast<uchar>(QtFullToolBarManagerPrivate::MarkerType::CustomToolBarMarker);
   stream << toolBars.size() - defaultToolBars.size();
   itToolBar = toolBars.constBegin();

   while (itToolBar != toolBars.constEnd()) {
      QToolBar *tb = itToolBar.key();
      if (!defaultToolBars.contains(tb)) {
         stream << tb->objectName();
         stream << tb->windowTitle();

         stream << toolBars[tb].size();
         QListIterator<QAction *> itAction(toolBars[tb]);
         while (itAction.hasNext()) {
            QAction *action = itAction.next();

            if (action) {
               if (action->objectName().isEmpty()) {
                  qWarning("QtToolBarManager::saveState() No objectName was set for QAction "
                        "%s, using value for text", csPrintable(action->text()));

                  stream << action->text();
               } else {
                  stream << action->objectName();
               }
            } else {
               stream << QString();
            }
         }
      }
      ++itToolBar;
   }
}

bool QtFullToolBarManagerPrivate::restoreState(QDataStream &stream) const
{
   uchar tmarker;
   stream >> tmarker;

   if (tmarker != QtFullToolBarManagerPrivate::MarkerType::ToolBarMarker) {
      return false;
   }

   int toolBarCount;
   stream >> toolBarCount;

   for (int i = 0; i < toolBarCount; i++) {
      QString objectName;
      stream >> objectName;
      int actionCount;
      stream >> actionCount;

      QList<QAction *> actions;

      for (int j = 0; j < actionCount; j++) {
         QString actionName;
         stream >> actionName;

         if (actionName.isEmpty()) {
            actions.append(nullptr);

         } else {
            QAction *action = findAction(actionName);
            if (action) {
               actions.append(action);
            }
         }
      }

      QToolBar *toolBar = findDefaultToolBar(objectName);
      if (toolBar) {
         q_ptr->setToolBar(toolBar, actions);
      }
   }

   uchar ctmarker;
   stream >> ctmarker;

   if (ctmarker != QtFullToolBarManagerPrivate::MarkerType::CustomToolBarMarker) {
      return false;
   }

   QList<QToolBar *> oldCustomToolBars = customToolBars;

   stream >> toolBarCount;

   for (int i = 0; i < toolBarCount; i++) {
      QString objectName;
      QString toolBarName;

      int actionCount;
      stream >> objectName;
      stream >> toolBarName;
      stream >> actionCount;
      QList<QAction *> actions;

      for (int j = 0; j < actionCount; j++) {
         QString actionName;
         stream >> actionName;

         if (actionName.isEmpty()) {
            actions.append(nullptr);

         } else {
            QAction *action = findAction(actionName);

            if (action) {
               actions.append(action);
            }
         }
      }

      QToolBar *toolBar = toolBarByName(objectName);
      if (toolBar) {
         toolBar->setWindowTitle(toolBarName);
         oldCustomToolBars.removeAll(toolBar);
      } else {
         toolBar = q_ptr->createToolBar(toolBarName);
      }
      if (toolBar) {
         toolBar->setObjectName(objectName);
         q_ptr->setToolBar(toolBar, actions);
      }
   }
   QListIterator<QToolBar *> itToolBar(oldCustomToolBars);
   while (itToolBar.hasNext()) {
      q_ptr->deleteToolBar(itToolBar.next());
   }
   return true;
}

QToolBar *QtFullToolBarManagerPrivate::findDefaultToolBar(const QString &objectName) const
{
   auto itToolBar = defaultToolBars.constBegin();

   while (itToolBar != defaultToolBars.constEnd()) {
      QToolBar *tb = itToolBar.key();
      if (tb->objectName() == objectName) {
         return tb;
      }

      ++itToolBar;
   }

   qWarning("QtToolBarManager::findDefaultToolBar() Unable to find a QToolBar named "
      "%s, trying to match using the value for windowTitle", csPrintable(objectName));

   itToolBar = defaultToolBars.constBegin();
   while (itToolBar != defaultToolBars.constEnd()) {
      QToolBar *tb = itToolBar.key();
      if (tb->windowTitle() == objectName) {
         return tb;
      }

      ++itToolBar;
   }

   qWarning("QtToolBarManager::findDefaultToolBar() Unable to find a QToolBar which "
         "matches the value for windowTitle, using %s", csPrintable(objectName));

   return nullptr;
}

QAction *QtFullToolBarManagerPrivate::findAction(const QString &actionName) const
{
   QSetIterator<QAction *> itAction(allActions);
   while (itAction.hasNext()) {
      QAction *action = itAction.next();

      if (action->objectName() == actionName) {
         return action;
      }
   }

   qWarning("QtToolBarManager::findAction() Unable to find a QAction named "
      "%s, trying to match using the value for text", csPrintable(actionName));

   itAction.toFront();
   while (itAction.hasNext()) {
      QAction *action = itAction.next();

      if (action->text() == actionName) {
         return action;
      }
   }

   qWarning("QtToolBarManager::findAction() Unable to find a QAction with "
         "matches the value for text, using %s", csPrintable(actionName));

   return nullptr;
}

QToolBar *QtFullToolBarManagerPrivate::toolBarByName(const QString &toolBarName) const
{
   auto itToolBar = toolBars.constBegin();

   while (itToolBar != toolBars.constEnd()) {
      QToolBar *toolBar = itToolBar.key();
      if (toolBar->objectName() == toolBarName) {
         return toolBar;
      }

      ++itToolBar;
   }
   return nullptr;
}

QtFullToolBarManager::QtFullToolBarManager(QObject *parent)
   : QObject(parent), d_ptr(new QtFullToolBarManagerPrivate)
{
   d_ptr->q_ptr = this;
}

QtFullToolBarManager::~QtFullToolBarManager()
{
}

void QtFullToolBarManager::setMainWindow(QMainWindow *mainWindow)
{
   d_ptr->theMainWindow = mainWindow;
}

QMainWindow *QtFullToolBarManager::mainWindow() const
{
   return d_ptr->theMainWindow;
}

void QtFullToolBarManager::addCategory(const QString &category)
{
   d_ptr->categoryToActions[category] = QList<QAction *>();
}

bool QtFullToolBarManager::hasCategory(const QString &category) const
{
   return d_ptr->categoryToActions.contains(category);
}

QStringList QtFullToolBarManager::categories() const
{
   return d_ptr->categoryToActions.keys();
}

QList<QAction *> QtFullToolBarManager::categoryActions(const QString &category) const
{
   auto it = d_ptr->categoryToActions.find(category);

   if (it != d_ptr->categoryToActions.constEnd()) {
      return it.value();
   }

   return QList<QAction *>();
}

QString QtFullToolBarManager::actionCategory(QAction *action) const
{
   auto it = d_ptr->actionToCategory.find(action);

   if (it != d_ptr->actionToCategory.constEnd()) {
      return it.value();
   }
   return QString();
}

void QtFullToolBarManager::addAction(QAction *action, const QString &category)
{
   if (!action) {
      return;
   }

   if (action->isSeparator()) {
      return;
   }

   if (d_ptr->allActions.contains(action)) {
      return;
   }

   if (action->metaObject()->className() == "QToolBarWidgetAction") {
      d_ptr->widgetActions.insert(action, nullptr);
   } else {
      d_ptr->regularActions.insert(action);
   }

   d_ptr->allActions.insert(action);
   d_ptr->categoryToActions[category].append(action);
   d_ptr->actionToCategory[action] = category;
}

void QtFullToolBarManager::removeAction(QAction *action)
{
   if (!d_ptr->allActions.contains(action)) {
      return;
   }

   QList<QToolBar *> toolBars = d_ptr->actionToToolBars[action];
   QListIterator<QToolBar *> itToolBar(toolBars);
   while (itToolBar.hasNext()) {
      QToolBar *toolBar = itToolBar.next();

      d_ptr->toolBars[toolBar].removeAll(action);
      d_ptr->toolBarsWithSeparators[toolBar].removeAll(action);

      toolBar->removeAction(action);
   }

   auto itDefault = d_ptr->defaultToolBars.constBegin();

   while (itDefault != d_ptr->defaultToolBars.constEnd()) {
      if (itDefault.value().contains(action)) {
         d_ptr->defaultToolBars[itDefault.key()].removeAll(action);
      }

      itDefault++;
   }

   d_ptr->allActions.remove(action);
   d_ptr->widgetActions.remove(action);
   d_ptr->regularActions.remove(action);
   d_ptr->actionToToolBars.remove(action);

   QString category = d_ptr->actionToCategory.value(action);
   d_ptr->actionToCategory.remove(action);
   d_ptr->categoryToActions[category].removeAll(action);

   if (d_ptr->categoryToActions[category].isEmpty()) {
      d_ptr->categoryToActions.remove(category);
   }
}

QSet<QAction *> QtFullToolBarManager::actions() const
{
   return d_ptr->allActions;
}

bool QtFullToolBarManager::isWidgetAction(QAction *action) const
{
   if (d_ptr->widgetActions.contains(action)) {
      return true;
   }
   return false;
}

void QtFullToolBarManager::addDefaultToolBar(QToolBar *toolBar, const QString &category)
{
   if (! toolBar) {
      return;
   }
   if (d_ptr->toolBars.contains(toolBar)) {
      return;
   }
   // could be also checked if toolBar belongs to mainwindow

   QList<QAction *> newActionsWithSeparators;
   QList<QAction *> newActions;
   QList<QAction *> actions = toolBar->actions();
   QListIterator<QAction *> itAction(actions);

   while (itAction.hasNext()) {
      QAction *action = itAction.next();
      addAction(action, category);

      if (d_ptr->widgetActions.contains(action)) {
         d_ptr->widgetActions.insert(action, toolBar);
      }

      newActionsWithSeparators.append(action);
      if (action->isSeparator()) {
         action = nullptr;
      } else {
         d_ptr->actionToToolBars[action].append(toolBar);
      }

      newActions.append(action);
   }

   d_ptr->defaultToolBars.insert(toolBar, newActions);

   //Below could be done by call setToolBar() if we want signal emission here.
   d_ptr->toolBars.insert(toolBar, newActions);
   d_ptr->toolBarsWithSeparators.insert(toolBar, newActionsWithSeparators);
}

void QtFullToolBarManager::removeDefaultToolBar(QToolBar *toolBar)
{
   if (! d_ptr->defaultToolBars.contains(toolBar)) {
      return;
   }

   QList<QAction *> defaultActions = d_ptr->defaultToolBars[toolBar];
   setToolBar(toolBar, QList<QAction *>());
   QListIterator<QAction *> itAction(defaultActions);
   while (itAction.hasNext()) {
      removeAction(itAction.next());
   }

   d_ptr->toolBars.remove(toolBar);
   d_ptr->toolBarsWithSeparators.remove(toolBar);
   d_ptr->defaultToolBars.remove(toolBar);

   itAction.toFront();
   while (itAction.hasNext()) {
      QAction *action = itAction.next();
      if (action) {
         toolBar->insertAction(nullptr, action);
      } else {
         toolBar->insertSeparator(nullptr);
      }
   }
}

QMap<QToolBar *, QList<QAction *>> QtFullToolBarManager::defaultToolBars() const
{
   return d_ptr->defaultToolBars;
}

bool QtFullToolBarManager::isDefaultToolBar(QToolBar *toolBar) const
{
   if (d_ptr->defaultToolBars.contains(toolBar)) {
      return true;
   }
   return false;
}

QToolBar *QtFullToolBarManager::createToolBar(const QString &toolBarName)
{
   if (!mainWindow()) {
      return nullptr;
   }

   QToolBar *toolBar = new QToolBar(toolBarName, mainWindow());

   int i = 1;
   const QString prefix = "_Custom_Toolbar_%1";
   QString name = prefix.formatArg(i);

   while (d_ptr->toolBarByName(name)) {
      name = prefix.formatArg(++i);
   }

   toolBar->setObjectName(name);

   mainWindow()->addToolBar(toolBar);
   d_ptr->customToolBars.append(toolBar);
   d_ptr->toolBars.insert(toolBar, QList<QAction *>());
   d_ptr->toolBarsWithSeparators.insert(toolBar, QList<QAction *>());

   return toolBar;
}

void QtFullToolBarManager::deleteToolBar(QToolBar *toolBar)
{
   if (!d_ptr->toolBars.contains(toolBar)) {
      return;
   }

   if (d_ptr->defaultToolBars.contains(toolBar)) {
      return;
   }

   setToolBar(toolBar, QList<QAction *>());
   d_ptr->customToolBars.removeAll(toolBar);
   d_ptr->toolBars.remove(toolBar);
   d_ptr->toolBarsWithSeparators.remove(toolBar);
   delete toolBar;
}

QList<QAction *> QtFullToolBarManager::actions(QToolBar *toolBar) const
{
   if (d_ptr->toolBars.contains(toolBar)) {
      return d_ptr->toolBars.value(toolBar);
   }
   return QList<QAction *>();
}

void QtFullToolBarManager::setToolBars(const QMap<QToolBar *, QList<QAction *>> &actions)
{
   auto it = actions.constBegin();

   while (it != actions.constEnd()) {
      setToolBar(it.key(), it.value());
      ++it;
   }
}

void QtFullToolBarManager::setToolBar(QToolBar *toolBar, const QList<QAction *> &actions)
{
   if (!toolBar) {
      return;
   }
   if (!d_ptr->toolBars.contains(toolBar)) {
      return;
   }

   if (actions == d_ptr->toolBars[toolBar]) {
      return;
   }

   QMap<QToolBar *, QList<QAction *>> toRemove;

   QList<QAction *> newActions;
   QListIterator<QAction *> itAction(actions);
   while (itAction.hasNext()) {
      QAction *action = itAction.next();
      if (!action || (!newActions.contains(action) && d_ptr->allActions.contains(action))) {
         newActions.append(action);
      }

      QToolBar *oldToolBar = d_ptr->toolBarWidgetAction(action);
      if (oldToolBar && oldToolBar != toolBar) {
         toRemove[oldToolBar].append(action);
      }
   }

   d_ptr->removeWidgetActions(toRemove);

   QList<QAction *> oldActions = d_ptr->toolBarsWithSeparators.value(toolBar);
   QListIterator<QAction *> itOldAction(oldActions);

   while (itOldAction.hasNext()) {
      QAction *action = itOldAction.next();
      /*
      When addDefaultToolBar() separator actions could be checked if they are
      inserted in other toolbars - if yes then create new one.
      */
      if (d_ptr->toolBarWidgetAction(action) == toolBar) {
         d_ptr->widgetActions.insert(action, nullptr);
      }

      toolBar->removeAction(action);

      if (action->isSeparator()) {
         delete action;
      } else {
         d_ptr->actionToToolBars[action].removeAll(toolBar);
      }
   }

   QList<QAction *> newActionsWithSeparators;
   QListIterator<QAction *> itNewActions(newActions);

   while (itNewActions.hasNext()) {
      QAction *action = itNewActions.next();

      QAction *newAction = nullptr;

      if (! action) {
         newAction = toolBar->insertSeparator(nullptr);
      }

      if (d_ptr->allActions.contains(action)) {
         toolBar->insertAction(nullptr, action);
         newAction = action;
         d_ptr->actionToToolBars[action].append(toolBar);
      }

      newActionsWithSeparators.append(newAction);
   }

   d_ptr->toolBars.insert(toolBar, newActions);
   d_ptr->toolBarsWithSeparators.insert(toolBar, newActionsWithSeparators);
}

QMap<QToolBar *, QList<QAction *>> QtFullToolBarManager::toolBarsActions() const
{
   return d_ptr->toolBars;
}

void QtFullToolBarManager::resetToolBar(QToolBar *toolBar)
{
   if (! isDefaultToolBar(toolBar)) {
      return;
   }
   setToolBar(toolBar, defaultToolBars().value(toolBar));
}

void QtFullToolBarManager::resetAllToolBars()
{
   setToolBars(defaultToolBars());
   QList<QToolBar *> oldCustomToolBars = d_ptr->customToolBars;
   QListIterator<QToolBar *> itToolBar(oldCustomToolBars);
   while (itToolBar.hasNext()) {
      deleteToolBar(itToolBar.next());
   }
}

QByteArray QtFullToolBarManager::saveState(int version) const
{
   QByteArray data;
   QDataStream stream(&data, QIODevice::WriteOnly);
   stream << QtFullToolBarManagerPrivate::MarkerType::VersionMarker;
   stream << version;
   d_ptr->saveState(stream);

   return data;
}

bool QtFullToolBarManager::restoreState(const QByteArray &state, int version)
{
   QByteArray sd = state;
   QDataStream stream(&sd, QIODevice::ReadOnly);
   int marker, v;
   stream >> marker;
   stream >> v;
   if (marker != QtFullToolBarManagerPrivate::MarkerType::VersionMarker || v != version) {
      return false;
   }
   return d_ptr->restoreState(stream);
}

class QtToolBarManagerPrivate
{
   class QtToolBarManager *q_ptr;
   Q_DECLARE_PUBLIC(QtToolBarManager)

 public:
   QtFullToolBarManager *manager;
};

QtToolBarManager::QtToolBarManager(QObject *parent)
   : QObject(parent), d_ptr(new QtToolBarManagerPrivate)
{
   d_ptr->q_ptr = this;

   d_ptr->manager = new QtFullToolBarManager(this);
}

QtToolBarManager::~QtToolBarManager()
{
}

void QtToolBarManager::setMainWindow(QMainWindow *mainWindow)
{
   d_ptr->manager->setMainWindow(mainWindow);
}

QMainWindow *QtToolBarManager::mainWindow() const
{
   return d_ptr->manager->mainWindow();
}

void QtToolBarManager::addAction(QAction *action, const QString &category)
{
   d_ptr->manager->addAction(action, category);
}

void QtToolBarManager::removeAction(QAction *action)
{
   d_ptr->manager->removeAction(action);
}

void QtToolBarManager::addToolBar(QToolBar *toolBar, const QString &category)
{
   d_ptr->manager->addDefaultToolBar(toolBar, category);
}

void QtToolBarManager::removeToolBar(QToolBar *toolBar)
{
   d_ptr->manager->removeDefaultToolBar(toolBar);
}

QList<QToolBar *> QtToolBarManager::toolBars() const
{
   return d_ptr->manager->toolBarsActions().keys();
}

QByteArray QtToolBarManager::saveState(int version) const
{
   return d_ptr->manager->saveState(version);
}

bool QtToolBarManager::restoreState(const QByteArray &state, int version)
{
   return d_ptr->manager->restoreState(state, version);
}

class ToolBarItem
{
 public:
   ToolBarItem()
      : tb(nullptr)
   {
   }

   ToolBarItem(QToolBar *toolBar)
      : tb(toolBar)
   {
   }

   ToolBarItem(QToolBar *toolBar, const QString &toolBarName)
      : tb(toolBar), tbName(toolBarName)
   {
   }

   ToolBarItem(const QString &toolBarName)
      : tb(nullptr), tbName(toolBarName)
   {
   }

   QToolBar *toolBar() const {
      return tb;
   }

   void setToolBar(QToolBar *toolBar) {
      tb = toolBar;
   }

   QString toolBarName() const {
      return tbName;
   }

   void setToolBarName(const QString &toolBarName) {
      tbName = toolBarName;
   }

 private:
   QToolBar *tb;
   QString tbName;
};

class QtToolBarDialogPrivate
{
   QtToolBarDialog *q_ptr;
   Q_DECLARE_PUBLIC(QtToolBarDialog)

 public:
   QtToolBarDialogPrivate()
      : toolBarManager(nullptr), currentAction(nullptr), currentToolBar(nullptr)
   { }

   ToolBarItem *createItem(QToolBar *toolBar);
   ToolBarItem *createItem(const QString &toolBarName);
   void deleteItem(ToolBarItem *item);

   void newClicked();
   void removeClicked();
   void defaultClicked();
   void okClicked();
   void applyClicked();
   void cancelClicked();
   void upClicked();
   void downClicked();
   void leftClicked();
   void rightClicked();
   void renameClicked();
   void toolBarRenamed(QListWidgetItem *item);
   void currentActionChanged(QTreeWidgetItem *current);
   void currentToolBarChanged(QListWidgetItem *current);
   void currentToolBarActionChanged(QListWidgetItem *current);

   void removeToolBar(ToolBarItem *item);
   bool isDefaultToolBar(ToolBarItem *item) const;
   void setButtons();
   void clearOld();
   void fillNew();

   QtFullToolBarManager *toolBarManager;
   QMap<ToolBarItem *, QList<QAction *>> currentState;
   QMap<QToolBar *, ToolBarItem *> toolBarItems;
   QSet<ToolBarItem *> createdItems;
   QSet<ToolBarItem *> removedItems;

   QSet<ToolBarItem *> allToolBarItems;

   // static
   QTreeWidgetItem *currentAction;
   QMap<QAction *, QTreeWidgetItem *> actionToItem;
   QMap<QTreeWidgetItem *, QAction *> itemToAction;

   // dynamic
   ToolBarItem *currentToolBar;
   QMap<ToolBarItem *, QListWidgetItem *> toolBarToItem;
   QMap<QListWidgetItem *, ToolBarItem *> itemToToolBar;

   // dynamic
   QMap<QAction *, QListWidgetItem *> actionToCurrentItem;
   QMap<QListWidgetItem *, QAction *> currentItemToAction;

   QMap<QAction *, ToolBarItem *> widgetActionToToolBar;
   QMap<ToolBarItem *, QSet<QAction *>> toolBarToWidgetActions;

   QString separatorText;
   Ui::QtToolBarDialog ui;
};

ToolBarItem *QtToolBarDialogPrivate::createItem(QToolBar *toolBar)
{
   if (! toolBar) {
      return nullptr;
   }

   ToolBarItem *item = new ToolBarItem(toolBar, toolBar->windowTitle());
   allToolBarItems.insert(item);

   return item;
}

ToolBarItem *QtToolBarDialogPrivate::createItem(const QString &toolBarName)
{
   ToolBarItem *item = new ToolBarItem(toolBarName);
   allToolBarItems.insert(item);

   return item;
}

void QtToolBarDialogPrivate::deleteItem(ToolBarItem *item)
{
   if (! allToolBarItems.contains(item)) {
      return;
   }

   allToolBarItems.remove(item);
   delete item;
}

void QtToolBarDialogPrivate::clearOld()
{
   ui.actionTree->clear();
   ui.toolBarList->clear();
   ui.currentToolBarList->clear();
   ui.removeButton->setEnabled(false);
   ui.newButton->setEnabled(false);
   ui.upButton->setEnabled(false);
   ui.downButton->setEnabled(false);
   ui.leftButton->setEnabled(false);
   ui.rightButton->setEnabled(false);

   actionToItem.clear();
   itemToAction.clear();
   toolBarToItem.clear();
   itemToToolBar.clear();
   actionToCurrentItem.clear();
   currentItemToAction.clear();
   widgetActionToToolBar.clear();
   toolBarToWidgetActions.clear();

   toolBarItems.clear();
   currentState.clear();
   createdItems.clear();
   removedItems.clear();
   QSetIterator<ToolBarItem *> itItem(allToolBarItems);

   while (itItem.hasNext()) {
      delete itItem.next();
   }

   allToolBarItems.clear();

   currentToolBar = nullptr;
   currentAction  = nullptr;
}

void QtToolBarDialogPrivate::fillNew()
{
   if (!toolBarManager) {
      return;
   }

   QTreeWidgetItem *treeItem = new QTreeWidgetItem(ui.actionTree);
   treeItem->setText(0, separatorText);

   ui.actionTree->setCurrentItem(treeItem);
   currentAction = treeItem;
   actionToItem.insert(nullptr, treeItem);
   itemToAction.insert(treeItem, nullptr);

   QStringList categories = toolBarManager->categories();
   QStringListIterator itCategory(categories);

   while (itCategory.hasNext()) {
      QString category = itCategory.next();
      QTreeWidgetItem *categoryItem = new QTreeWidgetItem(ui.actionTree);

      categoryItem->setText(0, category);

      QList<QAction *> actions = toolBarManager->categoryActions(category);
      QListIterator<QAction *> itAction(actions);

      while (itAction.hasNext()) {

         QAction *action = itAction.next();
         treeItem = new QTreeWidgetItem(categoryItem);
         treeItem->setText(0, action->text());
         treeItem->setIcon(0, action->icon());
         treeItem->setTextAlignment(0, Qt::AlignLeft | Qt::AlignVCenter | Qt::TextShowMnemonic);

         actionToItem.insert(action, treeItem);
         itemToAction.insert(treeItem, action);

         if (toolBarManager->isWidgetAction(action)) {
            treeItem->setData(0, Qt::TextColorRole, QColor(Qt::blue));
            widgetActionToToolBar.insert(action, nullptr);
         }

         treeItem->setFlags(categoryItem->flags() | Qt::ItemIsDragEnabled);
      }
      ui.actionTree->setItemExpanded(categoryItem, true);
   }
   //ui.actionTree->sortItems(0, Qt::AscendingOrder);

   auto toolBars = toolBarManager->toolBarsActions();
   auto it       = toolBars.constBegin();

   while (it != toolBars.constEnd()) {
      QToolBar *toolBar = it.key();
      ToolBarItem *tbItem = createItem(toolBar);
      toolBarItems.insert(toolBar, tbItem);

      QListWidgetItem *listItem = new QListWidgetItem(toolBar->windowTitle(), ui.toolBarList);
      toolBarToItem.insert(tbItem, listItem);
      itemToToolBar.insert(listItem, tbItem);

      QList<QAction *> actions = it.value();
      QListIterator<QAction *> itAction(actions);

      while (itAction.hasNext()) {
         QAction *action = itAction.next();

         if (toolBarManager->isWidgetAction(action)) {
            widgetActionToToolBar.insert(action, tbItem);
            toolBarToWidgetActions[tbItem].insert(action);
         }
      }

      currentState.insert(tbItem, actions);

      if (it == toolBars.constBegin()) {
         ui.toolBarList->setCurrentItem(listItem);
      }

      if (isDefaultToolBar(tbItem)) {
         listItem->setData(Qt::TextColorRole, QColor(Qt::darkGreen));
      } else {
         listItem->setFlags(listItem->flags() | Qt::ItemIsEditable);
      }

      ++it;
   }
   ui.toolBarList->sortItems();
   setButtons();
}

bool QtToolBarDialogPrivate::isDefaultToolBar(ToolBarItem *item) const
{
   if (!item) {
      return false;
   }
   if (!item->toolBar()) {
      return false;
   }
   return toolBarManager->isDefaultToolBar(item->toolBar());
}

void QtToolBarDialogPrivate::setButtons()
{
   bool newEnabled = false;
   bool removeEnabled = false;
   bool renameEnabled = false;
   bool upEnabled = false;
   bool downEnabled = false;
   bool leftEnabled = false;
   bool rightEnabled = false;

   if (toolBarManager) {
      newEnabled = true;
      removeEnabled = !isDefaultToolBar(currentToolBar);
      renameEnabled = removeEnabled;
      QListWidgetItem *currentToolBarAction = ui.currentToolBarList->currentItem();
      if (currentToolBarAction) {
         int row = ui.currentToolBarList->row(currentToolBarAction);
         upEnabled = row > 0;
         downEnabled = row < ui.currentToolBarList->count() - 1;
         leftEnabled = true;
      }
      if (currentAction && currentToolBar) {
         rightEnabled = true;
      }
   }

   ui.newButton->setEnabled(newEnabled);
   ui.removeButton->setEnabled(removeEnabled);
   ui.renameButton->setEnabled(renameEnabled);
   ui.upButton->setEnabled(upEnabled);
   ui.downButton->setEnabled(downEnabled);
   ui.leftButton->setEnabled(leftEnabled);
   ui.rightButton->setEnabled(rightEnabled);
}

void QtToolBarDialogPrivate::newClicked()
{
   QString toolBarName = QtToolBarDialog::tr("Custom Toolbar"); // = QInputDialog::getString();

   // produce unique name
   ToolBarItem *item = createItem(toolBarName);
   currentState.insert(item, QList<QAction *>());
   createdItems.insert(item);
   QListWidgetItem *i = new QListWidgetItem(toolBarName, ui.toolBarList);
   i->setFlags(i->flags() | Qt::ItemIsEditable);
   ui.toolBarList->setCurrentItem(i);
   itemToToolBar.insert(i, item);
   toolBarToItem.insert(item, i);
   ui.toolBarList->sortItems();
   ui.toolBarList->setCurrentItem(i);
   currentToolBarChanged(i);
   renameClicked();
}

void QtToolBarDialogPrivate::removeToolBar(ToolBarItem *item)
{
   if (!item) {
      return;
   }

   if (item->toolBar() && toolBarManager->isDefaultToolBar(item->toolBar())) {
      return;
   }

   if (!toolBarToItem.contains(item)) {
      return;
   }

   QListWidgetItem *i = toolBarToItem.value(item);
   bool wasCurrent = false;

   if (i == ui.toolBarList->currentItem()) {
      wasCurrent = true;
   }
   int row = ui.toolBarList->row(i);

   auto itToolBar = toolBarToWidgetActions.find(item);

   if (itToolBar != toolBarToWidgetActions.constEnd()) {
      QSet<QAction *> actions = itToolBar.value();
      QSetIterator<QAction *> itAction(actions);

      while (itAction.hasNext()) {
         QAction *action = itAction.next();
         widgetActionToToolBar.insert(action, nullptr);
      }
      toolBarToWidgetActions.remove(item);
   }

   currentState.remove(item);
   createdItems.remove(item);
   toolBarToItem.remove(item);
   itemToToolBar.remove(i);
   delete i;

   if (item->toolBar()) {
      removedItems.insert(item);
   } else {
      deleteItem(item);
   }
   if (wasCurrent) {
      if (row == ui.toolBarList->count()) {
         row--;
      }
      if (row < 0)
         ;
      else {
         ui.toolBarList->setCurrentRow(row);
      }
   }
   setButtons();
}

void QtToolBarDialogPrivate::removeClicked()
{
   QListWidgetItem *i = ui.toolBarList->currentItem();
   if (!i) {
      return;
   }

   ToolBarItem *item = itemToToolBar.value(i);
   removeToolBar(item);
}

void QtToolBarDialogPrivate::defaultClicked()
{
   auto defaultToolBars = toolBarManager->defaultToolBars();
   auto itToolBar = defaultToolBars.constBegin();

   while (itToolBar != defaultToolBars.constEnd()) {
      QToolBar *toolBar = itToolBar.key();
      ToolBarItem *toolBarItem = toolBarItems.value(toolBar);

      if (toolBarToWidgetActions.contains(toolBarItem)) {
         QSetIterator<QAction *> itAction(toolBarToWidgetActions.value(toolBarItem));
         while (itAction.hasNext()) {
            widgetActionToToolBar.insert(itAction.next(), nullptr);
         }
         toolBarToWidgetActions.remove(toolBarItem);
      }

      currentState.remove(toolBarItem);

      QListIterator<QAction *> itAction(itToolBar.value());

      while (itAction.hasNext()) {
         QAction *action = itAction.next();
         if (toolBarManager->isWidgetAction(action)) {
            ToolBarItem *otherToolBar = widgetActionToToolBar.value(action);
            if (otherToolBar) {
               toolBarToWidgetActions[otherToolBar].remove(action);
               currentState[otherToolBar].removeAll(action);
            }
            widgetActionToToolBar.insert(action, toolBarItem);
            toolBarToWidgetActions[toolBarItem].insert(action);
         }
      }
      currentState.insert(toolBarItem, itToolBar.value());

      ++itToolBar;
   }
   currentToolBarChanged(toolBarToItem.value(currentToolBar));

   QList<ToolBarItem *> toolBars = currentState.keys();
   QListIterator<ToolBarItem *> itTb(toolBars);
   while (itTb.hasNext()) {
      removeToolBar(itTb.next());
   }
}

void QtToolBarDialogPrivate::okClicked()
{
   applyClicked();
   q_ptr->accept();
}

void QtToolBarDialogPrivate::applyClicked()
{
   auto toolBars  = currentState;
   auto itToolBar = toolBars.constBegin();

   while (itToolBar != toolBars.constEnd()) {
      ToolBarItem *item = itToolBar.key();
      QToolBar *toolBar = item->toolBar();
      if (toolBar) {
         toolBarManager->setToolBar(toolBar, itToolBar.value());
         toolBar->setWindowTitle(item->toolBarName());
      }

      ++itToolBar;
   }

   QSet<ToolBarItem *> toRemove = removedItems;
   QSetIterator<ToolBarItem *> itRemove(toRemove);
   while (itRemove.hasNext()) {
      ToolBarItem *item = itRemove.next();
      QToolBar *toolBar = item->toolBar();
      removedItems.remove(item);
      currentState.remove(item);
      deleteItem(item);
      if (toolBar) {
         toolBarManager->deleteToolBar(toolBar);
      }
   }

   QSet<ToolBarItem *> toCreate = createdItems;
   QSetIterator<ToolBarItem *> itCreate(toCreate);
   while (itCreate.hasNext()) {
      ToolBarItem *item = itCreate.next();
      QString toolBarName = item->toolBarName();
      createdItems.remove(item);
      QList<QAction *> actions = currentState.value(item);
      QToolBar *toolBar = toolBarManager->createToolBar(toolBarName);
      item->setToolBar(toolBar);
      toolBarManager->setToolBar(toolBar, actions);
   }
}

void QtToolBarDialogPrivate::upClicked()
{
   QListWidgetItem *currentToolBarAction = ui.currentToolBarList->currentItem();
   if (!currentToolBarAction) {
      return;
   }

   int row = ui.currentToolBarList->row(currentToolBarAction);
   if (row == 0) {
      return;
   }

   ui.currentToolBarList->takeItem(row);
   int newRow = row - 1;
   ui.currentToolBarList->insertItem(newRow, currentToolBarAction);

   QList<QAction *> actions = currentState.value(currentToolBar);
   QAction *action = actions.at(row);
   actions.removeAt(row);
   actions.insert(newRow, action);
   currentState.insert(currentToolBar, actions);
   ui.currentToolBarList->setCurrentItem(currentToolBarAction);
   setButtons();
}

void QtToolBarDialogPrivate::downClicked()
{
   QListWidgetItem *currentToolBarAction = ui.currentToolBarList->currentItem();

   if (!currentToolBarAction) {
      return;
   }

   int row = ui.currentToolBarList->row(currentToolBarAction);
   if (row == ui.currentToolBarList->count() - 1) {
      return;
   }

   ui.currentToolBarList->takeItem(row);
   int newRow = row + 1;
   ui.currentToolBarList->insertItem(newRow, currentToolBarAction);

   QList<QAction *> actions = currentState.value(currentToolBar);
   QAction *action = actions.at(row);
   actions.removeAt(row);
   actions.insert(newRow, action);
   currentState.insert(currentToolBar, actions);
   ui.currentToolBarList->setCurrentItem(currentToolBarAction);
   setButtons();
}

void QtToolBarDialogPrivate::leftClicked()
{
   QListWidgetItem *currentToolBarAction = ui.currentToolBarList->currentItem();
   if (!currentToolBarAction) {
      return;
   }

   int row = ui.currentToolBarList->row(currentToolBarAction);
   currentState[currentToolBar].removeAt(row);
   QAction *action = currentItemToAction.value(currentToolBarAction);

   if (widgetActionToToolBar.contains(action)) {
      ToolBarItem *item = widgetActionToToolBar.value(action);

      if (item == currentToolBar) {
         toolBarToWidgetActions[item].remove(action);

         if (toolBarToWidgetActions[item].empty()) {
            toolBarToWidgetActions.remove(item);
         }
      }
      widgetActionToToolBar.insert(action, nullptr);
   }

   if (action) {
      actionToCurrentItem.remove(action);
   }

   currentItemToAction.remove(currentToolBarAction);
   delete currentToolBarAction;

   if (row == ui.currentToolBarList->count()) {
      row--;
   }
   if (row >= 0) {
      QListWidgetItem *item = ui.currentToolBarList->item(row);
      ui.currentToolBarList->setCurrentItem(item);
   }
   setButtons();
}

void QtToolBarDialogPrivate::rightClicked()
{
   if (!currentAction) {
      return;
   }
   if (!currentToolBar) {
      return;
   }
   QListWidgetItem *currentToolBarAction = ui.currentToolBarList->currentItem();

   QAction *action = itemToAction.value(currentAction);
   QListWidgetItem *item = nullptr;

   if (action) {
      if (currentState[currentToolBar].contains(action)) {
         item = actionToCurrentItem.value(action);
         if (item == currentToolBarAction) {
            return;
         }
         int row = ui.currentToolBarList->row(item);
         ui.currentToolBarList->takeItem(row);
         currentState[currentToolBar].removeAt(row);
         // only reorder here
      } else {
         item = new QListWidgetItem(action->text());
         item->setIcon(action->icon());
         item->setTextAlignment(Qt::AlignLeft | Qt::AlignVCenter | Qt::TextShowMnemonic);
         currentItemToAction.insert(item, action);
         actionToCurrentItem.insert(action, item);

         if (widgetActionToToolBar.contains(action)) {
            item->setData(Qt::TextColorRole, QColor(Qt::blue));
            ToolBarItem *toolBar = widgetActionToToolBar.value(action);
            if (toolBar) {
               currentState[toolBar].removeAll(action);
               toolBarToWidgetActions[toolBar].remove(action);
               if (toolBarToWidgetActions[toolBar].empty()) {
                  toolBarToWidgetActions.remove(toolBar);
               }
            }
            widgetActionToToolBar.insert(action, currentToolBar);
            toolBarToWidgetActions[currentToolBar].insert(action);
         }
      }
   } else {
      item = new QListWidgetItem(separatorText);
      currentItemToAction.insert(item, nullptr);
   }

   int row = ui.currentToolBarList->count();
   if (currentToolBarAction) {
      row = ui.currentToolBarList->row(currentToolBarAction) + 1;
   }
   ui.currentToolBarList->insertItem(row, item);
   currentState[currentToolBar].insert(row, action);
   ui.currentToolBarList->setCurrentItem(item);

   setButtons();
}

void QtToolBarDialogPrivate::renameClicked()
{
   if (!currentToolBar) {
      return;
   }

   QListWidgetItem *item = toolBarToItem.value(currentToolBar);
   ui.toolBarList->editItem(item);
}

void QtToolBarDialogPrivate::toolBarRenamed(QListWidgetItem *item)
{
   if (!currentToolBar) {
      return;
   }

   ToolBarItem *tbItem = itemToToolBar.value(item);
   if (!tbItem) {
      return;
   }
   tbItem->setToolBarName(item->text());
   //ui.toolBarList->sortItems();
}

void QtToolBarDialogPrivate::currentActionChanged(QTreeWidgetItem *current)
{
   if (itemToAction.contains(current)) {
      currentAction = current;
   } else {
      currentAction = nullptr;
   }

   setButtons();
}

void QtToolBarDialogPrivate::currentToolBarChanged(QListWidgetItem *current)
{
   currentToolBar = itemToToolBar.value(current);
   ui.currentToolBarList->clear();
   actionToCurrentItem.clear();
   currentItemToAction.clear();
   setButtons();

   if (!currentToolBar) {
      return;
   }

   QList<QAction *> actions = currentState.value(currentToolBar);
   QListIterator<QAction *> itAction(actions);
   QListWidgetItem *first = nullptr;

   while (itAction.hasNext()) {
      QAction *action = itAction.next();
      QString actionName = separatorText;

      if (action) {
         actionName = action->text();
      }

      QListWidgetItem *item = new QListWidgetItem(actionName, ui.currentToolBarList);

      if (action) {
         item->setIcon(action->icon());
         item->setTextAlignment(Qt::AlignLeft | Qt::AlignVCenter | Qt::TextShowMnemonic);
         actionToCurrentItem.insert(action, item);
         if (widgetActionToToolBar.contains(action)) {
            item->setData(Qt::TextColorRole, QColor(Qt::blue));
         }
      }
      currentItemToAction.insert(item, action);
      if (!first) {
         first = item;
      }
   }
   if (first) {
      ui.currentToolBarList->setCurrentItem(first);
   }
}

void QtToolBarDialogPrivate::currentToolBarActionChanged(QListWidgetItem *)
{
   setButtons();
}

void QtToolBarDialogPrivate::cancelClicked()
{
   // just nothing
   q_ptr->reject();
}

QtToolBarDialog::QtToolBarDialog(QWidget *parent, Qt::WindowFlags flags)
   : QDialog(parent, flags), d_ptr(new QtToolBarDialogPrivate)
{
   d_ptr->q_ptr = this;
   d_ptr->ui.setupUi(this);
   d_ptr->separatorText = tr("< S E P A R A T O R >");

   d_ptr->ui.actionTree->setColumnCount(1);
   d_ptr->ui.actionTree->setRootIsDecorated(false);
   d_ptr->ui.actionTree->header()->hide();

   d_ptr->ui.upButton->setIcon(QIcon(":/resources/toolbar_dialog/images/up.png"));
   d_ptr->ui.downButton->setIcon(QIcon(":/resources/toolbar_dialog/images/down.png"));
   d_ptr->ui.leftButton->setIcon(QIcon(":/resources/toolbar_dialog/images/back.png"));
   d_ptr->ui.rightButton->setIcon(QIcon(":/resources/toolbar_dialog/images/forward.png"));
   d_ptr->ui.newButton->setIcon(QIcon(":/resources/toolbar_dialog/images/plus.png"));
   d_ptr->ui.removeButton->setIcon(QIcon(":/resources/toolbar_dialog/images/minus.png"));

   connect(d_ptr->ui.newButton,    &QToolButton::clicked, this, &QtToolBarDialog::newClicked);
   connect(d_ptr->ui.removeButton, &QToolButton::clicked, this, &QtToolBarDialog::removeClicked);
   connect(d_ptr->ui.renameButton, &QToolButton::clicked, this, &QtToolBarDialog::renameClicked);
   connect(d_ptr->ui.upButton,     &QToolButton::clicked, this, &QtToolBarDialog::upClicked);
   connect(d_ptr->ui.downButton,   &QToolButton::clicked, this, &QtToolBarDialog::downClicked);
   connect(d_ptr->ui.leftButton,   &QToolButton::clicked, this, &QtToolBarDialog::leftClicked);
   connect(d_ptr->ui.rightButton,  &QToolButton::clicked, this, &QtToolBarDialog::rightClicked);

   connect(d_ptr->ui.buttonBox->button(QDialogButtonBox::RestoreDefaults), &QPushButton::clicked,
         this, &QtToolBarDialog::defaultClicked);

   connect(d_ptr->ui.buttonBox->button(QDialogButtonBox::Ok), &QPushButton::clicked,
         this, &QtToolBarDialog::okClicked);

   connect(d_ptr->ui.buttonBox->button(QDialogButtonBox::Apply), &QPushButton::clicked,
         this, &QtToolBarDialog::applyClicked);

   connect(d_ptr->ui.buttonBox->button(QDialogButtonBox::Cancel), &QPushButton::clicked,
         this, &QtToolBarDialog::cancelClicked);

   connect(d_ptr->ui.actionTree, &QTreeWidget::currentItemChanged,
         this, &QtToolBarDialog::currentActionChanged);

   connect(d_ptr->ui.toolBarList, &QListWidget::currentItemChanged,
         this, &QtToolBarDialog::currentToolBarChanged);

   connect(d_ptr->ui.currentToolBarList, &QListWidget::currentItemChanged,
         this, &QtToolBarDialog::currentToolBarActionChanged);

   connect(d_ptr->ui.actionTree,  &QTreeWidget::itemDoubleClicked,
         this, &QtToolBarDialog::rightClicked);

   connect(d_ptr->ui.currentToolBarList, &QListWidget::itemDoubleClicked,
         this, &QtToolBarDialog::leftClicked);

   connect(d_ptr->ui.toolBarList, &QListWidget::itemChanged,
         this, &QtToolBarDialog::toolBarRenamed);
}

QtToolBarDialog::~QtToolBarDialog()
{
   d_ptr->clearOld();
}

void QtToolBarDialog::setToolBarManager(QtToolBarManager *toolBarManager)
{
   if (d_ptr->toolBarManager == toolBarManager->d_ptr->manager) {
      return;
   }

   if (isVisible()) {
      d_ptr->clearOld();
   }

   d_ptr->toolBarManager = toolBarManager->d_ptr->manager;

   if (isVisible()) {
      d_ptr->fillNew();
   }
}

void QtToolBarDialog::showEvent(QShowEvent *event)
{
   if (!event->spontaneous()) {
      d_ptr->fillNew();
   }
}

void QtToolBarDialog::hideEvent(QHideEvent *event)
{
   if (!event->spontaneous()) {
      d_ptr->clearOld();
   }
}

void QtToolBarDialog::newClicked()
{
   Q_D(QtToolBarDialog);
   d->newClicked();
}

void QtToolBarDialog::removeClicked()
{
   Q_D(QtToolBarDialog);
   d->removeClicked();
}

void QtToolBarDialog::defaultClicked()
{
   Q_D(QtToolBarDialog);
   d->defaultClicked();
}

void QtToolBarDialog::okClicked()
{
   Q_D(QtToolBarDialog);
   d->okClicked();
}

void QtToolBarDialog::applyClicked()
{
   Q_D(QtToolBarDialog);
   d->applyClicked();
}

void QtToolBarDialog::cancelClicked()
{
   Q_D(QtToolBarDialog);
   d->cancelClicked();
}

void QtToolBarDialog::upClicked()
{
   Q_D(QtToolBarDialog);
   d->upClicked();
}

void QtToolBarDialog::downClicked()
{
   Q_D(QtToolBarDialog);
   d->downClicked();
}

void QtToolBarDialog::leftClicked()
{
   Q_D(QtToolBarDialog);
   d->leftClicked();
}

void QtToolBarDialog::rightClicked()
{
   Q_D(QtToolBarDialog);
   d->rightClicked();
}

void QtToolBarDialog::renameClicked()
{
   Q_D(QtToolBarDialog);
   d->renameClicked();
}

void QtToolBarDialog::toolBarRenamed(QListWidgetItem *un_named_arg1)
{
   Q_D(QtToolBarDialog);
   d->toolBarRenamed(un_named_arg1);
}

void QtToolBarDialog::currentActionChanged(QTreeWidgetItem *un_named_arg1)
{
   Q_D(QtToolBarDialog);
   d->currentActionChanged(un_named_arg1);
}

void QtToolBarDialog::currentToolBarChanged(QListWidgetItem *un_named_arg1)
{
   Q_D(QtToolBarDialog);
   d->currentToolBarChanged(un_named_arg1);
}

void QtToolBarDialog::currentToolBarActionChanged(QListWidgetItem *un_named_arg1)
{
   Q_D(QtToolBarDialog);
   d->currentToolBarActionChanged(un_named_arg1);
}