/***********************************************************************
*
* Copyright (c) 2021-2023 Barbara Geller
* Copyright (c) 2021-2023 Ansel Sermersheim
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

#ifndef ORDERDIALOG_P_H
#define ORDERDIALOG_P_H

#include <QDialog>
#include <QMap>

class QDesignerFormEditorInterface;

namespace qdesigner_internal {

namespace Ui {
class OrderDialog;
}

class OrderDialog: public QDialog
{
   CS_OBJECT(OrderDialog)

 public:
   OrderDialog(QWidget *parent);
   virtual ~OrderDialog();

   static QWidgetList pagesOfContainer(const QDesignerFormEditorInterface *core, QWidget *container);

   void setPageList(const QWidgetList &pages);
   QWidgetList pageList() const;

   void setDescription(const QString &d);

   enum Format {        // Display format
      PageOrderFormat, // Container pages, ranging 0..[n-1]
      TabOrderFormat   // List of widgets,  ranging 1..1
   };

   void setFormat(Format f)  {
      m_format = f;
   }
   Format format() const     {
      return m_format;
   }

 private:
   void slot_upButton_clicked();
   void slot_downButton_clicked();
   void slot_pageList_currentRowChanged(int row);

   CS_SLOT_1(Private, void slotEnableButtonsAfterDnD())
   CS_SLOT_2(slotEnableButtonsAfterDnD)

   CS_SLOT_1(Private, void slotReset())
   CS_SLOT_2(slotReset)

   void buildList();
   void enableButtons(int r);

   typedef QMap<int, QWidget *> OrderMap;
   OrderMap m_orderMap;
   Ui::OrderDialog *m_ui;
   Format m_format;
};

}  // namespace qdesigner_internal

#endif
