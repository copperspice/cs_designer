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
#include <abstract_formwindow.h>
#include <abstract_formwindowcursor.h>
#include <abstract_integration.h>
#include <abstract_introspection.h>
#include <abstract_language.h>
#include <designer_command.h>
#include <designer_membersheet.h>
#include <designer_object_inspector.h>
#include <designer_property_command.h>
#include <designer_propertysheet.h>
#include <designer_taskmenu.h>
#include <designer_utils.h>
#include <extension.h>
#include <layout.h>
#include <plaintext_editor.h>
#include <promotion_taskmenu.h>
#include <richtext_editor.h>
#include <signalslot_dialog.h>
#include <stylesheet_editor.h>
#include <textproperty_editor.h>
#include <ui_signal_select_dialog.h>
#include <widgetfactory.h>

#include <formlayoutmenu_p.h>
#include <metadatabase_p.h>
#include <morphmenu_p.h>
#include <shared_enums_p.h>
#include <spacer_widget_p.h>
#include <widgetdatabase_p.h>

#include <QAction>
#include <QActionGroup>
#include <QCoreApplication>
#include <QDialogButtonBox>
#include <QLabel>
#include <QMainWindow>
#include <QMenuBar>
#include <QPushButton>
#include <QSignalMapper>
#include <QStatusBar>
#include <QUndoStack>
#include <QVBoxLayout>
#include <QWidget>

constexpr const int APPLY_MIN_WIDTH  = 0x1;
constexpr const int APPLY_MIN_HEIGHT = 0x2;

constexpr const int APPLY_MAX_WIDTH  = 0x4;
constexpr const int APPLY_MAX_HEIGHT = 0x8;

static QMenuBar *findMenuBar(const QWidget *widget)
{
   const QList<QObject *> children = widget->children();

   for (QObject *obj : widget->children()) {
      if (QMenuBar *mb = dynamic_cast<QMenuBar *>(obj)) {
         return mb;
      }
   }

   return nullptr;
}

static QStatusBar *findStatusBar(const QWidget *widget)
{
   const QList<QObject *> children = widget->children();

   for (QObject *obj : widget->children()) {
      if (QStatusBar *sb = dynamic_cast<QStatusBar *>(obj)) {
         return sb;
      }
   }

   return nullptr;
}

static inline QAction *createSeparatorHelper(QObject *parent)
{
   QAction *rc = new QAction(parent);
   rc->setSeparator(true);

   return rc;
}

static QString objName(const QDesignerFormEditorInterface *core, QObject *object)
{
   QDesignerPropertySheetExtension *sheet =
         qt_extension<QDesignerPropertySheetExtension *>(core->extensionManager(), object);

   Q_ASSERT(sheet != nullptr);

   const QString objectNameProperty = QString("objectName");
   const int index = sheet->indexOf(objectNameProperty);

   QVariant data = sheet->property(index);

   const qdesigner_internal::PropertySheetStringValue objectNameValue =
         data.value<qdesigner_internal::PropertySheetStringValue>();

   return objectNameValue.value();
}

class ObjectNameDialog : public QDialog
{
 public:
   ObjectNameDialog(QWidget *parent, const QString &oldName);
   QString newObjectName() const;

 private:
   qdesigner_internal::TextPropertyEditor *m_editor;
};

ObjectNameDialog::ObjectNameDialog(QWidget *parent, const QString &oldName)
   : QDialog(parent)
{
   setWindowTitle(QCoreApplication::translate("ObjectNameDialog", "Change Object Name"));
   setWindowFlags(windowFlags() & ~Qt::WindowContextHelpButtonHint);

   QLabel *label = new QLabel(QCoreApplication::translate("ObjectNameDialog", "Object Name:"));

   m_editor = new qdesigner_internal::TextPropertyEditor(this,
         qdesigner_internal::TextPropertyEditor::EmbeddingNone, qdesigner_internal::ValidationObjectName);

   m_editor->setText(oldName);
   m_editor->selectAll();
   m_editor->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);

   QDialogButtonBox *buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel,
      Qt::Horizontal, this);

   QPushButton *okButton = buttonBox->button(QDialogButtonBox::Ok);
   okButton->setDefault(true);

   QGridLayout *layout = new QGridLayout(this);
   layout->addWidget(label,     0, 0);
   layout->addWidget(m_editor,  0, 1);
   layout->addItem(new QSpacerItem(0, 5), 1, 0, 1, 2);
   layout->addWidget(buttonBox, 2, 0, 1, 2);

   // stretch the editor as the screen width changes
   layout->setColumnStretch(1, 1);

   // adjust the window width ( for the window title )
   QSize size = QSize(300, this->height());
   resize(size);

   connect(buttonBox, &QDialogButtonBox::accepted, this, &QDialog::accept);
   connect(buttonBox, &QDialogButtonBox::rejected, this, &QDialog::reject);
}

QString ObjectNameDialog::newObjectName() const
{
   return m_editor->text();
}

namespace qdesigner_internal {

// Sub menu displaying the alignment options of a widget in a managed
// grid/box layout cell.
class LayoutAlignmentMenu
{
 public:
   explicit LayoutAlignmentMenu(QObject *parent);

   QAction *subMenuAction() const {
      return m_subMenuAction;
   }

   void connect(QObject *receiver, const QString &aSlot);

   // Set up enabled state and checked actions according to widget (managed box/grid)
   bool setAlignment(const QDesignerFormEditorInterface *core, QWidget *w);

   // Return the currently checked alignment
   Qt::Alignment alignment() const;

 private:
   enum Actions { HorizNone, Left, HorizCenter, Right, VerticalNone, Top, VerticalCenter, Bottom };
   static QAction *createAction(const QString &text, int data, QMenu *menu, QActionGroup *ag);

   QAction *m_subMenuAction;
   QActionGroup *m_horizGroup;
   QActionGroup *m_verticalGroup;
   QAction *m_actions[Bottom + 1];
};

QAction *LayoutAlignmentMenu::createAction(const QString &text, int data, QMenu *menu, QActionGroup *ag)
{
   QAction *a = new QAction(text, nullptr);
   a->setCheckable(true);
   a->setData(QVariant(data));
   menu->addAction(a);
   ag->addAction(a);

   return a;
}

LayoutAlignmentMenu::LayoutAlignmentMenu(QObject *parent)
   : m_subMenuAction(new QAction(QDesignerTaskMenu::tr("Layout Alignment"), parent)),
     m_horizGroup(new QActionGroup(parent)), m_verticalGroup(new QActionGroup(parent))
{
   m_horizGroup->setExclusive(true);
   m_verticalGroup->setExclusive(true);

   QMenu *menu = new QMenu;
   m_subMenuAction->setMenu(menu);

   m_actions[HorizNone] = createAction(QDesignerTaskMenu::tr("No Horizontal Alignment"), 0, menu, m_horizGroup);
   m_actions[Left] = createAction(QDesignerTaskMenu::tr("Left"), Qt::AlignLeft, menu, m_horizGroup);
   m_actions[HorizCenter] = createAction(QDesignerTaskMenu::tr("Center Horizontally"), Qt::AlignHCenter, menu, m_horizGroup);
   m_actions[Right] = createAction(QDesignerTaskMenu::tr("Right"), Qt::AlignRight, menu, m_horizGroup);

   menu->addSeparator();

   m_actions[VerticalNone] = createAction(QDesignerTaskMenu::tr("No Vertical Alignment"), 0, menu, m_verticalGroup);
   m_actions[Top] = createAction(QDesignerTaskMenu::tr("Top"), Qt::AlignTop, menu, m_verticalGroup);
   m_actions[VerticalCenter] = createAction(QDesignerTaskMenu::tr("Center Vertically"), Qt::AlignVCenter, menu, m_verticalGroup);
   m_actions[Bottom] = createAction(QDesignerTaskMenu::tr("Bottom"), Qt::AlignBottom, menu, m_verticalGroup);
}

void LayoutAlignmentMenu::connect(QObject *receiver, const QString &aSlot)
{
   QObject::connect(m_horizGroup,    SIGNAL(triggered(QAction *)), receiver, aSlot);
   QObject::connect(m_verticalGroup, SIGNAL(triggered(QAction *)), receiver, aSlot);
}

bool LayoutAlignmentMenu::setAlignment(const QDesignerFormEditorInterface *core, QWidget *w)
{
   bool enabled;
   const Qt::Alignment alignment = LayoutAlignmentCommand::alignmentOf(core, w, &enabled);

   if (!enabled) {
      m_subMenuAction->setEnabled(false);
      m_actions[HorizNone]->setChecked(true);
      m_actions[VerticalNone]->setChecked(true);

      return false;
   }

   // Get alignment
   switch (alignment & Qt::AlignHorizontal_Mask) {
      case Qt::AlignLeft:
         m_actions[Left]->setChecked(true);
         break;

      case Qt::AlignHCenter:
         m_actions[HorizCenter]->setChecked(true);
         break;

      case Qt::AlignRight:
         m_actions[Right]->setChecked(true);
         break;

      default:
         m_actions[HorizNone]->setChecked(true);
         break;
   }

   switch (alignment & Qt::AlignVertical_Mask) {
      case Qt::AlignTop:
         m_actions[Top]->setChecked(true);
         break;

      case Qt::AlignVCenter:
         m_actions[VerticalCenter]->setChecked(true);
         break;

      case Qt::AlignBottom:
         m_actions[Bottom]->setChecked(true);
         break;

      default:
         m_actions[VerticalNone]->setChecked(true);
         break;
   }

   return true;
}

Qt::Alignment LayoutAlignmentMenu::alignment() const
{
   Qt::Alignment alignment = nullptr;

   if (const QAction *horizAction = m_horizGroup->checkedAction()) {
      if (const int horizAlign = horizAction->data().toInt()) {
         alignment |= static_cast<Qt::Alignment>(horizAlign);
      }
   }

   if (const QAction *vertAction = m_verticalGroup->checkedAction()) {
      if (const int vertAlign = vertAction->data().toInt()) {
         alignment |= static_cast<Qt::Alignment>(vertAlign);
      }
   }

   return alignment;
}

class QDesignerTaskMenuPrivate
{
 public:
   QDesignerTaskMenuPrivate(QWidget *widget, QObject *parent);

   QDesignerTaskMenu *m_q;
   QPointer<QWidget> m_widget;
   QAction *m_separator;
   QAction *m_separator2;
   QAction *m_separator3;
   QAction *m_separator4;
   QAction *m_separator5;
   QAction *m_separator6;
   QAction *m_separator7;
   QAction *m_changeObjectNameAction;
   QAction *m_changeToolTip;
   QAction *m_changeWhatsThis;
   QAction *m_changeStyleSheet;

   MorphMenu *m_morphMenu;
   FormLayoutMenu *m_formLayoutMenu;

   QAction *m_addMenuBar;
   QAction *m_addToolBar;
   QAction *m_addStatusBar;
   QAction *m_removeStatusBar;
   QAction *m_containerFakeMethods;
   QAction *m_navigateToSlot;

   PromotionTaskMenu *m_promotionTaskMenu;

   QActionGroup *m_sizeActionGroup;
   LayoutAlignmentMenu m_layoutAlignmentMenu;
   QAction *m_sizeActionsSubMenu;
};

QDesignerTaskMenuPrivate::QDesignerTaskMenuPrivate(QWidget *widget, QObject *parent)
   : m_q(nullptr), m_widget(widget),
     m_separator(createSeparatorHelper(parent)),
     m_separator2(createSeparatorHelper(parent)),
     m_separator3(createSeparatorHelper(parent)),
     m_separator4(createSeparatorHelper(parent)),
     m_separator5(createSeparatorHelper(parent)),
     m_separator6(createSeparatorHelper(parent)),
     m_separator7(createSeparatorHelper(parent)),
     m_changeObjectNameAction(new QAction(QDesignerTaskMenu::tr("Change objectName..."), parent)),
     m_changeToolTip(new QAction(QDesignerTaskMenu::tr("Change toolTip..."), parent)),
     m_changeWhatsThis(new QAction(QDesignerTaskMenu::tr("Change whatsThis..."), parent)),
     m_changeStyleSheet(new QAction(QDesignerTaskMenu::tr("Change styleSheet..."), parent)),
     m_morphMenu(new MorphMenu(parent)),
     m_formLayoutMenu(new FormLayoutMenu(parent)),
     m_addMenuBar(new QAction(QDesignerTaskMenu::tr("Create Menu Bar"), parent)),
     m_addToolBar(new QAction(QDesignerTaskMenu::tr("Add Tool Bar"), parent)),
     m_addStatusBar(new QAction(QDesignerTaskMenu::tr("Create Status Bar"), parent)),
     m_removeStatusBar(new QAction(QDesignerTaskMenu::tr("Remove Status Bar"), parent)),
     m_containerFakeMethods(new QAction(QDesignerTaskMenu::tr("Change signals/slots..."), parent)),
     m_navigateToSlot(new QAction(QDesignerTaskMenu::tr("Go to slot..."), parent)),
     m_promotionTaskMenu(new PromotionTaskMenu(widget, PromotionTaskMenu::ModeManagedMultiSelection, parent)),
     m_sizeActionGroup(new QActionGroup(parent)),
     m_layoutAlignmentMenu(parent),
     m_sizeActionsSubMenu(new QAction(QDesignerTaskMenu::tr("Size Constraints"), parent))
{
   QMenu *sizeMenu = new QMenu;
   m_sizeActionsSubMenu->setMenu(sizeMenu);
   QAction *sizeAction = m_sizeActionGroup->addAction(QDesignerTaskMenu::tr("Set Minimum Width"));
   sizeAction->setData(APPLY_MIN_WIDTH);
   sizeMenu->addAction(sizeAction);

   sizeAction = m_sizeActionGroup->addAction(QDesignerTaskMenu::tr("Set Minimum Height"));
   sizeAction->setData(APPLY_MIN_HEIGHT);
   sizeMenu->addAction(sizeAction);

   sizeAction = m_sizeActionGroup->addAction(QDesignerTaskMenu::tr("Set Minimum Size"));
   sizeAction->setData(APPLY_MIN_WIDTH | APPLY_MIN_HEIGHT);
   sizeMenu->addAction(sizeAction);

   sizeMenu->addSeparator();

   sizeAction = m_sizeActionGroup->addAction(QDesignerTaskMenu::tr("Set Maximum Width"));
   sizeAction->setData(APPLY_MAX_WIDTH);
   sizeMenu->addAction(sizeAction);

   sizeAction = m_sizeActionGroup->addAction(QDesignerTaskMenu::tr("Set Maximum Height"));
   sizeAction->setData(APPLY_MAX_HEIGHT);
   sizeMenu->addAction(sizeAction);

   sizeAction = m_sizeActionGroup->addAction(QDesignerTaskMenu::tr("Set Maximum Size"));
   sizeAction->setData(APPLY_MAX_WIDTH | APPLY_MAX_HEIGHT);
   sizeMenu->addAction(sizeAction);
}

QDesignerTaskMenu::QDesignerTaskMenu(QWidget *widget, QObject *parent)
   : QObject(parent), d(new QDesignerTaskMenuPrivate(widget, parent))
{
   d->m_q = this;
   Q_ASSERT(dynamic_cast<QDesignerFormWindowInterface *>(widget) == nullptr);

   connect(d->m_changeObjectNameAction, &QAction::triggered,      this, &QDesignerTaskMenu::changeObjectName);
   connect(d->m_changeToolTip,          &QAction::triggered,      this, &QDesignerTaskMenu::changeToolTip);
   connect(d->m_changeWhatsThis,        &QAction::triggered,      this, &QDesignerTaskMenu::changeWhatsThis);
   connect(d->m_changeStyleSheet,       &QAction::triggered,      this, &QDesignerTaskMenu::changeStyleSheet);
   connect(d->m_addMenuBar,             &QAction::triggered,      this, &QDesignerTaskMenu::createMenuBar);
   connect(d->m_addToolBar,             &QAction::triggered,      this, &QDesignerTaskMenu::addToolBar);
   connect(d->m_addStatusBar,           &QAction::triggered,      this, &QDesignerTaskMenu::createStatusBar);
   connect(d->m_removeStatusBar,        &QAction::triggered,      this, &QDesignerTaskMenu::removeStatusBar);
   connect(d->m_containerFakeMethods,   &QAction::triggered,      this, &QDesignerTaskMenu::containerFakeMethods);
   connect(d->m_navigateToSlot,         &QAction::triggered,      this, &QDesignerTaskMenu::slotNavigateToSlot);
   connect(d->m_sizeActionGroup,        &QActionGroup::triggered, this, &QDesignerTaskMenu::applySize);

   d->m_layoutAlignmentMenu.connect(this, SLOT(slotLayoutAlignment()));
}

QDesignerTaskMenu::~QDesignerTaskMenu()
{
   delete d;
}

QAction *QDesignerTaskMenu::createSeparator()
{
   return createSeparatorHelper(this);
}

QWidget *QDesignerTaskMenu::widget() const
{
   return d->m_widget;
}

QDesignerFormWindowInterface *QDesignerTaskMenu::formWindow() const
{
   QDesignerFormWindowInterface *result = QDesignerFormWindowInterface::findFormWindow(widget());
   Q_ASSERT(result != nullptr);
   return result;
}

void QDesignerTaskMenu::createMenuBar()
{
   if (QDesignerFormWindowInterface *fw = formWindow()) {
      QMainWindow *mw = dynamic_cast<QMainWindow *>(fw->mainContainer());
      if (!mw) {
         // ### warning message
         return;
      }

      CreateMenuBarCommand *cmd = new CreateMenuBarCommand(fw);
      cmd->init(mw);
      fw->commandHistory()->push(cmd);
   }
}

void QDesignerTaskMenu::addToolBar()
{
   if (QDesignerFormWindowInterface *fw = formWindow()) {
      QMainWindow *mw = dynamic_cast<QMainWindow *>(fw->mainContainer());
      if (!mw) {
         // ### warning message
         return;
      }

      AddToolBarCommand *cmd = new AddToolBarCommand(fw);
      cmd->init(mw);
      fw->commandHistory()->push(cmd);
   }
}

void QDesignerTaskMenu::createStatusBar()
{
   if (QDesignerFormWindowInterface *fw = formWindow()) {
      QMainWindow *mw = dynamic_cast<QMainWindow *>(fw->mainContainer());
      if (!mw) {
         // ### warning message
         return;
      }

      CreateStatusBarCommand *cmd = new CreateStatusBarCommand(fw);
      cmd->init(mw);
      fw->commandHistory()->push(cmd);
   }
}

void QDesignerTaskMenu::removeStatusBar()
{
   if (QDesignerFormWindowInterface *fw = formWindow()) {
      QMainWindow *mw = dynamic_cast<QMainWindow *>(fw->mainContainer());
      if (!mw) {
         // ### warning message
         return;
      }

      DeleteStatusBarCommand *cmd = new DeleteStatusBarCommand(fw);
      cmd->init(findStatusBar(mw));
      fw->commandHistory()->push(cmd);
   }
}

QList<QAction *> QDesignerTaskMenu::taskActions() const
{
   QDesignerFormWindowInterface *formWindow = QDesignerFormWindowInterface::findFormWindow(widget());
   Q_ASSERT(formWindow);

   const bool isMainContainer = formWindow->mainContainer() == widget();

   QList<QAction *> actions;

   if (const QMainWindow *mw = dynamic_cast<const QMainWindow *>(formWindow->mainContainer()))  {
      if (isMainContainer || mw->centralWidget() == widget()) {
         if (! findMenuBar(mw)) {
            actions.append(d->m_addMenuBar);
         }

         actions.append(d->m_addToolBar);

         // ### create the status bar
         if (! findStatusBar(mw)) {
            actions.append(d->m_addStatusBar);
         } else {
            actions.append(d->m_removeStatusBar);
         }
         actions.append(d->m_separator);
      }
   }

   actions.append(d->m_changeObjectNameAction);

   d->m_morphMenu->populate(d->m_widget, formWindow, actions);
   d->m_formLayoutMenu->populate(d->m_widget, formWindow, actions);

   actions.append(d->m_separator2);
   actions.append(d->m_changeToolTip);
   actions.append(d->m_changeWhatsThis);
   actions.append(d->m_changeStyleSheet);
   actions.append(d->m_separator6);
   actions.append(d->m_sizeActionsSubMenu);

   if (d->m_layoutAlignmentMenu.setAlignment(formWindow->core(), d->m_widget)) {
      actions.append(d->m_layoutAlignmentMenu.subMenuAction());
   }

   d->m_promotionTaskMenu->setMode(formWindow->isManaged(d->m_widget) ?
      PromotionTaskMenu::ModeManagedMultiSelection : PromotionTaskMenu::ModeUnmanagedMultiSelection);
   d->m_promotionTaskMenu->addActions(formWindow, PromotionTaskMenu::LeadingSeparator, actions);

   if (isMainContainer && ! qt_extension<QDesignerLanguageExtension *>(formWindow->core()->extensionManager(), formWindow->core())) {
      actions.append(d->m_separator5);
      actions.append(d->m_containerFakeMethods);
   }

   if (isSlotNavigationEnabled(formWindow->core())) {
      actions.append(d->m_separator7);
      actions.append(d->m_navigateToSlot);
   }

   return actions;
}

void QDesignerTaskMenu::changeObjectName()
{
   QDesignerFormWindowInterface *fw = formWindow();
   Q_ASSERT(fw != nullptr);

   const QString oldObjectName = objName(fw->core(), widget());
   ObjectNameDialog dialog(fw, oldObjectName);

   if (dialog.exec() == QDialog::Accepted) {
      const QString newObjectName = dialog.newObjectName();

      if (! newObjectName.isEmpty() && newObjectName  != oldObjectName ) {
         const QString objectNameProperty = "objectName";

         PropertySheetStringValue objectNameValue;
         objectNameValue.setValue(newObjectName);

         setProperty(fw, CurrentWidgetMode, objectNameProperty, QVariant::fromValue(objectNameValue));
      }
   }
}

void QDesignerTaskMenu::changeTextProperty(const QString &propertyName, const QString &windowTitle, PropertyMode pm,
   Qt::TextFormat desiredFormat)
{
   QDesignerFormWindowInterface *fw = formWindow();
   if (! fw) {
      return;
   }

   Q_ASSERT(d->m_widget->parentWidget() != nullptr);

   const QDesignerPropertySheetExtension *sheet =
         qt_extension<QDesignerPropertySheetExtension *>(fw->core()->extensionManager(), d->m_widget);
   const int index = sheet->indexOf(propertyName);

   if (index == -1) {
      // "Invalid property" << propertyName
      return;
   }

   QVariant data = sheet->property(index);

   PropertySheetStringValue textValue = data.value<PropertySheetStringValue>();
   const QString oldText = textValue.value();

   // Pop up respective dialog
   bool accepted = false;
   QString newText;

   switch (desiredFormat) {
      case Qt::PlainText: {
         PlainTextEditorDialog dlg(fw->core(), fw);
         if (!windowTitle.isEmpty()) {
            dlg.setWindowTitle(windowTitle);
         }

         dlg.setDefaultFont(d->m_widget->font());
         dlg.setText(oldText);
         accepted = dlg.showDialog() == QDialog::Accepted;
         newText = dlg.text();
      }
      break;

      default: {
         RichTextEditorDialog dlg(fw->core(), fw);
         if (!windowTitle.isEmpty()) {
            dlg.setWindowTitle(windowTitle);
         }
         dlg.setDefaultFont(d->m_widget->font());
         dlg.setText(oldText);
         accepted = dlg.showDialog() == QDialog::Accepted;
         newText = dlg.text(desiredFormat);
      }
      break;
   }

   // change property
   if (!accepted || oldText == newText) {
      return;
   }

   textValue.setValue(newText);
   setProperty(fw, pm, propertyName, QVariant::fromValue(textValue));
}

void QDesignerTaskMenu::changeToolTip()
{
   changeTextProperty(QString("toolTip"), tr("Edit ToolTip"), MultiSelectionMode, Qt::AutoText);
}

void QDesignerTaskMenu::changeWhatsThis()
{
   changeTextProperty(QString("whatsThis"), tr("Edit WhatsThis"), MultiSelectionMode, Qt::AutoText);
}

void QDesignerTaskMenu::changeStyleSheet()
{
   if (QDesignerFormWindowInterface *fw = formWindow()) {
      StyleSheetEditorDialog dlg(fw->core(), fw, d->m_widget);
      dlg.exec();
   }
}

void QDesignerTaskMenu::containerFakeMethods()
{
   QDesignerFormWindowInterface *fw = formWindow();

   if (! fw) {
      return;
   }

   SignalSlotDialog::editMetaDataBase(fw, d->m_widget, fw);
}

static QString declaredInClass(const QDesignerMetaObjectInterface *metaObject, const QString &member)
{
   // Find class whose superclass does not contain the method.
   const QDesignerMetaObjectInterface *meta = metaObject;

   for (;;) {
      const QDesignerMetaObjectInterface *tmpMeta = meta->superClass();

      if (tmpMeta == nullptr) {
         break;
      }

      if (tmpMeta->indexOfMethod(member) == -1) {
         break;
      }

      meta = tmpMeta;
   }

   return meta->className();
}

bool QDesignerTaskMenu::isSlotNavigationEnabled(const QDesignerFormEditorInterface *core)
{
   return core->integration()->hasFeature(QDesignerIntegration::SlotNavigationFeature);
}

void QDesignerTaskMenu::slotNavigateToSlot()
{
   QDesignerFormEditorInterface *core = formWindow()->core();
   Q_ASSERT(core);
   navigateToSlot(core, widget());
}

void QDesignerTaskMenu::navigateToSlot(QDesignerFormEditorInterface *core, QObject *object, const QString &defaultSignal)
{
   const QString objectName = objName(core, object);
   QMap<QString, QMap<QString, QStringList>> classToSignalList;

   // "real" signals
   if (const QDesignerMetaObjectInterface *metaObject = core->introspection()->metaObject(object)) {
      const int methodCount = metaObject->methodCount();

      for (int i = 0; i < methodCount; ++i) {
         const QDesignerMetaMethodInterface *metaMethod = metaObject->method(i);
         if (metaMethod->methodType() == QDesignerMetaMethodInterface::Signal) {
            const QString signature = metaMethod->signature();
            const QStringList parameterNames = metaMethod->parameterNames();
            classToSignalList[declaredInClass(metaObject, signature)][signature] = parameterNames;
         }
      }
   }

   // fake signals
   qdesigner_internal::MetaDataBase *metaDataBase =
         dynamic_cast<qdesigner_internal::MetaDataBase *>(core->metaDataBase());

   if (metaDataBase != nullptr) {

      qdesigner_internal::MetaDataBaseItem *item = metaDataBase->metaDataBaseItem(object);
      Q_ASSERT(item);

      const QStringList fakeSignals = item->fakeSignals();

      for (const QString &fakeSignal : fakeSignals) {
         classToSignalList[item->customClassName()][fakeSignal] = QStringList();
      }
   }

   if (object->isWidgetType()) {
      QWidget *widget = static_cast<QWidget *>(object);

      if (WidgetDataBase *db = dynamic_cast<WidgetDataBase *>(core->widgetDataBase())) {
         const QString promotedClassName = promotedCustomClassName(core, widget);
         const int index = core->widgetDataBase()->indexOfClassName(promotedClassName);

         if (index >= 0) {
            WidgetDataBaseItem *item = static_cast<WidgetDataBaseItem *>(db->item(index));
            const QStringList fakeSignals = item->fakeSignals();

            for (const QString &fakeSignal : fakeSignals) {
               classToSignalList[promotedClassName][fakeSignal] = QStringList();
            }
         }
      }
   }

   Ui::SelectSignalDialog dialogUi;
   QDialog selectSignalDialog(nullptr, Qt::WindowTitleHint | Qt::WindowSystemMenuHint);
   dialogUi.setupUi(&selectSignalDialog);

   auto iter = classToSignalList.constBegin();

   for (; iter != classToSignalList.constEnd(); ++iter) {
      const QString className = iter.key();
      QMap<QString, QStringList> signalNames = iter.value();

      auto itSignal = signalNames.constBegin();
      for (; itSignal != signalNames.constEnd(); ++itSignal) {
         const QString signalName = itSignal.key();
         QTreeWidgetItem *row = new QTreeWidgetItem(QStringList() << signalName << className);
         row->setData(0, Qt::UserRole, itSignal.value());
         dialogUi.signalList->addTopLevelItem(row);
      }
   }

   if (dialogUi.signalList->topLevelItemCount() == 0) {
      QTreeWidgetItem *row = new QTreeWidgetItem(QStringList() << tr("no signals available"));
      dialogUi.signalList->addTopLevelItem(row);
      dialogUi.buttonBox->button(QDialogButtonBox::Ok)->setEnabled(false);

   } else {
      connect(dialogUi.signalList, &QTreeWidget::itemDoubleClicked,
            &selectSignalDialog, &QDialog::accept);
   }

   if (defaultSignal.isEmpty()) {
      dialogUi.signalList->setCurrentItem(dialogUi.signalList->topLevelItem(0));
   } else {
      const QList<QTreeWidgetItem *> items = dialogUi.signalList->findItems (defaultSignal, Qt::MatchExactly, 0);
      if (! items.empty()) {
         dialogUi.signalList->setCurrentItem(items.front());
      }
   }

   dialogUi.signalList->resizeColumnToContents(0);

   if (selectSignalDialog.exec() != QDialog::Accepted) {
      return;
   }

   const QList<QTreeWidgetItem *> &selectedItems = dialogUi.signalList->selectedItems();
   if (selectedItems.isEmpty()) {
      return;
   }

   const QTreeWidgetItem *selectedItem = selectedItems.constFirst();
   const QString signalSignature = selectedItem->text(0);

   QVariant data = selectedItem->data(0, Qt::UserRole);
   const QStringList parameterNames = data.value<QStringList>();

   // TODO: Check whether signal is connected to slot
   core->integration()->emitNavigateToSlot(objectName, signalSignature, parameterNames);
}

// Add a command that takes over the value of the current geometry as
// minimum/maximum size according to the flags.
static void createSizeCommand(QDesignerFormWindowInterface *fw, QWidget *w, int flags)
{
   const QSize size = w->size();
   if (flags & (APPLY_MIN_WIDTH | APPLY_MIN_HEIGHT)) {
      QSize minimumSize = w-> minimumSize();
      if (flags & APPLY_MIN_WIDTH) {
         minimumSize.setWidth(size.width());
      }

      if (flags & APPLY_MIN_HEIGHT) {
         minimumSize.setHeight(size.height());
      }

      SetPropertyCommand *cmd = new SetPropertyCommand(fw);
      cmd->init(w, QString("minimumSize"), minimumSize);
      fw->commandHistory()->push(cmd);
   }

   if (flags & (APPLY_MAX_WIDTH | APPLY_MAX_HEIGHT)) {
      QSize maximumSize = w-> maximumSize();
      if (flags & APPLY_MAX_WIDTH) {
         maximumSize.setWidth(size.width());
      }

      if (flags & APPLY_MAX_HEIGHT) {
         maximumSize.setHeight(size.height());
      }

      SetPropertyCommand *cmd = new SetPropertyCommand(fw);
      cmd->init(w, QString("maximumSize"), maximumSize);
      fw->commandHistory()->push(cmd);
   }
}

void QDesignerTaskMenu::applySize(QAction *a)
{
   QDesignerFormWindowInterface *fw = formWindow();
   if (! fw) {
      return;
   }

   const QWidgetList selection = applicableWidgets(fw, MultiSelectionMode);
   if (selection.isEmpty()) {
      return;
   }

   const int mask = a->data().toInt();
   const int size = selection.size();
   fw->commandHistory()->beginMacro(tr("Set size constraint on %n widget(s)", nullptr, size));

   for (int i = 0; i < size; ++i) {
      createSizeCommand(fw, selection.at(i), mask);
   }

   fw->commandHistory()->endMacro();
}

template <class Container>
static void getApplicableObjects(const QDesignerFormWindowInterface *fw, QWidget *current,
   QDesignerTaskMenu::PropertyMode pm, Container *c)
{
   // Current is always first
   c->push_back(current);

   if (pm == QDesignerTaskMenu::CurrentWidgetMode) {
      return;
   }

   QDesignerObjectInspector *designerObjectInspector = dynamic_cast<QDesignerObjectInspector *>(fw->core()->objectInspector());

   if (! designerObjectInspector) {
      // an old-style Object Inspector was used
      return;
   }

   // Add managed or unmanaged selection according to current type, make current first
   Selection s;
   designerObjectInspector->getSelection(s);

   const QWidgetList &source = fw->isManaged(current) ? s.managed : s.unmanaged;
   const QWidgetList::const_iterator cend = source.constEnd();

   for ( QWidgetList::const_iterator it = source.constBegin(); it != cend; ++it) {
      if (*it != current) {
         // was first
         c->push_back(*it);
      }
   }
}

QObjectList QDesignerTaskMenu::applicableObjects(const QDesignerFormWindowInterface *fw, PropertyMode pm) const
{
   QObjectList rc;
   getApplicableObjects(fw, d->m_widget, pm, &rc);

   return rc;
}

QWidgetList QDesignerTaskMenu::applicableWidgets(const QDesignerFormWindowInterface *fw, PropertyMode pm) const
{
   QWidgetList rc;
   getApplicableObjects(fw, d->m_widget, pm, &rc);

   return rc;
}

void QDesignerTaskMenu::setProperty(QDesignerFormWindowInterface *fw,  PropertyMode pm,
      const QString &name, const QVariant &newValue)
{
   SetPropertyCommand *setPropertyCommand = new SetPropertyCommand(fw);

   if (setPropertyCommand->init(applicableObjects(fw, pm), name, newValue, d->m_widget)) {
      fw->commandHistory()->push(setPropertyCommand);
   } else {
      delete setPropertyCommand;
   }
}

void QDesignerTaskMenu::slotLayoutAlignment()
{
   QDesignerFormWindowInterface *fw = formWindow();
   const Qt::Alignment newAlignment = d->m_layoutAlignmentMenu.alignment();
   LayoutAlignmentCommand *cmd = new LayoutAlignmentCommand(fw);

   if (cmd->init(d->m_widget, newAlignment)) {
      fw->commandHistory()->push(cmd);
   } else {
      delete cmd;
   }
}

}   // end namespace qdesigner_internal
