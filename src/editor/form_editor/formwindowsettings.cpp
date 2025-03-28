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

#include <formwindowsettings.h>
#include <ui_designer_form_settings.h>

#include <formwindowbase_p.h>
#include <grid_p.h>

#include <QRegularExpression>
#include <QStyle>

namespace qdesigner_internal {

// Data structure containing form dialog data providing comparison
struct FormWindowData {
   FormWindowData();

   bool equals(const FormWindowData &) const;

   void fromFormWindow(FormWindowBase *fw);
   void applyToFormWindow(FormWindowBase *fw) const;

   bool layoutDefaultEnabled;
   int defaultMargin;
   int defaultSpacing;

   bool layoutFunctionsEnabled;
   QString marginFunction;
   QString spacingFunction;

   QString pixFunction;

   QString author;

   QStringList includeHints;

   bool hasFormGrid;
   Grid grid;
};

inline bool operator==(const FormWindowData &fd1, const FormWindowData &fd2)
{
   return fd1.equals(fd2);
}

inline bool operator!=(const FormWindowData &fd1, const FormWindowData &fd2)
{
   return !fd1.equals(fd2);
}

FormWindowData::FormWindowData()
   : layoutDefaultEnabled(false), defaultMargin(0), defaultSpacing(0), layoutFunctionsEnabled(false), hasFormGrid(false)
{
}

bool FormWindowData::equals(const FormWindowData &rhs) const
{
   return layoutDefaultEnabled   == rhs.layoutDefaultEnabled &&
      defaultMargin          == rhs.defaultMargin &&
      defaultSpacing         == rhs.defaultSpacing &&
      layoutFunctionsEnabled == rhs.layoutFunctionsEnabled &&
      marginFunction         == rhs.marginFunction &&
      spacingFunction        == rhs.spacingFunction &&
      pixFunction            == rhs.pixFunction  &&
      author                 == rhs.author &&
      includeHints           == rhs.includeHints &&
      hasFormGrid            == rhs.hasFormGrid &&
      grid                   == rhs.grid;
}

void FormWindowData::fromFormWindow(FormWindowBase *fw)
{
   defaultMargin =  defaultSpacing = INT_MIN;
   fw->layoutDefault(&defaultMargin, &defaultSpacing);

   QStyle *style = fw->formContainer()->style();
   layoutDefaultEnabled = defaultMargin != INT_MIN || defaultMargin != INT_MIN;

   if (defaultMargin == INT_MIN) {
      defaultMargin = style->pixelMetric(QStyle::PM_DefaultChildMargin, nullptr);
   }

   if (defaultSpacing == INT_MIN) {
      defaultSpacing = style->pixelMetric(QStyle::PM_DefaultLayoutSpacing, nullptr);
   }


   marginFunction.clear();
   spacingFunction.clear();
   fw->layoutFunction(&marginFunction, &spacingFunction);
   layoutFunctionsEnabled = !marginFunction.isEmpty() || !spacingFunction.isEmpty();

   pixFunction = fw->pixmapFunction();

   author = fw->author();

   includeHints = fw->includeHints();
   includeHints.removeAll(QString());

   hasFormGrid = fw->hasFormGrid();
   grid = hasFormGrid ? fw->designerGrid() : FormWindowBase::defaultDesignerGrid();
}

void FormWindowData::applyToFormWindow(FormWindowBase *fw) const
{
   fw->setAuthor(author);
   fw->setPixmapFunction(pixFunction);

   if (layoutDefaultEnabled) {
      fw->setLayoutDefault(defaultMargin, defaultSpacing);
   } else {
      fw->setLayoutDefault(INT_MIN, INT_MIN);
   }

   if (layoutFunctionsEnabled) {
      fw->setLayoutFunction(marginFunction, spacingFunction);
   } else {
      fw->setLayoutFunction(QString(), QString());
   }

   fw->setIncludeHints(includeHints);

   const bool hadFormGrid = fw->hasFormGrid();
   fw->setHasFormGrid(hasFormGrid);
   if (hasFormGrid || hadFormGrid != hasFormGrid) {
      fw->setDesignerGrid(hasFormGrid ? grid : FormWindowBase::defaultDesignerGrid());
   }
}

// -------------------------- FormWindowSettings

FormWindowSettings::FormWindowSettings(QDesignerFormWindowInterface *parent) :
   QDialog(parent),
   m_ui(new ::Ui::FormWindowSettings),
   m_formWindow(dynamic_cast<FormWindowBase*>(parent)),
   m_oldData(new FormWindowData)
{
   Q_ASSERT(m_formWindow);

   m_ui->setupUi(this);
   m_ui->gridPanel->setCheckable(true);
   m_ui->gridPanel->setResetButtonVisible(false);

   setWindowFlags(windowFlags() & ~Qt::WindowContextHelpButtonHint);

   QString deviceProfileName = m_formWindow->deviceProfileName();
   if (deviceProfileName.isEmpty()) {
      deviceProfileName = tr("None");
   }

   m_ui->deviceProfileLabel->setText(tr("Device Profile: %1").formatArg(deviceProfileName));

   m_oldData->fromFormWindow(m_formWindow);
   setData(*m_oldData);
}

FormWindowSettings::~FormWindowSettings()
{
   delete m_oldData;
   delete m_ui;
}

FormWindowData FormWindowSettings::data() const
{
   FormWindowData rc;
   rc.author = m_ui->authorLineEdit->text();

   if (m_ui->pixmapFunctionGroupBox->isChecked()) {
      rc.pixFunction = m_ui->pixmapFunctionLineEdit->text();
   } else {
      rc.pixFunction.clear();
   }

   rc.layoutDefaultEnabled = m_ui->layoutDefaultGroupBox->isChecked();
   rc.defaultMargin  = m_ui->defaultMarginSpinBox->value();
   rc.defaultSpacing = m_ui->defaultSpacingSpinBox->value();

   rc.layoutFunctionsEnabled = m_ui->layoutFunctionGroupBox->isChecked();
   rc.marginFunction  = m_ui->marginFunctionLineEdit->text();
   rc.spacingFunction = m_ui->spacingFunctionLineEdit->text();

   const QString hints = m_ui->includeHintsTextEdit->toPlainText();

   if (!hints.isEmpty()) {
      rc.includeHints = hints.split(QString(QLatin1Char('\n')));

      // Purge out any lines consisting of blanks only
      QRegularExpression blankLine("^\\s*$", QPatternOption::ExactMatchOption);
      Q_ASSERT(blankLine.isValid());

      auto iter = rc.includeHints.begin();

      while (iter != rc.includeHints.end()) {
         auto match = blankLine.match(*iter);

         if (match.hasMatch()) {
            iter = rc.includeHints.erase(iter);
         } else {
            ++iter;
         }
      }

      rc.includeHints.removeAll(QString());
   }

   rc.hasFormGrid = m_ui->gridPanel->isChecked();
   rc.grid = m_ui->gridPanel->grid();

   return rc;
}

void FormWindowSettings::setData(const FormWindowData &data)
{
   m_ui->layoutDefaultGroupBox->setChecked(data.layoutDefaultEnabled);
   m_ui->defaultMarginSpinBox->setValue(data.defaultMargin);
   m_ui->defaultSpacingSpinBox->setValue(data.defaultSpacing);

   m_ui->layoutFunctionGroupBox->setChecked(data.layoutFunctionsEnabled);
   m_ui->marginFunctionLineEdit->setText(data.marginFunction);
   m_ui->spacingFunctionLineEdit->setText(data.spacingFunction);

   m_ui->pixmapFunctionLineEdit->setText(data.pixFunction);
   m_ui->pixmapFunctionGroupBox->setChecked(!data.pixFunction.isEmpty());

   m_ui->authorLineEdit->setText(data.author);

   if (data.includeHints.empty()) {
      m_ui->includeHintsTextEdit->clear();
   } else {
      m_ui->includeHintsTextEdit->setText(data.includeHints.join(QString("\n")));
   }

   m_ui->gridPanel->setChecked(data.hasFormGrid);
   m_ui->gridPanel->setGrid(data.grid);
}

void FormWindowSettings::accept()
{
   // Anything changed? -> Apply and set dirty
   const FormWindowData newData = data();

   if (newData != *m_oldData) {
      newData.applyToFormWindow(m_formWindow);
      m_formWindow->setDirty(true);
   }

   QDialog::accept();
}

}   // end namespace
