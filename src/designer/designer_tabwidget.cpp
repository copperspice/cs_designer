/***********************************************************************
*
* Copyright (c) 2021-2025 Barbara Geller
* Copyright (c) 2021-2025 Ansel Sermersheim
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
#include <designer_command.h>
#include <designer_property_command.h>
#include <designer_tabwidget.h>
#include <promotion_taskmenu.h>

#include <formwindowbase_p.h>

#include <QAction>
#include <QApplication>
#include <QDrag>
#include <QLabel>
#include <QMenu>
#include <QMimeData>
#include <QMouseEvent>
#include <QTabBar>
#include <QTabWidget>

namespace qdesigner_internal {

// Store tab widget as drag source
class MyMimeData : public QMimeData
{
   CS_OBJECT(MyMimeData)

 public:
   MyMimeData(const QTabWidget *tab) : m_tab(tab) {}
   static bool fromMyTab(const QMimeData *mimeData, const QTabWidget *tab) {
      if (!mimeData) {
         return false;
      }
      const MyMimeData *m = dynamic_cast<const MyMimeData *>(mimeData);
      return m &&  m->m_tab ==  tab;
   }

 private:
   const QTabWidget *m_tab;
};

} // namespace qdesigner_internal

// ------------- QTabWidgetEventFilter

QTabWidgetEventFilter::QTabWidgetEventFilter(QTabWidget *parent)
   : QObject(parent), m_tabWidget(parent), m_dropIndicator(nullptr), m_dragPage(nullptr),
     m_mousePressed(false),
     m_actionDeletePage(new QAction(tr("Delete"),  this)),
     m_actionInsertPage(new QAction(tr("Before Current Page"), this)),
     m_actionInsertPageAfter(new QAction(tr("After Current Page"), this)),
     m_pagePromotionTaskMenu(new qdesigner_internal::PromotionTaskMenu(nullptr, qdesigner_internal::PromotionTaskMenu::ModeSingleWidget, this))
{
   tabBar()->setAcceptDrops(true);
   tabBar()->installEventFilter(this);

   connect(m_actionInsertPage,      &QAction::triggered, this, &QTabWidgetEventFilter::addPage);
   connect(m_actionInsertPageAfter, &QAction::triggered, this, &QTabWidgetEventFilter::addPageAfter);
   connect(m_actionDeletePage,      &QAction::triggered, this, &QTabWidgetEventFilter::removeCurrentPage);
}

QTabWidgetEventFilter::~QTabWidgetEventFilter()
{
}

void QTabWidgetEventFilter::install(QTabWidget *tabWidget)
{
   new QTabWidgetEventFilter(tabWidget);
}

QTabWidgetEventFilter *QTabWidgetEventFilter::eventFilterOf(const QTabWidget *tabWidget)
{
   // Look for 1st order children only..otherwise, we might get filters of nested tab widgets
   const QObjectList children = tabWidget->children();
   const QObjectList::const_iterator cend = children.constEnd();
   for (QObjectList::const_iterator it = children.constBegin(); it != cend; ++it) {
      QObject *o = *it;
      if (!o->isWidgetType())
         if (QTabWidgetEventFilter *ef = dynamic_cast<QTabWidgetEventFilter *>(o)) {
            return ef;
         }
   }
   return nullptr;
}

QMenu *QTabWidgetEventFilter::addTabWidgetContextMenuActions(const QTabWidget *tabWidget, QMenu *popup)
{
   QTabWidgetEventFilter *filter = eventFilterOf(tabWidget);
   if (!filter) {
      return nullptr;
   }
   return filter->addContextMenuActions(popup);
}

QTabBar *QTabWidgetEventFilter::tabBar() const
{
   // QTabWidget::tabBar() accessor is protected, grmbl...
   if (!m_cachedTabBar) {
      const QList<QTabBar *> tabBars = m_tabWidget->findChildren<QTabBar *>();
      Q_ASSERT(tabBars.size() == 1);
      m_cachedTabBar = tabBars.front();
   }
   return m_cachedTabBar;

}

static bool canMove(const QPoint &pressPoint, const QMouseEvent *e)
{
   const QPoint pt = pressPoint - e->pos();
   return pt.manhattanLength() > QApplication::startDragDistance();
}

bool QTabWidgetEventFilter::eventFilter(QObject *o, QEvent *e)
{
   const QEvent::Type type = e->type();
   // Do not try to locate tab bar and form window, etc. for uninteresting events and
   // avoid asserts about missing tab bars when being destroyed
   switch (type) {
      case QEvent::MouseButtonDblClick:
      case QEvent::MouseButtonPress:
      case QEvent::MouseButtonRelease:
      case QEvent::MouseMove:
      case QEvent::DragLeave:
      case QEvent::DragEnter:
      case QEvent::DragMove:
      case QEvent::Drop:
         break;
      default:
         return false;
   }

   if (o != tabBar()) {
      return false;
   }

   QDesignerFormWindowInterface *fw = formWindow();

   if (! fw) {
      return false;
   }

   bool handled = true;

   switch (type) {
      case QEvent::MouseButtonDblClick:
         break;

      case QEvent::MouseButtonPress: {
         QMouseEvent *mev = static_cast<QMouseEvent *>(e);

         fw->clearSelection();
         fw->selectWidget(m_tabWidget, true);

         if (mev->button() & Qt::LeftButton) {
            m_mousePressed = true;
            m_pressPoint   = mev->pos();

            QTabBar *tabbar = tabBar();
            const int count = tabbar->count();

            for (int i = 0; i < count; ++i) {
               if (tabbar->tabRect(i).contains(m_pressPoint)) {
                  if (i != tabbar->currentIndex()) {
                     qdesigner_internal::SetPropertyCommand *cmd = new qdesigner_internal::SetPropertyCommand(fw);
                     cmd->init(m_tabWidget, QString("currentIndex"), i);
                     fw->commandHistory()->push(cmd);
                  }
                  break;
               }
            }
         }
      }
      break;

      case QEvent::MouseButtonRelease:
         m_mousePressed = false;
         break;

      case QEvent::MouseMove: {
         QMouseEvent *mouseEvent = static_cast<QMouseEvent *>(e);

         if (m_mousePressed && canMove(m_pressPoint, mouseEvent)) {
            const int index = m_tabWidget->currentIndex();
            if (index == -1) {
               break;
            }

            m_mousePressed = false;
            QDrag *drg = new QDrag(m_tabWidget);
            drg->setMimeData(new qdesigner_internal::MyMimeData(m_tabWidget));

            m_dragIndex = index;
            m_dragPage  = m_tabWidget->currentWidget();
            m_dragLabel = m_tabWidget->tabText(index);
            m_dragIcon  = m_tabWidget->tabIcon(index);

            if (m_dragIcon.isNull()) {
               QLabel *label = new QLabel(m_dragLabel);
               label->adjustSize();
               drg->setPixmap(label->grab(QRect(0, 0, -1, -1)));
               label->deleteLater();
            } else {
               drg->setPixmap(m_dragIcon.pixmap(22, 22));
            }

            m_tabWidget->removeTab(m_dragIndex);

            const Qt::DropActions dropAction = drg->start(Qt::MoveAction);

            if (dropAction == Qt::IgnoreAction) {
               // abort
               m_tabWidget->insertTab(m_dragIndex, m_dragPage, m_dragIcon, m_dragLabel);
               m_tabWidget->setCurrentIndex(m_dragIndex);
            }

            if (m_dropIndicator) {
               m_dropIndicator->hide();
            }
         }
      }
      break;

      case QEvent::DragLeave: {
         if (m_dropIndicator) {
            m_dropIndicator->hide();
         }
      }
      break;

      case QEvent::DragEnter:
      case QEvent::DragMove: {
         QDragMoveEvent *de = static_cast<QDragMoveEvent *>(e);
         if (!qdesigner_internal::MyMimeData::fromMyTab(de->mimeData(), m_tabWidget)) {
            return false;
         }

         if (de->proposedAction() == Qt::MoveAction) {
            de->acceptProposedAction();
         } else {
            de->setDropAction(Qt::MoveAction);
            de->accept();
         }

         QRect rect;
         const int index = pageFromPosition(de->pos(), rect);

         if (!m_dropIndicator) {
            m_dropIndicator = new QWidget(m_tabWidget);
            QPalette p = m_dropIndicator->palette();
            p.setColor(m_tabWidget->backgroundRole(), Qt::red);
            m_dropIndicator->setPalette(p);
         }

         QPoint pos;
         if (index == m_tabWidget->count()) {
            pos = tabBar()->mapToParent(QPoint(rect.x() + rect.width(), rect.y()));
         } else {
            pos = tabBar()->mapToParent(QPoint(rect.x(), rect.y()));
         }

         m_dropIndicator->setGeometry(pos.x(), pos.y(), 3, rect.height());
         m_dropIndicator->show();
      }
      break;

      case QEvent::Drop: {
         QDropEvent *de = static_cast<QDropEvent *>(e);
         if (!qdesigner_internal::MyMimeData::fromMyTab(de->mimeData(), m_tabWidget)) {
            return false;
         }
         de->acceptProposedAction();
         de->accept();

         QRect rect;
         const int newIndex = pageFromPosition(de->pos(), rect);

         qdesigner_internal::MoveTabPageCommand *cmd = new qdesigner_internal::MoveTabPageCommand(fw);
         m_tabWidget->insertTab(m_dragIndex, m_dragPage, m_dragIcon, m_dragLabel);
         cmd->init(m_tabWidget, m_dragPage, m_dragIcon, m_dragLabel, m_dragIndex, newIndex);
         fw->commandHistory()->push(cmd);
      }
      break;

      default:
         handled = false;
         break;
   }

   return handled;
}

void QTabWidgetEventFilter::removeCurrentPage()
{
   if (!m_tabWidget->currentWidget()) {
      return;
   }

   if (QDesignerFormWindowInterface *fw = formWindow()) {
      qdesigner_internal::DeleteTabPageCommand *cmd = new qdesigner_internal::DeleteTabPageCommand(fw);
      cmd->init(m_tabWidget);
      fw->commandHistory()->push(cmd);
   }
}

void QTabWidgetEventFilter::addPage()
{
   if (QDesignerFormWindowInterface *fw = formWindow()) {
      qdesigner_internal::AddTabPageCommand *cmd = new qdesigner_internal::AddTabPageCommand(fw);
      cmd->init(m_tabWidget, qdesigner_internal::AddTabPageCommand::InsertBefore);
      fw->commandHistory()->push(cmd);
   }
}

void QTabWidgetEventFilter::addPageAfter()
{
   if (QDesignerFormWindowInterface *fw = formWindow()) {
      qdesigner_internal::AddTabPageCommand *cmd = new qdesigner_internal::AddTabPageCommand(fw);
      cmd->init(m_tabWidget, qdesigner_internal::AddTabPageCommand::InsertAfter);
      fw->commandHistory()->push(cmd);
   }
}

QDesignerFormWindowInterface *QTabWidgetEventFilter::formWindow() const
{
   return QDesignerFormWindowInterface::findFormWindow(const_cast<QTabWidget *>(m_tabWidget));
}

// Get page from mouse position. Default to new page if in right half of last page?
int QTabWidgetEventFilter::pageFromPosition(const QPoint &pos, QRect &rect) const
{
   int index = 0;
   const QTabBar *tabbar = tabBar();
   const int count = m_tabWidget->count();

   for (; index < count; index++) {
      const QRect rc = tabbar->tabRect(index);
      if (rc.contains(pos)) {
         rect = rc;
         break;
      }
   }

   if (index == count - 1) {
      QRect rect2 = rect;
      rect2.setLeft(rect2.left() + rect2.width() / 2);
      if (rect2.contains(pos)) {
         ++index;
      }
   }
   return index;
}

QMenu *QTabWidgetEventFilter::addContextMenuActions(QMenu *popup)
{
   QMenu *pageMenu = nullptr;
   const int count = m_tabWidget->count();

   m_actionDeletePage->setEnabled(count);

   if (count) {
      const int currentIndex = m_tabWidget->currentIndex();
      const QString pageSubMenuLabel = tr("Page %1 of %2").formatArg(currentIndex + 1).formatArg(count);
      pageMenu = popup->addMenu(pageSubMenuLabel);
      pageMenu->addAction(m_actionDeletePage);

      // Set up promotion menu for current widget.
      if (QWidget *page =  m_tabWidget->currentWidget ()) {
         m_pagePromotionTaskMenu->setWidget(page);

         m_pagePromotionTaskMenu->addActions(QDesignerFormWindowInterface::findFormWindow(m_tabWidget),
            qdesigner_internal::PromotionTaskMenu::SuppressGlobalEdit, pageMenu);
      }

      QMenu *insertPageMenu = popup->addMenu(tr("Insert Page"));
      insertPageMenu->addAction(m_actionInsertPageAfter);
      insertPageMenu->addAction(m_actionInsertPage);

   } else {
      QAction *insertPageAction = popup->addAction(tr("Insert Page"));
      connect(insertPageAction, &QAction::triggered, this, &QTabWidgetEventFilter::addPage);
   }
   popup->addSeparator();
   return pageMenu;
}

// ----------- QTabWidgetPropertySheet

static const QString currentTabIconKey      = "currentTabIcon";
static const QString currentTabNameKey      = "currentTabName";
static const QString currentTabTextKey      = "currentTabText";
static const QString currentTabToolTipKey   = "currentTabToolTip";
static const QString currentTabWhatsThisKey = "currentTabWhatsThis";
static const QString tabMovableKey          = "movable";

QTabWidgetPropertySheet::QTabWidgetPropertySheet(QTabWidget *object, QObject *parent)
   : QDesignerPropertySheet(object, parent), m_tabWidget(object)
{
   createFakeProperty(currentTabTextKey, QVariant::fromValue(qdesigner_internal::PropertySheetStringValue()));
   createFakeProperty(currentTabNameKey, QString());
   createFakeProperty(currentTabIconKey, QVariant::fromValue(qdesigner_internal::PropertySheetIconValue()));

   if (formWindowBase()) {
      formWindowBase()->addReloadableProperty(this, indexOf(currentTabIconKey));
   }

   createFakeProperty(currentTabToolTipKey, QVariant::fromValue(qdesigner_internal::PropertySheetStringValue()));
   createFakeProperty(currentTabWhatsThisKey, QVariant::fromValue(qdesigner_internal::PropertySheetStringValue()));

   // Prevent the tab widget's drag and drop handling from interfering with Designer's
   createFakeProperty(tabMovableKey, QVariant(false));
}

QTabWidgetPropertySheet::TabWidgetProperty QTabWidgetPropertySheet::tabWidgetPropertyFromName(const QString &name)
{
   typedef QHash<QString, TabWidgetProperty> TabWidgetPropertyHash;
   static TabWidgetPropertyHash tabWidgetPropertyHash;
   if (tabWidgetPropertyHash.empty()) {
      tabWidgetPropertyHash.insert(currentTabTextKey,      PropertyCurrentTabText);
      tabWidgetPropertyHash.insert(currentTabNameKey,      PropertyCurrentTabName);
      tabWidgetPropertyHash.insert(currentTabIconKey,      PropertyCurrentTabIcon);
      tabWidgetPropertyHash.insert(currentTabToolTipKey,   PropertyCurrentTabToolTip);
      tabWidgetPropertyHash.insert(currentTabWhatsThisKey, PropertyCurrentTabWhatsThis);
   }
   return tabWidgetPropertyHash.value(name, PropertyTabWidgetNone);
}

void QTabWidgetPropertySheet::setProperty(int index, const QVariant &value)
{
   const TabWidgetProperty tabWidgetProperty = tabWidgetPropertyFromName(propertyName(index));
   if (tabWidgetProperty == PropertyTabWidgetNone) {
      QDesignerPropertySheet::setProperty(index, value);
      return;
   }

   // index-dependent
   const int currentIndex = m_tabWidget->currentIndex();
   QWidget *currentWidget = m_tabWidget->currentWidget();
   if (!currentWidget) {
      return;
   }

   switch (tabWidgetProperty) {
      case PropertyCurrentTabText: {
         QVariant data = resolvePropertyValue(index, value);
         m_tabWidget->setTabText(currentIndex, data.value<QString>());

         m_pageToData[currentWidget].text = value.value<qdesigner_internal::PropertySheetStringValue>();
         break;
      }

      case PropertyCurrentTabName:
         currentWidget->setObjectName(value.toString());
         break;

      case PropertyCurrentTabIcon: {
         QVariant data = resolvePropertyValue(index, value);
         m_tabWidget->setTabIcon(currentIndex, data.value<QIcon>());

         m_pageToData[currentWidget].icon = value.value<qdesigner_internal::PropertySheetIconValue>();
         break;
      }

      case PropertyCurrentTabToolTip: {
         QVariant data = resolvePropertyValue(index, value);
         m_tabWidget->setTabToolTip(currentIndex, data.value<QString>());

         m_pageToData[currentWidget].tooltip = value.value<qdesigner_internal::PropertySheetStringValue>();
         break;
      }

      case PropertyCurrentTabWhatsThis: {
         QVariant data = resolvePropertyValue(index, value);
         m_tabWidget->setTabWhatsThis(currentIndex, data.value<QString>());

         m_pageToData[currentWidget].whatsthis = value.value<qdesigner_internal::PropertySheetStringValue>();
         break;
      }

      case PropertyTabWidgetNone:
         break;
   }
}

bool QTabWidgetPropertySheet::isEnabled(int index) const
{
   if (tabWidgetPropertyFromName(propertyName(index)) == PropertyTabWidgetNone) {
      return QDesignerPropertySheet::isEnabled(index);
   }

   return m_tabWidget->currentIndex() != -1;
}

QVariant QTabWidgetPropertySheet::property(int index) const
{
   const TabWidgetProperty tabWidgetProperty = tabWidgetPropertyFromName(propertyName(index));
   if (tabWidgetProperty == PropertyTabWidgetNone) {
      return  QDesignerPropertySheet::property(index);
   }

   // index-dependent
   QWidget *currentWidget = m_tabWidget->currentWidget();
   if (!currentWidget) {
      if (tabWidgetProperty == PropertyCurrentTabIcon) {
         return QVariant::fromValue(qdesigner_internal::PropertySheetIconValue());
      }
      if (tabWidgetProperty == PropertyCurrentTabText) {
         return QVariant::fromValue(qdesigner_internal::PropertySheetStringValue());
      }
      if (tabWidgetProperty == PropertyCurrentTabToolTip) {
         return QVariant::fromValue(qdesigner_internal::PropertySheetStringValue());
      }
      if (tabWidgetProperty == PropertyCurrentTabWhatsThis) {
         return QVariant::fromValue(qdesigner_internal::PropertySheetStringValue());
      }
      return QVariant(QString());
   }

   // index-dependent
   switch (tabWidgetProperty) {
      case PropertyCurrentTabText:
         return QVariant::fromValue(m_pageToData.value(currentWidget).text);
      case PropertyCurrentTabName:
         return currentWidget->objectName();
      case PropertyCurrentTabIcon:
         return QVariant::fromValue(m_pageToData.value(currentWidget).icon);
      case PropertyCurrentTabToolTip:
         return QVariant::fromValue(m_pageToData.value(currentWidget).tooltip);
      case PropertyCurrentTabWhatsThis:
         return QVariant::fromValue(m_pageToData.value(currentWidget).whatsthis);
      case PropertyTabWidgetNone:
         break;
   }
   return QVariant();
}

bool QTabWidgetPropertySheet::reset(int index)
{
   const TabWidgetProperty tabWidgetProperty = tabWidgetPropertyFromName(propertyName(index));
   if (tabWidgetProperty == PropertyTabWidgetNone) {
      return QDesignerPropertySheet::reset(index);
   }

   // index-dependent
   QWidget *currentWidget = m_tabWidget->currentWidget();
   if (!currentWidget) {
      return false;
   }

   // index-dependent
   switch (tabWidgetProperty) {
      case PropertyCurrentTabName:
         setProperty(index, QString());
         break;
      case PropertyCurrentTabToolTip:
         m_pageToData[currentWidget].tooltip = qdesigner_internal::PropertySheetStringValue();
         setProperty(index, QString());
         break;
      case PropertyCurrentTabWhatsThis:
         m_pageToData[currentWidget].whatsthis = qdesigner_internal::PropertySheetStringValue();
         setProperty(index, QString());
         break;
      case PropertyCurrentTabText:
         m_pageToData[currentWidget].text = qdesigner_internal::PropertySheetStringValue();
         setProperty(index, QString());
         break;
      case PropertyCurrentTabIcon:
         m_pageToData[currentWidget].icon = qdesigner_internal::PropertySheetIconValue();
         setProperty(index, QIcon());
         break;
      case PropertyTabWidgetNone:
         break;
   }
   return true;
}

bool QTabWidgetPropertySheet::checkProperty(const QString &propertyName)
{
   switch (tabWidgetPropertyFromName(propertyName)) {
      case PropertyCurrentTabText:
      case PropertyCurrentTabName:
      case PropertyCurrentTabToolTip:
      case PropertyCurrentTabWhatsThis:
      case PropertyCurrentTabIcon:
         return false;
      default:
         break;
   }
   return true;
}
