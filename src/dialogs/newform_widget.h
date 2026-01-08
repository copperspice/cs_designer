/***********************************************************************
*
* Copyright (c) 2021-2026 Barbara Geller
* Copyright (c) 2021-2026 Ansel Sermersheim
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

#ifndef NEWFORMWIDGET_H
#define NEWFORMWIDGET_H

#include <abstract_newformwidget.h>
#include <designer_workbench.h>

#include <deviceprofile_p.h>

#include <QMap>
#include <QPair>
#include <QPixmap>
#include <QStringList>
#include <QWidget>

class QIODevice;
class QTreeWidgetItem;

namespace qdesigner_internal {

namespace Ui {
class NewFormWidget;
}

class QDesignerWorkbench;

class NewFormWidget : public QDesignerNewFormWidgetInterface
{
   CS_OBJECT(NewFormWidget)
   Q_DISABLE_COPY(NewFormWidget)

 public:
   explicit NewFormWidget(QDesignerFormEditorInterface *core, QWidget *parentWidget);
   virtual ~NewFormWidget();

   bool hasCurrentTemplate() const override;
   QString currentTemplate(QString *errorMessage = nullptr) override;

   // Convenience for implementing file dialogs with preview
   static QImage grabForm(QDesignerFormEditorInterface *core, QIODevice &file,
      const QString &workingDir, const qdesigner_internal::DeviceProfile &dp);

 private:
   // slots
   void slot_treeWidget_itemActivated(QTreeWidgetItem *item);
   void slot_treeWidget_currentItemChanged(QTreeWidgetItem *current, QTreeWidgetItem *un_named_arg2);
   void slot_treeWidget_itemPressed(QTreeWidgetItem *item);

   CS_SLOT_1(Private, void slotDeviceProfileIndexChanged(int idx))
   CS_SLOT_2(slotDeviceProfileIndexChanged)

   QPixmap formPreviewPixmap(const QString &fileName) const;
   QPixmap formPreviewPixmap(QIODevice &file, const QString &workingDir = QString()) const;
   QPixmap formPreviewPixmap(const QTreeWidgetItem *item);

   void loadFrom(const QString &path, bool resourceFile, const QString &uiExtension,
      const QString &selectedItem, QTreeWidgetItem *&selectedItemFound);
   void loadFrom(const QString &title, const QStringList &nameList,
      const QString &selectedItem, QTreeWidgetItem *&selectedItemFound);

   QString itemToTemplate(const QTreeWidgetItem *item, QString *errorMessage) const;
   QString currentTemplateI(QString *ptrToErrorMessage);

   QSize templateSize() const;
   void setTemplateSize(const QSize &s);
   int profileComboIndex() const;
   qdesigner_internal::DeviceProfile currentDeviceProfile() const;
   bool showCurrentItemPixmap();

   // Pixmap cache (item, profile combo index)
   typedef QPair<const QTreeWidgetItem *, int> ItemPixmapCacheKey;
   typedef QMap<ItemPixmapCacheKey, QPixmap> ItemPixmapCache;
   ItemPixmapCache m_itemPixmapCache;

   QDesignerFormEditorInterface *m_core;
   Ui::NewFormWidget *m_ui;
   QTreeWidgetItem *m_currentItem;
   QTreeWidgetItem *m_acceptedItem;

   QList<qdesigner_internal::DeviceProfile> m_deviceProfiles;
};

}

#endif // NEWFORMWIDGET_H
