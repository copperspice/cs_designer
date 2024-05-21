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

#include <abstract_formeditor.h>
#include <abstract_formwindowcursor.h>
#include <abstract_introspection.h>
#include <abstract_language.h>
#include <container.h>
#include <customwidget.h>
#include <designer_dockwidget.h>
#include <designer_menu.h>
#include <designer_menubar.h>
#include <designer_propertysheet.h>
#include <designer_stackedbox.h>
#include <designer_tabwidget.h>
#include <designer_toolbar.h>
#include <designer_toolbox.h>
#include <designer_utils.h>
#include <designer_widget.h>
#include <extension.h>
#include <layout.h>
#include <layout_info.h>
#include <layout_widget.h>
#include <table_classes.h>
#include <widgetfactory.h>

#include <formwindowbase_p.h>
#include <metadatabase_p.h>
#include <spacer_widget_p.h>
#include <widgetdatabase_p.h>

#include <QAbstractSpinBox>
#include <QApplication>
#include <QButtonGroup>
#include <QMdiSubWindow>
#include <QMetaObject>
#include <QSizeGrip>
#include <QStyle>
#include <QStyleFactory>

/* Dynamic boolean property indicating object was created by the factory
 * for the form editor. */

static const QString formEditorDynamicProperty = "_q_formEditorObject";

namespace qdesigner_internal {

// A friendly SpinBox that grants access to its QLineEdit
class FriendlySpinBox : public QAbstractSpinBox
{
 public:
   friend class WidgetFactory;
};

// An event filter for form-combo boxes that prevents the embedded line edit
// from getting edit focus (and drawing blue artifacts/lines). It catches the
// ChildPolished event when the "editable" property flips to true and the
// QLineEdit is created and turns off the LineEdit's focus policy.

class ComboEventFilter : public QObject
{
 public:
   explicit ComboEventFilter(QComboBox *parent) : QObject(parent) {}
   bool eventFilter(QObject *watched, QEvent *event) override;
};

bool ComboEventFilter::eventFilter(QObject *watched, QEvent *event)
{
   if (event->type() == QEvent::ChildPolished) {
      QComboBox *cb = static_cast<QComboBox *>(watched);
      if (QLineEdit *le = cb->lineEdit()) {
         le->setFocusPolicy(Qt::NoFocus);
         le->setCursor(Qt::ArrowCursor);
      }
   }

   return QObject::eventFilter(watched, event);
}

/* Watch out for QWizards changing their pages and make sure that not some
 * selected widget becomes invisible on a hidden page (causing the selection
 * handles to shine through). Select the wizard in that case  in analogy to
 * the QTabWidget event filters, etc. */

class WizardPageChangeWatcher : public QObject
{
   CS_OBJECT(WizardPageChangeWatcher)

 public:
   explicit WizardPageChangeWatcher(QWizard *parent);

   CS_SLOT_1(Public, void pageChanged())
   CS_SLOT_2(pageChanged)
};

WizardPageChangeWatcher::WizardPageChangeWatcher(QWizard *parent)
   : QObject(parent)
{
   connect(parent, &QWizard::currentIdChanged, this, &WizardPageChangeWatcher::pageChanged);
}

void WizardPageChangeWatcher::pageChanged()
{
   /* Use a bit more conservative approach than that for the QTabWidget,
    * change the selection only if a selected child becomes invisible by
    * changing the page. */

   QWizard *wizard = static_cast<QWizard *>(parent());
   QDesignerFormWindowInterface *fw = QDesignerFormWindowInterface::findFormWindow(wizard);

   if (!fw) {
      return;
   }

   QDesignerFormWindowCursorInterface *cursor = fw->cursor();
   const int selCount = cursor->selectedWidgetCount();
   for (int i = 0; i <  selCount; i++) {
      if (!cursor->selectedWidget(i)->isVisible()) {
         fw->clearSelection(false);
         fw->selectWidget(wizard, true);
         break;
      }
   }
}

WidgetFactory::Strings::Strings() :
   m_alignment(QString("alignment")),
   m_bottomMargin(QString("bottomMargin")),
   m_geometry(QString("geometry")),
   m_leftMargin(QString("leftMargin")),
   m_line(QString("Line")),
   m_objectName(QString("objectName")),
   m_spacerName(QString("spacerName")),
   m_orientation(QString("orientation")),
   m_qAction(QString("QAction")),
   m_qButtonGroup(QString("QButtonGroup")),
   m_qAxWidget(QString("QAxWidget")),
   m_qDialog(QString("QDialog")),
   m_qDockWidget(QString("QDockWidget")),
   m_qLayoutWidget(QString("QLayoutWidget")),
   m_qMenu(QString("QMenu")),
   m_qMenuBar(QString("QMenuBar")),
   m_qWidget(QString("QWidget")),
   m_rightMargin(QString("rightMargin")),
   m_sizeHint(QString("sizeHint")),
   m_spacer(QString("Spacer")),
   m_text(QString("text")),
   m_title(QString("title")),
   m_topMargin(QString("topMargin")),
   m_windowIcon(QString("windowIcon")),
   m_windowTitle(QString("windowTitle"))
{
}

QPointer<QWidget> *WidgetFactory::m_lastPassiveInteractor = new QPointer<QWidget>();
bool WidgetFactory::m_lastWasAPassiveInteractor = false;

const QString WidgetFactory::disableStyleCustomPaintingPropertyC = "_q_custom_style_disabled";

WidgetFactory::WidgetFactory(QDesignerFormEditorInterface *core, QObject *parent)
   : QDesignerWidgetFactoryInterface(parent), m_core(core),
     m_formWindow(nullptr), m_currentStyle(nullptr)
{
}

WidgetFactory::~WidgetFactory()
{
}

QDesignerFormWindowInterface *WidgetFactory::currentFormWindow(QDesignerFormWindowInterface *fw)
{
   QDesignerFormWindowInterface *was = m_formWindow;
   m_formWindow = fw;
   return was;
}

void WidgetFactory::loadPlugins()
{
   m_customFactory.clear();

   QDesignerPluginManager *pluginManager = m_core->pluginManager();

   QList<QDesignerCustomWidgetInterface *> lst = pluginManager->registeredCustomWidgets();
   for (QDesignerCustomWidgetInterface *c : lst) {
      m_customFactory.insert(c->name(), c);
   }
}

// Convencience to create non-widget objects. Returns 0 if unknown
QObject *WidgetFactory::createObject(const QString &className, QObject *parent) const
{
   if (className.isEmpty()) {
      return nullptr;
   }
   if (className == m_strings.m_qAction) {
      return new QAction(parent);
   }
   if (className == m_strings.m_qButtonGroup) {
      return new QButtonGroup(parent);
   }
   return nullptr;
}

// Check for mismatched class names in plugins, which is hard to track.
static bool classNameMatches(const QObject *created, const QString &className)
{
   if ((created->metaObject()->className() == className) || created->inherits(className)) {
      return true;
   }

   return false;
}

QWidget  *WidgetFactory::createCustomWidget(const QString &className, QWidget *parentWidget, bool *creationError) const
{
   *creationError = false;

   auto it = m_customFactory.constFind(className);

   if (it == m_customFactory.constEnd()) {
      return nullptr;
   }

   QDesignerCustomWidgetInterface *factory = it.value();
   QWidget *rc = factory->createWidget(parentWidget);

   // shouldn't happen
   if (!rc) {
      *creationError = true;
      csWarning(tr("Custom widget factory registered for widgets of class %1 returned nullptr").formatArg(className));
      return nullptr;
   }

   // Figure out the base class unless it is known
   static QSet<QString> knownCustomClasses;
   if (!knownCustomClasses.contains(className)) {
      QDesignerWidgetDataBaseInterface *wdb = m_core->widgetDataBase();
      const int widgetInfoIndex = wdb->indexOfObject(rc, false);

      if (widgetInfoIndex != -1) {
         if (wdb->item(widgetInfoIndex)->extends().isEmpty()) {
            const QDesignerMetaObjectInterface *mo = core()->introspection()->metaObject(rc)->superClass();
            // If we hit on a 'Q3DesignerXXWidget' that claims to be a 'Q3XXWidget', step
            // over.
            if (mo && mo->className() == className) {
               mo = mo->superClass();
            }
            while (mo != nullptr) {
               if (core()->widgetDataBase()->indexOfClassName(mo->className()) != -1) {
                  wdb->item(widgetInfoIndex)->setExtends(mo->className());
                  break;
               }
               mo = mo->superClass();
            }
         }

         knownCustomClasses.insert(className);
      }
   }

   // Since a language plugin may not accurately give its names like Jambi does,
   // return immediately here

   QDesignerLanguageExtension *lang = qt_extension<QDesignerLanguageExtension *>(m_core->extensionManager(), m_core);

   if (lang) {
      return rc;
   }

   // Check for mismatched class names which is hard to track.
   if (! classNameMatches(rc, className)) {
      csWarning(tr("Class name mismatch occurred when creating a widget using the custom widget factory "
         "registered for widgets of class %1, returned a widget of class %2.")
         .formatArgs(className, rc->metaObject()->className()));
   }

   return rc;
}

QWidget *WidgetFactory::createWidget(const QString &widgetName, QWidget *parentWidget) const
{
   if (widgetName.isEmpty()) {
      csWarning("createWidget was called with an empty class name");
      return nullptr;
   }

   // Preview or for form window?
   QDesignerFormWindowInterface *fw = m_formWindow;

   if (! fw) {
      fw = QDesignerFormWindowInterface::findFormWindow(parentWidget);
   }

   QWidget *w = nullptr;

   do {
      // 1) custom. If there is an explicit failure(factory wants to indicate something is wrong),
      //    return 0, do not try to find fallback, which might be worse in the case of Q3 widget.
      bool customWidgetCreationError;
      w = createCustomWidget(widgetName, parentWidget, &customWidgetCreationError);

      if (w) {
         break;

      } else {
         if (customWidgetCreationError) {
            return nullptr;
         }
      }

      // 2) Special widgets
      if (widgetName == m_strings.m_line) {
         w = new Line(parentWidget);

      } else if (widgetName == m_strings.m_qDockWidget) {
         w = new QDesignerDockWidget(parentWidget);

      } else if (widgetName == m_strings.m_qMenuBar) {
         w = new QDesignerMenuBar(parentWidget);

      } else if (widgetName == m_strings.m_qMenu) {
         w = new QDesignerMenu(parentWidget);

      } else if (widgetName == m_strings.m_spacer) {
         w = new Spacer(parentWidget);

      } else if (widgetName == m_strings.m_qLayoutWidget) {
         w = fw ? new QLayoutWidget(fw, parentWidget) : new QWidget(parentWidget);

      } else if (widgetName == m_strings.m_qDialog) {
         if (fw) {
            w = new QDesignerDialog(fw, parentWidget);
         } else {
            w = new QDialog(parentWidget);
         }

      } else if (widgetName == m_strings.m_qWidget) {
         /* We want a 'QDesignerWidget' that draws a grid only for widget
          * forms and container extension pages (not for preview and not
          * for normal QWidget children on forms (legacy) */

         if (fw && parentWidget) {
            if (qt_extension<QDesignerContainerExtension *>(m_core->extensionManager(), parentWidget)) {
               w = new QDesignerWidget(fw, parentWidget);

            } else {
               if (parentWidget == fw->formContainer()) {
                  w = new QDesignerWidget(fw, parentWidget);
               }
            }
         }

         if (! w) {
            w = new QWidget(parentWidget);
         }
      }

      if (w != nullptr) {
         break;
      }

      // 3) table

      if (w != nullptr) {
         // symmetry for macro
      }

#define DECLARE_LAYOUT(L, C)
#define DECLARE_WIDGET(W, C)   else if (widgetName == #W) { Q_ASSERT(w == nullptr); w = new W(parentWidget); }
#define DECLARE_WIDGET_1(W, C) else if (widgetName == #W) { Q_ASSERT(w == nullptr); w = new W(nullptr, parentWidget); }

#include <widgets.table>

#undef DECLARE_LAYOUT
#undef DECLARE_WIDGET
#undef DECLARE_WIDGET_1

      if (w != nullptr) {
         break;
      }

      // 4) fallBack
      const QString fallBackBaseClass = m_strings.m_qWidget;
      QDesignerWidgetDataBaseInterface *db = core()->widgetDataBase();
      QDesignerWidgetDataBaseItemInterface *item = db->item(db->indexOfClassName(widgetName));

      if (item == nullptr) {
         // Emergency: Create, derived from QWidget
         QString includeFile = widgetName.toLower();
         includeFile += ".h";

         item = appendDerived(db, widgetName, tr("%1 Widget").formatArg(widgetName), fallBackBaseClass,
               includeFile, true, true);

         Q_ASSERT(item);
      }

      QString baseClass = item->extends();

      if (baseClass.isEmpty()) {
         baseClass = fallBackBaseClass;
      }

      if (QWidget *promotedWidget = createWidget(baseClass, parentWidget)) {
         promoteWidget(core(), promotedWidget, widgetName);
         return promotedWidget;  // Do not initialize twice.
      }

   } while (false);

   Q_ASSERT(w != nullptr);
   if (m_currentStyle) {
      w->setStyle(m_currentStyle);
   }

   initializeCommon(w);

   if (fw) {
      // form editor  initialization
      initialize(w);

   } else {
      // preview-only initialization
      initializePreview(w);
   }

   return w;
}

QString WidgetFactory::classNameOf(QDesignerFormEditorInterface *c, const QObject *o)
{
   if (o == nullptr) {
      return QString();
   }

   const QString className = o->metaObject()->className();

   if (! o->isWidgetType()) {
      return className;
   }

   const QWidget *w = static_cast<const QWidget *>(o);

   // check promoted before designer special
   const QString customClassName = promotedCustomClassName(c, const_cast<QWidget *>(w));

   if (!customClassName.isEmpty()) {
      return customClassName;
   }

   if (dynamic_cast<const QDesignerMenuBar *>(w)) {
      return QString("QMenuBar");
   }

   else if (dynamic_cast<const QDesignerMenu *>(w)) {
      return QString("QMenu");
   }

   else if (dynamic_cast<const QDesignerDockWidget *>(w)) {
      return QString("QDockWidget");
   }

   else if (dynamic_cast<const QDesignerDialog *>(w)) {
      return QString("QDialog");
   }

   else if (dynamic_cast<const QDesignerWidget *>(w)) {
      return QString("QWidget");
   }

   return className;
}

QLayout *WidgetFactory::createUnmanagedLayout(QWidget *parentWidget, int type)
{
   switch (type) {
      case LayoutInfo::HBox:
         return new QHBoxLayout(parentWidget);

      case LayoutInfo::VBox:
         return new QVBoxLayout(parentWidget);

      case LayoutInfo::Grid:
         return new QGridLayout(parentWidget);

      case LayoutInfo::Form:
         return new QFormLayout(parentWidget);

      default:
         Q_ASSERT(false);
         break;
   }

   return nullptr;
}

QLayout *WidgetFactory::createLayout(QWidget *widget, QLayout *parentLayout, int type) const
{
   QDesignerMetaDataBaseInterface *metaDataBase = core()->metaDataBase();

   if (parentLayout == nullptr) {
      QWidget *page = containerOfWidget(widget);

      if (page) {
         widget = page;

      } else {
         const QString msg = tr("The current page of the container '%1' (%2) could not be determined while creating a layout."
               "This indicates an inconsistency in the ui file, probably a layout being constructed on a container widget.")
               .formatArg(widget->objectName()).formatArg(classNameOf(core(), widget));

         csWarning(msg);
      }
   }

   Q_ASSERT(metaDataBase->item(widget) != nullptr);    // ensure the widget is managed

   if (parentLayout == nullptr) {
      auto tmpLayout = widget->layout();

      if (metaDataBase->item(tmpLayout) == nullptr) {
         parentLayout = tmpLayout;
      }
   }

   QWidget *parentWidget = nullptr;

   if (parentLayout == nullptr) {
      parentWidget = widget;
   }

   QLayout *layout = createUnmanagedLayout(parentWidget, type);
   metaDataBase->add(layout);       // add the layout in the MetaDataBase

   QDesignerPropertySheetExtension *sheet = qt_extension<QDesignerPropertySheetExtension *>(core()->extensionManager(), layout);

   if (sheet) {
      sheet->setChanged(sheet->indexOf(m_strings.m_objectName), true);

      if (widget->inherits("QLayoutWidget")) {
         sheet->setProperty(sheet->indexOf(m_strings.m_leftMargin),   0);
         sheet->setProperty(sheet->indexOf(m_strings.m_topMargin),    0);
         sheet->setProperty(sheet->indexOf(m_strings.m_rightMargin),  0);
         sheet->setProperty(sheet->indexOf(m_strings.m_bottomMargin), 0);
      }

      const int index = sheet->indexOf(m_strings.m_alignment);
      if (index != -1) {
         sheet->setChanged(index, true);
      }
   }

   if (metaDataBase->item(widget->layout()) == nullptr) {
      Q_ASSERT(layout->parent() == nullptr);

      QBoxLayout *box = dynamic_cast<QBoxLayout *>(widget->layout());

      if (! box) {
         // supports only unmanaged box layouts
         const QString msg = tr("Attempt to add a layout to a widget '%1' (%2) which already has an unmanaged "
               "layout of type %3.\nThis indicates an inconsistency in the ui file.")
               .formatArg(widget->objectName()).formatArg(classNameOf(core(), widget))
               .formatArg(classNameOf(core(), widget->layout()));

         csWarning(msg);

         return nullptr;
      }

      box->addLayout(layout);
   }

   return layout;
}

QWidget *WidgetFactory::containerOfWidget(QWidget *w) const
{
   if (QDesignerContainerExtension *container = qt_extension<QDesignerContainerExtension *>(core()->extensionManager(), w)) {
      return container->widget(container->currentIndex());
   }

   return w;
}

QWidget *WidgetFactory::widgetOfContainer(QWidget *w) const
{
   // ### cleanup
   if (!w) {
      return nullptr;
   }

   if (w->parentWidget() && w->parentWidget()->parentWidget() &&
      w->parentWidget()->parentWidget()->parentWidget() &&
      dynamic_cast<QToolBox *>(w->parentWidget()->parentWidget()->parentWidget())) {
      return w->parentWidget()->parentWidget()->parentWidget();
   }

   while (w != nullptr) {
      if (core()->widgetDataBase()->isContainer(w) ||
         (w && dynamic_cast<QDesignerFormWindowInterface *>(w->parentWidget()))) {
         return w;
      }

      w = w->parentWidget();
   }

   return w;
}

QDesignerFormEditorInterface *WidgetFactory::core() const
{
   return m_core;
}

// Necessary initializations for form editor/preview objects
void WidgetFactory::initializeCommon(QWidget *widget) const
{
   // Apply style
   if (m_currentStyle) {
      widget->setStyle(m_currentStyle);
   }
}

// Necessary initializations for preview objects
void WidgetFactory::initializePreview(QWidget *widget) const
{

   if (QStackedWidget *stackedWidget = dynamic_cast<QStackedWidget *>(widget)) {
      QStackedWidgetPreviewEventFilter::install(stackedWidget); // Add browse button only.
      return;
   }
}

// Necessary initializations for form editor objects
void WidgetFactory::initialize(QObject *object) const
{
   // Indicate that this is a form object (for QDesignerFormWindowInterface::findFormWindow)
   object->setProperty(formEditorDynamicProperty, QVariant(true));
   QDesignerPropertySheetExtension *sheet = qt_extension<QDesignerPropertySheetExtension *>(m_core->extensionManager(), object);
   if (!sheet) {
      return;
   }

   sheet->setChanged(sheet->indexOf(m_strings.m_objectName), true);

   if (!object->isWidgetType()) {
      if (dynamic_cast<QAction *>(object)) {
         sheet->setChanged(sheet->indexOf(m_strings.m_text), true);
      }
      return;
   }

   QWidget *widget = static_cast<QWidget *>(object);
   const bool isMenu = dynamic_cast<QMenu *>(widget);
   const bool isMenuBar = !isMenu && dynamic_cast<QMenuBar *>(widget);

   widget->setAttribute(Qt::WA_TransparentForMouseEvents, false);
   widget->setFocusPolicy((isMenu || isMenuBar) ? Qt::StrongFocus : Qt::NoFocus);

   if (!isMenu) {
      sheet->setChanged(sheet->indexOf(m_strings.m_geometry), true);
   }

   if (dynamic_cast<Spacer *>(widget)) {
      sheet->setChanged(sheet->indexOf(m_strings.m_spacerName), true);
      return;
   }

   const int o = sheet->indexOf(m_strings.m_orientation);
   if (o != -1 && widget->inherits("QSplitter")) {
      sheet->setChanged(o, true);
   }

   if (QToolBar *toolBar = dynamic_cast<QToolBar *>(widget)) {
      ToolBarEventFilter::install(toolBar);
      sheet->setVisible(sheet->indexOf(m_strings.m_windowTitle), true);
      toolBar->setFloatable(false);  // prevent toolbars from being dragged off
      return;
   }

   if (dynamic_cast<QDockWidget *>(widget)) {
      sheet->setVisible(sheet->indexOf(m_strings.m_windowTitle), true);
      sheet->setVisible(sheet->indexOf(m_strings.m_windowIcon), true);
      return;
   }

   if (isMenu) {
      sheet->setChanged(sheet->indexOf(m_strings.m_title), true);
      return;
   }

   // helpers
   if (QToolBox *toolBox = dynamic_cast<QToolBox *>(widget)) {
      QToolBoxHelper::install(toolBox);
      return;
   }

   if (QStackedWidget *stackedWidget = dynamic_cast<QStackedWidget *>(widget)) {
      QStackedWidgetEventFilter::install(stackedWidget);
      return;
   }

   if (QTabWidget *tabWidget = dynamic_cast<QTabWidget *>(widget)) {
      QTabWidgetEventFilter::install(tabWidget);
      return;
   }

   // Prevent embedded line edits from getting focus
   if (QAbstractSpinBox *asb = dynamic_cast<QAbstractSpinBox *>(widget)) {
      if (QLineEdit *lineEdit = static_cast<FriendlySpinBox *>(asb)->lineEdit()) {
         lineEdit->setFocusPolicy(Qt::NoFocus);
      }
      return;
   }
   if (QComboBox *cb =  dynamic_cast<QComboBox *>(widget)) {
      if (QFontComboBox *fcb =  dynamic_cast<QFontComboBox *>(widget)) {
         fcb->lineEdit()->setFocusPolicy(Qt::NoFocus); // Always present
         return;
      }
      cb->installEventFilter(new ComboEventFilter(cb));
      return;
   }
   if (QWizard *wz = dynamic_cast<QWizard *>(widget)) {
      WizardPageChangeWatcher *pw = new WizardPageChangeWatcher(wz);
      (void) pw;
   }
}

static inline QString classNameOfStyle(const QStyle *s)
{
   return s->metaObject()->className();
}

QString WidgetFactory::styleName() const
{
   return classNameOfStyle(style());
}

static inline bool isApplicationStyle(const QString &styleName)
{
   return styleName.isEmpty() || styleName == classNameOfStyle(qApp->style());
}

void WidgetFactory::setStyleName(const QString &styleName)
{
   m_currentStyle = isApplicationStyle(styleName) ? nullptr : getStyle(styleName);
}

QStyle *WidgetFactory::style() const
{
   return m_currentStyle ? m_currentStyle : qApp->style();
}

QStyle *WidgetFactory::getStyle(const QString &styleName)
{
   if (isApplicationStyle(styleName)) {
      return qApp->style();
   }

   StyleCache::iterator it = m_styleCache.find(styleName);
   if (it == m_styleCache.end()) {
      QStyle *style = QStyleFactory::create(styleName);
      if (!style) {
         const QString msg = tr("Cannot create style '%1'.").formatArg(styleName);
         csWarning(msg);
         return nullptr;
      }
      it = m_styleCache.insert(styleName, style);
   }
   return it.value();
}

void WidgetFactory::applyStyleTopLevel(const QString &styleName, QWidget *w)
{
   if (QStyle *style = getStyle(styleName)) {
      applyStyleToTopLevel(style, w);
   }
}

void WidgetFactory::applyStyleToTopLevel(QStyle *style, QWidget *widget)
{
   if (!style) {
      return;
   }
   const QPalette standardPalette = style->standardPalette();
   if (widget->style() == style && widget->palette() == standardPalette) {
      return;
   }

   widget->setStyle(style);
   widget->setPalette(standardPalette);
   const QWidgetList lst = widget->findChildren<QWidget *>();
   const QWidgetList::const_iterator cend = lst.constEnd();
   for (QWidgetList::const_iterator it = lst.constBegin(); it != cend; ++it) {
      (*it)->setStyle(style);
   }
}

// Check for 'interactor' click on a tab bar,
// which can appear within a QTabWidget or as a standalone widget.

static bool isTabBarInteractor(const QTabBar *tabBar)
{
   // Tabbar embedded in Q(Designer)TabWidget, ie, normal tab widget case
   if (dynamic_cast<const QTabWidget *>(tabBar->parentWidget())) {
      return true;
   }

   // Standalone tab bar on the form. Return true for tab rect areas
   // only to allow the user to select the tab bar by clicking outside the actual tabs.
   const int count = tabBar->count();
   if (count == 0) {
      return false;
   }

   // click into current tab: No Interaction
   const int currentIndex = tabBar->currentIndex();
   const QPoint pos = tabBar->mapFromGlobal(QCursor::pos());
   if (tabBar->tabRect(currentIndex).contains(pos)) {
      return false;
   }

   // click outside: No Interaction
   const QRect geometry = QRect(QPoint(0, 0), tabBar->size());
   if (!geometry.contains(pos)) {
      return false;
   }
   // click into another tab: Let's interact, switch tabs.
   for (int i = 0; i < count; i++)
      if (tabBar->tabRect(i).contains(pos)) {
         return true;
      }
   return false;
}

bool WidgetFactory::isPassiveInteractor(QWidget *widget)
{
   static const QString qtPassive            = "__qt__passive_";
   static const QString qtMainWindowSplitter = "qt_qmainwindow_extended_splitter";

   if (m_lastPassiveInteractor != nullptr && (QWidget *)(*m_lastPassiveInteractor) == widget) {
      return m_lastWasAPassiveInteractor;
   }

   if (QApplication::activePopupWidget() || widget == nullptr) {
      // if a popup is open, we have to make sure that this one is closed, else X might do funny things
      return true;
   }

   m_lastWasAPassiveInteractor = false;
   (*m_lastPassiveInteractor) = widget;

   if (const QTabBar *tabBar = dynamic_cast<const QTabBar *>(widget)) {
      if (isTabBarInteractor(tabBar)) {
         m_lastWasAPassiveInteractor = true;
      }

      return m_lastWasAPassiveInteractor;

   }  else if (dynamic_cast<QSizeGrip *>(widget)) {
      return (m_lastWasAPassiveInteractor = true);

   }  else if (dynamic_cast<QMdiSubWindow *>(widget)) {
      return (m_lastWasAPassiveInteractor = true);
   }

   else if (dynamic_cast<QAbstractButton *>(widget) && (dynamic_cast<QTabBar *>(widget->parent()) ||
         dynamic_cast<QToolBox *>(widget->parent()))) {
      return (m_lastWasAPassiveInteractor = true);
   }

   else if (dynamic_cast<QMenuBar *>(widget)) {
      return (m_lastWasAPassiveInteractor = true);
   }

   else if (dynamic_cast<QToolBar *>(widget)) {
      return (m_lastWasAPassiveInteractor = true);
   }

   else if (dynamic_cast<QScrollBar *>(widget)) {
      // A scroll bar is an interactor on a QAbstractScrollArea only.
      if (const QWidget *parent = widget->parentWidget()) {
         const QString objectName = parent->objectName();

         static const QString scrollAreaVContainer = QString("qt_scrollarea_vcontainer");
         static const QString scrollAreaHContainer = QString("qt_scrollarea_hcontainer");

         if (objectName == scrollAreaVContainer || objectName == scrollAreaHContainer) {
            m_lastWasAPassiveInteractor = true;
            return m_lastWasAPassiveInteractor;
         }
      }

   } else if (widget->metaObject()->className() == "QDockWidgetTitle") {
      return (m_lastWasAPassiveInteractor = true);
   }

   else if (widget->metaObject()->className() == "QWorkspaceTitleBar") {
      return (m_lastWasAPassiveInteractor = true);
   }

   const QString name = widget->objectName();

   if (name.startsWith(qtPassive) || name == qtMainWindowSplitter) {
      m_lastWasAPassiveInteractor = true;
      return m_lastWasAPassiveInteractor;
   }

   return m_lastWasAPassiveInteractor;
}

void WidgetFactory::formWindowAdded(QDesignerFormWindowInterface *formWindow)
{
   setFormWindowStyle(formWindow);
}

void WidgetFactory::activeFormWindowChanged(QDesignerFormWindowInterface *formWindow)
{
   setFormWindowStyle(formWindow);
}

void WidgetFactory::setFormWindowStyle(QDesignerFormWindowInterface *formWindow)
{
   if (FormWindowBase *fwb = dynamic_cast<FormWindowBase *>(formWindow)) {
      setStyleName(fwb->styleName());
   }
}

bool WidgetFactory::isFormEditorObject(const QObject *object)
{
   return object->property(formEditorDynamicProperty).isValid();
}
} // namespace qdesigner_internal

