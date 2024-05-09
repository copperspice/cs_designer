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

#ifndef FORMWINDOW_H
#define FORMWINDOW_H

#include <undostack.h>

#include <formwindowbase_p.h>

#include <QHash>
#include <QList>
#include <QMap>
#include <QPointer>
#include <QSet>

#include <qalgorithms.h>

class QDesignerDnDItemInterface;
class QDesignerTaskMenuExtension;
class DomConnections;
class DomUI;

class QAction;
class QAction;
class QLabel;
class QMenu;
class QRubberBand;
class QTimer;
class QWidget;

namespace qdesigner_internal {

class FormEditor;
class FormWindowCursor;
class FormWindowDnDItem;
class FormWindowManager;
class FormWindowWidgetStack;
class SetPropertyCommand;
class WidgetEditorTool;

class FormWindow: public FormWindowBase
{
   CS_OBJECT(FormWindow)

 public:
   explicit FormWindow(FormEditor *core, QWidget *parent = nullptr, Qt::WindowFlags flags = Qt::EmptyFlag);
   virtual ~FormWindow();

   QDesignerFormEditorInterface *core() const override;

   QDesignerFormWindowCursorInterface *cursor() const override;

   // Overwritten: FormWindowBase
   QWidget *formContainer() const override;

   int toolCount() const override;
   int currentTool() const override;
   void setCurrentTool(int index) override;
   QDesignerFormWindowToolInterface *tool(int index) const override;
   void registerTool(QDesignerFormWindowToolInterface *tool) override;

   QString author() const override;
   void setAuthor(const QString &author) override;

   QString comment() const override;
   void setComment(const QString &comment) override;

   void layoutDefault(int *margin, int *spacing) override;
   void setLayoutDefault(int margin, int spacing) override;

   void layoutFunction(QString *margin, QString *spacing) override;
   void setLayoutFunction(const QString &margin, const QString &spacing) override;

   QString pixmapFunction() const override;
   void setPixmapFunction(const QString &pixmapFunction) override;

   QString exportMacro() const override;
   void setExportMacro(const QString &exportMacro) override;

   QStringList includeHints() const override;
   void setIncludeHints(const QStringList &includeHints) override;

   QString fileName() const override;
   void setFileName(const QString &fileName) override;

   QString contents() const override;
   bool setContents(QIODevice *dev, QString *errorMessage = nullptr) override;
   bool setContents(const QString &) override;

   QDir absoluteDir() const override;

   void simplifySelection(QWidgetList *sel) const override;

   void ensureUniqueObjectName(QObject *object) override;

   QWidget *mainContainer() const override;
   void setMainContainer(QWidget *mainContainer) override;
   bool isMainContainer(const QWidget *w) const;

   QWidget *currentWidget() const;

   bool hasInsertedChildren(QWidget *w) const;

   QList<QWidget *> selectedWidgets() const;
   void clearSelection(bool changePropertyDisplay = true) override;
   bool isWidgetSelected(QWidget *w) const;
   void selectWidget(QWidget *w, bool select = true) override;

   void selectWidgets();
   void repaintSelection();
   void updateSelection(QWidget *w);
   void updateChildSelections(QWidget *w);
   void raiseChildSelections(QWidget *w);
   void raiseSelection(QWidget *w);

   inline const QList<QWidget *> &widgets() const {
      return m_widgets;
   }
   inline int widgetCount() const {
      return m_widgets.count();
   }
   inline QWidget *widgetAt(int index) const {
      return m_widgets.at(index);
   }

   QList<QWidget *> widgets(QWidget *widget) const;

   QWidget *createWidget(DomUI *ui, const QRect &rect, QWidget *target);

   bool isManaged(QWidget *w) const override;

   void manageWidget(QWidget *w) override;
   void unmanageWidget(QWidget *w) override;

   QUndoStack *commandHistory() const override;
   void beginCommand(const QString &description) override;
   void endCommand() override;

   bool blockSelectionChanged(bool blocked) override;
   void emitSelectionChanged() override;

   bool unify(QObject *w, QString &s, bool changeIt);

   bool isDirty() const override;
   void setDirty(bool dirty) override;

   static FormWindow *findFormWindow(QWidget *w);

   virtual QWidget *containerAt(const QPoint &pos);
   QWidget *widgetAt(const QPoint &pos) override;
   void highlightWidget(QWidget *w, const QPoint &pos, HighlightMode mode = Highlight) override;

   void updateOrderIndicators();

   bool handleEvent(QWidget *widget, QWidget *managedWidget, QEvent *event);

   QStringList resourceFiles() const override;
   void addResourceFile(const QString &path) override;
   void removeResourceFile(const QString &path) override;

   void resizeWidget(QWidget *widget, const QRect &geometry);

   bool dropDockWidget(QDesignerDnDItemInterface *item, const QPoint &global_mouse_pos);
   bool dropWidgets(const QList<QDesignerDnDItemInterface *> &item_list, QWidget *target,
      const QPoint &global_mouse_pos) override;

   QWidget *findContainer(QWidget *w, bool excludeLayout) const override;

   // for WidgetSelection only.
   QWidget *designerWidget(QWidget *w) const;

   //  Initialize and return a popup menu for a managed widget
   QMenu *initializePopupMenu(QWidget *managedWidget) override;

   void paste(PasteMode pasteMode) override;

   QEditorFormBuilder *createFormBuilder() override;

   bool eventFilter(QObject *watched, QEvent *event) override;

   CS_SIGNAL_1(Public, void contextMenuRequested(QMenu *menu, QWidget *widget))
   CS_SIGNAL_2(contextMenuRequested, menu, widget)

   CS_SLOT_1(Public, void deleteWidgets())
   CS_SLOT_2(deleteWidgets)

   CS_SLOT_1(Public, void raiseWidgets())
   CS_SLOT_2(raiseWidgets)

   CS_SLOT_1(Public, void lowerWidgets())
   CS_SLOT_2(lowerWidgets)

   CS_SLOT_1(Public, void copy())
   CS_SLOT_2(copy)

   CS_SLOT_1(Public, void cut())
   CS_SLOT_2(cut)

   CS_SLOT_1(Public, void paste())
   CS_SLOT_OVERLOAD(paste, ())

   CS_SLOT_1(Public, void selectAll())
   CS_SLOT_2(selectAll)

   CS_SLOT_1(Public, void createLayout(int type, QWidget *container = nullptr))
   CS_SLOT_2(createLayout)
   CS_SLOT_1(Public, void morphLayout(QWidget *container, int newType))
   CS_SLOT_2(morphLayout)
   CS_SLOT_1(Public, void breakLayout(QWidget *w))
   CS_SLOT_2(breakLayout)

   CS_SLOT_1(Public, void editContents())
   CS_SLOT_2(editContents)

 protected:
   virtual QMenu * createPopupMenu(QWidget *w);
   void resizeEvent(QResizeEvent *e) override;

   void insertWidget(QWidget *w, const QRect &rect, QWidget *target, bool already_in_form = false);

 private:
   CS_SLOT_1(Private, void selectionChangedTimerDone())
   CS_SLOT_2(selectionChangedTimerDone)

   CS_SLOT_1(Private, void checkSelection())
   CS_SLOT_2(checkSelection)

   CS_SLOT_1(Private, void checkSelectionNow())
   CS_SLOT_2(checkSelectionNow)

   CS_SLOT_1(Private, void slotSelectWidget(QAction *un_named_arg1))
   CS_SLOT_2(slotSelectWidget)

   enum MouseState {
      NoMouseState,
      // Double click received
      MouseDoubleClicked,
      // Drawing selection rubber band rectangle
      MouseDrawRubber,
      // Started a move operation
      MouseMoveDrag,
      // Click on a widget whose parent is selected. Defer selection to release
      MouseDeferredSelection
   };

   MouseState m_mouseState;
   QPointer<QWidget> m_lastClickedWidget;

   void init();
   void initializeCoreTools();

   int getValue(const QRect &rect, int key, bool size) const;
   int calcValue(int val, bool forward, bool snap, int snapOffset) const;
   void handleClickSelection(QWidget *managedWidget, unsigned mouseFlags);

   bool frameNeeded(QWidget *w) const;

   enum RectType { Insert, Rubber };

   void startRectDraw(const QPoint &global, QWidget *, RectType t);
   void continueRectDraw(const QPoint &global, QWidget *, RectType t);
   void endRectDraw();

   QWidget *containerAt(const QPoint &pos, QWidget *notParentOf);

   void checkPreviewGeometry(QRect &r);

   bool handleContextMenu(QWidget *widget, QWidget *managedWidget, QContextMenuEvent *e);
   bool handleMouseButtonDblClickEvent(QWidget *widget, QWidget *managedWidget, QMouseEvent *e);
   bool handleMousePressEvent(QWidget *widget, QWidget *managedWidget, QMouseEvent *e);
   bool handleMouseMoveEvent(QWidget *widget, QWidget *managedWidget, QMouseEvent *e);
   bool handleMouseReleaseEvent(QWidget *widget, QWidget *managedWidget, QMouseEvent *e);
   bool handleKeyPressEvent(QWidget *widget, QWidget *managedWidget, QKeyEvent *e);
   bool handleKeyReleaseEvent(QWidget *widget, QWidget *managedWidget, QKeyEvent *e);

   bool isCentralWidget(QWidget *w) const;

   bool setCurrentWidget(QWidget *currentWidget);
   bool trySelectWidget(QWidget *w, bool select);

   void dragWidgetWithinForm(QWidget *widget, const QRect &targetGeometry, QWidget *targetContainer);

   void setCursorToAll(const QCursor &c, QWidget *start);

   QPoint mapToForm(const QWidget *w, const QPoint &pos) const;
   bool canBeBuddy(QWidget *w) const;

   QWidget *findTargetContainer(QWidget *widget) const;

   void clearMainContainer();

   static int widgetDepth(const QWidget *w);
   static bool isChildOf(const QWidget *c, const QWidget *p);

   void editWidgets() override;

   void updateWidgets();

   void handleArrowKeyEvent(int key, Qt::KeyboardModifiers modifiers);

   void layoutSelection(int type);
   void layoutContainer(QWidget *w, int type);

   QWidget *innerContainer(QWidget *outerContainer) const;
   QWidget *containerForPaste() const;
   QAction *createSelectAncestorSubMenu(QWidget *w);
   void selectSingleWidget(QWidget *w);

   FormEditor *m_core;
   FormWindowCursor *m_cursor;
   QWidget *m_mainContainer;
   QWidget *m_currentWidget;

   bool m_blockSelectionChanged;

   QPoint m_rectAnchor;
   QRect m_currRect;

   QWidgetList m_widgets;
   QSet<QWidget *> m_insertedWidgets;

   class Selection;
   Selection *m_selection;

   QPoint m_startPos;

   QDesignerUndoStack m_undoStack;

   QString m_fileName;

   typedef QPair<QPalette, bool> PaletteAndFill;
   typedef QMap<QWidget *, PaletteAndFill> WidgetPaletteMap;
   WidgetPaletteMap m_palettesBeforeHighlight;

   QRubberBand *m_rubberBand;

   QTimer *m_selectionChangedTimer;
   QTimer *m_checkSelectionTimer;
   QTimer *m_geometryChangedTimer;

   FormWindowWidgetStack *m_widgetStack;
   WidgetEditorTool *m_widgetEditor;

   QStringList m_resourceFiles;

   QString m_comment;
   QString m_author;
   QString m_pixmapFunction;
   int m_defaultMargin, m_defaultSpacing;
   QString m_marginFunction, m_spacingFunction;
   QString m_exportMacro;
   QStringList m_includeHints;

   QPoint m_contextMenuPosition;

   friend class WidgetEditorTool;
};

}  // namespace qdesigner_internal

#endif
