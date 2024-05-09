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

#ifndef CONNECTDIALOG_H
#define CONNECTDIALOG_H

#include <ui_signal_connect_dialog.h>

#include <QDialog>

class QDesignerFormWindowInterface;

class QPushButton;

namespace qdesigner_internal {

class ConnectDialog : public QDialog
{
   CS_OBJECT(ConnectDialog)

 public:
   ConnectDialog(QDesignerFormWindowInterface *formWindow, QWidget *sender, QWidget *receiver, QWidget *parent = nullptr);

   QString signal() const;
   QString slot() const;

   void setSignalSlot(const QString &signal, const QString &slot);

   bool showAllSignalsSlots() const;
   void setShowAllSignalsSlots(bool showIt);

 private:
   CS_SLOT_1(Private, void populateLists())
   CS_SLOT_2(populateLists)
   CS_SLOT_1(Private, void selectSignal(QListWidgetItem *item))
   CS_SLOT_2(selectSignal)
   CS_SLOT_1(Private, void selectSlot(QListWidgetItem *item))
   CS_SLOT_2(selectSlot)
   CS_SLOT_1(Private, void populateSignalList())
   CS_SLOT_2(populateSignalList)
   CS_SLOT_1(Private, void populateSlotList(const QString &signal = QString()))
   CS_SLOT_2(populateSlotList)
   CS_SLOT_1(Private, void editSignals())
   CS_SLOT_2(editSignals)
   CS_SLOT_1(Private, void editSlots())
   CS_SLOT_2(editSlots)

   enum WidgetMode { NormalWidget, MainContainer, PromotedWidget };

   static WidgetMode widgetMode(QWidget *w,  QDesignerFormWindowInterface *formWindow);
   QPushButton *okButton();
   void setOkButtonEnabled(bool);
   void editSignalsSlots(QWidget *w, WidgetMode mode, int signalSlotDialogMode);

   QWidget *m_source;
   QWidget *m_destination;
   const WidgetMode m_sourceMode;
   const WidgetMode m_destinationMode;
   QDesignerFormWindowInterface *m_formWindow;
   Ui::ConnectDialog m_ui;
};

}   // end namespace

#endif
