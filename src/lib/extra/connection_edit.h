/***********************************************************************
*
* Copyright (c) 2021-2022 Barbara Geller
* Copyright (c) 2021-2022 Ansel Sermersheim
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

#ifndef CONNECTIONEDIT_H
#define CONNECTIONEDIT_H

class QDesignerFormWindowInterface;

#include <QList>
#include <QMultiMap>
#include <QPixmap>
#include <QPointer>
#include <QPolygonF>
#include <QUndoCommand>
#include <QWidget>

class QUndoStack;
class QMenu;

namespace qdesigner_internal {

class Connection;
class ConnectionEdit;

class CETypes
{
 public:
   typedef QList<Connection *> ConnectionList;
   typedef QMap<Connection *, Connection *> ConnectionSet;
   typedef QMap<QWidget *, QWidget *> WidgetSet;

   class EndPoint
   {
    public:
      enum Type { Source, Target };

      explicit EndPoint(Connection *_con = nullptr, Type _type = Source)
         : con(_con), type(_type)
      {
      }

      bool isNull() const {
         return con == nullptr;
      }
      bool operator == (const EndPoint &other) const {
         return con == other.con && type == other.type;
      }
      bool operator != (const EndPoint &other) const {
         return !operator == (other);
      }
      Connection *con;
      Type type;
   };

   enum LineDir {
      UpDir = 0,
      DownDir,
      RightDir,
      LeftDir
   };
};

class Connection : public CETypes
{
 public:
   explicit Connection(ConnectionEdit *edit);
   explicit Connection(ConnectionEdit *edit, QObject *source, QObject *target);
   virtual ~Connection() {}

   QObject *object(EndPoint::Type type) const {
      return (type == EndPoint::Source ? m_source : m_target);
   }

   QWidget *widget(EndPoint::Type type) const {
      return dynamic_cast<QWidget *>(object(type));
   }

   QPoint endPointPos(EndPoint::Type type) const;
   QRect endPointRect(EndPoint::Type) const;
   void setEndPoint(EndPoint::Type type, QObject *w, const QPoint &pos) {
      type == EndPoint::Source ? setSource(w, pos) : setTarget(w, pos);
   }

   bool isVisible() const;
   virtual void updateVisibility();
   void setVisible(bool b);

   virtual QRegion region() const;
   bool contains(const QPoint &pos) const;
   virtual void paint(QPainter *p) const;

   void update(bool update_widgets = true) const;
   void checkWidgets();

   QString label(EndPoint::Type type) const {
      return type == EndPoint::Source ? m_source_label : m_target_label;
   }
   void setLabel(EndPoint::Type type, const QString &text);
   QRect labelRect(EndPoint::Type type) const;
   QPixmap labelPixmap(EndPoint::Type type) const {
      return type == EndPoint::Source ? m_source_label_pm : m_target_label_pm;
   }

   ConnectionEdit *edit() const {
      return m_edit;
   }

   virtual void inserted() {}
   virtual void removed() {}

 private:
   QPoint m_source_pos, m_target_pos;
   QObject *m_source, *m_target;
   QList<QPoint> m_knee_list;
   QPolygonF m_arrow_head;
   ConnectionEdit *m_edit;
   QString m_source_label, m_target_label;
   QPixmap m_source_label_pm, m_target_label_pm;
   QRect m_source_rect, m_target_rect;
   bool m_visible;

   void setSource(QObject *source, const QPoint &pos);
   void setTarget(QObject *target, const QPoint &pos);
   void updateKneeList();
   void trimLine();
   void updatePixmap(EndPoint::Type type);
   LineDir labelDir(EndPoint::Type type) const;
   bool ground() const;
   QRect groundRect() const;
};

class ConnectionEdit : public QWidget, public CETypes
{
   CS_OBJECT(ConnectionEdit)

 public:
   ConnectionEdit(QWidget *parent, QDesignerFormWindowInterface *form);
   virtual ~ConnectionEdit();

   inline const QPointer<QWidget> &background() const {
      return m_bg_widget;
   }

   void setSelected(Connection *con, bool sel);
   bool selected(const Connection *con) const;

   int connectionCount() const {
      return m_con_list.size();
   }
   Connection *connection(int i) const {
      return m_con_list.at(i);
   }
   int indexOfConnection(Connection *con) const {
      return m_con_list.indexOf(con);
   }

   virtual void setSource(Connection *con, const QString &obj_name);
   virtual void setTarget(Connection *con, const QString &obj_name);

   QUndoStack *undoStack() const {
      return m_undo_stack;
   }

   void clear();

   void showEvent(QShowEvent *) override {
      updateBackground();
   }

   CS_SIGNAL_1(Public, void aboutToAddConnection(int idx))
   CS_SIGNAL_2(aboutToAddConnection, idx)

   CS_SIGNAL_1(Public, void connectionAdded(Connection *con))
   CS_SIGNAL_2(connectionAdded, con)

   CS_SIGNAL_1(Public, void aboutToRemoveConnection(Connection *con))
   CS_SIGNAL_2(aboutToRemoveConnection, con)

   CS_SIGNAL_1(Public, void connectionRemoved(int idx))
   CS_SIGNAL_2(connectionRemoved, idx)

   CS_SIGNAL_1(Public, void connectionSelected(Connection *con))
   CS_SIGNAL_2(connectionSelected, con)

   CS_SIGNAL_1(Public, void widgetActivated(QWidget *wgt))
   CS_SIGNAL_2(widgetActivated, wgt)

   CS_SIGNAL_1(Public, void connectionChanged(Connection *con))
   CS_SIGNAL_2(connectionChanged, con)

   CS_SLOT_1(Public, void selectNone())
   CS_SLOT_2(selectNone)
   CS_SLOT_1(Public, void selectAll())
   CS_SLOT_2(selectAll)
   CS_SLOT_1(Public, virtual void deleteSelected())
   CS_SLOT_2(deleteSelected)
   CS_SLOT_1(Public, virtual void setBackground(QWidget *background))
   CS_SLOT_2(setBackground)
   CS_SLOT_1(Public, virtual void updateBackground())
   CS_SLOT_2(updateBackground)
   CS_SLOT_1(Public, virtual void widgetRemoved(QWidget *w))
   CS_SLOT_2(widgetRemoved)
   CS_SLOT_1(Public, virtual void objectRemoved(QObject *o))
   CS_SLOT_2(objectRemoved)

   CS_SLOT_1(Public, void updateLines())
   CS_SLOT_2(updateLines)
   CS_SLOT_1(Public, void enableUpdateBackground(bool enable))
   CS_SLOT_2(enableUpdateBackground)

 protected:
   void paintEvent(QPaintEvent *e) override;
   void mouseMoveEvent(QMouseEvent *e) override;
   void mousePressEvent(QMouseEvent *e) override;
   void mouseReleaseEvent(QMouseEvent *e) override;
   void keyPressEvent(QKeyEvent *e) override;
   void mouseDoubleClickEvent(QMouseEvent *e) override;
   void resizeEvent(QResizeEvent *e) override;
   void contextMenuEvent(QContextMenuEvent *event) override;

   virtual Connection *createConnection(QWidget *source, QWidget *target);
   virtual void modifyConnection(Connection *con);

   virtual QWidget *widgetAt(const QPoint &pos) const;
   virtual void createContextMenu(QMenu &menu);
   void addConnectionX(Connection *con);
   QRect widgetRect(QWidget *w) const;

   enum State { Editing, Connecting, Dragging };
   State state() const;

   virtual void endConnection(QWidget *target, const QPoint &pos);

   const ConnectionList &connectionList() const {
      return m_con_list;
   }
   const ConnectionSet &selection()  const      {
      return m_sel_con_set;
   }
   Connection *takeConnection(Connection *con);
   Connection *newlyAddedConnection()           {
      return m_tmp_con;
   }
   void clearNewlyAddedConnection();

   void findObjectsUnderMouse(const QPoint &pos);

 private:
   void startConnection(QWidget *source, const QPoint &pos);
   void continueConnection(QWidget *target, const QPoint &pos);
   void abortConnection();

   void startDrag(const EndPoint &end_point, const QPoint &pos);
   void continueDrag(const QPoint &pos);
   void endDrag(const QPoint &pos);
   void adjustHotSopt(const EndPoint &end_point, const QPoint &pos);
   Connection *connectionAt(const QPoint &pos) const;
   EndPoint endPointAt(const QPoint &pos) const;
   void paintConnection(QPainter *p, Connection *con,
      WidgetSet *heavy_highlight_set,
      WidgetSet *light_highlight_set) const;
   void paintLabel(QPainter *p, EndPoint::Type type, Connection *con);


   QPointer<QWidget> m_bg_widget;
   QUndoStack *m_undo_stack;
   bool m_enable_update_background;

   Connection *m_tmp_con;       // the connection we are currently editing
   ConnectionList m_con_list;
   bool m_start_connection_on_drag;
   EndPoint m_end_point_under_mouse;
   QPointer<QWidget> m_widget_under_mouse;

   EndPoint m_drag_end_point;
   QPoint m_old_source_pos, m_old_target_pos;
   ConnectionSet m_sel_con_set;
   const QColor m_inactive_color;
   const QColor m_active_color;

   friend class Connection;
   friend class AddConnectionCommand;
   friend class DeleteConnectionsCommand;
   friend class SetEndPointCommand;
};

class CECommand : public QUndoCommand, public CETypes
{
 public:
   explicit  CECommand(ConnectionEdit *edit)
      : m_edit(edit)
   {
   }

   bool mergeWith(const QUndoCommand *) override {
      return false;
   }

   ConnectionEdit *edit() const {
      return m_edit;
   }

 private:
   ConnectionEdit *m_edit;
};

class AddConnectionCommand : public CECommand
{
 public:
   AddConnectionCommand(ConnectionEdit *edit, Connection *con);
   void redo() override;
   void undo() override;

 private:
   Connection *m_con;
};

class DeleteConnectionsCommand : public CECommand
{
 public:
   DeleteConnectionsCommand(ConnectionEdit *edit, const ConnectionList &con_list);
   void redo() override;
   void undo() override;

 private:
   ConnectionList m_con_list;
};

} // namespace qdesigner_internal



#endif
