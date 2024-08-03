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
#include <abstract_propertyeditor.h>
#include <designer_formwindow.h>
#include <designer_propertysheet.h>
#include <designer_taskmenu.h>
#include <designer_workbench.h>
#include <extension.h>

#include <formwindowbase_p.h>

#include <QAction>
#include <QCloseEvent>
#include <QEvent>
#include <QFile>
#include <QFileDialog>
#include <QMessageBox>
#include <QPushButton>
#include <QUndoCommand>
#include <QVBoxLayout>
#include <QWindowStateChangeEvent>

QDesignerFormWindow::QDesignerFormWindow(QDesignerFormWindowInterface *editor, QDesignerWorkbench *workbench,
      QWidget *parent, Qt::WindowFlags flags)
   : QWidget(parent, flags), m_editor(editor), m_workbench(workbench), m_action(new QAction(this)),
     m_initialized(false), m_windowTitleInitialized(false)
{
   Q_ASSERT(workbench != nullptr);

   setMaximumSize(0xFFF, 0xFFF);

   QDesignerFormEditorInterface *core = workbench->core();

   if (m_editor) {
      m_editor->setParent(this);
   } else {
      m_editor = core->formWindowManager()->createFormWindow(this);
   }

   QVBoxLayout *boxLayout = new QVBoxLayout(this);
   boxLayout->setMargin(0);
   boxLayout->addWidget(m_editor);

   m_action->setCheckable(true);

   connect(m_editor->commandHistory(), &QUndoStack::indexChanged,
      this, &QDesignerFormWindow::updateChanged);

   connect(m_editor.data(), &QDesignerFormWindowInterface::geometryChanged,
      this, &QDesignerFormWindow::slotGeometryChanged);
}

QDesignerFormWindow::~QDesignerFormWindow()
{
   if (workbench()) {
      workbench()->removeFormWindow(this);
   }
}

QAction *QDesignerFormWindow::action() const
{
   return m_action;
}

void QDesignerFormWindow::changeEvent(QEvent *e)
{
   switch (e->type()) {
      case QEvent::WindowTitleChange:
         m_action->setText(windowTitle().remove(QString("[*]")));
         break;
      case QEvent::WindowIconChange:
         m_action->setIcon(windowIcon());
         break;
      case QEvent::WindowStateChange: {
         const  QWindowStateChangeEvent *wsce =  static_cast<const QWindowStateChangeEvent *>(e);
         const bool wasMinimized = Qt::WindowMinimized & wsce->oldState();
         const bool isMinimizedNow = isMinimized();
         if (wasMinimized != isMinimizedNow ) {
            emit minimizationStateChanged(m_editor, isMinimizedNow);
         }
      }
      break;
      default:
         break;
   }
   QWidget::changeEvent(e);
}

QRect QDesignerFormWindow::geometryHint() const
{
   const QPoint point(0, 0);
   // If we have a container, we want to be just as big.
   // QMdiSubWindow attempts to resize its children to sizeHint() when switching user interface modes.
   if (QWidget *mainContainer = m_editor->mainContainer()) {
      return QRect(point, mainContainer->size());
   }

   return QRect(point, sizeHint());
}

QDesignerFormWindowInterface *QDesignerFormWindow::editor() const
{
   return m_editor;
}

QDesignerWorkbench *QDesignerFormWindow::workbench() const
{
   return m_workbench;
}

void QDesignerFormWindow::firstShow()
{
   // Set up handling of file name changes and set initial title.
   if (!m_windowTitleInitialized) {
      m_windowTitleInitialized = true;
      if (m_editor) {
         connect(m_editor.data(), &QDesignerFormWindowInterface::fileNameChanged,
            this, &QDesignerFormWindow::updateWindowTitle);
         updateWindowTitle(m_editor->fileName());
         updateChanged();
      }
   }
   show();
}

int QDesignerFormWindow::getNumberOfUntitledWindows() const
{
   const int totalWindows = m_workbench->formWindowCount();
   if (! totalWindows) {
      return 0;
   }

   int maxUntitled = 0;

   // Find the number of untitled windows excluding ourselves.
   // Do not fall for 'untitled.ui', match with modified place holder.
   // This will cause some problems with i18n, but for now I need the string to be "static"

   QRegularExpression regex(QString("untitled( (\\d+))?\\[\\*\\]"));

   for (int i = 0; i < totalWindows; ++i) {
      QDesignerFormWindow *fw =  m_workbench->formWindow(i);

      if (fw != this) {
         const QString title = m_workbench->formWindow(i)->windowTitle();

         auto match = regex.match(title);

         if (match.hasMatch()) {
            if (maxUntitled == 0) {
               ++maxUntitled;
            }

            if (match.lastCapturedIndex() > 1) {
               const QString numberCapture = match.captured(2);

               if (! numberCapture.isEmpty()) {
                  maxUntitled = qMax(numberCapture.toInteger<int>(), maxUntitled);
               }
            }
         }
      }
   }

   return maxUntitled;
}

void QDesignerFormWindow::updateWindowTitle(const QString &fileName)
{
   if (!m_windowTitleInitialized) {
      m_windowTitleInitialized = true;
      if (m_editor)
         connect(m_editor.data(), &QDesignerFormWindowInterface::fileNameChanged,
            this, &QDesignerFormWindow::updateWindowTitle);
   }

   QString fileNameTitle;

   if (fileName.isEmpty()) {
      fileNameTitle = "untitled";

      if (const int maxUntitled = getNumberOfUntitledWindows()) {
         fileNameTitle += QChar(' ');
         fileNameTitle += QString::number(maxUntitled + 1);
      }

   } else {
      fileNameTitle = QFileInfo(fileName).fileName();
   }

   if (const QWidget *mc = m_editor->mainContainer()) {
      setWindowIcon(mc->windowIcon());

      QString winTitle = mc->windowTitle();

      if (winTitle.isEmpty()) {
         winTitle = "[Empty Window Title]";
      }

      setWindowTitle(tr("%1 - %2[*]").formatArgs(winTitle, fileNameTitle));

   } else {
      setWindowTitle(fileNameTitle);
   }
}

void QDesignerFormWindow::closeEvent(QCloseEvent *ev)
{
   if (m_editor->isDirty()) {
      raise();

      QMessageBox box(QMessageBox::Information, tr("Save Form?"),
         tr("Do you want to save the changes to this document before closing?"),
         QMessageBox::Discard | QMessageBox::Cancel | QMessageBox::Save, m_editor);

      box.setInformativeText(tr("If you do not save your changes will be lost."));
      box.setWindowModality(Qt::WindowModal);

      static_cast<QPushButton *>(box.button(QMessageBox::Save))->setDefault(true);

      switch (box.exec()) {
         case QMessageBox::Save: {
            bool ok = workbench()->saveForm(m_editor);
            ev->setAccepted(ok);
            m_editor->setDirty(!ok);
            break;
         }

         case QMessageBox::Discard:
            m_editor->setDirty(false); // Not really necessary, but stops problems if we get close again.
            ev->accept();
            break;
         case QMessageBox::Cancel:
            ev->ignore();
            break;
      }
   }
}

void QDesignerFormWindow::updateChanged()
{
   // Sometimes called after form window destruction.
   if (m_editor) {
      setWindowModified(m_editor->isDirty());
      updateWindowTitle(m_editor->fileName());
   }
}

void QDesignerFormWindow::resizeEvent(QResizeEvent *rev)
{
   if (m_initialized) {
      m_editor->setDirty(true);
      setWindowModified(true);
   }

   m_initialized = true;
   QWidget::resizeEvent(rev);
}

void QDesignerFormWindow::slotGeometryChanged()
{
   // If the form window changes, re-update the geometry of the current widget in the property editor.
   // Note that in the case of layouts, non-maincontainer widgets must also be updated,
   // so, do not do it for the main container only
   const QDesignerFormEditorInterface *core = m_editor->core();
   QObject *object = core->propertyEditor()->object();

   if (object == nullptr || !object->isWidgetType()) {
      return;
   }
   static const QString geometryProperty = QString("geometry");
   const QDesignerPropertySheetExtension *sheet = qt_extension<QDesignerPropertySheetExtension *>(core->extensionManager(), object);
   const int geometryIndex = sheet->indexOf(geometryProperty);

   if (geometryIndex == -1) {
      return;
   }

   core->propertyEditor()->setPropertyValue(geometryProperty, sheet->property(geometryIndex));
}

