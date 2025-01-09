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

#ifndef PROMOTIONMODEL_H
#define PROMOTIONMODEL_H

#include <QSet>
#include <QStandardItemModel>

class QDesignerFormEditorInterface;
class QDesignerWidgetDataBaseItemInterface;

namespace qdesigner_internal {

// Item model representing the promoted widgets.
class PromotionModel : public QStandardItemModel
{
   CS_OBJECT(PromotionModel)

 public:
   explicit PromotionModel(QDesignerFormEditorInterface *core);

   void updateFromWidgetDatabase();

   // Return item at model index or 0.
   QDesignerWidgetDataBaseItemInterface *databaseItemAt(const QModelIndex &, bool *referenced) const;

   QModelIndex indexOfClass(const QString &className) const;

   CS_SIGNAL_1(Public, void includeFileChanged(QDesignerWidgetDataBaseItemInterface *un_named_arg1, const QString &includeFile))
   CS_SIGNAL_2(includeFileChanged, un_named_arg1, includeFile)
   CS_SIGNAL_1(Public, void classNameChanged(QDesignerWidgetDataBaseItemInterface *un_named_arg1, const QString &newName))
   CS_SIGNAL_2(classNameChanged, un_named_arg1, newName)

 private:
   CS_SLOT_1(Private, void slotItemChanged(QStandardItem *item))
   CS_SLOT_2(slotItemChanged)

   void initializeHeaders();
   // Retrieve data base item of item or return 0.
   QDesignerWidgetDataBaseItemInterface *databaseItem(const QStandardItem *item, bool *referenced) const;

   QDesignerFormEditorInterface *m_core;
};

} // end namespace - qdesigner_internal

#endif // PROMOTIONMODEL_H
