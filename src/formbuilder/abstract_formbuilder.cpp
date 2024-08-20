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

#include <abstract_formbuilder.h>
#include <formbuilderextra.h>
#include <properties.h>
#include <resourcebuilder.h>
#include <textbuilder.h>
#include <ui4.h>

#include <QAction>
#include <QBoxLayout>
#include <QButtonGroup>
#include <QCoreApplication>
#include <QDir>
#include <QDockWidget>
#include <QFileInfo>
#include <QFontComboBox>
#include <QFormLayout>
#include <QGridLayout>
#include <QHash>
#include <QHeaderView>
#include <QLayout>
#include <QListWidget>
#include <QMainWindow>
#include <QMdiArea>
#include <QMenu>
#include <QMenuBar>
#include <QMetaProperty>
#include <QPair>
#include <QPushButton>
#include <QQueue>
#include <QScrollArea>
#include <QSplitter>
#include <QStackedWidget>
#include <QStatusBar>
#include <QTabWidget>
#include <QTableWidget>
#include <QToolBar>
#include <QToolBox>
#include <QTreeWidget>
#include <QVariant>
#include <QWizard>
#include <QXmlStreamReader>

#include <limits.h>

CS_DECLARE_METATYPE(QWidgetList)

static const QString buttonGroupPropertyC = "buttonGroup";

class QFriendlyLayout: public QLayout
{
 public:
   QFriendlyLayout() {
      Q_ASSERT(false);
   }

   friend class QAbstractFormBuilder;
};

QAbstractFormBuilder::QAbstractFormBuilder() : d(new QFormBuilderExtra)
{
   setResourceBuilder(new QResourceBuilder());
   setTextBuilder(new QTextBuilder());
}

QAbstractFormBuilder::~QAbstractFormBuilder()
{
}

// Return UI file version from attribute 'version="4.0"'
static QPair<int, int> uiVersion(const QString &attr)
{
   const QStringList versions = attr.split('.', QStringParser::SkipEmptyParts);

   if (versions.size() >= 2) {
      bool okMajor, okMinor;
      const int majorVersion = versions.at(0).toInteger<int>(&okMajor);
      const int minorVersion = versions.at(1).toInteger<int>(&okMinor);

      if (okMajor &&  okMinor) {
         return QPair<int, int>(majorVersion, minorVersion);
      }
   }

   return QPair<int, int>(-1, -1);
}

static inline QString msgXmlError(const QXmlStreamReader &reader)
{
   return QCoreApplication::translate("QAbstractFormBuilder", "An error has occurred while reading the UI file at line %1, column %2: %3")
      .formatArg(reader.lineNumber()).formatArg(reader.columnNumber()).formatArg(reader.errorString());
}

// Read and check the  version and the (optional) language attribute
// of an <ui> element and leave reader positioned at <ui>.
static bool inline readUiAttributes(QXmlStreamReader &reader, const QString &language, QString *errorMessage)
{
   const QString uiElement = "ui";

   // Read up to first element
   while (!reader.atEnd()) {
      switch (reader.readNext()) {
         case QXmlStreamReader::Invalid:
            *errorMessage = msgXmlError(reader);
            return false;

         case QXmlStreamReader::StartElement:
            if (reader.name().compare(uiElement, Qt::CaseInsensitive) == 0) {
               const QString versionAttribute  = "version";
               const QString languageAttribute = "language";
               const QXmlStreamAttributes attributes = reader.attributes();

               if (attributes.hasAttribute(versionAttribute)) {
                  const QString versionString = attributes.value(versionAttribute).toString();

                  if (uiVersion(versionString).first < 4) {
                     *errorMessage = QCoreApplication::translate("QAbstractFormBuilder",
                           "This file was created using Designer from CS-%1 and can not be read.")
                           .formatArg(versionString);
                     return false;

                  } // version error

               }     // has version
               if (attributes.hasAttribute(languageAttribute)) {
                  // Check on optional language (Jambi)
                  const QString formLanguage = attributes.value(languageAttribute).toString();

                  if (!formLanguage.isEmpty() && formLanguage.compare(language, Qt::CaseInsensitive)) {
                     *errorMessage = QCoreApplication::translate("QAbstractFormBuilder",
                           "This file can not be read because it was created using %1.").formatArg(formLanguage);
                     return false;
                  } // language error
               }    // has language

               return true;
            }  // <ui> matched
            break;
         default:
            break;
      }
   }

   // No <ui> found

   *errorMessage = QCoreApplication::translate("QAbstractFormBuilder",  "Invalid UI file: The root element <ui> is missing.");

   return false;
}

QWidget *QAbstractFormBuilder::load(QIODevice *dev, QWidget *parentWidget)
{
   QXmlStreamReader reader(dev);
   d->m_errorString.clear();

   if (! readUiAttributes(reader, d->m_language, &d->m_errorString)) {
      csWarning(d->m_errorString);
      return nullptr;
   }

   DomUI ui;
   ui.read(reader);

   if (reader.hasError()) {
      d->m_errorString = msgXmlError(reader);
      csWarning(d->m_errorString);
      return nullptr;
   }

   QWidget *widget = create(&ui, parentWidget);

   if (! widget && d->m_errorString.isEmpty())  {
      d->m_errorString = QCoreApplication::translate("QAbstractFormBuilder", "Invalid UI file");
   }

   return widget;
}

QWidget *QAbstractFormBuilder::create(DomUI *ui, QWidget *parentWidget)
{
   typedef QFormBuilderExtra::ButtonGroupHash ButtonGroupHash;

   d->clear();
   if (const DomLayoutDefault *def = ui->elementLayoutDefault()) {
      d->m_defaultMargin  = def->hasAttributeMargin() ? def->attributeMargin() : INT_MIN;
      d->m_defaultSpacing = def->hasAttributeSpacing() ? def->attributeSpacing() : INT_MIN;
   }

   DomWidget *ui_widget = ui->elementWidget();
   if (! ui_widget) {
      return nullptr;
   }

   initialize(ui);

   if (const DomButtonGroups *domButtonGroups = ui->elementButtonGroups()) {
      d->registerButtonGroups(domButtonGroups);
   }

   if (QWidget *widget = create(ui_widget, parentWidget)) {
      // Reparent button groups that were actually created to main container for them to be found in the signal/slot part
      const ButtonGroupHash &buttonGroups = d->buttonGroups();

      if (!buttonGroups.empty()) {
         const ButtonGroupHash::const_iterator cend = buttonGroups.constEnd();
         for (ButtonGroupHash::const_iterator it = buttonGroups.constBegin(); it != cend; ++it)
            if (it.value().second) {
               it.value().second->setParent(widget);
            }
      }

      createConnections(ui->elementConnections(), widget);
      createResources(ui->elementResources()); // maybe this should go first, before create()...
      applyTabStops(widget, ui->elementTabStops());
      d->applyInternalProperties();
      reset();
      d->clear();

      return widget;
   }

   d->clear();

   return nullptr;
}

void QAbstractFormBuilder::initialize(const DomUI *ui)
{
   typedef QList<DomCustomWidget *> DomCustomWidgetList;

   DomCustomWidgets *domCustomWidgets  = ui->elementCustomWidgets();
   createCustomWidgets(domCustomWidgets);

   if (domCustomWidgets) {
      const DomCustomWidgetList customWidgets = domCustomWidgets->elementCustomWidget();
      if (!customWidgets.empty()) {
         const DomCustomWidgetList::const_iterator cend = customWidgets.constEnd();
         for (DomCustomWidgetList::const_iterator it = customWidgets.constBegin(); it != cend; ++it) {
            d->storeCustomWidgetData((*it)->elementClass(), *it);
         }
      }
   }
}

QWidget *QAbstractFormBuilder::create(DomWidget *ui_widget, QWidget *parentWidget)
{
   QWidget *w = createWidget(ui_widget->attributeClass(), parentWidget, ui_widget->attributeName());

   if (! w) {
      return nullptr;
   }

   applyProperties(w, ui_widget->elementProperty());

   for (DomAction *ui_action : ui_widget->elementAction()) {
      QAction *child_action = create(ui_action, w);
      (void) child_action;
   }

   for (DomActionGroup *ui_action_group : ui_widget->elementActionGroup()) {
      QActionGroup *child_action_group = create(ui_action_group, w);
      (void) child_action_group;
   }

   QWidgetList children;
   for (DomWidget *ui_child : ui_widget->elementWidget()) {
      if (QWidget *child  = create(ui_child, w)) {
         children += child;

      } else {
         const QString className = ui_child->elementClass().empty() ? QString() : ui_child->elementClass().front();
         csWarning(QCoreApplication::translate("QAbstractFormBuilder",
               "The creation of a widget of the class '%1' failed.").formatArg(className));
      }
   }

   for (DomLayout *ui_lay : ui_widget->elementLayout()) {
      QLayout *child_lay = create(ui_lay, nullptr, w);
      (void) child_lay;
   }

   const QList<DomActionRef *> addActions = ui_widget->elementAddAction();

   if (!addActions.empty()) {
      const QFormBuilderStrings &strings = QFormBuilderStrings::instance();

      for (DomActionRef *ui_action_ref : addActions) {
         const QString name = ui_action_ref->attributeName();

         if (name == strings.separator) {
            QAction *sep = new QAction(w);
            sep->setSeparator(true);
            w->addAction(sep);
            addMenuAction(sep);

         } else if (QAction *a = d->m_actions.value(name)) {
            w->addAction(a);

         } else if (QActionGroup *g = d->m_actionGroups.value(name)) {
            w->addActions(g->actions());

         } else if (QMenu *menu = w->findChild<QMenu *>(name)) {
            w->addAction(menu->menuAction());
            addMenuAction(menu->menuAction());
         }
      }
   }

   loadExtraInfo(ui_widget, w, parentWidget);
   addItem(ui_widget, w, parentWidget);

   if (dynamic_cast<QDialog *>(w) && parentWidget) {
      w->setAttribute(Qt::WA_Moved, false);   // so QDialog::setVisible(true) will center it
   }

   const QStringList zOrderNames = ui_widget->elementZOrder();

   if (! zOrderNames.isEmpty()) {
      QVariant data = w->property("_q_zOrder");
      QList<QWidget *> zOrder = data.value<QWidgetList>();

      for (const QString &widgetName : zOrderNames) {

         if (QWidget *child = w->findChild<QWidget *>(widgetName)) {
            if (child->parentWidget() == w) {
               zOrder.removeAll(child);
               zOrder.append(child);
               child->raise();
            }
         }
      }

      w->setProperty("_q_zOrder", QVariant::fromValue(zOrder));
   }

   return w;
}

QAction *QAbstractFormBuilder::create(DomAction *ui_action, QObject *parent)
{
   QAction *a = createAction(parent, ui_action->attributeName());
   if (!a) {
      return nullptr;
   }

   d->m_actions.insert(ui_action->attributeName(), a);
   applyProperties(a, ui_action->elementProperty());
   return a;
}

QActionGroup *QAbstractFormBuilder::create(DomActionGroup *ui_action_group, QObject *parent)
{
   QActionGroup *a = createActionGroup(parent, ui_action_group->attributeName());
   if (!a) {
      return nullptr;
   }
   d->m_actionGroups.insert(ui_action_group->attributeName(), a);
   applyProperties(a, ui_action_group->elementProperty());

   for (DomAction *ui_action : ui_action_group->elementAction()) {
      QAction *child_action = create(ui_action, a);
      (void) child_action;
   }

   for (DomActionGroup *g : ui_action_group->elementActionGroup()) {
      QActionGroup *child_action_group = create(g, parent);
      (void) child_action_group;
   }

   return a;
}

// figure out the toolbar area of a DOM attrib list
// for newer files it is the enumeration value
Qt::ToolBarArea QAbstractFormBuilder::toolbarAreaFromDOMAttributes(const DomPropertyHash &attributes)
{
   const DomProperty *attr = attributes.value(QFormBuilderStrings::instance().toolBarAreaAttribute);
   if (! attr) {
      return Qt::TopToolBarArea;
   }

   switch (attr->kind()) {
      case DomProperty::Number:
         // legacy
         return static_cast<Qt::ToolBarArea>(attr->elementNumber());

      case DomProperty::Enum:
         return enumKeyOfObjectToValue<QAbstractFormBuilderGadget, Qt::ToolBarArea>("toolBarArea",  attr->elementEnum().toLatin1());

      default:
         break;
   }

   return Qt::TopToolBarArea;
}

bool QAbstractFormBuilder::addItem(DomWidget *ui_widget, QWidget *widget, QWidget *parentWidget)
{
   const QFormBuilderStrings &strings = QFormBuilderStrings::instance();
   const DomPropertyHash attributes = propertyMap(ui_widget->elementAttribute());

   if (parentWidget == nullptr) {
      return true;
   }

   // Check special cases. First: Custom container
   const QString className     = parentWidget->metaObject()->className();
   const QString addPageMethod = d->customWidgetAddPageMethod(className);

   if (!addPageMethod.isEmpty()) {
      // If this fails ( non-existent or non-slot), use ContainerExtension in Designer, else it can't be helped
      return QMetaObject::invokeMethod(parentWidget, addPageMethod, Qt::DirectConnection, Q_ARG(QWidget *, widget));
   }

   if (QMainWindow *mw = dynamic_cast<QMainWindow *>(parentWidget)) {

      if (QMenuBar *menuBar = dynamic_cast<QMenuBar *>(widget)) {
         // the menubar
         mw->setMenuBar(menuBar);
         return true;

      } else if (QToolBar *toolBar = dynamic_cast<QToolBar *>(widget)) {
         // apply the toolbar's attributes

         mw->addToolBar(toolbarAreaFromDOMAttributes(attributes), toolBar);
         // check break
         if (const DomProperty *attr = attributes.value(strings.toolBarBreakAttribute))
            if (attr->elementBool() == strings.trueValue) {
               mw->insertToolBarBreak (toolBar);
            }

         return true;

      } else if (QStatusBar *statusBar = dynamic_cast<QStatusBar *>(widget)) {
         // statusBar

         mw->setStatusBar(statusBar);
         return true;

      } else if (QDockWidget *dockWidget = dynamic_cast<QDockWidget *>(widget)) {
         // apply the dockwidget's attributes

         if (const DomProperty *attr = attributes.value(strings.dockWidgetAreaAttribute)) {
            Qt::DockWidgetArea area = static_cast<Qt::DockWidgetArea>(attr->elementNumber());
            if (!dockWidget->isAreaAllowed(area)) {
               if (dockWidget->isAreaAllowed(Qt::LeftDockWidgetArea)) {
                  area = Qt::LeftDockWidgetArea;
               } else if (dockWidget->isAreaAllowed(Qt::RightDockWidgetArea)) {
                  area = Qt::RightDockWidgetArea;
               } else if (dockWidget->isAreaAllowed(Qt::TopDockWidgetArea)) {
                  area = Qt::TopDockWidgetArea;
               } else if (dockWidget->isAreaAllowed(Qt::BottomDockWidgetArea)) {
                  area = Qt::BottomDockWidgetArea;
               }
            }
            mw->addDockWidget(area, dockWidget);

         } else {
            mw->addDockWidget(Qt::LeftDockWidgetArea, dockWidget);
         }
         return true;
      }

      else if (! mw->centralWidget()) {
         mw->setCentralWidget(widget);
         return true;
      }

   } else if (QTabWidget *tabWidget = dynamic_cast<QTabWidget *>(parentWidget)) {
      widget->setParent(nullptr);

      const int tabIndex = tabWidget->count();
      if (const DomProperty *titleP = attributes.value(strings.titleAttribute, nullptr)) {
         tabWidget->addTab(widget, toString(titleP->elementString()));
      } else {
         tabWidget->addTab(widget, strings.defaultTitle);
      }

      if (DomProperty *picon = attributes.value(strings.iconAttribute)) {
         QVariant v = resourceBuilder()->loadResource(workingDirectory(), picon);

         QVariant nativeValue = resourceBuilder()->toNativeValue(v);
         tabWidget->setTabIcon(tabIndex, nativeValue.value<QIcon>());
      }

      if (const DomProperty *ptoolTip = attributes.value(strings.toolTipAttribute)) {
         tabWidget->setTabToolTip(tabIndex, toString(ptoolTip->elementString()));
      }

      if (const DomProperty *pwhatsThis = attributes.value(strings.whatsThisAttribute)) {
         tabWidget->setTabWhatsThis(tabIndex, toString(pwhatsThis->elementString()));
      }

      return true;

   } else if (QToolBox *toolBox = dynamic_cast<QToolBox *>(parentWidget)) {
      const int tabIndex = toolBox->count();
      if (const DomProperty *labelP =  attributes.value(strings.labelAttribute, nullptr)) {
         toolBox->addItem(widget, toString(labelP->elementString()));
      } else {
         toolBox->addItem(widget, strings.defaultTitle);
      }

      if (DomProperty *picon = attributes.value(strings.iconAttribute)) {
         QVariant v = resourceBuilder()->loadResource(workingDirectory(), picon);

         QVariant nativeValue = resourceBuilder()->toNativeValue(v);
         toolBox->setItemIcon(tabIndex, nativeValue.value<QIcon>());
      }

      if (const DomProperty *ptoolTip = attributes.value(strings.toolTipAttribute)) {
         toolBox->setItemToolTip(tabIndex, toString(ptoolTip->elementString()));
      }

      return true;

   } else if (QStackedWidget *stackedWidget = dynamic_cast<QStackedWidget *>(parentWidget)) {
      stackedWidget->addWidget(widget);
      return true;

   } else if (QSplitter *splitter = dynamic_cast<QSplitter *>(parentWidget)) {
      splitter->addWidget(widget);
      return true;

   } else if (QMdiArea *mdiArea = dynamic_cast<QMdiArea *>(parentWidget)) {
      mdiArea->addSubWindow(widget);
      return true;

   } else if (QDockWidget *dockWidget = dynamic_cast<QDockWidget *>(parentWidget)) {
      dockWidget->setWidget(widget);
      return true;

   } else if (QScrollArea *scrollArea = dynamic_cast<QScrollArea *>(parentWidget)) {
      scrollArea->setWidget(widget);
      return true;

   } else if (QWizard *wizard = dynamic_cast<QWizard *>(parentWidget)) {
      QWizardPage *page = dynamic_cast<QWizardPage *>(widget);
      if (!page) {
         csWarning(QCoreApplication::translate("QAbstractFormBuilder", "Attempt to add child that is not of class QWizardPage to QWizard."));
         return false;
      }
      wizard->addPage(page);
      return true;
   }

   return false;
}

void QAbstractFormBuilder::layoutInfo(DomLayout *ui_layout, QObject *parent, int *margin, int *spacing)
{
   (void) parent;

   const QFormBuilderStrings &strings = QFormBuilderStrings::instance();
   const DomPropertyHash properties = propertyMap(ui_layout->elementProperty());

   int mar = INT_MIN;
   int spac = INT_MIN;
   if (const DomProperty *p = properties.value(strings.marginProperty, nullptr)) {
      mar = p->elementNumber();
   }

   if (const DomProperty *p = properties.value(strings.spacingProperty, nullptr)) {
      spac = p->elementNumber();
   }

#ifdef Q_OS_DARWIN
   // here we recognize UI file < 4.3 (no we don't store margin property)

   if (mar != INT_MIN) {
      const int defaultMargin = parent->inherits("QLayoutWidget") ? 0 : 9;
      if (mar == defaultMargin) {
         mar = INT_MIN;
      }
      if (spac == 6) {
         spac = INT_MIN;
      }

      if (mar == INT_MIN || spac == INT_MIN) {
         QList<DomProperty *> propertyList = ui_layout->elementProperty();
         QMutableListIterator<DomProperty *> it(propertyList);

         while (it.hasNext()) {
            DomProperty *prop = it.next();

            if ((mar == INT_MIN && prop->attributeName() == strings.marginProperty) ||
               (spac == INT_MIN && prop->attributeName() == strings.spacingProperty)) {
               it.remove();
               delete prop;
            }
         }

         ui_layout->setElementProperty(propertyList);
      }
   }
#endif

   if (margin) {
      *margin = mar;
   }

   if (spacing) {
      *spacing = spac;
   }
}

QLayout *QAbstractFormBuilder::create(DomLayout *ui_layout, QLayout *parentLayout, QWidget *parentWidget)
{
   QObject *p = parentLayout;

   if (p == nullptr) {
      p = parentWidget;
   }

   Q_ASSERT(p != nullptr);

   bool tracking = false;

   if (p == parentWidget && parentWidget->layout()) {
      tracking = true;
      p = parentWidget->layout();
   }

   QLayout *layout = createLayout(ui_layout->attributeClass(), p, ui_layout->hasAttributeName()
         ? ui_layout->attributeName() : QString());

   if (layout == nullptr) {
      return nullptr;
   }

   if (tracking && layout->parent() == nullptr) {
      QBoxLayout *box = dynamic_cast<QBoxLayout *>(parentWidget->layout());

      if (!box) {
         // only QBoxLayout is supported
         const QString widgetClass = parentWidget->metaObject()->className();
         const QString layoutClass = parentWidget->layout()->metaObject()->className();

         const QString msg = QCoreApplication::translate("QAbstractFormBuilder",
               "Attempt to add a layout to a widget '%1' (%2) which already has a layout of non-box type %3.\n"
               "This indicates an inconsistency in the ui-file.")
               .formatArgs(parentWidget->objectName(), widgetClass, layoutClass);

         csWarning(msg);
         return nullptr;
      }

      box->addLayout(layout);
   }

   int margin = INT_MIN, spacing = INT_MIN;
   layoutInfo(ui_layout, p, &margin, &spacing);

   if (margin != INT_MIN) {
      layout->setMargin(margin);
   } else {
      const QFormBuilderStrings &strings = QFormBuilderStrings::instance();
      int left;
      int top;
      int right;
      int bottom;

      left = top = right = bottom = -1;
      layout->getContentsMargins(&left, &top, &right, &bottom);

      const DomPropertyHash properties = propertyMap(ui_layout->elementProperty());

      if (const DomProperty *p1 = properties.value(strings.leftMarginProperty, nullptr)) {
         left = p1->elementNumber();
      }

      if (const DomProperty *p2 = properties.value(strings.topMarginProperty, nullptr)) {
         top = p2->elementNumber();
      }

      if (const DomProperty *p3 = properties.value(strings.rightMarginProperty, nullptr)) {
         right = p3->elementNumber();
      }

      if (const DomProperty *p4 = properties.value(strings.bottomMarginProperty, nullptr)) {
         bottom = p4->elementNumber();
      }

      layout->setContentsMargins(left, top, right, bottom);
   }

   if (spacing != INT_MIN) {
      layout->setSpacing(spacing);

   } else {
      QGridLayout *grid = dynamic_cast<QGridLayout *>(layout);
      if (grid) {
         const QFormBuilderStrings &strings = QFormBuilderStrings::instance();
         const DomPropertyHash properties = propertyMap(ui_layout->elementProperty());

         if (const DomProperty *p5= properties.value(strings.horizontalSpacingProperty, nullptr)) {
            grid->setHorizontalSpacing(p5->elementNumber());
         }
         if (const DomProperty *p6 = properties.value(strings.verticalSpacingProperty, nullptr)) {
            grid->setVerticalSpacing(p6->elementNumber());
         }
      }
   }

   applyProperties(layout, ui_layout->elementProperty());

   for (DomLayoutItem *ui_item : ui_layout->elementItem()) {
      if (QLayoutItem *item = create(ui_item, layout, parentWidget)) {
         addItem(ui_item, item, layout);
      }
   }

   // Check the box stretch attributes
   if (QBoxLayout *box = dynamic_cast<QBoxLayout *>(layout)) {
      const QString boxStretch = ui_layout->attributeStretch();
      if (!boxStretch.isEmpty()) {
         QFormBuilderExtra::setBoxLayoutStretch(boxStretch, box);
      }
   }

   // Check the grid stretch/minimum size attributes
   if (QGridLayout *grid = dynamic_cast<QGridLayout *>(layout)) {
      // Stretch
      const QString gridRowStretch = ui_layout->attributeRowStretch();
      if (!gridRowStretch.isEmpty()) {
         QFormBuilderExtra::setGridLayoutRowStretch(gridRowStretch, grid);
      }

      const QString gridColumnStretch = ui_layout->attributeColumnStretch();
      if (!gridColumnStretch.isEmpty()) {
         QFormBuilderExtra::setGridLayoutColumnStretch(gridColumnStretch, grid);
      }

      // Minimum size
      const QString gridColumnMinimumWidth = ui_layout->attributeColumnMinimumWidth();
      if (!gridColumnMinimumWidth.isEmpty()) {
         QFormBuilderExtra::setGridLayoutColumnMinimumWidth(gridColumnMinimumWidth, grid);
      }

      const QString gridRowMinimumHeight = ui_layout->attributeRowMinimumHeight();
      if (!gridRowMinimumHeight.isEmpty()) {
         QFormBuilderExtra::setGridLayoutRowMinimumHeight(gridRowMinimumHeight, grid);
      }
   }

   return layout;
}

static inline QFormLayout::ItemRole formLayoutRole(int column, int colspan)
{
   if (colspan > 1) {
      return QFormLayout::SpanningRole;
   }

   return column == 0 ? QFormLayout::LabelRole : QFormLayout::FieldRole;
}

static inline QString alignmentValue(Qt::Alignment a)
{
   QString h, v;

   switch (a & Qt::AlignHorizontal_Mask) {
      case Qt::AlignLeft:
         h = QString("Qt::AlignLeft");
         break;

      case Qt::AlignRight:
         h = QString("Qt::AlignRight");
         break;

      case Qt::AlignHCenter:
         h = QString("Qt::AlignHCenter");
         break;

      case Qt::AlignJustify:
         h = QString("Qt::AlignJustify");
         break;
   }

   switch (a & Qt::AlignVertical_Mask) {
      case Qt::AlignTop:
         v = QString("Qt::AlignTop");
         break;

      case Qt::AlignBottom:
         v = QString("Qt::AlignBottom");
         break;

      case Qt::AlignVCenter:
         v = QString("Qt::AlignVCenter");
         break;
   }

   if (h.isEmpty() && v.isEmpty()) {
      return QString();
   }

   if (!v.isEmpty()) {
      if (!h.isEmpty()) {
         h += QLatin1Char('|');
      }
      h += v;
   }
   return h;
}

static inline Qt::Alignment alignmentFromDom(const QString &in)
{
   Qt::Alignment rc = nullptr;

   if (! in.isEmpty()) {
      for (const QString &f : in.split(QLatin1Char('|'))) {
         if (f == QString("Qt::AlignLeft")) {
            rc |= Qt::AlignLeft;

         } else if (f == QString("Qt::AlignRight")) {
            rc |= Qt::AlignRight;

         } else if (f == QString("Qt::AlignHCenter")) {
            rc |= Qt::AlignHCenter;

         } else if (f == QString("Qt::AlignJustify")) {
            rc |= Qt::AlignJustify;

         } else if (f == QString("Qt::AlignTop")) {
            rc |= Qt::AlignTop;

         } else if (f == QString("Qt::AlignBottom")) {
            rc |= Qt::AlignBottom;
         } else if (f == QString("Qt::AlignVCenter")) {
            rc |= Qt::AlignVCenter;
         }
      }
   }
   return rc;
}

bool QAbstractFormBuilder::addItem(DomLayoutItem *ui_item, QLayoutItem *item, QLayout *layout)
{
   // Calling addChildWidget(), etc.  is required to maintain consistency of the layouts,
   // see documentation of addItem(), which should ideally not be used.

   if (item->widget()) {
      static_cast<QFriendlyLayout *>(layout)->addChildWidget(item->widget());

   } else if (item->layout()) {
      static_cast<QFriendlyLayout *>(layout)->addChildLayout(item->layout());

   } else if (item->spacerItem()) {
      // nothing to do

   } else {
      return false;
   }

   if (QGridLayout *grid = dynamic_cast<QGridLayout *>(layout)) {
      const int rowSpan = ui_item->hasAttributeRowSpan() ? ui_item->attributeRowSpan() : 1;
      const int colSpan = ui_item->hasAttributeColSpan() ? ui_item->attributeColSpan() : 1;
      grid->addItem(item, ui_item->attributeRow(), ui_item->attributeColumn(),
         rowSpan, colSpan, item->alignment());

      return true;
   }

   if (QFormLayout *form = dynamic_cast<QFormLayout *>(layout)) {
      const int row =  ui_item->attributeRow();
      const int colSpan = ui_item->hasAttributeColSpan() ? ui_item->attributeColSpan() : 1;
      form->setItem(row, formLayoutRole(ui_item->attributeColumn(), colSpan), item);
      return true;
   }

   layout->addItem(item);

   return true;
}

QLayoutItem *QAbstractFormBuilder::create(DomLayoutItem *ui_layoutItem, QLayout *layout, QWidget *parentWidget)
{
   switch (ui_layoutItem->kind()) {

      case DomLayoutItem::Widget: {

         if (QWidget *w = create(ui_layoutItem->elementWidget(), parentWidget)) {

            QWidgetItem *item;
            auto tmp = QLayout::getWidgetItemFactory();

            if (tmp == nullptr) {
               item = new QWidgetItemV2(w);

            } else {
               item = (*tmp)(layout, w);

               if (item == nullptr) {
                  item = new QWidgetItemV2(w);
               }
            }

            item->setAlignment(alignmentFromDom(ui_layoutItem->attributeAlignment()));

            return item;
         }

         csWarning(QCoreApplication::translate("QAbstractFormBuilder",
               "Unable to create item using class %1 for object named '%2'.")
               .formatArgs(layout->metaObject()->className(), layout->objectName()));

         return nullptr;
      }

      case DomLayoutItem::Spacer: {
         QSize size(0, 0);
         QSizePolicy::Policy sizeType = QSizePolicy::Expanding;
         bool isVspacer = false;

         const DomSpacer *ui_spacer = ui_layoutItem->elementSpacer();
         const QList<DomProperty *> spacerProperties =  ui_spacer->elementProperty();

         if (!spacerProperties.empty()) {
            const QFormBuilderStrings &strings = QFormBuilderStrings::instance();

            for (DomProperty *p : spacerProperties) {
               const QVariant v = toVariant(QAbstractFormBuilderGadget::staticMetaObject(), p);

               if (! v.isValid()) {
                  continue;
               }

               if (p->attributeName() == strings.sizeHintProperty && p->kind() == DomProperty::Size) {
                  size = v.toSize();

               } else if (p->attributeName() == strings.sizeTypeProperty && p->kind() == DomProperty::Enum) {
                  sizeType = static_cast<QSizePolicy::Policy>(v.toInt());

               } else if (p->attributeName() == strings.orientationProperty && p->kind() == DomProperty::Enum) {
                  const Qt::Orientation o = static_cast<Qt::Orientation>(v.toInt());
                  isVspacer = (o == Qt::Vertical);
               }
            }
         }

         QSpacerItem *spacer = nullptr;

         if (isVspacer) {
            spacer = new QSpacerItem(size.width(), size.height(), QSizePolicy::Minimum, sizeType);
         } else {
            spacer = new QSpacerItem(size.width(), size.height(), sizeType, QSizePolicy::Minimum);
         }

         return spacer;
      }

      case DomLayoutItem::Layout:
         return create(ui_layoutItem->elementLayout(), layout, parentWidget);

      default:
         break;
   }

   return nullptr;
}

void QAbstractFormBuilder::applyProperties(QObject *o, const QList<DomProperty *> &properties)
{
   if (properties.empty()) {
      return;
   }

   auto cend = properties.constEnd();

   for (auto it = properties.constBegin(); it != cend; ++it) {
      const QVariant v = toVariant(*(o->metaObject()), *it);

      if (v.isValid()) {
         QString attributeName = (*it)->attributeName();

         if (attributeName == "numDigits" && o->inherits("QLCDNumber"))  {
            // Deprecated, consider removing

            attributeName = "digitCount";
         }

         if (! d->applyPropertyInternally(o, attributeName, v)) {
            o->setProperty(attributeName, v);
         }
      }
   }
}

bool QAbstractFormBuilder::applyPropertyInternally(QObject *o, const QString &propertyName, const QVariant &value)
{
   return d->applyPropertyInternally(o, propertyName, value);
}

QVariant QAbstractFormBuilder::toVariant(const QMetaObject &meta, DomProperty *p)
{
   return domPropertyToVariant(this, meta, p);
}

void QAbstractFormBuilder::setupColorGroup(QPalette &palette, QPalette::ColorGroup colorGroup, DomColorGroup *group)
{
   // old format
   const QList<DomColor *> colors = group->elementColor();

   for (int role = 0; role < colors.size(); ++role) {
      const DomColor *color = colors.at(role);
      const QColor c(color->elementRed(), color->elementGreen(), color->elementBlue());
      palette.setColor(colorGroup, QPalette::ColorRole(role), c);
   }

   // new format
   const QMetaEnum colorRole_enum = metaEnum<QAbstractFormBuilderGadget>("colorRole");

   const QList<DomColorRole *> colorRoles = group->elementColorRole();
   for (int role = 0; role < colorRoles.size(); ++role) {
      const DomColorRole *colorRole = colorRoles.at(role);

      if (colorRole->hasAttributeRole()) {
         const int r = colorRole_enum.keyToValue(colorRole->attributeRole().toLatin1());
         if (r != -1) {
            const QBrush br = setupBrush(colorRole->elementBrush());
            palette.setBrush(colorGroup, static_cast<QPalette::ColorRole>(r), br);
         }
      }
   }
}

DomColorGroup *QAbstractFormBuilder::saveColorGroup(const QPalette &palette)
{
   const QMetaEnum colorRole_enum = metaEnum<QAbstractFormBuilderGadget>("colorRole");

   DomColorGroup *group = new DomColorGroup();
   QList<DomColorRole *> colorRoles;

   const uint mask = palette.resolve();
   for (int role = QPalette::WindowText; role < QPalette::NColorRoles; ++role) {
      if (mask & (1 << role)) {
         QBrush br = palette.brush(QPalette::ColorRole(role));

         DomColorRole *colorRole = new DomColorRole();
         colorRole->setElementBrush(saveBrush(br));
         colorRole->setAttributeRole(colorRole_enum.valueToKey(role));
         colorRoles.append(colorRole);
      }
   }

   group->setElementColorRole(colorRoles);
   return group;
}

QBrush QAbstractFormBuilder::setupBrush(DomBrush *brush)
{
   QBrush br;

   if (! brush->hasAttributeBrushStyle()) {
      return br;
   }

   const Qt::BrushStyle style = enumKeyOfObjectToValue<QAbstractFormBuilderGadget, Qt::BrushStyle>("brushStyle",
         brush->attributeBrushStyle().toLatin1());

   if (style == Qt::LinearGradientPattern || style == Qt::RadialGradientPattern || style == Qt::ConicalGradientPattern) {

      const QMetaEnum gradientType_enum       = metaEnum<QAbstractFormBuilderGadget>("gradientType");
      const QMetaEnum gradientSpread_enum     = metaEnum<QAbstractFormBuilderGadget>("gradientSpread");
      const QMetaEnum gradientCoordinate_enum = metaEnum<QAbstractFormBuilderGadget>("gradientCoordinate");

      const DomGradient *gradient = brush->elementGradient();
      const QGradient::Type type = enumKeyToValue<QGradient::Type>(gradientType_enum, gradient->attributeType().toLatin1());

      QGradient *gr = nullptr;

      if (type == QGradient::LinearGradient) {
         gr = new QLinearGradient(QPointF(gradient->attributeStartX(), gradient->attributeStartY()),
            QPointF(gradient->attributeEndX(), gradient->attributeEndY()));

      } else if (type == QGradient::RadialGradient) {
         gr = new QRadialGradient(QPointF(gradient->attributeCentralX(), gradient->attributeCentralY()),
            gradient->attributeRadius(),

            QPointF(gradient->attributeFocalX(), gradient->attributeFocalY()));

      } else if (type == QGradient::ConicalGradient) {
         gr = new QConicalGradient(QPointF(gradient->attributeCentralX(), gradient->attributeCentralY()),
            gradient->attributeAngle());

      }

      if (! gr) {
         return br;
      }

      const QGradient::Spread spread = enumKeyToValue<QGradient::Spread>(gradientSpread_enum, gradient->attributeSpread().toLatin1());
      gr->setSpread(spread);

      const QGradient::CoordinateMode coord = enumKeyToValue<QGradient::CoordinateMode>(gradientCoordinate_enum,
            gradient->attributeCoordinateMode().toLatin1());
      gr->setCoordinateMode(coord);

      const QList<DomGradientStop *> stops = gradient->elementGradientStop();
      QListIterator<DomGradientStop *> it(stops);

      while (it.hasNext()) {
         const DomGradientStop *stop = it.next();
         const DomColor *color = stop->elementColor();

         gr->setColorAt(stop->attributePosition(), QColor::fromRgb(color->elementRed(),
               color->elementGreen(), color->elementBlue(), color->attributeAlpha()));
      }

      br = QBrush(*gr);
      delete gr;

   } else if (style == Qt::TexturePattern) {
      const DomProperty *texture = brush->elementTexture();

      if (texture && texture->kind() == DomProperty::Pixmap) {
         br.setTexture(QPixmap());
      }

   } else {
      const DomColor *color = brush->elementColor();
      br.setColor(QColor::fromRgb(color->elementRed(), color->elementGreen(), color->elementBlue(), color->attributeAlpha()));
      br.setStyle((Qt::BrushStyle)style);
   }

   return br;
}

DomBrush *QAbstractFormBuilder::saveBrush(const QBrush &br)
{
   const QMetaEnum brushStyle_enum = metaEnum<QAbstractFormBuilderGadget>("brushStyle");

   DomBrush *brush = new DomBrush();
   const Qt::BrushStyle style = br.style();
   brush->setAttributeBrushStyle(brushStyle_enum.valueToKey(style));

   if (style == Qt::LinearGradientPattern ||
      style == Qt::RadialGradientPattern ||
      style == Qt::ConicalGradientPattern) {
      const QMetaEnum gradientType_enum       = metaEnum<QAbstractFormBuilderGadget>("gradientType");
      const QMetaEnum gradientSpread_enum     = metaEnum<QAbstractFormBuilderGadget>("gradientSpread");
      const QMetaEnum gradientCoordinate_enum = metaEnum<QAbstractFormBuilderGadget>("gradientCoordinate");

      DomGradient *gradient = new DomGradient();

      const QGradient *gr = br.gradient();
      const QGradient::Type type = gr->type();
      gradient->setAttributeType(gradientType_enum.valueToKey(type));
      gradient->setAttributeSpread(gradientSpread_enum.valueToKey(gr->spread()));
      gradient->setAttributeCoordinateMode(gradientCoordinate_enum.valueToKey(gr->coordinateMode()));

      QList<DomGradientStop *> stops;
      QVector<QPair<qreal, QColor>> st = gr->stops();
      QVectorIterator<QPair<qreal, QColor>> it(st);

      while (it.hasNext()) {
         const QPair<qreal, QColor> pair = it.next();
         DomGradientStop *stop = new DomGradientStop();
         stop->setAttributePosition(pair.first);
         DomColor *color = new DomColor();
         color->setElementRed(pair.second.red());
         color->setElementGreen(pair.second.green());
         color->setElementBlue(pair.second.blue());
         color->setAttributeAlpha(pair.second.alpha());
         stop->setElementColor(color);
         stops.append(stop);
      }
      gradient->setElementGradientStop(stops);
      if (type == QGradient::LinearGradient) {
         const QLinearGradient *lgr = (const QLinearGradient *)(gr);
         gradient->setAttributeStartX(lgr->start().x());
         gradient->setAttributeStartY(lgr->start().y());
         gradient->setAttributeEndX(lgr->finalStop().x());
         gradient->setAttributeEndY(lgr->finalStop().y());
      } else if (type == QGradient::RadialGradient) {
         const QRadialGradient *rgr = (const QRadialGradient *)(gr);
         gradient->setAttributeCentralX(rgr->center().x());
         gradient->setAttributeCentralY(rgr->center().y());
         gradient->setAttributeFocalX(rgr->focalPoint().x());
         gradient->setAttributeFocalY(rgr->focalPoint().y());
         gradient->setAttributeRadius(rgr->radius());
      } else if (type == QGradient::ConicalGradient) {
         const QConicalGradient *cgr = (const QConicalGradient *)(gr);
         gradient->setAttributeCentralX(cgr->center().x());
         gradient->setAttributeCentralY(cgr->center().y());
         gradient->setAttributeAngle(cgr->angle());
      }

      brush->setElementGradient(gradient);
   } else if (style == Qt::TexturePattern) {
      const QPixmap pixmap = br.texture();

      if (!pixmap.isNull()) {
         DomProperty *p = new DomProperty;
         setPixmapProperty(*p, IconPaths());
         brush->setElementTexture(p);
      }

   } else {
      QColor c = br.color();
      DomColor *color = new DomColor();
      color->setElementRed(c.red());
      color->setElementGreen(c.green());
      color->setElementBlue(c.blue());
      color->setAttributeAlpha(c.alpha());
      brush->setElementColor(color);
   }
   return brush;
}

QWidget *QAbstractFormBuilder::createWidget(const QString &widgetName, QWidget *parentWidget, const QString &name)
{
   (void) widgetName;
   (void) parentWidget;
   (void) name;

   return nullptr;
}

QLayout *QAbstractFormBuilder::createLayout(const QString &layoutName, QObject *parent, const QString &name)
{
   (void) layoutName;
   (void) parent;
   (void) name;

   return nullptr;
}

QAction *QAbstractFormBuilder::createAction(QObject *parent, const QString &name)
{
   QAction *action = new QAction(parent);
   action->setObjectName(name);

   return action;
}

QActionGroup *QAbstractFormBuilder::createActionGroup(QObject *parent, const QString &name)
{
   QActionGroup *g = new QActionGroup(parent);
   g->setObjectName(name);

   return g;
}

void QAbstractFormBuilder::save(QIODevice *dev, QWidget *widget)
{
   DomWidget *ui_widget = createDom(widget, nullptr);
   Q_ASSERT(ui_widget != nullptr);

   DomUI *ui = new DomUI();
   ui->setAttributeVersion("4.0");
   ui->setElementWidget(ui_widget);

   saveDom(ui, widget);

   QXmlStreamWriter writer(dev);
   writer.setAutoFormatting(true);
   writer.setAutoFormattingIndent(1);
   writer.writeStartDocument();
   ui->write(writer);
   writer.writeEndDocument();

   d->m_laidout.clear();

   delete ui;
}

void QAbstractFormBuilder::saveDom(DomUI *ui, QWidget *widget)
{
   ui->setElementClass(widget->objectName());

   if (DomConnections *ui_connections = saveConnections()) {
      ui->setElementConnections(ui_connections);
   }

   if (DomCustomWidgets *ui_customWidgets = saveCustomWidgets()) {
      ui->setElementCustomWidgets(ui_customWidgets);
   }

   if (DomTabStops *ui_tabStops = saveTabStops()) {
      ui->setElementTabStops(ui_tabStops);
   }

   if (DomResources *ui_resources = saveResources()) {
      ui->setElementResources(ui_resources);
   }
   if (DomButtonGroups *ui_buttonGroups = saveButtonGroups(widget)) {
      ui->setElementButtonGroups(ui_buttonGroups);
   }
}

DomConnections *QAbstractFormBuilder::saveConnections()
{
   return new DomConnections;
}

DomWidget *QAbstractFormBuilder::createDom(QWidget *widget, DomWidget *ui_parentWidget, bool recursive)
{
   DomWidget *ui_widget = new DomWidget();
   ui_widget->setAttributeClass(widget->metaObject()->className());
   ui_widget->setElementProperty(computeProperties(widget));

   if (recursive) {
      if (QLayout *layout = widget->layout()) {
         if (DomLayout *ui_layout = createDom(layout, nullptr, ui_parentWidget)) {
            QList<DomLayout *> ui_layouts;
            ui_layouts.append(ui_layout);

            ui_widget->setElementLayout(ui_layouts);
         }
      }
   }

   // widgets, actions and action groups
   QList<DomWidget *> ui_widgets;
   QList<DomAction *> ui_actions;
   QList<DomActionGroup *> ui_action_groups;

   QList<QObject *> children;

   // splitters need to store their children in the order specified by child indexes,
   // not the order of the child list.

   if (const QSplitter *splitter = dynamic_cast<const QSplitter *>(widget)) {
      const int count = splitter->count();
      for (int i = 0; i < count; ++i) {
         children.append(splitter->widget(i));
      }

   } else  {
      QList<QObject *> childObjects = widget->children();

      QVariant data = widget->property("_q_widgetOrder");
      const QList<QWidget *> list = data.value<QWidgetList>();

      for (QWidget *w : list) {
         if (childObjects.contains(w)) {
            children.append(w);
            childObjects.removeAll(w);
         }
      }
      children += childObjects;

      data = widget->property("_q_zOrder");
      const QList<QWidget *> zOrder = data.value<QWidgetList>();

      if (list != zOrder) {
         QStringList zOrderList;
         QListIterator<QWidget * > itZOrder(zOrder);

         while (itZOrder.hasNext()) {
            zOrderList.append(itZOrder.next()->objectName());
         }

         ui_widget->setElementZOrder(zOrderList);
      }
   }

   for (QObject *obj : children) {
      if (QWidget *childWidget = dynamic_cast<QWidget *>(obj)) {
         if (d->m_laidout.contains(childWidget) || recursive == false) {
            continue;
         }

         if (QMenu *menu = dynamic_cast<QMenu *>(childWidget)) {
            QList<QAction *> actions = menu->parentWidget()->actions();
            QListIterator<QAction *> it(actions);
            bool found = false;

            while (it.hasNext()) {
               if (it.next()->menu() == menu) {
                  found = true;
               }
            }
            if (!found) {
               continue;
            }
         }

         if (DomWidget *ui_child = createDom(childWidget, ui_widget)) {
            ui_widgets.append(ui_child);
         }
      } else if (QAction *childAction = dynamic_cast<QAction *>(obj)) {
         if (childAction->actionGroup() != nullptr) {
            // it will be added later
            continue;
         }

         if (DomAction *ui_action = createDom(childAction)) {
            ui_actions.append(ui_action);
         }
      } else if (QActionGroup *childActionGroup = dynamic_cast<QActionGroup *>(obj)) {
         if (DomActionGroup *ui_action_group = createDom(childActionGroup)) {
            ui_action_groups.append(ui_action_group);
         }
      }
   }

   // add-action
   QList<DomActionRef *> ui_action_refs;

   for (QAction *action : widget->actions()) {
      if (DomActionRef *ui_action_ref = createActionRefDom(action)) {
         ui_action_refs.append(ui_action_ref);
      }
   }

   if (recursive) {
      ui_widget->setElementWidget(ui_widgets);
   }

   ui_widget->setElementAction(ui_actions);
   ui_widget->setElementActionGroup(ui_action_groups);
   ui_widget->setElementAddAction(ui_action_refs);

   saveExtraInfo(widget, ui_widget, ui_parentWidget);

   return ui_widget;
}

DomActionRef *QAbstractFormBuilder::createActionRefDom(QAction *action)
{
   QString name = action->objectName();

   if (action->menu() != nullptr) {
      name = action->menu()->objectName();
   }

   DomActionRef *ui_action_ref = new DomActionRef();
   if (action->isSeparator()) {
      ui_action_ref->setAttributeName(QFormBuilderStrings::instance().separator);
   } else {
      ui_action_ref->setAttributeName(name);
   }

   return ui_action_ref;
}

// Struct to store layout item parameters for saving layout items
struct FormBuilderSaveLayoutEntry {
   explicit FormBuilderSaveLayoutEntry(QLayoutItem *li = nullptr)
      : item(li), row(-1), column(-1), rowSpan(0), columnSpan(0), alignment(nullptr)
   {
   }

   void setAlignment(Qt::Alignment al);

   QLayoutItem *item;
   int row;
   int column;
   int rowSpan;
   int columnSpan;
   Qt::Alignment alignment;
};

// filter out the case of "Spacer" and "QLayoutWidget" widgets
void FormBuilderSaveLayoutEntry::setAlignment(Qt::Alignment al)
{
   if (const QWidget *widget = item->widget()) {
      const QString className = widget->metaObject()->className();

      if ((className != "Spacer") && (className != "QLayoutWidget")) {
         alignment = al;
      }
   }
}

// Create list from standard box layout
static QList<FormBuilderSaveLayoutEntry> saveLayoutEntries(const QLayout *layout)
{
   QList<FormBuilderSaveLayoutEntry> rc;

   if (const int count = layout->count()) {

      for (int idx = 0; idx < count; ++idx) {
         QLayoutItem *item = layout->itemAt(idx);
         FormBuilderSaveLayoutEntry entry(item);
         entry.setAlignment(item->alignment());
         rc.append(entry);
      }
   }

   return rc;
}

// Create list from grid layout
static QList<FormBuilderSaveLayoutEntry> saveGridLayoutEntries(QGridLayout *gridLayout)
{
   QList<FormBuilderSaveLayoutEntry> rc;

   if (const int count = gridLayout->count()) {

      for (int idx = 0; idx < count; ++idx) {
         QLayoutItem *item = gridLayout->itemAt(idx);
         FormBuilderSaveLayoutEntry entry(item);
         gridLayout->getItemPosition(idx, &entry.row, &entry.column, &entry.rowSpan, &entry.columnSpan);
         entry.setAlignment(item->alignment());
         rc.append(entry);
      }
   }
   return rc;
}

// Create list from form layout
static QList<FormBuilderSaveLayoutEntry> saveFormLayoutEntries(const QFormLayout *formLayout)
{
   QList<FormBuilderSaveLayoutEntry> rc;

   if (const int count = formLayout->count()) {

      for (int idx = 0; idx < count; ++idx) {
         QLayoutItem *item = formLayout->itemAt(idx);
         QFormLayout::ItemRole role = QFormLayout::LabelRole;
         FormBuilderSaveLayoutEntry entry(item);
         formLayout->getItemPosition(idx, &entry.row, &role);

         switch (role ) {
            case QFormLayout::LabelRole:
               entry.column = 0;
               break;

            case QFormLayout::FieldRole:
               entry.column = 1;
               break;

            case QFormLayout::SpanningRole:
               entry.column = 0;
               entry.columnSpan = 2;
               break;
         }

         rc.push_back(entry);
      }
   }
   return rc;
}

DomLayout *QAbstractFormBuilder::createDom(QLayout *layout, DomLayout *ui_layout, DomWidget *ui_parentWidget)
{
   (void) ui_layout;

   DomLayout *lay = new DomLayout();
   lay->setAttributeClass(layout->metaObject()->className());
   const QString objectName = layout->objectName();

   if (!objectName.isEmpty()) {
      lay->setAttributeName(objectName);
   }

   lay->setElementProperty(computeProperties(layout));

   QList<FormBuilderSaveLayoutEntry> newList;
   if (QGridLayout *gridLayout = dynamic_cast<QGridLayout *>(layout)) {
      newList = saveGridLayoutEntries(gridLayout);

   } else if (const QFormLayout *formLayout = dynamic_cast<const QFormLayout *>(layout)) {
      newList = saveFormLayoutEntries(formLayout);

   } else {
      newList = saveLayoutEntries(layout);
   }

   QList<DomLayoutItem *> ui_items;
   for (const FormBuilderSaveLayoutEntry &item : newList) {
      if (DomLayoutItem *ui_item = createDom(item.item, lay, ui_parentWidget)) {
         if (item.row >= 0) {
            ui_item->setAttributeRow(item.row);
         }
         if (item.column >= 0) {
            ui_item->setAttributeColumn(item.column);
         }
         if (item.rowSpan > 1) {
            ui_item->setAttributeRowSpan(item.rowSpan);
         }
         if (item.columnSpan > 1) {
            ui_item->setAttributeColSpan(item.columnSpan);
         }
         if (item.alignment) {
            ui_item->setAttributeAlignment(alignmentValue(item.alignment));
         }
         ui_items.append(ui_item);
      }
   }

   lay->setElementItem(ui_items);

   return lay;
}

DomLayoutItem *QAbstractFormBuilder::createDom(QLayoutItem *item, DomLayout *ui_layout, DomWidget *ui_parentWidget)
{
   DomLayoutItem *ui_item = new DomLayoutItem();

   if (item->widget())  {
      ui_item->setElementWidget(createDom(item->widget(), ui_parentWidget));
      d->m_laidout.insert(item->widget(), true);
   } else if (item->layout()) {
      ui_item->setElementLayout(createDom(item->layout(), ui_layout, ui_parentWidget));
   } else if (item->spacerItem()) {
      ui_item->setElementSpacer(createDom(item->spacerItem(), ui_layout, ui_parentWidget));
   }

   return ui_item;
}

DomSpacer *QAbstractFormBuilder::createDom(QSpacerItem *spacer, DomLayout *ui_layout, DomWidget *ui_parentWidget)
{
   (void) ui_layout;
   (void) ui_parentWidget;

   DomSpacer *ui_spacer = new DomSpacer();
   QList<DomProperty *> properties;

   DomProperty *prop = nullptr;
   const QFormBuilderStrings &strings = QFormBuilderStrings::instance();
   // sizeHint property
   prop = new DomProperty();
   prop->setAttributeName(strings.sizeHintProperty);
   prop->setElementSize(new DomSize());
   prop->elementSize()->setElementWidth(spacer->sizeHint().width());
   prop->elementSize()->setElementHeight(spacer->sizeHint().height());
   properties.append(prop);

   // orientation property
   prop = new DomProperty(); // ### we don't implemented the case where expandingDirections() is both Vertical and Horizontal
   prop->setAttributeName(strings.orientationProperty);
   prop->setElementEnum((spacer->expandingDirections() & Qt::Horizontal) ? strings.qtHorizontal : strings.qtVertical);
   properties.append(prop);

   ui_spacer->setElementProperty(properties);
   return ui_spacer;
}

DomProperty *QAbstractFormBuilder::createProperty(QObject *obj, const QString &pname, const QVariant &v)
{
   if (! checkProperty(obj, pname)) {
      return nullptr;
   }

   return variantToDomProperty(this, *(obj->metaObject()), pname, v);
}

QList<DomProperty *> QAbstractFormBuilder::computeProperties(QObject *obj)
{
   QList<DomProperty *> lst;

   const QMetaObject *meta = obj->metaObject();

   QHash<QString, bool> properties;
   const int propertyCount = meta->propertyCount();

   for (int i = 0; i < propertyCount; ++i) {
      properties.insert(meta->property(i).name(), true);
   }

   const QList<QString> propertyNames = properties.keys();
   const int propertyNamesCount       = propertyNames.size();

   for (int i = 0; i < propertyNamesCount ; ++i) {
      const QString pname = propertyNames.at(i);
      const QMetaProperty prop = meta->property(meta->indexOfProperty(pname));

      if (! prop.isWritable() || !checkProperty(obj, prop.name())) {
         continue;
      }

      const QVariant v = prop.read(obj);

      DomProperty *dom_prop = nullptr;

      if (v.type() == QVariant::Int) {
         dom_prop = new DomProperty();

         if (prop.isFlagType()) {
            csWarning(QCoreApplication::translate("QAbstractFormBuilder", "Flags property are not supported"));
         }

         if (prop.isEnumType()) {
            QString scope = prop.enumerator().scope();

            if (scope.size()) {
               scope += "::";
            }

            const QString e = prop.enumerator().valueToKey(v.toInt());

            if (e.size()) {
               dom_prop->setElementEnum(scope + e);
            }

         } else {
            dom_prop->setElementNumber(v.toInt());
         }
         dom_prop->setAttributeName(pname);

      } else {
         dom_prop = createProperty(obj, pname, v);
      }

      if (!dom_prop || dom_prop->kind() == DomProperty::Unknown) {
         delete dom_prop;
      } else {
         lst.append(dom_prop);
      }
   }

   return lst;
}

QAbstractFormBuilder::DomPropertyHash QAbstractFormBuilder::propertyMap(const QList<DomProperty *> &properties)
{
   DomPropertyHash map;

   for (DomProperty *p : properties) {
      map.insert(p->attributeName(), p);
   }

   return map;
}

bool QAbstractFormBuilder::checkProperty(QObject *obj, const QString &prop) const
{
   (void) obj;
   (void) prop;

   return true;
}

QString QAbstractFormBuilder::toString(const DomString *str)
{
   return str ? str->text() : QString();
}

void QAbstractFormBuilder::applyTabStops(QWidget *widget, DomTabStops *tabStops)
{
   if (! tabStops) {
      return;
   }

   QWidget *lastWidget = nullptr;

   const QStringList l = tabStops->elementTabStop();

   for (int i = 0; i < l.size(); ++i) {
      const QString name = l.at(i);

      QWidget *child = widget->findChild<QWidget *>(name);
      if (! child) {
         csWarning(QCoreApplication::translate("QAbstractFormBuilder",
               "While applying tab stops: The widget '%1' could not be found.").formatArg(name));
         continue;
      }

      if (i == 0) {
         lastWidget = widget->findChild<QWidget *>(name);
         continue;
      } else if (! child || ! lastWidget) {
         continue;
      }

      QWidget::setTabOrder(lastWidget, child);

      lastWidget = widget->findChild<QWidget *>(name);
   }
}

DomCustomWidgets *QAbstractFormBuilder::saveCustomWidgets()
{
   return nullptr;
}

DomTabStops *QAbstractFormBuilder::saveTabStops()
{
   return nullptr;
}

DomResources *QAbstractFormBuilder::saveResources()
{
   return nullptr;
}

DomButtonGroups *QAbstractFormBuilder::saveButtonGroups(const QWidget *mainContainer)
{
   // Save fst order buttongroup children of maincontainer
   const QObjectList mchildren = mainContainer->children();
   if (mchildren.empty()) {
      return nullptr;
   }

   QList<DomButtonGroup *> domGroups;
   auto cend = mchildren.constEnd();

   for (auto it = mchildren.constBegin(); it != cend; ++it) {
      if (QButtonGroup *bg = dynamic_cast<QButtonGroup *>(*it))
         if (DomButtonGroup *dg = createDom(bg)) {
            domGroups.push_back(dg);
         }
   }

   if (domGroups.empty()) {
      return nullptr;
   }

   DomButtonGroups *rc = new DomButtonGroups;
   rc->setElementButtonGroup(domGroups);
   return rc;
}

// VC6 does not find templated members so we use statics and this work around
class FriendlyFB : public QAbstractFormBuilder
{
 public:
   using QAbstractFormBuilder::saveResource;
   using QAbstractFormBuilder::saveText;
   using QAbstractFormBuilder::resourceBuilder;
   using QAbstractFormBuilder::textBuilder;
   using QAbstractFormBuilder::toVariant;
};

template<class T>
static void storeItemFlags(const T *item, QList<DomProperty *> *properties)
{
   static const QFormBuilderStrings &strings = QFormBuilderStrings::instance();
   static const Qt::ItemFlags defaultFlags = T().flags();
   static const QMetaEnum itemFlags_enum = metaEnum<QAbstractFormBuilderGadget>("itemFlags");

   if (item->flags() != defaultFlags) {
      DomProperty *p = new DomProperty;
      p->setAttributeName(strings.flagsAttribute);
      p->setElementSet(itemFlags_enum.valueToKeys(item->flags()));
      properties->append(p);
   }
}

template<class T>
static void storeItemProps(QAbstractFormBuilder *abstractFormBuilder, const T *item,
   QList<DomProperty *> *properties)
{
   static const QFormBuilderStrings &strings = QFormBuilderStrings::instance();
   FriendlyFB *const formBuilder = static_cast<FriendlyFB *>(abstractFormBuilder);

   DomProperty *p;
   QVariant v;

   for (const QFormBuilderStrings::TextRoleNName &it : strings.itemTextRoles)
      if ((p = formBuilder->saveText(it.second, item->data(it.first.second)))) {
         properties->append(p);
      }

   for (const QFormBuilderStrings::RoleNName &it : strings.itemRoles)

      if ((v = item->data(it.first)).isValid() && (p = variantToDomProperty(abstractFormBuilder,
                  QAbstractFormBuilderGadget::staticMetaObject(), it.second, v))) {
         properties->append(p);
      }

   if ((p = formBuilder->saveResource(item->data(Qt::DecorationPropertyRole)))) {
      properties->append(p);
   }
}

template<class T>
static void storeItemPropsNFlags(QAbstractFormBuilder *abstractFormBuilder, const T *item,
   QList<DomProperty *> *properties)
{
   storeItemProps<T>(abstractFormBuilder, item, properties);
   storeItemFlags<T>(item, properties);
}

template<class T>
static void loadItemProps(QAbstractFormBuilder *abstractFormBuilder, T *item,
   const QHash<QString, DomProperty *> &properties)
{
   static const QFormBuilderStrings &strings = QFormBuilderStrings::instance();
   FriendlyFB *const formBuilder = static_cast<FriendlyFB *>(abstractFormBuilder);

   DomProperty *p;
   QVariant v;

   for (const QFormBuilderStrings::TextRoleNName &it : strings.itemTextRoles) {
      if ((p = properties.value(it.second))) {
         v = formBuilder->textBuilder()->loadText(p);
         QVariant nativeValue = formBuilder->textBuilder()->toNativeValue(v);

         item->setData(it.first.first, nativeValue.value<QString>());
         item->setData(it.first.second, v);
      }
   }

   for (const QFormBuilderStrings::RoleNName &it : strings.itemRoles) {
      if ((p = properties.value(it.second)) && (v = formBuilder->toVariant(QAbstractFormBuilderGadget::staticMetaObject(), p)).isValid()) {
         item->setData(it.first, v);
      }
   }

   if ((p = properties.value(strings.iconAttribute))) {
      v = formBuilder->resourceBuilder()->loadResource(formBuilder->workingDirectory(), p);
      QVariant nativeValue = formBuilder->resourceBuilder()->toNativeValue(v);

      item->setIcon(nativeValue.value<QIcon>());
      item->setData(Qt::DecorationPropertyRole, v);
   }
}

template<class T>
static void loadItemPropsNFlags(QAbstractFormBuilder *abstractFormBuilder, T *item,
      const QHash<QString, DomProperty *> &properties)
{
   static const QFormBuilderStrings &strings = QFormBuilderStrings::instance();
   static const QMetaEnum itemFlags_enum = metaEnum<QAbstractFormBuilderGadget>("itemFlags");

   loadItemProps<T>(abstractFormBuilder, item, properties);

   DomProperty *p;
   if ((p = properties.value(strings.flagsAttribute)) && p->kind() == DomProperty::Set) {
      item->setFlags(enumKeysToValue<Qt::ItemFlags>(itemFlags_enum, p->elementSet().toLatin1()));
   }
}

void QAbstractFormBuilder::saveTreeWidgetExtraInfo(QTreeWidget *treeWidget, DomWidget *ui_widget, DomWidget *ui_parentWidget)
{
   (void) ui_parentWidget;

   QList<DomColumn *> columns;
   DomProperty *p;
   QVariant v;
   const QFormBuilderStrings &strings = QFormBuilderStrings::instance();

   // save the header
   for (int c = 0; c < treeWidget->columnCount(); ++c) {
      DomColumn *column = new DomColumn;

      QList<DomProperty *> properties;

      for (const QFormBuilderStrings::TextRoleNName &it : strings.itemTextRoles) {
         p = saveText(it.second, treeWidget->headerItem()->data(c, it.first.second));
         // Prevent uic 4.4.X from crashing if it cannot find a column text
         if (!p && it.first.first == Qt::EditRole && it.second == QString("text")) {
            DomString *defaultHeader = new DomString;
            defaultHeader->setText(QString::number(c + 1));
            defaultHeader->setAttributeNotr(QString("true"));
            p = new DomProperty;
            p->setAttributeName(it.second);
            p->setElementString(defaultHeader);
         }

         if (p) {
            properties.append(p);
         }
      }

      for (const QFormBuilderStrings::RoleNName &it : strings.itemRoles)
         if ((v = treeWidget->headerItem()->data(c, it.first)).isValid() &&
            (p = variantToDomProperty(this, QAbstractFormBuilderGadget::staticMetaObject(), it.second, v))) {
            properties.append(p);
         }

      if ((p = saveResource(treeWidget->headerItem()->data(c, Qt::DecorationPropertyRole)))) {
         properties.append(p);
      }

      column->setElementProperty(properties);
      columns.append(column);
   }

   ui_widget->setElementColumn(columns);

   QList<DomItem *> items = ui_widget->elementItem();

   QQueue<QPair<QTreeWidgetItem *, DomItem *>> pendingQueue;

   for (int i = 0; i < treeWidget->topLevelItemCount(); i++) {
      pendingQueue.enqueue(qMakePair(treeWidget->topLevelItem(i), (DomItem *)nullptr));
   }

   while (! pendingQueue.isEmpty()) {
      const QPair<QTreeWidgetItem *, DomItem *> pair = pendingQueue.dequeue();
      QTreeWidgetItem *item = pair.first;
      DomItem *parentDomItem = pair.second;

      DomItem *currentDomItem = new DomItem;

      QList<DomProperty *> properties;
      for (int c = 0; c < treeWidget->columnCount(); c++) {
         for (const QFormBuilderStrings::TextRoleNName &it : strings.itemTextRoles)
            if ((p = saveText(it.second, item->data(c, it.first.second)))) {
               properties.append(p);
            }

         for (const QFormBuilderStrings::RoleNName &it : strings.itemRoles)
            if ((v = item->data(c, it.first)).isValid() &&
               (p = variantToDomProperty(this, QAbstractFormBuilderGadget::staticMetaObject(), it.second, v))) {
               properties.append(p);
            }

         if ((p = saveResource(item->data(c, Qt::DecorationPropertyRole)))) {
            properties.append(p);
         }
      }

      storeItemFlags(item, &properties);
      currentDomItem->setElementProperty(properties);

      if (parentDomItem) {
         QList<DomItem *> childrenItems = parentDomItem->elementItem();
         childrenItems.append(currentDomItem);
         parentDomItem->setElementItem(childrenItems);
      } else {
         items.append(currentDomItem);
      }

      for (int i = 0; i < item->childCount(); i++) {
         pendingQueue.enqueue(qMakePair(item->child(i), currentDomItem));
      }
   }

   ui_widget->setElementItem(items);
}

void QAbstractFormBuilder::saveTableWidgetExtraInfo(QTableWidget *tableWidget, DomWidget *ui_widget, DomWidget *ui_parentWidget)
{
   (void) ui_parentWidget;

   // save the horizontal header
   QList<DomColumn *> columns;

   for (int c = 0; c < tableWidget->columnCount(); c++) {
      QList<DomProperty *> properties;
      QTableWidgetItem *item = tableWidget->horizontalHeaderItem(c);
      if (item) {
         storeItemProps(this, item, &properties);
      }

      DomColumn *column = new DomColumn;
      column->setElementProperty(properties);
      columns.append(column);
   }

   ui_widget->setElementColumn(columns);

   // save the vertical header
   QList<DomRow *> rows;
   for (int r = 0; r < tableWidget->rowCount(); r++) {
      QList<DomProperty *> properties;
      QTableWidgetItem *item = tableWidget->verticalHeaderItem(r);
      if (item) {
         storeItemProps(this, item, &properties);
      }

      DomRow *row = new DomRow;
      row->setElementProperty(properties);
      rows.append(row);
   }
   ui_widget->setElementRow(rows);

   QList<DomItem *> items = ui_widget->elementItem();
   for (int r = 0; r < tableWidget->rowCount(); r++)
      for (int c = 0; c < tableWidget->columnCount(); c++) {
         QTableWidgetItem *item = tableWidget->item(r, c);
         if (item) {
            QList<DomProperty *> properties;
            storeItemPropsNFlags(this, item, &properties);

            DomItem *domItem = new DomItem;
            domItem->setAttributeRow(r);
            domItem->setAttributeColumn(c);
            domItem->setElementProperty(properties);
            items.append(domItem);
         }
      }

   ui_widget->setElementItem(items);
}

void QAbstractFormBuilder::saveListWidgetExtraInfo(QListWidget *listWidget, DomWidget *ui_widget, DomWidget *ui_parentWidget)
{
   (void) ui_parentWidget;

   QList<DomItem *> ui_items = ui_widget->elementItem();
   for (int i = 0; i < listWidget->count(); ++i) {
      QList<DomProperty *> properties;
      storeItemPropsNFlags(this, listWidget->item(i), &properties);

      DomItem *ui_item = new DomItem();
      ui_item->setElementProperty(properties);
      ui_items.append(ui_item);
   }

   ui_widget->setElementItem(ui_items);
}

void QAbstractFormBuilder::saveComboBoxExtraInfo(QComboBox *comboBox, DomWidget *ui_widget, DomWidget *ui_parentWidget)
{
   (void) ui_parentWidget;
   QList<DomItem *> ui_items = ui_widget->elementItem();

   const int count = comboBox->count();

   for (int i = 0; i < count; ++i) {
      // We might encounter items for which both builders return 0 in Designer
      // (indicating a custom combo adding items in the constructor). Ignore those.
      DomProperty *textProperty = saveText(QFormBuilderStrings::instance().textAttribute,
            comboBox->itemData(i, Qt::DisplayPropertyRole));
      DomProperty *iconProperty = saveResource(comboBox->itemData(i, Qt::DecorationPropertyRole));
      if (textProperty || iconProperty) {
         QList<DomProperty *> properties;
         if (textProperty) {
            properties.push_back(textProperty);
         }
         if (iconProperty) {
            properties.push_back(iconProperty);
         }

         DomItem *ui_item = new DomItem();
         ui_item->setElementProperty(properties);
         ui_items.push_back(ui_item);
      }
   }

   ui_widget->setElementItem(ui_items);
}

void QAbstractFormBuilder::saveButtonExtraInfo(const QAbstractButton *widget, DomWidget *ui_widget, DomWidget *)
{
   if (const QButtonGroup *buttonGroup = widget->group()) {
      QList<DomProperty *> attributes = ui_widget->elementAttribute();

      DomString *domString = new DomString();
      domString->setText(buttonGroup->objectName());
      domString->setAttributeNotr(QString("true"));
      DomProperty *domProperty = new DomProperty();
      domProperty->setAttributeName(buttonGroupPropertyC);
      domProperty->setElementString(domString);
      attributes += domProperty;
      ui_widget->setElementAttribute(attributes);
   }
}

void QAbstractFormBuilder::saveItemViewExtraInfo(const QAbstractItemView *itemView,
   DomWidget *ui_widget, DomWidget *)
{
   //
   // Special handling for qtableview/qtreeview fake header attributes
   //
   static QStringList realPropertyNames =
      (QStringList() << QString("visible")
         << QString("cascadingSectionResizes")
         << QString("defaultSectionSize")
         << QString("highlightSections")
         << QString("minimumSectionSize")
         << QString("showSortIndicator")
         << QString("stretchLastSection"));

   if (const QTreeView *treeView = dynamic_cast<const QTreeView *>(itemView)) {
      QList<DomProperty *> viewProperties = ui_widget->elementAttribute();
      QList<DomProperty *> headerProperties = computeProperties(treeView->header());
      for (const QString &realPropertyName : realPropertyNames) {
         const QString upperPropertyName = realPropertyName.at(0).toUpper()
            + realPropertyName.mid(1);
         const QString fakePropertyName = QString("header") + upperPropertyName;
         for (DomProperty *property : headerProperties) {
            if (property->attributeName() == realPropertyName) {
               property->setAttributeName(fakePropertyName);
               viewProperties << property;
            }
         }
      }
      ui_widget->setElementAttribute(viewProperties);
   } else if (const QTableView *tableView = dynamic_cast<const QTableView *>(itemView)) {
      static QStringList headerPrefixes =
         (QStringList() << QString("horizontalHeader")
            << QString("verticalHeader"));

      QList<DomProperty *> viewProperties = ui_widget->elementAttribute();
      for (const QString &headerPrefix : headerPrefixes) {
         QList<DomProperty *> headerProperties;
         if (headerPrefix == QString("horizontalHeader")) {
            headerProperties = computeProperties(tableView->horizontalHeader());
         } else {
            headerProperties = computeProperties(tableView->verticalHeader());
         }
         for (const QString &realPropertyName : realPropertyNames) {
            const QString upperPropertyName = realPropertyName.at(0).toUpper()
               + realPropertyName.mid(1);
            const QString fakePropertyName = headerPrefix + upperPropertyName;
            for (DomProperty *property : headerProperties) {
               if (property->attributeName() == realPropertyName) {
                  property->setAttributeName(fakePropertyName);
                  viewProperties << property;
               }
            }
         }
      }
      ui_widget->setElementAttribute(viewProperties);
   }
}

void QAbstractFormBuilder::setResourceBuilder(QResourceBuilder *builder)
{
   d->setResourceBuilder(builder);
}

QResourceBuilder *QAbstractFormBuilder::resourceBuilder() const
{
   return d->resourceBuilder();
}

void QAbstractFormBuilder::setTextBuilder(QTextBuilder *builder)
{
   d->setTextBuilder(builder);
}

QTextBuilder *QAbstractFormBuilder::textBuilder() const
{
   return d->textBuilder();
}

void QAbstractFormBuilder::saveExtraInfo(QWidget *widget, DomWidget *ui_widget,
   DomWidget *ui_parentWidget)
{
   if (QListWidget *listWidget = dynamic_cast<QListWidget *>(widget)) {
      saveListWidgetExtraInfo(listWidget, ui_widget, ui_parentWidget);

   } else if (QTreeWidget *treeWidget = dynamic_cast<QTreeWidget *>(widget)) {
      saveTreeWidgetExtraInfo(treeWidget, ui_widget, ui_parentWidget);

   } else if (QTableWidget *tableWidget = dynamic_cast<QTableWidget *>(widget)) {
      saveTableWidgetExtraInfo(tableWidget, ui_widget, ui_parentWidget);

   } else if (QComboBox *comboBox = dynamic_cast<QComboBox *>(widget)) {
      if (!dynamic_cast<QFontComboBox *>(widget)) {
         saveComboBoxExtraInfo(comboBox, ui_widget, ui_parentWidget);
      }

   } else if (QAbstractButton *ab = dynamic_cast<QAbstractButton *>(widget)) {
      saveButtonExtraInfo(ab, ui_widget, ui_parentWidget);
   }

   if (QAbstractItemView *itemView = dynamic_cast<QAbstractItemView *>(widget)) {
      saveItemViewExtraInfo(itemView, ui_widget, ui_parentWidget);
   }
}

void QAbstractFormBuilder::loadListWidgetExtraInfo(DomWidget *ui_widget, QListWidget *listWidget, QWidget *parentWidget)
{
   (void) parentWidget;

   const QFormBuilderStrings &strings = QFormBuilderStrings::instance();

   for (DomItem *ui_item : ui_widget->elementItem()) {
      const DomPropertyHash properties = propertyMap(ui_item->elementProperty());
      QListWidgetItem *item = new QListWidgetItem(listWidget);
      loadItemPropsNFlags<QListWidgetItem>(this, item, properties);
   }

   DomProperty *currentRow = propertyMap(ui_widget->elementProperty()).value(strings.currentRowProperty);
   if (currentRow) {
      listWidget->setCurrentRow(currentRow->elementNumber());
   }
}

void QAbstractFormBuilder::loadTreeWidgetExtraInfo(DomWidget *ui_widget, QTreeWidget *treeWidget, QWidget *parentWidget)
{
   (void) parentWidget;

   const QFormBuilderStrings &strings = QFormBuilderStrings::instance();
   const QMetaEnum itemFlags_enum     = metaEnum<QAbstractFormBuilderGadget>("itemFlags");
   const QList<DomColumn *> columns   = ui_widget->elementColumn();

   if (columns.count() > 0) {
      treeWidget->setColumnCount(columns.count());
   }

   for (int i = 0; i < columns.count(); ++i) {
      const DomColumn *c = columns.at(i);
      const DomPropertyHash properties = propertyMap(c->elementProperty());

      DomProperty *p;
      QVariant v;

      for (const QFormBuilderStrings::RoleNName &it : strings.itemRoles)
         if ((p = properties.value(it.second)) && (v = toVariant(QAbstractFormBuilderGadget::staticMetaObject(), p)).isValid()) {
            treeWidget->headerItem()->setData(i, it.first, v);
         }

      for (const QFormBuilderStrings::TextRoleNName &it : strings.itemTextRoles) {
         if ((p = properties.value(it.second))) {
            v = textBuilder()->loadText(p);
            QVariant nativeValue = textBuilder()->toNativeValue(v);

            treeWidget->headerItem()->setData(i, it.first.first, nativeValue.value<QString>());
            treeWidget->headerItem()->setData(i, it.first.second, v);
         }
      }

      if ((p = properties.value(strings.iconAttribute))) {
         v = resourceBuilder()->loadResource(workingDirectory(), p);
         QVariant nativeValue = resourceBuilder()->toNativeValue(v);

         treeWidget->headerItem()->setIcon(i, nativeValue.value<QIcon>());
         treeWidget->headerItem()->setData(i, Qt::DecorationPropertyRole, v);
      }
   }

   QQueue<QPair<DomItem *, QTreeWidgetItem *>> pendingQueue;

   for (DomItem *ui_item : ui_widget->elementItem()) {
      pendingQueue.enqueue(std::make_pair(ui_item, nullptr));
   }

   while (! pendingQueue.isEmpty()) {
      const QPair<DomItem *, QTreeWidgetItem *> pair = pendingQueue.dequeue();
      const DomItem *domItem = pair.first;
      QTreeWidgetItem *parentItem = pair.second;

      QTreeWidgetItem *currentItem = nullptr;

      if (parentItem) {
         currentItem = new QTreeWidgetItem(parentItem);
      } else {
         currentItem = new QTreeWidgetItem(treeWidget);
      }

      const QList<DomProperty *> properties = domItem->elementProperty();
      int col = -1;

      for (DomProperty *property : properties) {
         if (property->attributeName() == strings.flagsAttribute && !property->elementSet().isEmpty()) {
            currentItem->setFlags(enumKeysToValue<Qt::ItemFlags>(itemFlags_enum, property->elementSet().toLatin1()));

         } else if (property->attributeName() == strings.textAttribute && property->elementString()) {
            ++col;

            QVariant textV = textBuilder()->loadText(property);
            QVariant nativeValue = textBuilder()->toNativeValue(textV);

            currentItem->setText(col, nativeValue.value<QString>());
            currentItem->setData(col, Qt::DisplayPropertyRole, textV);

         } else if (col >= 0) {
            if (property->attributeName() == strings.iconAttribute) {
               QVariant v = resourceBuilder()->loadResource(workingDirectory(), property);

               if (v.isValid()) {
                  QVariant nativeValue = resourceBuilder()->toNativeValue(v);
                  currentItem->setIcon(col, nativeValue.value<QIcon>());
                  currentItem->setData(col, Qt::DecorationPropertyRole, v);
               }

            } else {
               QVariant v;
               int role = strings.treeItemRoleHash.value(property->attributeName(), (Qt::ItemDataRole) - 1);

               if (role >= 0) {
                  if ((v = toVariant(QAbstractFormBuilderGadget::staticMetaObject(), property)).isValid()) {
                     currentItem->setData(col, role, v);
                  }

               } else {
                  QPair<Qt::ItemDataRole, Qt::ItemDataRole> rolePair =
                     strings.treeItemTextRoleHash.value(property->attributeName(),
                        std::make_pair((Qt::ItemDataRole) - 1, (Qt::ItemDataRole) - 1));

                  if (rolePair.first >= 0) {
                     QVariant textV = textBuilder()->loadText(property);
                     QVariant nativeValue = textBuilder()->toNativeValue(textV);

                     currentItem->setData(col, rolePair.first, nativeValue.value<QString>());
                     currentItem->setData(col, rolePair.second, textV);
                  }
               }
            }
         }
      }

      for (DomItem *childItem : domItem->elementItem()) {
         pendingQueue.enqueue(qMakePair(childItem, currentItem));
      }
   }
}

void QAbstractFormBuilder::loadTableWidgetExtraInfo(DomWidget *ui_widget, QTableWidget *tableWidget, QWidget *parentWidget)
{
   (void) parentWidget;

   const QList<DomColumn *> columns = ui_widget->elementColumn();

   if (columns.count() > 0) {
      tableWidget->setColumnCount(columns.count());
   }

   for (int i = 0; i < columns.count(); i++) {
      DomColumn *c = columns.at(i);
      const DomPropertyHash properties = propertyMap(c->elementProperty());

      if (! properties.isEmpty()) {
         QTableWidgetItem *item = new QTableWidgetItem;
         loadItemProps(this, item, properties);
         tableWidget->setHorizontalHeaderItem(i, item);
      }
   }

   const QList<DomRow *> rows = ui_widget->elementRow();
   if (rows.count() > 0) {
      tableWidget->setRowCount(rows.count());
   }

   for (int i = 0; i < rows.count(); i++) {
      const DomRow *r = rows.at(i);
      const DomPropertyHash properties = propertyMap(r->elementProperty());

      if (!properties.isEmpty()) {
         QTableWidgetItem *item = new QTableWidgetItem;
         loadItemProps(this, item, properties);
         tableWidget->setVerticalHeaderItem(i, item);
      }
   }

   for (DomItem *ui_item : ui_widget->elementItem()) {
      if (ui_item->hasAttributeRow() && ui_item->hasAttributeColumn()) {
         const DomPropertyHash properties = propertyMap(ui_item->elementProperty());
         QTableWidgetItem *item = new QTableWidgetItem;
         loadItemPropsNFlags(this, item, properties);
         tableWidget->setItem(ui_item->attributeRow(), ui_item->attributeColumn(), item);
      }
   }
}

void QAbstractFormBuilder::loadComboBoxExtraInfo(DomWidget *ui_widget, QComboBox *comboBox, QWidget *parentWidget)
{
   (void) parentWidget;

   const QFormBuilderStrings &strings = QFormBuilderStrings::instance();

   for (DomItem *ui_item : ui_widget->elementItem()) {
      const DomPropertyHash properties = propertyMap(ui_item->elementProperty());
      QString text;
      QIcon icon;
      QVariant textData;
      QVariant iconData;

      DomProperty *p = nullptr;

      p = properties.value(strings.textAttribute);

      if (p && p->elementString()) {
         textData = textBuilder()->loadText(p);

         QVariant data = textBuilder()->toNativeValue(textData);
         text = data.value<QString>();
      }

      p = properties.value(strings.iconAttribute);
      if (p) {
         iconData = resourceBuilder()->loadResource(workingDirectory(), p);

         QVariant data = resourceBuilder()->toNativeValue(iconData);
         icon = data.value<QIcon>();
      }

      comboBox->addItem(icon, text);
      comboBox->setItemData((comboBox->count() - 1), iconData, Qt::DecorationPropertyRole);
      comboBox->setItemData((comboBox->count() - 1), textData, Qt::DisplayPropertyRole);
   }

   DomProperty *currentIndex = propertyMap(ui_widget->elementProperty()).value(strings.currentIndexProperty);
   if (currentIndex) {
      comboBox->setCurrentIndex(currentIndex->elementNumber());
   }
}

// Get the button group name out of a widget's attribute list
static QString buttonGroupName(const DomWidget *ui_widget)
{
   const QList<DomProperty *> attributes = ui_widget->elementAttribute();

   if (attributes.empty()) {
      return QString();
   }

   const QString buttonGroupProperty = buttonGroupPropertyC;
   auto cend = attributes.constEnd();

   for (auto it = attributes.constBegin(); it != cend; ++it) {
      if ( (*it)->attributeName() == buttonGroupProperty) {
         return (*it)->elementString()->text();
      }
   }

   return QString();
}

void QAbstractFormBuilder::loadButtonExtraInfo(const DomWidget *ui_widget, QAbstractButton *button, QWidget *)
{
   typedef QFormBuilderExtra::ButtonGroupHash ButtonGroupHash;

   const QString groupName = buttonGroupName(ui_widget);
   if (groupName.isEmpty()) {
      return;
   }

   // Find entry
   ButtonGroupHash &buttonGroups = d->buttonGroups();
   ButtonGroupHash::iterator it = buttonGroups.find(groupName);

   if (it == buttonGroups.end()) {
      return;
   }

   // Create button group on demand?
   QButtonGroup *&group = it.value().second;

   if (group == nullptr) {
      group = new QButtonGroup;
      group->setObjectName(groupName);
      applyProperties(group,  it.value().first->elementProperty());
   }

   group->addButton(button);
}

void QAbstractFormBuilder::loadItemViewExtraInfo(DomWidget *ui_widget, QAbstractItemView *itemView, QWidget *)
{
   // Special handling for qtableview/qtreeview fake header attributes
   static QStringList realPropertyNames =
      (QStringList() << QString("visible")
         << QString("cascadingSectionResizes")
         << QString("defaultSectionSize")
         << QString("highlightSections")
         << QString("minimumSectionSize")
         << QString("showSortIndicator")
         << QString("stretchLastSection"));

   if (QTreeView *treeView = dynamic_cast<QTreeView *>(itemView)) {
      QList<DomProperty *> allAttributes = ui_widget->elementAttribute();
      QList<DomProperty *> headerProperties;

      for (const QString &realPropertyName : realPropertyNames) {
         const QString upperPropertyName = realPropertyName.at(0).toUpper()
            + realPropertyName.mid(1);
         const QString fakePropertyName = QString("header") + upperPropertyName;
         for (DomProperty *attr : allAttributes) {
            if (attr->attributeName() == fakePropertyName) {
               attr->setAttributeName(realPropertyName);
               headerProperties << attr;
            }
         }
      }
      applyProperties(treeView->header(), headerProperties);
   } else if (QTableView *tableView = dynamic_cast<QTableView *>(itemView)) {
      static QStringList headerPrefixes =
         (QStringList() << QString("horizontalHeader")
            << QString("verticalHeader"));

      QList<DomProperty *> allAttributes = ui_widget->elementAttribute();
      for (const QString &headerPrefix : headerPrefixes) {
         QList<DomProperty *> headerProperties;
         for (const QString &realPropertyName : realPropertyNames) {
            const QString upperPropertyName = realPropertyName.at(0).toUpper()
               + realPropertyName.mid(1);
            const QString fakePropertyName = headerPrefix + upperPropertyName;
            for (DomProperty *attr : allAttributes) {
               if (attr->attributeName() == fakePropertyName) {
                  attr->setAttributeName(realPropertyName);
                  headerProperties << attr;
               }
            }
         }
         if (headerPrefix == QString("horizontalHeader")) {
            applyProperties(tableView->horizontalHeader(), headerProperties);
         } else {
            applyProperties(tableView->verticalHeader(), headerProperties);
         }
      }
   }
}

void QAbstractFormBuilder::loadExtraInfo(DomWidget *ui_widget, QWidget *widget, QWidget *parentWidget)
{
   const QFormBuilderStrings &strings = QFormBuilderStrings::instance();

   if (QListWidget *listWidget = dynamic_cast<QListWidget *>(widget)) {
      loadListWidgetExtraInfo(ui_widget, listWidget, parentWidget);

   } else if (QTreeWidget *treeWidget = dynamic_cast<QTreeWidget *>(widget)) {
      loadTreeWidgetExtraInfo(ui_widget, treeWidget, parentWidget);

   } else if (QTableWidget *tableWidget = dynamic_cast<QTableWidget *>(widget)) {
      loadTableWidgetExtraInfo(ui_widget, tableWidget, parentWidget);

   } else if (QComboBox *comboBox = dynamic_cast<QComboBox *>(widget)) {
      if (!dynamic_cast<QFontComboBox *>(widget)) {
         loadComboBoxExtraInfo(ui_widget, comboBox, parentWidget);
      }

   } else if (QTabWidget *tabWidget = dynamic_cast<QTabWidget *>(widget)) {
      const DomProperty *currentIndex = propertyMap(ui_widget->elementProperty()).value(strings.currentIndexProperty);
      if (currentIndex) {
         tabWidget->setCurrentIndex(currentIndex->elementNumber());
      }

   } else if (QStackedWidget *stackedWidget = dynamic_cast<QStackedWidget *>(widget)) {
      const DomProperty *currentIndex = propertyMap(ui_widget->elementProperty()).value(strings.currentIndexProperty);
      if (currentIndex) {
         stackedWidget->setCurrentIndex(currentIndex->elementNumber());
      }

   } else if (QToolBox *toolBox = dynamic_cast<QToolBox *>(widget)) {
      const DomProperty *currentIndex = propertyMap(ui_widget->elementProperty()).value(strings.currentIndexProperty);
      if (currentIndex) {
         toolBox->setCurrentIndex(currentIndex->elementNumber());
      }
      const DomProperty *tabSpacing = propertyMap(ui_widget->elementProperty()).value(strings.tabSpacingProperty);
      if (tabSpacing) {
         toolBox->layout()->setSpacing(tabSpacing->elementNumber());
      }

   } else if (QAbstractButton *ab = dynamic_cast<QAbstractButton *>(widget)) {
      loadButtonExtraInfo(ui_widget, ab, parentWidget);
   }
   if (QAbstractItemView *itemView = dynamic_cast<QAbstractItemView *>(widget)) {
      loadItemViewExtraInfo(ui_widget, itemView, parentWidget);
   }
}

QDir QAbstractFormBuilder::workingDirectory() const
{
   return d->m_workingDirectory;
}

void QAbstractFormBuilder::setWorkingDirectory(const QDir &directory)
{
   d->m_workingDirectory = directory;
}

DomAction *QAbstractFormBuilder::createDom(QAction *action)
{
   if (action->parentWidget() == action->menu() || action->isSeparator()) {
      return nullptr;
   }

   DomAction *ui_action = new DomAction;
   ui_action->setAttributeName(action->objectName());

   const QList<DomProperty *> properties = computeProperties(action);
   ui_action->setElementProperty(properties);

   return ui_action;
}

DomButtonGroup *QAbstractFormBuilder::createDom(QButtonGroup *buttonGroup)
{
   if (buttonGroup->buttons().count() == 0) { // Empty group left over on form?
      return nullptr;
   }
   DomButtonGroup *domButtonGroup = new DomButtonGroup;
   domButtonGroup->setAttributeName(buttonGroup->objectName());

   QList<DomProperty *> properties = computeProperties(buttonGroup);
   domButtonGroup->setElementProperty(properties);
   return domButtonGroup;
}

DomActionGroup *QAbstractFormBuilder::createDom(QActionGroup *actionGroup)
{
   DomActionGroup *ui_action_group = new DomActionGroup;
   ui_action_group->setAttributeName(actionGroup->objectName());

   QList<DomProperty *> properties = computeProperties(actionGroup);
   ui_action_group->setElementProperty(properties);

   QList<DomAction *> ui_actions;

   for (QAction *action : actionGroup->actions()) {
      if (DomAction *ui_action = createDom(action)) {
         ui_actions.append(ui_action);
      }
   }

   ui_action_group->setElementAction(ui_actions);

   return ui_action_group;
}

void QAbstractFormBuilder::addMenuAction(QAction *action)
{
   (void) action;
}

void QAbstractFormBuilder::reset()
{
   d->m_laidout.clear();
   d->m_actions.clear();
   d->m_actionGroups.clear();
   d->m_defaultMargin  = INT_MIN;
   d->m_defaultSpacing = INT_MIN;
}

QMetaEnum QAbstractFormBuilder::toolBarAreaMetaEnum()
{
   return metaEnum<QAbstractFormBuilderGadget>("toolBarArea");
}

void QAbstractFormBuilder::setIconProperty(DomProperty &p, const IconPaths &ip) const
{
   DomResourceIcon *dpi = new DomResourceIcon;

   /* TODO
      if (!ip.second.isEmpty())
          pix->setAttributeResource(ip.second);
   */
   dpi->setText(ip.first);

   p.setAttributeName(QFormBuilderStrings::instance().iconAttribute);
   p.setElementIconSet(dpi);
}

void QAbstractFormBuilder::setPixmapProperty(DomProperty &p, const IconPaths &ip) const
{
   DomResourcePixmap *pix = new DomResourcePixmap;
   if (!ip.second.isEmpty()) {
      pix->setAttributeResource(ip.second);
   }

   pix->setText(ip.first);

   p.setAttributeName(QFormBuilderStrings::instance().pixmapAttribute);
   p.setElementPixmap(pix);
}

DomProperty *QAbstractFormBuilder::saveResource(const QVariant &v) const
{
   if (! v.isValid()) {
      return nullptr;
   }

   DomProperty *p = resourceBuilder()->saveResource(workingDirectory(), v);
   if (p) {
      p->setAttributeName(QFormBuilderStrings::instance().iconAttribute);
   }
   return p;
}

DomProperty *QAbstractFormBuilder::saveText(const QString &attributeName, const QVariant &v) const
{
   if (! v.isValid()) {
      return nullptr;
   }

   DomProperty *p = textBuilder()->saveText(v);

   if (p) {
      p->setAttributeName(attributeName);
   }

   return p;
}

const DomResourcePixmap *QAbstractFormBuilder::domPixmap(const DomProperty *p)
{
   switch (p->kind()) {
      case DomProperty::IconSet:
         break;

      case DomProperty::Pixmap:
         return p->elementPixmap();

      default:
         break;
   }

   return nullptr;
}

QString QAbstractFormBuilder::errorString() const
{
   return d->m_errorString;
}
