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

#ifndef QDESIGNER_FORMWINDOW_H
#define QDESIGNER_FORMWINDOW_H

#include <QPointer>
#include <QWidget>

class QDesignerFormWindowInterface;
class QDesignerWorkbench;

class QDesignerFormWindow: public QWidget
{
   CS_OBJECT(QDesignerFormWindow)

 public:
   QDesignerFormWindow(QDesignerFormWindowInterface *formWindow, QDesignerWorkbench *workbench,
      QWidget *parent = nullptr, Qt::WindowFlags flags = Qt::EmptyFlag);

   void firstShow();

   virtual ~QDesignerFormWindow();

   QAction *action() const;
   QDesignerWorkbench *workbench() const;
   QDesignerFormWindowInterface *editor() const;

   QRect geometryHint() const;

   CS_SLOT_1(Public, void updateChanged())
   CS_SLOT_2(updateChanged)

 private:
   CS_SLOT_1(Private, void updateWindowTitle(const QString &fileName))
   CS_SLOT_2(updateWindowTitle)
   CS_SLOT_1(Private, void slotGeometryChanged())
   CS_SLOT_2(slotGeometryChanged)

 public:
   CS_SIGNAL_1(Public, void minimizationStateChanged(QDesignerFormWindowInterface *formWindow, bool minimized))
   CS_SIGNAL_2(minimizationStateChanged, formWindow, minimized)
   CS_SIGNAL_1(Public, void triggerAction())
   CS_SIGNAL_2(triggerAction)

 protected:
   void changeEvent(QEvent *e) override;
   void closeEvent(QCloseEvent *ev) override;
   void resizeEvent(QResizeEvent *rev) override;

 private:
   int getNumberOfUntitledWindows() const;
   QPointer<QDesignerFormWindowInterface> m_editor;
   QPointer<QDesignerWorkbench> m_workbench;
   QAction *m_action;
   bool m_initialized;
   bool m_windowTitleInitialized;
};



#endif // QDESIGNER_FORMWINDOW_H
