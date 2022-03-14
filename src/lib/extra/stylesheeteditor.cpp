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

#include <qtgradientmanager.h>
#include <gradient_select.h>
#include <qtgradientutils.h>
#include <abstract_formwindow.h>
#include <abstract_formwindowcursor.h>
#include <abstract_formeditor.h>
#include <abstract_integration.h>
#include <qdesigner_settings.h>
#include <extension.h>

#include <stylesheeteditor_p.h>
#include <csshighlighter_p.h>
#include <iconselector_p.h>
#include <qdesigner_utils_p.h>
#include <qdesigner_propertysheet_p.h>

#include <QSignalMapper>
#include <QAction>
#include <QColorDialog>
#include <QDialogButtonBox>
#include <QFontDialog>
#include <QMenu>
#include <QPushButton>
#include <QTextDocument>
#include <QToolBar>
#include <QVBoxLayout>

static const QString styleSheetProperty = "styleSheet";
static const QString StyleSheetDialogC  = "StyleSheetDialog";
static const QString  Geometry          = "Geometry";

namespace qdesigner_internal {

StyleSheetEditor::StyleSheetEditor(QWidget *parent)
   : QTextEdit(parent)
{
   setTabStopWidth(fontMetrics().width(' ') * 4);
   setAcceptRichText(false);
   new CssHighlighter(document());
}

// --- StyleSheetEditorDialog
StyleSheetEditorDialog::StyleSheetEditorDialog(QDesignerFormEditorInterface *core, QWidget *parent, Mode mode)
   : QDialog(parent),
     m_buttonBox(new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel | QDialogButtonBox::Help, Qt::Horizontal)),
     m_editor(new StyleSheetEditor),
     m_addResourceAction(new QAction(tr("Add Resource..."), this)),
     m_addGradientAction(new QAction(tr("Add Gradient..."), this)),
     m_addColorAction(new QAction(tr("Add Color..."), this)),
     m_addFontAction(new QAction(tr("Add Font..."), this))
{
   typedef void (QSignalMapper::*MapperVoidSlot)();
   typedef void (QSignalMapper::*MapperQStringSignal)(const QString &);

   setWindowTitle(tr("Edit Style Sheet"));
   setWindowFlags(windowFlags() & ~Qt::WindowContextHelpButtonHint);

   connect(m_buttonBox, &QDialogButtonBox::accepted, this, &QDialog::accept);
   connect(m_buttonBox, &QDialogButtonBox::rejected, this, &QDialog::reject);

   m_buttonBox->button(QDialogButtonBox::Help)->setShortcut(QKeySequence::HelpContents);

   QToolBar *toolBar = new QToolBar;

   QGridLayout *layout = new QGridLayout;
   layout->addWidget(toolBar, 0, 0, 1, 2);
   layout->addWidget(m_editor, 1, 0, 1, 2);
   layout->addWidget(m_buttonBox, 2, 1, 1, 1);
   setLayout(layout);

   m_editor->setContextMenuPolicy(Qt::CustomContextMenu);
   connect(m_editor, &QWidget::customContextMenuRequested,
      this, &StyleSheetEditorDialog::slotContextMenuRequested);

   QSignalMapper *resourceActionMapper = new QSignalMapper(this);
   QSignalMapper *gradientActionMapper = new QSignalMapper(this);
   QSignalMapper *colorActionMapper    = new QSignalMapper(this);

   resourceActionMapper->setMapping(m_addResourceAction, QString());
   gradientActionMapper->setMapping(m_addGradientAction, QString());
   colorActionMapper->setMapping(m_addColorAction, QString());

   connect(m_addResourceAction, &QAction::triggered,
      resourceActionMapper, static_cast<MapperVoidSlot>(&QSignalMapper::map));

   connect(m_addGradientAction, &QAction::triggered,
      gradientActionMapper, static_cast<MapperVoidSlot>(&QSignalMapper::map));

   connect(m_addColorAction, &QAction::triggered,
      colorActionMapper, static_cast<MapperVoidSlot>(&QSignalMapper::map));

   connect(m_addFontAction, &QAction::triggered, this, &StyleSheetEditorDialog::slotAddFont);

   m_addResourceAction->setEnabled(mode == ModePerForm);

   const QStringList resourceProperties = {
      "background-image",
      "border-image",
      "image"
   };

   const QStringList colorProperties = {
      "color",
      "background-color",
      "alternate-background-color",
      "border-color",
      "border-top-color",
      "border-right-color",
      "border-bottom-color",
      "border-left-color",
      "gridline-color",
      "selection-color",
      "selection-background-color"
   };

   QMenu *resourceActionMenu = new QMenu(this);
   QMenu *gradientActionMenu = new QMenu(this);
   QMenu *colorActionMenu    = new QMenu(this);

   for (auto item : resourceProperties) {
      QAction *action = resourceActionMenu->addAction(item);

      connect(action, &QAction::triggered, resourceActionMapper, static_cast<MapperVoidSlot>(&QSignalMapper::map));

      resourceActionMapper->setMapping(action, item);
   }

   for (auto item : colorProperties) {
      QAction *gradientAction = gradientActionMenu->addAction(item);
      QAction *colorAction    = colorActionMenu->addAction(item);

      connect(gradientAction, &QAction::triggered,
         gradientActionMapper, static_cast<MapperVoidSlot>(&QSignalMapper::map));

      connect(colorAction, &QAction::triggered,
         colorActionMapper, static_cast<MapperVoidSlot>(&QSignalMapper::map));

      gradientActionMapper->setMapping(gradientAction, item);
      colorActionMapper->setMapping(colorAction, item);
   }

   connect(resourceActionMapper, static_cast<MapperQStringSignal>(&QSignalMapper::mapped),
      this, &StyleSheetEditorDialog::slotAddResource);

   connect(gradientActionMapper, static_cast<MapperQStringSignal>(&QSignalMapper::mapped),
      this, &StyleSheetEditorDialog::slotAddGradient);

   connect(colorActionMapper, static_cast<MapperQStringSignal>(&QSignalMapper::mapped),
      this, &StyleSheetEditorDialog::slotAddColor);

   m_addResourceAction->setMenu(resourceActionMenu);
   m_addGradientAction->setMenu(gradientActionMenu);
   m_addColorAction->setMenu(colorActionMenu);

   toolBar->addAction(m_addResourceAction);
   toolBar->addAction(m_addGradientAction);
   toolBar->addAction(m_addColorAction);
   toolBar->addAction(m_addFontAction);

   m_editor->setFocus();

   QDesignerSettingsInterface *settings = core->settingsManager();
   settings->beginGroup(StyleSheetDialogC);

   if (settings->contains(Geometry)) {
      restoreGeometry(settings->value(Geometry).toByteArray());
   }

   settings->endGroup();
}

StyleSheetEditorDialog::~StyleSheetEditorDialog()
{
   QDesignerSettingsInterface *settings = m_core->settingsManager();
   settings->beginGroup(StyleSheetDialogC);

   settings->setValue(Geometry, saveGeometry());
   settings->endGroup();
}

void StyleSheetEditorDialog::setOkButtonEnabled(bool v)
{
   m_buttonBox->button(QDialogButtonBox::Ok)->setEnabled(v);
   if (QPushButton *applyButton = m_buttonBox->button(QDialogButtonBox::Apply)) {
      applyButton->setEnabled(v);
   }
}

void StyleSheetEditorDialog::slotContextMenuRequested(const QPoint &pos)
{
   QMenu *menu = m_editor->createStandardContextMenu();
   menu->addSeparator();
   menu->addAction(m_addResourceAction);
   menu->addAction(m_addGradientAction);
   menu->exec(mapToGlobal(pos));

   delete menu;
}

void StyleSheetEditorDialog::slotAddResource(const QString &property)
{
   const QString path = IconSelector::choosePixmapResource(m_core, m_core->resourceModel(), QString(), this);
   if (!path.isEmpty()) {
      insertCssProperty(property, QString(QString("url(%1)")).formatArg(path));
   }
}

void StyleSheetEditorDialog::slotAddGradient(const QString &property)
{
   bool ok;
   const QGradient grad = QtGradientSelect::getGradient(&ok, m_core->gradientManager(), this);

   if (ok) {
      insertCssProperty(property, QtGradientUtils::styleSheetCode(grad));
   }
}

void StyleSheetEditorDialog::slotAddColor(const QString &property)
{
   const QColor color = QColorDialog::getColor(0xffffffff, this, QString(), QColorDialog::ShowAlphaChannel);
   if (!color.isValid()) {
      return;
   }

   QString colorStr;

   if (color.alpha() == 255) {
      colorStr = QString(QString("rgb(%1, %2, %3)")).formatArg(
            color.red()).formatArg(color.green()).formatArg(color.blue());
   } else {
      colorStr = QString(QString("rgba(%1, %2, %3, %4)")).formatArg(
            color.red()).formatArg(color.green()).formatArg(color.blue()).formatArg(color.alpha());
   }

   insertCssProperty(property, colorStr);
}

void StyleSheetEditorDialog::slotAddFont()
{
   bool ok;
   QFont font = QFontDialog::getFont(&ok, this);
   if (ok) {
      QString fontStr;
      if (font.weight() != QFont::Normal) {
         fontStr += QString::number(font.weight());
         fontStr += ' ';
      }

      switch (font.style()) {
         case QFont::StyleItalic:
            fontStr += QString("italic ");
            break;
         case QFont::StyleOblique:
            fontStr += QString("oblique ");
            break;
         default:
            break;
      }
      fontStr += QString::number(font.pointSize());
      fontStr += "pt \"" + fontStr + font.family() + '"';

      insertCssProperty("font", fontStr);

      QString decoration;

      if (font.underline()) {
         decoration += QString("underline");
      }

      if (font.strikeOut()) {
         if (!decoration.isEmpty()) {
            decoration += ' ';
         }
         decoration += QString("line-through");
      }
      insertCssProperty(QString("text-decoration"), decoration);
   }
}

void StyleSheetEditorDialog::insertCssProperty(const QString &name, const QString &value)
{
   if (! value.isEmpty()) {
      QTextCursor cursor = m_editor->textCursor();

      if (! name.isEmpty()) {
         cursor.beginEditBlock();
         cursor.removeSelectedText();
         cursor.movePosition(QTextCursor::EndOfLine);

         // Simple check to see if we are in a selector scope
         const QTextDocument *doc  = m_editor->document();
         const QTextCursor closing = doc->find("}", cursor, QTextDocument::FindBackward);
         const QTextCursor opening = doc->find("{", cursor, QTextDocument::FindBackward);
         const bool inSelector     = !opening.isNull() && (closing.isNull() || closing.position() < opening.position());

         QString insertion;
         if (m_editor->textCursor().block().length() != 1) {
            insertion += '\n';
         }

         if (inSelector) {
            insertion += '\t';
         }

         insertion += name + ": " + value + ';';

         cursor.insertText(insertion);
         cursor.endEditBlock();

      } else {
         cursor.insertText(value);
      }
   }
}

QDialogButtonBox *StyleSheetEditorDialog::buttonBox() const
{
   return m_buttonBox;
}

QString StyleSheetEditorDialog::text() const
{
   return m_editor->toPlainText();
}

void StyleSheetEditorDialog::setText(const QString &t)
{
   m_editor->setText(t);
}

StyleSheetPropertyEditorDialog::StyleSheetPropertyEditorDialog(QWidget *parent,
         QDesignerFormWindowInterface *fw, QWidget *widget)
   :  StyleSheetEditorDialog(fw->core(), parent), m_fw(fw), m_widget(widget)
{
   Q_ASSERT(m_fw != nullptr);

   QPushButton *apply = buttonBox()->addButton(QDialogButtonBox::Apply);
   QObject::connect(apply, &QAbstractButton::clicked,
         this, &StyleSheetPropertyEditorDialog::applyStyleSheet);

   QObject::connect(buttonBox(), &QDialogButtonBox::accepted,
         this, &StyleSheetPropertyEditorDialog::applyStyleSheet);

   QDesignerPropertySheetExtension *sheet =
      qt_extension<QDesignerPropertySheetExtension *>(m_fw->core()->extensionManager(), m_widget);

   Q_ASSERT(sheet != nullptr);

   const int index = sheet->indexOf(styleSheetProperty);
   QVariant tmp     = sheet->property(index);

   const PropertySheetStringValue value = tmp.value<PropertySheetStringValue>();

   setText(value.value());
}

void StyleSheetPropertyEditorDialog::applyStyleSheet()
{
   const PropertySheetStringValue value(text(), false);
   m_fw->cursor()->setWidgetProperty(m_widget, styleSheetProperty, QVariant::fromValue(value));
}

}   // end namespace qdesigner_internal
