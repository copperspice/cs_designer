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

#include <abstract_formeditor.h>
#include <designer_settings.h>
#include <ui_richtext_addlink.h>

#include <richtexteditor_p.h>
#include <htmlhighlighter_p.h>
#include <iconselector_p.h>
#include <iconloader_p.h>

#include <QList>
#include <QMap>
#include <QPointer>
#include <QXmlStreamReader>
#include <QXmlStreamWriter>
#include <QXmlStreamAttributes>
#include <QAction>
#include <QColorDialog>
#include <QComboBox>
#include <QFontDatabase>
#include <QTextCursor>
#include <QPainter>
#include <QIcon>
#include <QMenu>
#include <QMoveEvent>
#include <QTabWidget>
#include <QTextDocument>
#include <QTextBlock>
#include <QToolBar>
#include <QToolButton>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QPushButton>
#include <QDialogButtonBox>

static const QString RichTextDialogGroupC = "RichTextDialog";
static const QString GeometryKeyC         = "Geometry";
static const QString TabKeyC              = "Tab";

const bool simplifyRichTextDefault = true;

namespace qdesigner_internal {

// Richtext simplification filter helpers: Elements to be discarded
static inline bool filterElement(const QString &name)
{
   return name != "meta" && name != "style";
}

// Richtext simplification filter helpers: Filter attributes of elements
static inline void filterAttributes(const QString &name, QXmlStreamAttributes *atts,
   bool *paragraphAlignmentFound)
{
   typedef QXmlStreamAttributes::iterator AttributeIt;

   if (atts->isEmpty()) {
      return;
   }

   // No style attributes for <body>
   if (name == "body") {
      atts->clear();
      return;
   }

   // Clean out everything except 'align' for 'p'
   if (name == "p") {
      for (AttributeIt it = atts->begin(); it != atts->end(); ) {
         if (it->name() == "align") {
            ++it;
            *paragraphAlignmentFound = true;

         } else {
            it = atts->erase(it);
         }
      }
      return;
   }
}

// Richtext simplification filter helpers: Check for blank QString
static inline bool isWhiteSpace(const QString &in)
{
   const int count = in.size();

   for (int i = 0; i < count; ++i)
      if (! in.at(i).isSpace()) {
         return false;
      }

   return true;
}

// Richtext simplification filter: Remove hard-coded font settings,
// <style> elements, <p> attributes other than 'align' and
// and unnecessary meta-information.
QString simplifyRichTextFilter(const QString &in, bool *isPlainTextPtr = nullptr)
{
   unsigned elementCount = 0;
   bool paragraphAlignmentFound = false;
   QString out;

   QXmlStreamReader reader(in);
   QXmlStreamWriter writer(&out);

   writer.setAutoFormatting(false);
   writer.setAutoFormattingIndent(0);

   while (!reader.atEnd()) {
      switch (reader.readNext()) {

         case QXmlStreamReader::StartElement:
            ++elementCount;

            if (filterElement(reader.name())) {
               const QStringView name = reader.name();

               QXmlStreamAttributes attributes = reader.attributes();
               filterAttributes(name, &attributes, &paragraphAlignmentFound);
               writer.writeStartElement(name.toString());

               if (!attributes.isEmpty()) {
                  writer.writeAttributes(attributes);
               }

            } else {
               reader.readElementText(); // Skip away all nested elements and characters.
            }
            break;

         case QXmlStreamReader::Characters:
            if (!isWhiteSpace(reader.text())) {
               writer.writeCharacters(reader.text().toString());
            }
            break;

         case QXmlStreamReader::EndElement:
            writer.writeEndElement();
            break;

         default:
            break;
      }
   }

   // Check for plain text (no spans, just <html><head><body><p>)
   if (isPlainTextPtr) {
      *isPlainTextPtr = !paragraphAlignmentFound && elementCount == 4u;   //
   }

   return out;
}

class RichTextEditor : public QTextEdit
{
   CS_OBJECT(RichTextEditor)

 public:
   explicit RichTextEditor(QWidget *parent = nullptr);
   void setDefaultFont(QFont font);

   QToolBar *createToolBar(QDesignerFormEditorInterface *core, QWidget *parent = nullptr);

   bool simplifyRichText() const      {
      return m_simplifyRichText;
   }

   CS_SLOT_1(Public, void setFontBold(bool b))
   CS_SLOT_2(setFontBold)
   CS_SLOT_1(Public, void setFontPointSize(double un_named_arg1))
   CS_SLOT_2(setFontPointSize)
   CS_SLOT_1(Public, void setText(const QString &text))
   CS_SLOT_2(setText)
   CS_SLOT_1(Public, void setSimplifyRichText(bool v))
   CS_SLOT_2(setSimplifyRichText)
   CS_SLOT_1(Public, QString text(Qt::TextFormat format)const)
   CS_SLOT_2(text)

   CS_SIGNAL_1(Public, void stateChanged())
   CS_SIGNAL_2(stateChanged)

   CS_SIGNAL_1(Public, void simplifyRichTextChanged(bool un_named_arg1))
   CS_SIGNAL_2(simplifyRichTextChanged, un_named_arg1)

 private:
   bool m_simplifyRichText;
};

class AddLinkDialog : public QDialog
{
   CS_OBJECT(AddLinkDialog)

 public:
   AddLinkDialog(RichTextEditor *editor, QWidget *parent = nullptr);
   ~AddLinkDialog();

   int showDialog();

   CS_SLOT_1(Public, void accept())
   CS_SLOT_2(accept)

 private:
   RichTextEditor *m_editor;
   Ui::AddLinkDialog *m_ui;
};

AddLinkDialog::AddLinkDialog(RichTextEditor *editor, QWidget *parent) :
   QDialog(parent),
   m_ui(new Ui::AddLinkDialog)
{
   m_ui->setupUi(this);

   setWindowFlags(windowFlags() & ~Qt::WindowContextHelpButtonHint);

   m_editor = editor;
}

AddLinkDialog::~AddLinkDialog()
{
   delete m_ui;
}

int AddLinkDialog::showDialog()
{
   // Set initial focus
   const QTextCursor cursor = m_editor->textCursor();
   if (cursor.hasSelection()) {
      m_ui->titleInput->setText(cursor.selectedText());
      m_ui->urlInput->setFocus();
   } else {
      m_ui->titleInput->setFocus();
   }

   return exec();
}

void AddLinkDialog::accept()
{
   const QString title = m_ui->titleInput->text();
   const QString url = m_ui->urlInput->text();

   if (! title.isEmpty()) {
      QString html = QString("<a href=\"");
      html += url;
      html += QString("\">");
      html += title;
      html += QString("</a>");

      m_editor->insertHtml(html);
   }

   m_ui->titleInput->clear();
   m_ui->urlInput->clear();

   QDialog::accept();
}

class HtmlTextEdit : public QTextEdit
{
   CS_OBJECT(HtmlTextEdit)

 public:
   HtmlTextEdit(QWidget *parent = nullptr)
      : QTextEdit(parent)
   {}

   void contextMenuEvent(QContextMenuEvent *event);

 private:
   CS_SLOT_1(Private, void actionTriggered(QAction *action))
   CS_SLOT_2(actionTriggered)
};

void HtmlTextEdit::contextMenuEvent(QContextMenuEvent *event)
{
   QMenu *menu = createStandardContextMenu();
   QMenu *htmlMenu = new QMenu(tr("Insert HTML entity"), menu);

   typedef struct {
      const QString text;
      const QString entity;
   } Entry;

   const Entry entries[] = {
      { "&&amp; (&&)", "&amp;"  },
      { "&&nbsp;",     "&nbsp;" },
      { "&&lt; (<)",   "&lt;"   },
      { "&&gt; (>)",   "&gt;"   },
      { "&&copy; (Copyright)", "&copy;" },
      { "&&reg; (Trade Mark)", "&reg;" },
   };

   for (int i = 0; i < 6; ++i) {
      QAction *entityAction = new QAction(entries[i].text, htmlMenu);
      entityAction->setData(entries[i].entity);
      htmlMenu->addAction(entityAction);
   }

   menu->addMenu(htmlMenu);
   connect(htmlMenu, &QMenu::triggered, this, &HtmlTextEdit::actionTriggered);
   menu->exec(event->globalPos());
   delete menu;
}

void HtmlTextEdit::actionTriggered(QAction *action)
{
   insertPlainText(action->data().toString());
}

class ColorAction : public QAction
{
   CS_OBJECT(ColorAction)

 public:
   ColorAction(QObject *parent);

   const QColor &color() const {
      return m_color;
   }
   void setColor(const QColor &color);

 public:
   CS_SIGNAL_1(Public, void colorChanged(const QColor &color))
   CS_SIGNAL_2(colorChanged, color)

 private:
   CS_SLOT_1(Private, void chooseColor())
   CS_SLOT_2(chooseColor)

 private:
   QColor m_color;
};

ColorAction::ColorAction(QObject *parent)
   : QAction(parent)
{
   setText(tr("Text Color"));
   setColor(Qt::black);
   connect(this, SIGNAL(triggered()), this, SLOT(chooseColor()));
}

void ColorAction::setColor(const QColor &color)
{
   if (color == m_color) {
      return;
   }
   m_color = color;
   QPixmap pix(24, 24);
   QPainter painter(&pix);
   painter.setRenderHint(QPainter::Antialiasing, false);
   painter.fillRect(pix.rect(), m_color);
   painter.setPen(m_color.darker());
   painter.drawRect(pix.rect().adjusted(0, 0, -1, -1));
   setIcon(pix);
}

void ColorAction::chooseColor()
{
   const QColor col = QColorDialog::getColor(m_color, 0);
   if (col.isValid() && col != m_color) {
      setColor(col);
      emit colorChanged(m_color);
   }
}

class RichTextEditorToolBar : public QToolBar
{
   CS_OBJECT(RichTextEditorToolBar)
 public:
   RichTextEditorToolBar(QDesignerFormEditorInterface *core,
      RichTextEditor *editor,
      QWidget *parent = nullptr);

 public :
   CS_SLOT_1(Public, void updateActions())
   CS_SLOT_2(updateActions)

 private:
   CS_SLOT_1(Private, void alignmentActionTriggered(QAction *action))
   CS_SLOT_2(alignmentActionTriggered)
   CS_SLOT_1(Private, void sizeInputActivated(const QString &size))
   CS_SLOT_2(sizeInputActivated)
   CS_SLOT_1(Private, void colorChanged(const QColor &color))
   CS_SLOT_2(colorChanged)
   CS_SLOT_1(Private, void setVAlignSuper(bool super))
   CS_SLOT_2(setVAlignSuper)
   CS_SLOT_1(Private, void setVAlignSub(bool sub))
   CS_SLOT_2(setVAlignSub)
   CS_SLOT_1(Private, void insertLink())
   CS_SLOT_2(insertLink)
   CS_SLOT_1(Private, void insertImage())
   CS_SLOT_2(insertImage)
   CS_SLOT_1(Private, void layoutDirectionChanged())
   CS_SLOT_2(layoutDirectionChanged)

 private:
   QAction *m_bold_action;
   QAction *m_italic_action;
   QAction *m_underline_action;
   QAction *m_valign_sup_action;
   QAction *m_valign_sub_action;
   QAction *m_align_left_action;
   QAction *m_align_center_action;
   QAction *m_align_right_action;
   QAction *m_align_justify_action;
   QAction *m_layoutDirectionAction;
   QAction *m_link_action;
   QAction *m_image_action;
   QAction *m_simplify_richtext_action;
   ColorAction *m_color_action;
   QComboBox *m_font_size_input;

   QDesignerFormEditorInterface *m_core;
   QPointer<RichTextEditor> m_editor;
};

template <typename T, typename R>
static QAction *createCheckableAction(const QIcon &icon, const QString &text, R *receiver,
   T slot, QObject *parent = nullptr)
{
   QAction *result = new QAction(parent);
   result->setIcon(icon);
   result->setText(text);
   result->setCheckable(true);
   result->setChecked(false);

   if constexpr (! std::is_same_v<T, std::nullptr_t>) {
      QObject::connect(result, &QAction::triggered, receiver, slot);
   }

   return result;
}

RichTextEditorToolBar::RichTextEditorToolBar(QDesignerFormEditorInterface *core,
   RichTextEditor *editor,
   QWidget *parent) :
   QToolBar(parent),
   m_link_action(new QAction(this)),
   m_image_action(new QAction(this)),
   m_color_action(new ColorAction(this)),
   m_font_size_input(new QComboBox),
   m_core(core),
   m_editor(editor)
{
   typedef void (QComboBox::*QComboStringSignal)(const QString &);

   // Font size combo box
   m_font_size_input->setEditable(false);
   const QList<int> font_sizes = QFontDatabase::standardSizes();
   for (int font_size : font_sizes) {
      m_font_size_input->addItem(QString::number(font_size));
   }

   connect(m_font_size_input, static_cast<QComboStringSignal>(&QComboBox::activated),
      this, &RichTextEditorToolBar::sizeInputActivated);
   addWidget(m_font_size_input);

   addSeparator();

   // Bold, italic and underline buttons

   m_bold_action = createCheckableAction(createIconSet("textbold.png"),
         tr("Bold"), editor, &RichTextEditor::setFontBold, this);

   m_bold_action->setShortcut(tr("CTRL+B"));

   addAction(m_bold_action);

   m_italic_action = createCheckableAction(createIconSet("textitalic.png"),
         tr("Italic"), editor, &RichTextEditor::setFontItalic, this);

   m_italic_action->setShortcut(tr("CTRL+I"));
   addAction(m_italic_action);

   m_underline_action = createCheckableAction(createIconSet("textunder.png"),
         tr("Underline"), editor, &RichTextEditor::setFontUnderline, this);

   m_underline_action->setShortcut(tr("CTRL+U"));
   addAction(m_underline_action);

   addSeparator();

   // Left, center, right and justified alignment buttons

   QActionGroup *alignment_group = new QActionGroup(this);
   connect(alignment_group, &QActionGroup::triggered,
      this, &RichTextEditorToolBar::alignmentActionTriggered);
   m_align_left_action = createCheckableAction(createIconSet("textleft.png"),
         tr("Left Align"), editor, nullptr, alignment_group);

   addAction(m_align_left_action);

   m_align_center_action = createCheckableAction(createIconSet("textcenter.png"),
         tr("Center"), editor, nullptr, alignment_group);

   addAction(m_align_center_action);

   m_align_right_action = createCheckableAction(createIconSet("textright.png"),
         tr("Right Align"), editor, nullptr, alignment_group);

   addAction(m_align_right_action);

   m_align_justify_action = createCheckableAction(createIconSet("textjustify.png"),
         tr("Justify"), editor, nullptr, alignment_group);

   addAction(m_align_justify_action);

   m_layoutDirectionAction = createCheckableAction(createIconSet("righttoleft.png"),
         tr("Right to Left"), this, &RichTextEditorToolBar::layoutDirectionChanged);

   addAction(m_layoutDirectionAction);

   addSeparator();

   // Superscript and subscript buttons

   m_valign_sup_action = createCheckableAction(createIconSet("textsuperscript.png"),
         tr("Superscript"), this, &RichTextEditorToolBar::setVAlignSuper, this);

   addAction(m_valign_sup_action);

   m_valign_sub_action = createCheckableAction(createIconSet("textsubscript.png"),
         tr("Subscript"), this, &RichTextEditorToolBar::setVAlignSub, this);

   addAction(m_valign_sub_action);

   addSeparator();

   // Insert hyperlink and image buttons

   m_link_action->setIcon(createIconSet(QString("textanchor.png")));
   m_link_action->setText(tr("Insert &Link"));
   connect(m_link_action, &QAction::triggered, this, &RichTextEditorToolBar::insertLink);
   addAction(m_link_action);

   m_image_action->setIcon(createIconSet(QString("insertimage.png")));
   m_image_action->setText(tr("Insert &Image"));
   connect(m_image_action, &QAction::triggered, this, &RichTextEditorToolBar::insertImage);
   addAction(m_image_action);

   addSeparator();

   // Text color button
   connect(m_color_action, &ColorAction::colorChanged,
      this, &RichTextEditorToolBar::colorChanged);
   addAction(m_color_action);

   addSeparator();

   // Simplify rich text
   m_simplify_richtext_action =
         createCheckableAction(createIconSet("simplifyrichtext.png"),
         tr("Simplify Rich Text"), m_editor.data(), &RichTextEditor::setSimplifyRichText);

   m_simplify_richtext_action->setChecked(m_editor->simplifyRichText());
   connect(m_editor.data(), &RichTextEditor::simplifyRichTextChanged,
      m_simplify_richtext_action, &QAction::setChecked);
   addAction(m_simplify_richtext_action);

   connect(editor, &QTextEdit::textChanged, this, &RichTextEditorToolBar::updateActions);
   connect(editor, &RichTextEditor::stateChanged, this, &RichTextEditorToolBar::updateActions);

   updateActions();
}

void RichTextEditorToolBar::alignmentActionTriggered(QAction *action)
{
   Qt::Alignment new_alignment;

   if (action == m_align_left_action) {
      new_alignment = Qt::AlignLeft;
   } else if (action == m_align_center_action) {
      new_alignment = Qt::AlignCenter;
   } else if (action == m_align_right_action) {
      new_alignment = Qt::AlignRight;
   } else {
      new_alignment = Qt::AlignJustify;
   }

   m_editor->setAlignment(new_alignment);
}

void RichTextEditorToolBar::colorChanged(const QColor &color)
{
   m_editor->setTextColor(color);
   m_editor->setFocus();
}

void RichTextEditorToolBar::sizeInputActivated(const QString &size)
{
   bool ok;
   int i = size.toInteger<int>(&ok);
   if (!ok) {
      return;
   }

   m_editor->setFontPointSize(i);
   m_editor->setFocus();
}

void RichTextEditorToolBar::setVAlignSuper(bool super)
{
   const QTextCharFormat::VerticalAlignment align = super ?
      QTextCharFormat::AlignSuperScript : QTextCharFormat::AlignNormal;

   QTextCharFormat charFormat = m_editor->currentCharFormat();
   charFormat.setVerticalAlignment(align);
   m_editor->setCurrentCharFormat(charFormat);

   m_valign_sub_action->setChecked(false);
}

void RichTextEditorToolBar::setVAlignSub(bool sub)
{
   const QTextCharFormat::VerticalAlignment align = sub ?
      QTextCharFormat::AlignSubScript : QTextCharFormat::AlignNormal;

   QTextCharFormat charFormat = m_editor->currentCharFormat();
   charFormat.setVerticalAlignment(align);
   m_editor->setCurrentCharFormat(charFormat);

   m_valign_sup_action->setChecked(false);
}

void RichTextEditorToolBar::insertLink()
{
   AddLinkDialog linkDialog(m_editor, this);
   linkDialog.showDialog();
   m_editor->setFocus();
}

void RichTextEditorToolBar::insertImage()
{
   const QString path = IconSelector::choosePixmapResource(m_core, m_core->resourceModel(), QString(), this);
   if (!path.isEmpty()) {
      m_editor->insertHtml(QString("<img src=\"") + path + QString("\"/>"));
   }
}

void RichTextEditorToolBar::layoutDirectionChanged()
{
   QTextCursor cursor = m_editor->textCursor();
   QTextBlock block = cursor.block();
   if (block.isValid()) {
      QTextBlockFormat format = block.blockFormat();
      const Qt::LayoutDirection newDirection = m_layoutDirectionAction->isChecked() ? Qt::RightToLeft : Qt::LeftToRight;
      if (format.layoutDirection() != newDirection) {
         format.setLayoutDirection(newDirection);
         cursor.setBlockFormat(format);
      }
   }
}

void RichTextEditorToolBar::updateActions()
{
   if (m_editor == 0) {
      setEnabled(false);
      return;
   }

   const Qt::Alignment alignment = m_editor->alignment();
   const QTextCursor cursor = m_editor->textCursor();
   const QTextCharFormat charFormat = cursor.charFormat();
   const QFont font = charFormat.font();
   const QTextCharFormat::VerticalAlignment valign =
      charFormat.verticalAlignment();
   const bool superScript = valign == QTextCharFormat::AlignSuperScript;
   const bool subScript = valign == QTextCharFormat::AlignSubScript;

   if (alignment & Qt::AlignLeft) {
      m_align_left_action->setChecked(true);
   } else if (alignment & Qt::AlignRight) {
      m_align_right_action->setChecked(true);
   } else if (alignment & Qt::AlignHCenter) {
      m_align_center_action->setChecked(true);
   } else {
      m_align_justify_action->setChecked(true);
   }
   m_layoutDirectionAction->setChecked(cursor.blockFormat().layoutDirection() == Qt::RightToLeft);

   m_bold_action->setChecked(font.bold());
   m_italic_action->setChecked(font.italic());
   m_underline_action->setChecked(font.underline());
   m_valign_sup_action->setChecked(superScript);
   m_valign_sub_action->setChecked(subScript);

   const int size = font.pointSize();
   const int idx = m_font_size_input->findText(QString::number(size));
   if (idx != -1) {
      m_font_size_input->setCurrentIndex(idx);
   }

   m_color_action->setColor(m_editor->textColor());
}

RichTextEditor::RichTextEditor(QWidget *parent)
   : QTextEdit(parent), m_simplifyRichText(simplifyRichTextDefault)
{
   connect(this, &RichTextEditor::currentCharFormatChanged,
      this, &RichTextEditor::stateChanged);
   connect(this, &RichTextEditor::cursorPositionChanged,
      this, &RichTextEditor::stateChanged);
}

QToolBar *RichTextEditor::createToolBar(QDesignerFormEditorInterface *core, QWidget *parent)
{
   return new RichTextEditorToolBar(core, this, parent);
}

void RichTextEditor::setFontBold(bool b)
{
   if (b) {
      setFontWeight(QFont::Bold);
   } else {
      setFontWeight(QFont::Normal);
   }
}

void RichTextEditor::setFontPointSize(double d)
{
   QTextEdit::setFontPointSize(qreal(d));
}

void RichTextEditor::setText(const QString &text)
{

   if (Qt::mightBeRichText(text)) {
      setHtml(text);
   } else {
      setPlainText(text);
   }
}

void RichTextEditor::setSimplifyRichText(bool v)
{
   if (v != m_simplifyRichText) {
      m_simplifyRichText = v;
      emit simplifyRichTextChanged(v);
   }
}

void RichTextEditor::setDefaultFont(QFont font)
{
   // Some default fonts on Windows have a default size of 7.8,
   // which results in complicated rich text generated by toHtml().
   // Use an integer value.
   const int pointSize = qRound(font.pointSizeF());
   if (pointSize > 0 && !qFuzzyCompare(qreal(pointSize), font.pointSizeF())) {
      font.setPointSize(pointSize);
   }

   document()->setDefaultFont(font);
   if (font.pointSize() > 0) {
      setFontPointSize(font.pointSize());
   } else {
      setFontPointSize(QFontInfo(font).pointSize());
   }
   emit textChanged();
}

QString RichTextEditor::text(Qt::TextFormat format) const
{
   switch (format) {
      case Qt::PlainText:
         return toPlainText();
      case Qt::RichText:
         return m_simplifyRichText ? simplifyRichTextFilter(toHtml()) : toHtml();
      case Qt::AutoText:
         break;
   }
   const QString html = toHtml();
   bool isPlainText;
   const QString simplifiedHtml = simplifyRichTextFilter(html, &isPlainText);
   if (isPlainText) {
      return toPlainText();
   }
   return m_simplifyRichText ? simplifiedHtml : html;
}

RichTextEditorDialog::RichTextEditorDialog(QDesignerFormEditorInterface *core, QWidget *parent)  :
   QDialog(parent),
   m_editor(new RichTextEditor()),
   m_text_edit(new HtmlTextEdit),
   m_tab_widget(new QTabWidget),
   m_state(Clean),
   m_core(core),
   m_initialTab(RichTextIndex)
{
   setWindowTitle(tr("Edit text"));
   setWindowFlags(windowFlags() & ~Qt::WindowContextHelpButtonHint);

   // Read settings
   const QDesignerSettingsInterface *settings = core->settingsManager();
   const QString rootKey = RichTextDialogGroupC + '/';
   const QByteArray lastGeometry = settings->value(rootKey + GeometryKeyC).toByteArray();
   const int initialTab = settings->value(rootKey + TabKeyC, QVariant(m_initialTab)).toInt();
   if (initialTab == RichTextIndex || initialTab == SourceIndex) {
      m_initialTab = initialTab;
   }

   m_text_edit->setAcceptRichText(false);
   new HtmlHighlighter(m_text_edit);

   connect(m_editor, &QTextEdit::textChanged, this, &RichTextEditorDialog::richTextChanged);
   connect(m_editor, &RichTextEditor::simplifyRichTextChanged,
      this, &RichTextEditorDialog::richTextChanged);
   connect(m_text_edit, &QTextEdit::textChanged, this, &RichTextEditorDialog::sourceChanged);

   // The toolbar needs to be created after the RichTextEditor
   QToolBar *tool_bar = m_editor->createToolBar(core);
   tool_bar->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Minimum);

   QWidget *rich_edit = new QWidget;
   QVBoxLayout *rich_edit_layout = new QVBoxLayout(rich_edit);
   rich_edit_layout->addWidget(tool_bar);
   rich_edit_layout->addWidget(m_editor);

   QWidget *plain_edit = new QWidget;
   QVBoxLayout *plain_edit_layout = new QVBoxLayout(plain_edit);
   plain_edit_layout->addWidget(m_text_edit);

   m_tab_widget->setTabPosition(QTabWidget::South);
   m_tab_widget->addTab(rich_edit, tr("Rich Text"));
   m_tab_widget->addTab(plain_edit, tr("Source"));
   connect(m_tab_widget, &QTabWidget::currentChanged,
      this, &RichTextEditorDialog::tabIndexChanged);

   QDialogButtonBox *buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, Qt::Horizontal);
   QPushButton *ok_button = buttonBox->button(QDialogButtonBox::Ok);
   ok_button->setText(tr("&OK"));
   ok_button->setDefault(true);
   buttonBox->button(QDialogButtonBox::Cancel)->setText(tr("&Cancel"));
   connect(buttonBox, &QDialogButtonBox::accepted, this, &QDialog::accept);
   connect(buttonBox, &QDialogButtonBox::rejected, this, &QDialog::reject);

   QVBoxLayout *layout = new QVBoxLayout(this);
   layout->addWidget(m_tab_widget);
   layout->addWidget(buttonBox);

   if (!lastGeometry.isEmpty()) {
      restoreGeometry(lastGeometry);
   }
}

RichTextEditorDialog::~RichTextEditorDialog()
{
   QDesignerSettingsInterface *settings = m_core->settingsManager();
   settings->beginGroup(RichTextDialogGroupC);

   settings->setValue(GeometryKeyC, saveGeometry());
   settings->setValue(TabKeyC, m_tab_widget->currentIndex());
   settings->endGroup();
}

int RichTextEditorDialog::showDialog()
{
   m_tab_widget->setCurrentIndex(m_initialTab);
   switch (m_initialTab) {
      case RichTextIndex:
         m_editor->selectAll();
         m_editor->setFocus();
         break;
      case SourceIndex:
         m_text_edit->selectAll();
         m_text_edit->setFocus();
         break;
   }
   return exec();
}

void RichTextEditorDialog::setDefaultFont(const QFont &font)
{
   m_editor->setDefaultFont(font);
}

void RichTextEditorDialog::setText(const QString &text)
{
   // Generally simplify rich text unless verbose text is found.
   const bool isSimplifiedRichText = !text.startsWith(
         QString("<!DOCTYPE HTML PUBLIC \"-//W3C//DTD HTML 4.0//EN\" \"http://www.w3.org/TR/REC-html40/strict.dtd\">"));
   m_editor->setSimplifyRichText(isSimplifiedRichText);
   m_editor->setText(text);
   m_text_edit->setPlainText(text);
   m_state = Clean;
}

QString RichTextEditorDialog::text(Qt::TextFormat format) const
{
   // In autotext mode, if the user has changed the source, use that
   if (format == Qt::AutoText && (m_state == Clean || m_state == SourceChanged)) {
      return m_text_edit->toPlainText();
   }
   // If the plain text HTML editor is selected, first copy its contents over
   // to the rich text editor so that it is converted to Qt-HTML or actual
   // plain text.
   if (m_tab_widget->currentIndex() == SourceIndex && m_state == SourceChanged) {
      m_editor->setHtml(m_text_edit->toPlainText());
   }
   return m_editor->text(format);
}

void RichTextEditorDialog::tabIndexChanged(int newIndex)
{
   // Anything changed, is there a need for a conversion?
   if (newIndex == SourceIndex && m_state != RichTextChanged) {
      return;
   }
   if (newIndex == RichTextIndex && m_state != SourceChanged) {
      return;
   }
   const State oldState = m_state;
   // Remember the cursor position, since it is invalidated by setPlainText
   QTextEdit *new_edit = (newIndex == SourceIndex) ? m_text_edit : m_editor;
   const int position = new_edit->textCursor().position();

   if (newIndex == SourceIndex) {
      m_text_edit->setPlainText(m_editor->text(Qt::RichText));
   } else {
      m_editor->setHtml(m_text_edit->toPlainText());
   }

   QTextCursor cursor = new_edit->textCursor();
   cursor.movePosition(QTextCursor::End);
   if (cursor.position() > position) {
      cursor.setPosition(position);
   }
   new_edit->setTextCursor(cursor);
   m_state = oldState; // Changed is triggered by setting the text
}

void RichTextEditorDialog::richTextChanged()
{
   m_state = RichTextChanged;
}

void RichTextEditorDialog::sourceChanged()
{
   m_state = SourceChanged;
}

}   // end namespace qdesigner_internal

