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

#ifndef FORMWINDOWBASE_H
#define FORMWINDOWBASE_H

#include <abstract_formwindow.h>

#include <QList>
#include <QVariant>

class QDesignerDnDItemInterface;
class QtResourceSet;
class QDesignerPropertySheet;

class QMenu;

namespace qdesigner_internal {

class DesignerIconCache;
class DesignerPixmapCache;
class DeviceProfile;
class FormWindowBasePrivate;
class Grid;
class QEditorFormBuilder;

class FormWindowBase: public QDesignerFormWindowInterface
{
   CS_OBJECT(FormWindowBase)

 public:
   enum HighlightMode  { Restore, Highlight };

   explicit FormWindowBase(QDesignerFormEditorInterface *core, QWidget *parent = nullptr,
         Qt::WindowFlags flags = Qt::EmptyFlag);

   virtual ~FormWindowBase();

   QVariantMap formData();
   void setFormData(const QVariantMap &vm);

   QStringList checkContents() const override;

   // Deprecated
   QPoint grid() const override;

   // Deprecated
   void setGrid(const QPoint &grid) override;

   bool hasFeature(Feature f) const override;
   Feature features() const override;
   void setFeatures(Feature f) override;

   const Grid &designerGrid() const;
   void setDesignerGrid(const  Grid &grid);

   bool hasFormGrid() const;
   void setHasFormGrid(bool b);

   bool gridVisible() const;

   ResourceFileSaveMode resourceFileSaveMode() const override;
   void setResourceFileSaveMode(ResourceFileSaveMode behavior) override;

   static const Grid &defaultDesignerGrid();
   static void setDefaultDesignerGrid(const Grid &grid);

   // Overwrite to initialize and return a full popup menu for a managed widget
   virtual QMenu *initializePopupMenu(QWidget *managedWidget);
   // Helper to create a basic popup menu from task menu extensions (internal/public)
   static QMenu *createExtensionTaskMenu(QDesignerFormWindowInterface *fw, QObject *o, bool trailingSeparator = true);

   virtual bool dropWidgets(const QList<QDesignerDnDItemInterface *> &item_list, QWidget *target,
      const QPoint &global_mouse_pos) = 0;

   // Helper to find the widget at the mouse position with some flags.
   enum WidgetUnderMouseMode { FindSingleSelectionDropTarget, FindMultiSelectionDropTarget };
   QWidget *widgetUnderMouse(const QPoint &formPos, WidgetUnderMouseMode m);

   virtual QWidget *widgetAt(const QPoint &pos) = 0;
   virtual QWidget *findContainer(QWidget *w, bool excludeLayout) const = 0;

   void deleteWidgetList(const QWidgetList &widget_list);

   virtual void highlightWidget(QWidget *w, const QPoint &pos, HighlightMode mode = Highlight) = 0;

   enum PasteMode { PasteAll, PasteActionsOnly };

   virtual void paste(PasteMode pasteMode) = 0;

   // Factory method to create a form builder
   virtual QEditorFormBuilder *createFormBuilder() = 0;

   virtual bool blockSelectionChanged(bool blocked) = 0;

   DesignerPixmapCache *pixmapCache() const;
   DesignerIconCache *iconCache() const;
   QtResourceSet *resourceSet() const override;
   void setResourceSet(QtResourceSet *resourceSet) override;
   void addReloadableProperty(QDesignerPropertySheet *sheet, int index);
   void removeReloadableProperty(QDesignerPropertySheet *sheet, int index);
   void addReloadablePropertySheet(QDesignerPropertySheet *sheet, QObject *object);
   void removeReloadablePropertySheet(QDesignerPropertySheet *sheet);
   void reloadProperties();

   void emitWidgetRemoved(QWidget *w);
   void emitObjectRemoved(QObject *o);

   DeviceProfile deviceProfile() const;
   QString styleName() const;
   QString deviceProfileName() const;

   enum LineTerminatorMode {
      LFLineTerminator,
      CRLFLineTerminator,
      NativeLineTerminator =
#if defined (Q_OS_WIN)
         CRLFLineTerminator
#else
         LFLineTerminator
#endif
   };

   void setLineTerminatorMode(LineTerminatorMode mode);
   LineTerminatorMode lineTerminatorMode() const;

   CS_SLOT_1(Public, void resourceSetActivated(QtResourceSet *resourceSet, bool resourceSetChanged))
   CS_SLOT_2(resourceSetActivated)

 private:
   CS_SLOT_1(Private, void triggerDefaultAction(QWidget *w))
   CS_SLOT_2(triggerDefaultAction)

   void syncGridFeature();

   FormWindowBasePrivate *m_d;
};

}  // namespace qdesigner_internal

#endif
