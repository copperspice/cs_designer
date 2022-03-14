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

#ifndef PALETTEEDITOR_H
#define PALETTEEDITOR_H

#include <ui_palette_editor.h>

class QtColorButton;
class QDesignerFormEditorInterface;

#include <QItemDelegate>
#include <QPair>

class QListView;
class QLabel;

namespace qdesigner_internal {

class PaletteEditor: public QDialog
{
   CS_OBJECT(PaletteEditor)

 public:
   virtual ~PaletteEditor();

   static QPalette getPalette(QDesignerFormEditorInterface *core,
      QWidget *parent, const QPalette &init = QPalette(),
      const QPalette &parentPal = QPalette(), int *result = 0);

   QPalette palette() const;
   void setPalette(const QPalette &palette);
   void setPalette(const QPalette &palette, const QPalette &parentPalette);

 private:
   CS_SLOT_1(Private, void on_buildButton_colorChanged(const QColor &un_named_arg1))
   CS_SLOT_2(on_buildButton_colorChanged)

   CS_SLOT_1(Private, void on_activeRadio_clicked())
   CS_SLOT_2(on_activeRadio_clicked)

   CS_SLOT_1(Private, void on_inactiveRadio_clicked())
   CS_SLOT_2(on_inactiveRadio_clicked)

   CS_SLOT_1(Private, void on_disabledRadio_clicked())
   CS_SLOT_2(on_disabledRadio_clicked)

   CS_SLOT_1(Private, void on_computeRadio_clicked())
   CS_SLOT_2(on_computeRadio_clicked)

   CS_SLOT_1(Private, void on_detailsRadio_clicked())
   CS_SLOT_2(on_detailsRadio_clicked)

   CS_SLOT_1(Private, void paletteChanged(const QPalette &palette))
   CS_SLOT_2(paletteChanged)

   PaletteEditor(QDesignerFormEditorInterface *core, QWidget *parent);
   void buildPalette();

   void updatePreviewPalette();
   void updateStyledButton();

   QPalette::ColorGroup currentColorGroup() const {
      return m_currentColorGroup;
   }

   Ui::PaletteEditor ui;
   QPalette m_editPalette;
   QPalette m_parentPalette;
   QPalette::ColorGroup m_currentColorGroup;
   class PaletteModel *m_paletteModel;
   bool m_modelUpdated;
   bool m_paletteUpdated;
   bool m_compute;
   QDesignerFormEditorInterface *m_core;
};

class PaletteModel : public QAbstractTableModel
{
   CS_OBJECT(PaletteModel)

   CS_PROPERTY_READ(colorRole, colorRole)

 public:
   explicit PaletteModel(QObject *parent = nullptr);

   int rowCount(const QModelIndex &parent    = QModelIndex()) const;
   int columnCount(const QModelIndex &parent = QModelIndex()) const;

   QVariant data(const QModelIndex &index, int role) const;
   bool setData(const QModelIndex &index, const QVariant &value, int role);
   Qt::ItemFlags flags(const QModelIndex &index) const;
   QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const;

   QPalette getPalette() const;
   void setPalette(const QPalette &palette, const QPalette &parentPalette);

   QPalette::ColorRole colorRole() const {
      return QPalette::NoRole;
   }
   void setCompute(bool on) {
      m_compute = on;
   }

   CS_SIGNAL_1(Public, void paletteChanged(const QPalette &palette))
   CS_SIGNAL_2(paletteChanged, palette)

 private:
   QPalette::ColorGroup columnToGroup(int index) const;
   int groupToColumn(QPalette::ColorGroup group) const;

   QPalette m_palette;
   QPalette m_parentPalette;
   QMap<int, QPair<QString, QPalette::ColorRole>> m_roleNames;
   bool m_compute;
};

class BrushEditor : public QWidget
{
   CS_OBJECT(BrushEditor)

 public:
   explicit BrushEditor(QDesignerFormEditorInterface *core, QWidget *parent = nullptr);

   void setBrush(const QBrush &brush);
   QBrush brush() const;
   bool changed() const;

   CS_SIGNAL_1(Public, void changed(QWidget *widget))
   CS_SIGNAL_OVERLOAD(changed, (QWidget *), widget)

 private:
   CS_SLOT_1(Private, void brushChanged())
   CS_SLOT_2(brushChanged)

   QtColorButton *m_button;
   bool m_changed;
   QDesignerFormEditorInterface *m_core;
};

class RoleEditor : public QWidget
{
   CS_OBJECT(RoleEditor)

 public:
   explicit RoleEditor(QWidget *parent = nullptr);

   void setLabel(const QString &label);
   void setEdited(bool on);
   bool edited() const;

   CS_SIGNAL_1(Public, void changed(QWidget *widget))
   CS_SIGNAL_2(changed, widget)

 private:
   CS_SLOT_1(Private, void emitResetProperty())
   CS_SLOT_2(emitResetProperty)

   QLabel *m_label;
   bool    m_edited;
};

class ColorDelegate : public QItemDelegate
{
   CS_OBJECT(ColorDelegate)

 public:
   explicit ColorDelegate(QDesignerFormEditorInterface *core, QObject *parent = nullptr);

   QWidget *createEditor(QWidget *parent, const QStyleOptionViewItem &option,
      const QModelIndex &index) const override;

   void setEditorData(QWidget *ed, const QModelIndex &index) const override;
   void setModelData(QWidget *ed, QAbstractItemModel *model,
      const QModelIndex &index) const override;

   void updateEditorGeometry(QWidget *ed, const QStyleOptionViewItem &option,
      const QModelIndex &index) const override;

   void paint(QPainter *painter, const QStyleOptionViewItem &opt,
      const QModelIndex &index) const override;
   QSize sizeHint(const QStyleOptionViewItem &opt, const QModelIndex &index) const override;

 private:
   QDesignerFormEditorInterface *m_core;
};

}  // namespace qdesigner_internal

#endif
