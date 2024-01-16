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

#ifndef PREVIEW_MANAGER_H
#define PREVIEW_MANAGER_H

#include <abstract_settings.h>
#include <abstract_formwindow.h>

#include <QObject>
#include <QString>
#include <QSharedDataPointer>

class QWidget;
class QPixmap;
class QAction;
class QActionGroup;
class QMenu;
class QWidget;

namespace qdesigner_internal {

class PreviewConfigurationData;

class PreviewConfiguration
{
 public:
   PreviewConfiguration();
   explicit PreviewConfiguration(const QString &style,
      const QString &applicationStyleSheet = QString(), const QString &deviceSkin = QString());

   PreviewConfiguration(const PreviewConfiguration &);
   PreviewConfiguration &operator=(const PreviewConfiguration &);
   ~PreviewConfiguration();

   QString style() const;
   void setStyle(const QString &);

   // Style sheet to prepend (to simulate the effect od QApplication::setSyleSheet()).
   QString applicationStyleSheet() const;
   void setApplicationStyleSheet(const QString &);

   QString deviceSkin() const;
   void setDeviceSkin(const QString &);

   void clear();
   void toSettings(const QString &prefix, QDesignerSettingsInterface *settings) const;
   void fromSettings(const QString &prefix, const QDesignerSettingsInterface *settings);

 private:
   QSharedDataPointer<PreviewConfigurationData> m_d;
};

bool operator<(const PreviewConfiguration &pc1, const PreviewConfiguration &pc2);
bool operator==(const PreviewConfiguration &pc1, const PreviewConfiguration &pc2);
bool operator!=(const PreviewConfiguration &pc1, const PreviewConfiguration &pc2);

// ----------- Preview window manager.
// Maintains a list of preview widgets with their associated form windows and configuration.

class PreviewManagerPrivate;

class PreviewManager : public QObject
{
   CS_OBJECT(PreviewManager)

 public:
   enum PreviewMode {
      // Modal preview. Do not use on Macs as dialogs would have no close button
      ApplicationModalPreview,
      // Non modal previewing of one form in different configurations (closes if form window changes)
      SingleFormNonModalPreview,
      // Non modal previewing of several forms in different configurations
      MultipleFormNonModalPreview
   };

   explicit PreviewManager(PreviewMode mode, QObject *parent);
   virtual ~PreviewManager();

   // Show preview. Raise existing preview window if there is one with a matching
   // configuration, else create a new preview.
   QWidget *showPreview(const QDesignerFormWindowInterface *, const PreviewConfiguration &pc, int deviceProfileIndex /*=-1*/,
      QString *errorMessage);
   // Convenience that creates a preview using a configuration taken from the settings.
   QWidget *showPreview(const QDesignerFormWindowInterface *, const QString &style, int deviceProfileIndex /*=-1*/,
      QString *errorMessage);
   QWidget *showPreview(const QDesignerFormWindowInterface *, const QString &style, QString *errorMessage);

   int previewCount() const;

   // Create a pixmap for printing.
   QPixmap createPreviewPixmap(const QDesignerFormWindowInterface *fw, const PreviewConfiguration &pc, int deviceProfileIndex /*=-1*/,
      QString *errorMessage);

   // Convenience that creates a pixmap using a configuration taken from the settings.
   QPixmap createPreviewPixmap(const QDesignerFormWindowInterface *fw, const QString &style, int deviceProfileIndex /*=-1*/,
      QString *errorMessage);
   QPixmap createPreviewPixmap(const QDesignerFormWindowInterface *fw, const QString &style, QString *errorMessage);

   bool eventFilter(QObject *watched, QEvent *event) override;

   CS_SLOT_1(Public, void closeAllPreviews())
   CS_SLOT_2(closeAllPreviews)

   CS_SIGNAL_1(Public, void firstPreviewOpened())
   CS_SIGNAL_2(firstPreviewOpened)

   CS_SIGNAL_1(Public, void lastPreviewClosed())
   CS_SIGNAL_2(lastPreviewClosed)

 private:
   CS_SLOT_1(Private, void slotZoomChanged(int un_named_arg1))
   CS_SLOT_2(slotZoomChanged)

   virtual Qt::WindowFlags previewWindowFlags(const QWidget *widget) const;
   virtual QWidget *createDeviceSkinContainer(const QDesignerFormWindowInterface *) const;

   QWidget *raise(const QDesignerFormWindowInterface *, const PreviewConfiguration &pc);
   QWidget *createPreview(const QDesignerFormWindowInterface *,
      const PreviewConfiguration &pc,
      int deviceProfileIndex /* = -1 */,
      QString *errorMessage,
      /*Disabled by default, <0 */
      int initialZoom = -1);

   void updatePreviewClosed(QWidget *w);

   PreviewManagerPrivate *d;

   PreviewManager(const PreviewManager &other);
   PreviewManager &operator =(const PreviewManager &other);
};

}   // end namespace

#endif
