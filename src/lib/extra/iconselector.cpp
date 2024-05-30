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

#include <abstract_dialoggui.h>
#include <abstract_formeditor.h>
#include <abstract_integration.h>
#include <abstract_language.h>
#include <abstract_resourcebrowser.h>
#include <designer_utils.h>
#include <extension.h>
#include <resource_model.h>
#include <resource_view.h>

#include <formwindowbase_p.h>
#include <iconloader_p.h>
#include <iconselector_p.h>

#include <QAction>
#include <QApplication>
#include <QComboBox>
#include <QDialog>
#include <QDialogButtonBox>
#include <QDialogButtonBox>
#include <QImageReader>
#include <QLabel>
#include <QLineEdit>
#include <QMenu>
#include <QPushButton>
#include <QSignalMapper>
#include <QToolButton>
#include <QVBoxLayout>
#include <QVBoxLayout>
#include <QValidator>

namespace qdesigner_internal {

class LanguageResourceDialogPrivate
{
   LanguageResourceDialog *q_ptr;
   Q_DECLARE_PUBLIC(LanguageResourceDialog)

 public:
   LanguageResourceDialogPrivate(QDesignerResourceBrowserInterface *rb);
   void init(LanguageResourceDialog *p);

   void setCurrentPath(const QString &filePath);
   QString currentPath() const;

   void slotAccepted();
   void slotPathChanged(const QString &path);

 private:
   void setOkButtonEnabled(bool v)         {
      m_dialogButtonBox->button(QDialogButtonBox::Ok)->setEnabled(v);
   }
   static bool checkPath(const QString &p);

   QDesignerResourceBrowserInterface *m_browser;
   QDialogButtonBox *m_dialogButtonBox;
};

LanguageResourceDialogPrivate::LanguageResourceDialogPrivate(QDesignerResourceBrowserInterface *rb)
   : q_ptr(nullptr), m_browser(rb),
     m_dialogButtonBox(new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, Qt::Horizontal))
{
   setOkButtonEnabled(false);
}

void LanguageResourceDialogPrivate::init(LanguageResourceDialog *p)
{
   q_ptr = p;

   QLayout *layout = new QVBoxLayout(p);
   layout->addWidget(m_browser);
   layout->addWidget(m_dialogButtonBox);

   QObject::connect(m_dialogButtonBox, &QDialogButtonBox::accepted,
         p, &LanguageResourceDialog::slotAccepted);

   QObject::connect(m_dialogButtonBox, &QDialogButtonBox::rejected,
         p, &LanguageResourceDialog::reject);

   QObject::connect(m_browser, &QDesignerResourceBrowserInterface::currentPathChanged,
         p, &LanguageResourceDialog::slotPathChanged);

   QObject::connect(m_browser, &QDesignerResourceBrowserInterface::pathActivated,
         p, &LanguageResourceDialog::slotAccepted);

   p->setModal(true);
   p->setWindowTitle(LanguageResourceDialog::tr("Choose Resource"));
   p->setWindowFlags(p->windowFlags() & ~Qt::WindowContextHelpButtonHint);

   setOkButtonEnabled(false);
}

void LanguageResourceDialogPrivate::setCurrentPath(const QString &filePath)
{
   m_browser->setCurrentPath(filePath);
   setOkButtonEnabled(checkPath(filePath));
}

QString LanguageResourceDialogPrivate::currentPath() const
{
   return m_browser->currentPath();
}

bool LanguageResourceDialogPrivate::checkPath(const QString &p)
{
   return p.isEmpty() ? false : IconSelector::checkPixmap(p, IconSelector::CheckFast);
}

void LanguageResourceDialogPrivate::slotAccepted()
{
   if (checkPath(currentPath())) {
      q_ptr->accept();
   }
}

void LanguageResourceDialogPrivate::slotPathChanged(const QString &path)
{
   setOkButtonEnabled(checkPath(path));
}

// ------------ LanguageResourceDialog
LanguageResourceDialog::LanguageResourceDialog(QDesignerResourceBrowserInterface *rb, QWidget *parent) :
   QDialog(parent),
   d_ptr(new LanguageResourceDialogPrivate(rb))
{
   d_ptr->init( this);
}

LanguageResourceDialog::~LanguageResourceDialog()
{
}

void LanguageResourceDialog::setCurrentPath(const QString &filePath)
{
   d_ptr->setCurrentPath(filePath);
}

QString LanguageResourceDialog::currentPath() const
{
   return d_ptr->currentPath();
}

LanguageResourceDialog *LanguageResourceDialog::create(QDesignerFormEditorInterface *core, QWidget *parent)
{
   if (QDesignerLanguageExtension *lang = qt_extension<QDesignerLanguageExtension *>(core->extensionManager(), core)) {

      if (QDesignerResourceBrowserInterface *rb = lang->createResourceBrowser(nullptr)) {
         return new LanguageResourceDialog(rb, parent);
      }
   }

   if (QDesignerResourceBrowserInterface *rb = core->integration()->createResourceBrowser(nullptr)) {
      return new LanguageResourceDialog(rb, parent);
   }

   return nullptr;
}

static inline QPixmap emptyPixmap()
{
   QImage img(16, 16, QImage::Format_ARGB32_Premultiplied);
   img.fill(0);
   return QPixmap::fromImage(img);
}

class IconSelectorPrivate
{
   IconSelector *q_ptr;
   Q_DECLARE_PUBLIC(IconSelector)

 public:
   IconSelectorPrivate();

   void slotStateActivated();
   void slotSetActivated();
   void slotSetResourceActivated();
   void slotSetFileActivated();
   void slotResetActivated();
   void slotResetAllActivated();
   void slotUpdate();

   QList<QPair<QPair<QIcon::Mode, QIcon::State>, QString>> m_stateToName; // could be static map

   QMap<QPair<QIcon::Mode, QIcon::State>, int>  m_stateToIndex;
   QMap<int, QPair<QIcon::Mode, QIcon::State>> m_indexToState;

   const QIcon m_emptyIcon;
   QComboBox *m_stateComboBox;
   QToolButton *m_iconButton;
   QAction *m_resetAction;
   QAction *m_resetAllAction;
   PropertySheetIconValue m_icon;
   DesignerIconCache *m_iconCache;
   DesignerPixmapCache *m_pixmapCache;
   QtResourceModel *m_resourceModel;
   QDesignerFormEditorInterface *m_core;
};

IconSelectorPrivate::IconSelectorPrivate()
   : q_ptr(nullptr), m_emptyIcon(emptyPixmap()), m_stateComboBox(nullptr), m_iconButton(nullptr),
     m_resetAction(nullptr), m_resetAllAction(nullptr), m_iconCache(nullptr), m_pixmapCache(nullptr),
     m_resourceModel(nullptr), m_core(nullptr)
{
}
void IconSelectorPrivate::slotUpdate()
{
   QIcon icon;
   if (m_iconCache) {
      icon = m_iconCache->icon(m_icon);
   }

   QMap<QPair<QIcon::Mode, QIcon::State>, PropertySheetPixmapValue> paths = m_icon.paths();
   QMapIterator<QPair<QIcon::Mode, QIcon::State>, int> itIndex(m_stateToIndex);

   while (itIndex.hasNext()) {
      const QPair<QIcon::Mode, QIcon::State> state = itIndex.next().key();
      const PropertySheetPixmapValue pixmap = paths.value(state);
      const int index = itIndex.value();

      QIcon pixmapIcon = QIcon(icon.pixmap(16, 16, state.first, state.second));
      if (pixmapIcon.isNull()) {
         pixmapIcon = m_emptyIcon;
      }
      m_stateComboBox->setItemIcon(index, pixmapIcon);
      QFont font = q_ptr->font();
      if (!pixmap.path().isEmpty()) {
         font.setBold(true);
      }
      m_stateComboBox->setItemData(index, font, Qt::FontRole);
   }

   QPair<QIcon::Mode, QIcon::State> state = m_indexToState.value(m_stateComboBox->currentIndex());
   PropertySheetPixmapValue currentPixmap = paths.value(state);
   m_resetAction->setEnabled(!currentPixmap.path().isEmpty());
   m_resetAllAction->setEnabled(!paths.isEmpty());
   m_stateComboBox->update();
}

void IconSelectorPrivate::slotStateActivated()
{
   slotUpdate();
}

void IconSelectorPrivate::slotSetActivated()
{
   QPair<QIcon::Mode, QIcon::State> state = m_indexToState.value(m_stateComboBox->currentIndex());
   const PropertySheetPixmapValue pixmap = m_icon.pixmap(state.first, state.second);
   // Default to resource
   const PropertySheetPixmapValue::PixmapSource ps = pixmap.path().isEmpty() ? PropertySheetPixmapValue::ResourcePixmap :
      pixmap.pixmapSource(m_core);
   switch (ps) {
      case PropertySheetPixmapValue::LanguageResourcePixmap:
      case PropertySheetPixmapValue::ResourcePixmap:
         slotSetResourceActivated();
         break;
      case PropertySheetPixmapValue::FilePixmap:
         slotSetFileActivated();
         break;
   }
}

// Choose a pixmap from resource; use language-dependent resource browser if present
QString IconSelector::choosePixmapResource(QDesignerFormEditorInterface *core, QtResourceModel *resourceModel,
   const QString &oldPath, QWidget *parent)
{
   (void) resourceModel;
   QString rc;

   if (LanguageResourceDialog *ldlg = LanguageResourceDialog::create(core, parent)) {
      ldlg->setCurrentPath(oldPath);
      if (ldlg->exec() == QDialog::Accepted) {
         rc = ldlg->currentPath();
      }
      delete ldlg;
   } else {
      QtResourceViewDialog dlg(core, parent);
      dlg.setResourceEditingEnabled(core->integration()->hasFeature(QDesignerIntegration::ResourceEditorFeature));

      dlg.selectResource(oldPath);
      if (dlg.exec() == QDialog::Accepted) {
         rc = dlg.selectedResource();
      }
   }
   return rc;
}

void IconSelectorPrivate::slotSetResourceActivated()
{
   const QPair<QIcon::Mode, QIcon::State> state = m_indexToState.value(m_stateComboBox->currentIndex());

   PropertySheetPixmapValue pixmap = m_icon.pixmap(state.first, state.second);
   const QString oldPath = pixmap.path();
   const QString newPath = IconSelector::choosePixmapResource(m_core, m_resourceModel, oldPath, q_ptr);
   if (newPath.isEmpty() || newPath == oldPath) {
      return;
   }
   const PropertySheetPixmapValue newPixmap = PropertySheetPixmapValue(newPath);
   if (newPixmap != pixmap) {
      m_icon.setPixmap(state.first, state.second, newPixmap);
      slotUpdate();
      emit q_ptr->iconChanged(m_icon);
   }
}

// Helpers for choosing image files: Check for valid image.
bool IconSelector::checkPixmap(const QString &fileName, CheckMode cm, QString *errorMessage)
{
   const QFileInfo fi(fileName);

   if (!fi.exists() || !fi.isFile() || !fi.isReadable()) {
      if (errorMessage) {
         *errorMessage = tr("The pixmap file '%1' can not be read.").formatArg(fileName);
      }

      return false;
   }
   QImageReader reader(fileName);
   if (!reader.canRead()) {

      if (errorMessage) {
         *errorMessage = tr("The file '%1' does not appear to be a valid pixmap file: %2")
            .formatArg(fileName).formatArg(reader.errorString());
      }

      return false;
   }
   if (cm == CheckFast) {
      return true;
   }

   const QImage image = reader.read();
   if (image.isNull()) {
      if (errorMessage) {
         *errorMessage = tr("The file '%1' could not be read: %2").formatArg(fileName).formatArg(reader.errorString());
      }
      return false;
   }
   return true;
}

// Helpers for choosing image files: Return an image filter for QFileDialog, courtesy of StyledButton
static QString imageFilter()
{
   QString filter = QApplication::translate("IconSelector", "All Pixmaps (");

   const QList<QString> supportedImageFormats = QImageReader::supportedImageFormats();
   const QString jpeg = "JPEG";
   const int count    = supportedImageFormats.count();

   for (int i = 0; i < count; ++i) {

      if (i) {
         filter += ' ';
      }

      filter += "*.";

      const QString outputFormat = supportedImageFormats.at(i);

      if (outputFormat != jpeg) {
         filter += outputFormat.toLower();
      } else {
         filter += QString("jpg *.jpeg");
      }
   }

   filter += ')';

   return filter;
}

// Helpers for choosing image files: Choose a file
QString IconSelector::choosePixmapFile(const QString &directory, QDesignerDialogGuiInterface *dlgGui, QWidget *parent)
{
   QString errorMessage;
   QString newPath;
   do {
      const QString title = tr("Choose a Pixmap");
      static const  QString filter = imageFilter();
      newPath =  dlgGui->getOpenImageFileName(parent, title, directory, filter);
      if (newPath.isEmpty()) {
         break;
      }
      if (checkPixmap(newPath, CheckFully, &errorMessage)) {
         break;
      }
      dlgGui->message(parent, QDesignerDialogGuiInterface::ResourceEditorMessage, QMessageBox::Warning, tr("Pixmap Read Error"),
         errorMessage);
   } while (true);
   return  newPath;
}

void IconSelectorPrivate::slotSetFileActivated()
{
   QPair<QIcon::Mode, QIcon::State> state = m_indexToState.value(m_stateComboBox->currentIndex());

   PropertySheetPixmapValue pixmap = m_icon.pixmap(state.first, state.second);
   const QString newPath = IconSelector::choosePixmapFile(pixmap.path(), m_core->dialogGui(), q_ptr);
   if (!newPath.isEmpty()) {
      const PropertySheetPixmapValue newPixmap = PropertySheetPixmapValue(newPath);
      if (!(newPixmap == pixmap)) {
         m_icon.setPixmap(state.first, state.second, newPixmap);
         slotUpdate();
         emit q_ptr->iconChanged(m_icon);
      }
   }
}

void IconSelectorPrivate::slotResetActivated()
{
   QPair<QIcon::Mode, QIcon::State> state = m_indexToState.value(m_stateComboBox->currentIndex());

   PropertySheetPixmapValue pixmap = m_icon.pixmap(state.first, state.second);
   const PropertySheetPixmapValue newPixmap;
   if (!(newPixmap == pixmap)) {
      m_icon.setPixmap(state.first, state.second, newPixmap);
      slotUpdate();
      emit q_ptr->iconChanged(m_icon);
   }
}

void IconSelectorPrivate::slotResetAllActivated()
{
   const PropertySheetIconValue newIcon;
   if (!(m_icon == newIcon)) {
      m_icon = newIcon;
      slotUpdate();
      emit q_ptr->iconChanged(m_icon);
   }
}

// ------------- IconSelector
IconSelector::IconSelector(QWidget *parent) :
   QWidget(parent), d_ptr(new IconSelectorPrivate())
{
   d_ptr->q_ptr = this;

   d_ptr->m_stateComboBox = new QComboBox(this);

   QHBoxLayout *list = new QHBoxLayout(this);
   d_ptr->m_iconButton = new QToolButton(this);
   d_ptr->m_iconButton->setText(tr("..."));
   d_ptr->m_iconButton->setPopupMode(QToolButton::MenuButtonPopup);

   list->addWidget(d_ptr->m_stateComboBox);
   list->addWidget(d_ptr->m_iconButton);
   list->setMargin(0);

   d_ptr->m_stateToName << qMakePair(qMakePair(QIcon::Normal,   QIcon::Off), tr("Normal Off")   );
   d_ptr->m_stateToName << qMakePair(qMakePair(QIcon::Normal,   QIcon::On),  tr("Normal On")    );
   d_ptr->m_stateToName << qMakePair(qMakePair(QIcon::Disabled, QIcon::Off), tr("Disabled Off") );
   d_ptr->m_stateToName << qMakePair(qMakePair(QIcon::Disabled, QIcon::On),  tr("Disabled On")  );
   d_ptr->m_stateToName << qMakePair(qMakePair(QIcon::Active,   QIcon::Off), tr("Active Off")   );
   d_ptr->m_stateToName << qMakePair(qMakePair(QIcon::Active,   QIcon::On),  tr("Active On")    );
   d_ptr->m_stateToName << qMakePair(qMakePair(QIcon::Selected, QIcon::Off), tr("Selected Off") );
   d_ptr->m_stateToName << qMakePair(qMakePair(QIcon::Selected, QIcon::On),  tr("Selected On")  );

   QMenu *setMenu = new QMenu(this);

   QAction *setResourceAction = new QAction(tr("Choose Resource..."), this);
   QAction *setFileAction     = new QAction(tr("Choose File..."), this);
   d_ptr->m_resetAction       = new QAction(tr("Reset"), this);
   d_ptr->m_resetAllAction    = new QAction(tr("Reset All"), this);

   d_ptr->m_resetAction->setEnabled(false);
   d_ptr->m_resetAllAction->setEnabled(false);

   // d_ptr->m_resetAction->setIcon(createIconSet(QString::fromUtf8("resetproperty.png")));

   setMenu->addAction(setResourceAction);
   setMenu->addAction(setFileAction);
   setMenu->addSeparator();
   setMenu->addAction(d_ptr->m_resetAction);
   setMenu->addAction(d_ptr->m_resetAllAction);

   int index = 0;
   QStringList items;
   QListIterator<QPair<QPair<QIcon::Mode, QIcon::State>, QString>> itName(d_ptr->m_stateToName);

   while (itName.hasNext()) {
      QPair<QPair<QIcon::Mode, QIcon::State>, QString> item = itName.next();
      const QPair<QIcon::Mode, QIcon::State> state = item.first;
      const QString name = item.second;

      items.append(name);
      d_ptr->m_stateToIndex[state] = index;
      d_ptr->m_indexToState[index] = state;
      ++index;
   }
   d_ptr->m_stateComboBox->addItems(items);

   d_ptr->m_iconButton->setMenu(setMenu);

   connect(d_ptr->m_stateComboBox,  SIGNAL(activated(int)), this, SLOT(slotStateActivated()));
   connect(d_ptr->m_iconButton,     SIGNAL(clicked()),      this, SLOT(slotSetActivated()));
   connect(setResourceAction,       SIGNAL(triggered()),    this, SLOT(slotSetResourceActivated()));
   connect(setFileAction,           SIGNAL(triggered()),    this, SLOT(slotSetFileActivated()));
   connect(d_ptr->m_resetAction,    SIGNAL(triggered()),    this, SLOT(slotResetActivated()));
   connect(d_ptr->m_resetAllAction, SIGNAL(triggered()),    this, SLOT(slotResetAllActivated()));

   d_ptr->slotUpdate();
}

IconSelector::~IconSelector()
{
}

void IconSelector::setIcon(const PropertySheetIconValue &icon)
{
   if (d_ptr->m_icon == icon) {
      return;
   }

   d_ptr->m_icon = icon;
   d_ptr->slotUpdate();
}

PropertySheetIconValue IconSelector::icon() const
{
   return d_ptr->m_icon;
}

void IconSelector::setFormEditor(QDesignerFormEditorInterface *core)
{
   d_ptr->m_core = core;
   d_ptr->m_resourceModel = core->resourceModel();
   d_ptr->slotUpdate();
}

void IconSelector::setIconCache(DesignerIconCache *iconCache)
{
   d_ptr->m_iconCache = iconCache;
   connect(iconCache, SIGNAL(reloaded()), this, SLOT(slotUpdate()));
   d_ptr->slotUpdate();
}

void IconSelector::setPixmapCache(DesignerPixmapCache *pixmapCache)
{
   d_ptr->m_pixmapCache = pixmapCache;
   connect(pixmapCache, SIGNAL(reloaded()), this, SLOT(slotUpdate()));
   d_ptr->slotUpdate();
}

// --- IconThemeEditor

// Validator for theme line edit, accepts empty or non-blank strings.
class BlankSuppressingValidator : public QValidator
{
 public:
   explicit BlankSuppressingValidator(QObject *parent = nullptr)
      : QValidator(parent)
   {
   }

   virtual State validate(QString &input, int &pos) const override {
      const int blankPos = input.indexOf(' ');

      if (blankPos != -1) {
         pos = blankPos;
         return Invalid;
      }

      return Acceptable;
   }
};

struct IconThemeEditorPrivate {
   IconThemeEditorPrivate();

   const QPixmap m_emptyPixmap;
   QLineEdit *m_themeLineEdit;
   QLabel *m_themeLabel;
};

IconThemeEditorPrivate::IconThemeEditorPrivate() :
   m_emptyPixmap(emptyPixmap()),
   m_themeLineEdit(new QLineEdit),
   m_themeLabel(new QLabel)
{
}

IconThemeEditor::IconThemeEditor(QWidget *parent, bool wantResetButton) :
   QWidget (parent), d(new IconThemeEditorPrivate)
{
   QHBoxLayout *mainHLayout = new QHBoxLayout;
   mainHLayout->setMargin(0);

   // Vertically center theme preview label
   d->m_themeLabel->setPixmap(d->m_emptyPixmap);

   QVBoxLayout *themeLabelVLayout = new QVBoxLayout;
   d->m_themeLabel->setMargin(1);
   themeLabelVLayout->setMargin(0);
   themeLabelVLayout->addSpacerItem(new QSpacerItem(0, 0, QSizePolicy::Ignored, QSizePolicy::MinimumExpanding));
   themeLabelVLayout->addWidget(d->m_themeLabel);
   themeLabelVLayout->addSpacerItem(new QSpacerItem(0, 0, QSizePolicy::Ignored, QSizePolicy::MinimumExpanding));
   mainHLayout->addLayout(themeLabelVLayout);

   d->m_themeLineEdit = new QLineEdit;
   d->m_themeLineEdit->setValidator(new BlankSuppressingValidator(d->m_themeLineEdit));

   connect(d->m_themeLineEdit, &QLineEdit::textChanged, this, &IconThemeEditor::slotChanged);
   connect(d->m_themeLineEdit, &QLineEdit::textEdited,  this, &IconThemeEditor::edited);

   mainHLayout->addWidget(d->m_themeLineEdit);

   if (wantResetButton) {
      QToolButton *themeResetButton = new QToolButton;
      themeResetButton->setIcon(createIconSet(QString("resetproperty.png")));
      connect(themeResetButton, &QAbstractButton::clicked, this, &IconThemeEditor::reset);
      mainHLayout->addWidget(themeResetButton);
   }

   setLayout(mainHLayout);
   setFocusProxy(d->m_themeLineEdit);
}

IconThemeEditor::~IconThemeEditor()
{
}

void IconThemeEditor::reset()
{
   d->m_themeLineEdit->clear();
   emit edited(QString());
}

void IconThemeEditor::slotChanged(const QString &theme)
{
   updatePreview(theme);
}

void IconThemeEditor::updatePreview(const QString &t)
{
   // Update preview label with icon.
   if (t.isEmpty() || !QIcon::hasThemeIcon(t)) { // Empty
      const QPixmap *currentPixmap = d->m_themeLabel->pixmap();

      if (currentPixmap == nullptr || currentPixmap->cacheKey() != d->m_emptyPixmap.cacheKey()) {
         d->m_themeLabel->setPixmap(d->m_emptyPixmap);
      }

   } else {
      const QIcon icon = QIcon::fromTheme(t);
      d->m_themeLabel->setPixmap(icon.pixmap(d->m_emptyPixmap.size()));
   }
}

QString IconThemeEditor::theme() const
{
   return d->m_themeLineEdit->text();
}

void IconThemeEditor::setTheme(const QString &t)
{
   d->m_themeLineEdit->setText(t);
}

// private slots
void LanguageResourceDialog::slotAccepted()
{
   Q_D(LanguageResourceDialog);
   d->slotAccepted();
}

void LanguageResourceDialog::slotPathChanged(const QString &path)
{
   Q_D(LanguageResourceDialog);
   d->slotPathChanged(path);
}

void IconSelector::slotStateActivated()
{
   Q_D(IconSelector);
   d->slotStateActivated();
}

void IconSelector::slotSetActivated()
{
   Q_D(IconSelector);
   d->slotSetActivated();
}

void IconSelector::slotSetResourceActivated()
{
   Q_D(IconSelector);
   d->slotSetResourceActivated();
}

void IconSelector::slotSetFileActivated()
{
   Q_D(IconSelector);
   d->slotSetFileActivated();
}

void IconSelector::slotResetActivated()
{
   Q_D(IconSelector);
   d->slotResetActivated();
}

void IconSelector::slotResetAllActivated()
{
   Q_D(IconSelector);
   d->slotResetAllActivated();
}

void IconSelector::slotUpdate()
{
   Q_D(IconSelector);
   d->slotUpdate();
}

}   // end namespace - qdesigner_internal

