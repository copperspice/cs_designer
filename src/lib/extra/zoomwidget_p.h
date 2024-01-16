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

#ifndef ZOOMWIDGET_H
#define ZOOMWIDGET_H

#include <QGraphicsView>
#include <QGraphicsProxyWidget>
#include <QList>

class QGraphicsScene;
class QMenu;
class QAction;
class QActionGroup;

namespace qdesigner_internal {

// A checkable zoom menu action group. Operates in percent.

class ZoomMenu : public QObject
{
   CS_OBJECT(ZoomMenu)
   Q_DISABLE_COPY(ZoomMenu)

 public:
   ZoomMenu(QObject *parent = nullptr);
   void addActions(QMenu *m);

   int zoom() const;

   // Return a list of available zoom values.
   static QList<int> zoomValues();

   CS_SLOT_1(Public, void setZoom(int percent))
   CS_SLOT_2(setZoom)

   CS_SIGNAL_1(Public, void zoomChanged(int un_named_arg1))
   CS_SIGNAL_2(zoomChanged, un_named_arg1)

 private:
   CS_SLOT_1(Private, void slotZoomMenu(QAction *un_named_arg1))
   CS_SLOT_2(slotZoomMenu)

   static int zoomOf(const QAction *a);

   QActionGroup *m_menuActions;
};

/* Zoom view: A QGraphicsView with a zoom menu */

class ZoomView : public QGraphicsView
{
   CS_OBJECT(ZoomView)

   CS_PROPERTY_READ(zoom, zoom)
   CS_PROPERTY_WRITE(zoom, setZoom)
   CS_PROPERTY_DESIGNABLE(zoom, true)
   CS_PROPERTY_SCRIPTABLE(zoom, true)
   CS_PROPERTY_READ(zoomContextMenuEnabled, isZoomContextMenuEnabled)
   CS_PROPERTY_WRITE(zoomContextMenuEnabled, setZoomContextMenuEnabled)
   CS_PROPERTY_DESIGNABLE(zoomContextMenuEnabled, true)
   CS_PROPERTY_SCRIPTABLE(zoomContextMenuEnabled, true)

   Q_DISABLE_COPY(ZoomView)

 public:
   ZoomView(QWidget *parent = nullptr);

   /*  Zoom in percent (for easily implementing menus) and qreal zoomFactor
    * in sync */
   int zoom() const; // in percent
   qreal zoomFactor() const;

   // Zoom Menu on QGraphicsView.
   bool isZoomContextMenuEnabled() const;
   void setZoomContextMenuEnabled(bool e);

   QGraphicsScene &scene() {
      return *m_scene;
   }
   const QGraphicsScene &scene() const {
      return *m_scene;
   }

   // Helpers for menu
   ZoomMenu *zoomMenu();

   QPoint scrollPosition() const;
   void setScrollPosition(const QPoint &pos);
   void scrollToOrigin();

   CS_SLOT_1(Public, void setZoom(int percent))
   CS_SLOT_2(setZoom)

   CS_SLOT_1(Public, void showContextMenu(const QPoint &globalPos))
   CS_SLOT_2(showContextMenu)

 protected:
   void contextMenuEvent(QContextMenuEvent *event) override;

   // Overwrite for implementing additional behaviour when doing setZoom();
   virtual void applyZoom();

 private:
   QGraphicsScene *m_scene;
   int m_zoom;
   qreal m_zoomFactor;

   bool m_zoomContextMenuEnabled;
   bool m_resizeBlocked;
   ZoomMenu *m_zoomMenu;
};

/* The proxy widget used in  ZoomWidget. It  refuses to move away from 0,0,
 * This behaviour is required for Windows only. */

class  ZoomProxyWidget : public QGraphicsProxyWidget
{
   Q_DISABLE_COPY(ZoomProxyWidget)

 public:
   explicit ZoomProxyWidget(QGraphicsItem *parent = nullptr, Qt::WindowFlags wFlags = Qt::EmptyFlag);

 protected:
   QVariant itemChange(GraphicsItemChange change, const QVariant &value) override;
};

/* Zoom widget: A QGraphicsView-based container for a widget that allows for
 * zooming it. Communicates changes in size in the following ways:
 * 1) Embedded widget wants to resize: Listen for its resize in event filter
 *    and resize
 * 2) Zoom is changed: resize to fully display the embedded widget
 * 3) Outer widget changes (by manually resizing the window:
 *    Pass the scaled change on to the embedded widget.
 * Provides helper functions for a zoom context menu on the widget. */

class ZoomWidget : public ZoomView
{
   CS_OBJECT(ZoomWidget)

   CS_PROPERTY_READ(widgetZoomContextMenuEnabled, isWidgetZoomContextMenuEnabled)
   CS_PROPERTY_WRITE(widgetZoomContextMenuEnabled, setWidgetZoomContextMenuEnabled)
   CS_PROPERTY_DESIGNABLE(widgetZoomContextMenuEnabled, true)
   CS_PROPERTY_SCRIPTABLE(widgetZoomContextMenuEnabled, true)

   CS_PROPERTY_READ(itemAcceptDrops, itemAcceptDrops)
   CS_PROPERTY_WRITE(itemAcceptDrops, setItemAcceptDrops)
   CS_PROPERTY_DESIGNABLE(itemAcceptDrops, true)
   CS_PROPERTY_SCRIPTABLE(itemAcceptDrops, true)

   Q_DISABLE_COPY(ZoomWidget)

 public:
   ZoomWidget(QWidget *parent = nullptr);
   void setWidget(QWidget *w, Qt::WindowFlags wFlags = Qt::EmptyFlag);

   const QGraphicsProxyWidget *proxy() const {
      return m_proxy;
   }
   QGraphicsProxyWidget *proxy() {
      return m_proxy;
   }

   /* Enable the zoom Menu on the Widget (as opposed ZoomView's menu which
    * is on the canvas). */
   bool isWidgetZoomContextMenuEnabled() const;
   void setWidgetZoomContextMenuEnabled(bool e);

   void setItemAcceptDrops(bool);
   bool itemAcceptDrops() const;

   QSize minimumSizeHint() const override;
   QSize sizeHint() const override;

   bool zoomedEventFilter(QObject *watched, QEvent *event);

   // debug state
   CS_SLOT_1(Public, void dump() const)
   CS_SLOT_2(dump)

 protected:
   void resizeEvent(QResizeEvent *event) override;

   // Overwritten from ZoomView
   void applyZoom() override;
   // Overwrite to actually perform a resize. This is required if we are in a layout. Default does resize().
   virtual void doResize(const QSize &s);

 private:
   // Factory function for QGraphicsProxyWidgets which can be overwritten. Default creates a ZoomProxyWidget
   virtual QGraphicsProxyWidget *createProxyWidget(QGraphicsItem *parent = nullptr, Qt::WindowFlags wFlags = Qt::EmptyFlag) const;
   QSize widgetSizeToViewSize(const QSize &s, bool *ptrToValid = nullptr) const;

   void resizeToWidgetSize();
   QSize viewPortMargin() const;
   QSize widgetSize() const;
   QSizeF widgetDecorationSizeF() const;

   QGraphicsProxyWidget *m_proxy;
   bool m_viewResizeBlocked;
   bool m_widgetResizeBlocked;
   bool m_widgetZoomContextMenuEnabled;
};

} // namespace qdesigner_internal

#endif
