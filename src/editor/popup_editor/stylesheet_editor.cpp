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
#include <abstract_formeditor.h>
#include <abstract_integration.h>
#include <extension.h>
#include <gradient_select.h>
#include <qtgradientmanager.h>
#include <qtgradientutils.h>
#include <designer_settings.h>
#include <stylesheet_editor.h>

#include <csshighlighter_p.h>
#include <iconselector_p.h>
#include <designer_utils.h>
#include <designer_propertysheet.h>

#include <QSignalMapper>
#include <QAction>
#include <QColorDialog>
#include <QFontDialog>
#include <QMenu>
#include <QPushButton>
#include <QTextDocument>
#include <QToolBar>
#include <QVBoxLayout>

static const QString styleSheetProperty = "styleSheet";
static const QString StyleSheetDialog   = "StyleSheet_Dialog";
static const QString Geometry           = "Geometry";

using namespace qdesigner_internal;

StyleSheet_Widget::StyleSheet_Widget(QWidget *parent)
   : QTextEdit(parent)
{
   setTabStopWidth(fontMetrics().width(' ') * 4);
   setAcceptRichText(false);
   new CssHighlighter(document());
}

StyleSheetEditorDialog::StyleSheetEditorDialog(QDesignerFormEditorInterface *core, QWidget *parent,
         QWidget *applyWidget, Mode mode)
   : QDialog(parent), m_ui(new Ui::StyleSheetEditor), m_core(core), m_applyWidget(applyWidget)
{
   using MapperVoidSlot      = void (QSignalMapper::*)();
   using MapperQStringSignal = void (QSignalMapper::*)(const QString &);

   m_ui->setupUi(this);
   setWindowFlags(windowFlags() & ~Qt::WindowContextHelpButtonHint);

   if (m_applyWidget == nullptr) {
      m_ui->apply_pb->setEnabled(false);

   } else {
      // initialize the editor

      QDesignerPropertySheetExtension *sheet =
         qt_extension<QDesignerPropertySheetExtension *>(m_core->extensionManager(), m_applyWidget);

      const int index = sheet->indexOf(styleSheetProperty);
      QVariant tmp    = sheet->property(index);

      const PropertySheetStringValue value = tmp.value<PropertySheetStringValue>();
      setText(value.value());

      connect(m_ui->ok_pb, &QPushButton::clicked, this, &StyleSheetEditorDialog::applyStyleSheet);
   }

   // editor context menu
   m_ui->editor->setContextMenuPolicy(Qt::CustomContextMenu);

   QFont font = m_ui->editor->font();
   font.setPointSize(10);
   m_ui->editor->setFont(font);

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

   //
   if (mode == Mode::ModeGlobal) {
      m_ui->resource_pb->setEnabled(false);
      m_ui->resource_tb->setEnabled(false);
   }

   m_resourceMenu = new QMenu(this);

   for (auto item : resourceProperties) {
      QAction *resourceAction = m_resourceMenu->addAction(item);
      connect(resourceAction, &QAction::triggered, this, [item, this] () { slotAddResource(item); } );
   }

   m_gradientMenu = new QMenu(this);

   for (auto item : colorProperties) {
      QAction *gradientAction = m_gradientMenu->addAction(item);
      connect(gradientAction, &QAction::triggered, this, [item, this] () { slotAddGradient(item); } );
   }

   m_colorMenu = new QMenu(this);

   for (auto item : colorProperties) {
      QAction *colorAction = m_colorMenu->addAction(item);
      connect(colorAction, &QAction::triggered, this, [item, this] () { slotAddColor(item); } );
   }

   QDesignerSettingsInterface *settings = core->settingsManager();
   settings->beginGroup(StyleSheetDialog);

   if (settings->contains(Geometry)) {
      restoreGeometry(settings->value(Geometry).toByteArray());
   }

   settings->endGroup();

   connect(m_ui->resource_pb, &QPushButton::clicked, this, [this] () { slotAddResource(QString()); });
   connect(m_ui->gradient_pb, &QPushButton::clicked, this, [this] () { slotAddGradient(QString()); });
   connect(m_ui->color_pb,    &QPushButton::clicked, this, [this] () { slotAddColor(QString()); });
   connect(m_ui->font_pb,     &QPushButton::clicked, this, &StyleSheetEditorDialog::slotAddFont);

   connect(m_ui->resource_tb, &QToolButton::clicked, this, &StyleSheetEditorDialog::showResourceMenu);
   connect(m_ui->gradient_tb, &QToolButton::clicked, this, &StyleSheetEditorDialog::showGradientMenu);
   connect(m_ui->color_tb,    &QToolButton::clicked, this, &StyleSheetEditorDialog::showColorMenu);

   connect(m_ui->ok_pb,       &QPushButton::clicked, this, &QDialog::accept);
   connect(m_ui->apply_pb,    &QPushButton::clicked, this, &StyleSheetEditorDialog::applyStyleSheet);
   connect(m_ui->cancel_pb,   &QPushButton::clicked, this, &QDialog::reject);

   m_ui->editor->setFocus();
}

StyleSheetEditorDialog::~StyleSheetEditorDialog()
{
   QDesignerSettingsInterface *settings = m_core->settingsManager();

   settings->beginGroup(StyleSheetDialog);
   settings->setValue(Geometry, saveGeometry());
   settings->endGroup();
}

void StyleSheetEditorDialog::showResourceMenu()
{
   QPoint pos = QCursor::pos();
   m_resourceMenu->exec(pos);
}

void StyleSheetEditorDialog::showGradientMenu()
{
   QPoint pos = QCursor::pos();
   m_gradientMenu->exec(pos);
}

void StyleSheetEditorDialog::showColorMenu()
{
   QPoint pos = QCursor::pos();
   m_colorMenu->exec(pos);
}

void StyleSheetEditorDialog::slotAddResource(const QString &property)
{
   const QString path = IconSelector::choosePixmapResource(m_core, m_core->resourceModel(), QString(), this);

   if (! path.isEmpty()) {
      insertCssProperty(property, QString("url(%1)").formatArg(path));
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

   if (! color.isValid()) {
      return;
   }

   QString colorStr;

   if (color.alpha() == 255) {
      colorStr = QString("rgb(%1, %2, %3)")
         .formatArg(color.red()).formatArg(color.green()).formatArg(color.blue());

   } else {
      colorStr = QString("rgba(%1, %2, %3, %4)")
         .formatArg(color.red()).formatArg(color.green()).formatArg(color.blue()).formatArg(color.alpha());
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
            fontStr += "italic ";
            break;

         case QFont::StyleOblique:
            fontStr += "oblique ";
            break;

         default:
            break;
      }

      fontStr += QString::number(font.pointSize());
      fontStr += "pt \"" + fontStr + font.family() + '"';

      insertCssProperty("font", fontStr);

      QString decoration;

      if (font.underline()) {
         decoration += "underline";
      }

      if (font.strikeOut()) {
         if (! decoration.isEmpty()) {
            decoration += ' ';
         }
         decoration += "line-through";
      }

      insertCssProperty("text-decoration", decoration);
   }
}

void StyleSheetEditorDialog::insertCssProperty(const QString &name, const QString &value)
{
   if (! value.isEmpty()) {
      QTextCursor cursor = m_ui->editor->textCursor();

      if (! name.isEmpty()) {
         cursor.beginEditBlock();
         cursor.removeSelectedText();
         cursor.movePosition(QTextCursor::EndOfLine);

         // Simple check to see if we are in a selector scope
         const QTextDocument *doc  = m_ui->editor->document();
         const QTextCursor closing = doc->find("}", cursor, QTextDocument::FindBackward);
         const QTextCursor opening = doc->find("{", cursor, QTextDocument::FindBackward);
         const bool inSelector     = !opening.isNull() && (closing.isNull() || closing.position() < opening.position());

         QString insertion;
         if (m_ui->editor->textCursor().block().length() != 1) {
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

QString StyleSheetEditorDialog::text() const
{
   return m_ui->editor->toPlainText();
}

void StyleSheetEditorDialog::setText(const QString &text)
{
   m_ui->editor->setText(text);
}

void StyleSheetEditorDialog::applyStyleSheet()
{
   if (m_applyWidget != nullptr) {
      const PropertySheetStringValue value(text(), false);

      auto obj = dynamic_cast<QDesignerFormWindowInterface *>(parent());

      if (obj != nullptr) {
         obj->cursor()->setWidgetProperty(m_applyWidget, styleSheetProperty, QVariant::fromValue(value));
      }
   }
}
