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

#include <edit_property.h>
#include <property_view_utils.h>

#include <QSpinBox>
#include <QScrollBar>
#include <QComboBox>
#include <QAbstractItemView>
#include <QLineEdit>
#include <QDateTimeEdit>
#include <QHBoxLayout>
#include <QMenu>
#include <QKeyEvent>
#include <QApplication>
#include <QLabel>
#include <QToolButton>
#include <QColorDialog>
#include <QFontDialog>
#include <QSpacerItem>
#include <QKeySequenceEdit>
#include <QMap>
#include <qalgorithms.h>

// Set a hard coded left margin to account for the indentation
// of the tree view icon when switching to an editor

static inline void setupTreeViewEditorMargin(QLayout *lt)
{
   enum { DecorationMargin = 4 };
   if (QApplication::layoutDirection() == Qt::LeftToRight) {
      lt->setContentsMargins(DecorationMargin, 0, 0, 0);
   } else {
      lt->setContentsMargins(0, 0, DecorationMargin, 0);
   }
}

// Base class for editor factory private classes. Manages mapping of properties to editors and vice versa.

template <class Editor>
class EditorFactoryPrivate
{
 public:

   typedef QList<Editor *> EditorList;
   typedef QMap<QtProperty *, EditorList> PropertyToEditorListMap;
   typedef QMap<Editor *, QtProperty *> EditorToPropertyMap;

   Editor *createEditor(QtProperty *property, QWidget *parent);
   void initializeEditor(QtProperty *property, Editor *e);
   void slotEditorDestroyed(QObject *object);

   PropertyToEditorListMap  m_createdEditors;
   EditorToPropertyMap m_editorToProperty;
};

template <class Editor>
Editor *EditorFactoryPrivate<Editor>::createEditor(QtProperty *property, QWidget *parent)
{
   Editor *editor = new Editor(parent);
   initializeEditor(property, editor);
   return editor;
}

template <class Editor>
void EditorFactoryPrivate<Editor>::initializeEditor(QtProperty *property, Editor *editor)
{
   typename PropertyToEditorListMap::iterator it = m_createdEditors.find(property);
   if (it == m_createdEditors.end()) {
      it = m_createdEditors.insert(property, EditorList());
   }
   it.value().append(editor);
   m_editorToProperty.insert(editor, property);
}

template <class Editor>
void EditorFactoryPrivate<Editor>::slotEditorDestroyed(QObject *object)
{
   const typename EditorToPropertyMap::iterator ecend = m_editorToProperty.end();
   for (typename EditorToPropertyMap::iterator itEditor = m_editorToProperty.begin(); itEditor !=  ecend; ++itEditor) {
      if (itEditor.key() == object) {
         Editor *editor = itEditor.key();
         QtProperty *property = itEditor.value();
         const typename PropertyToEditorListMap::iterator pit = m_createdEditors.find(property);
         if (pit != m_createdEditors.end()) {
            pit.value().removeAll(editor);
            if (pit.value().empty()) {
               m_createdEditors.erase(pit);
            }
         }
         m_editorToProperty.erase(itEditor);
         return;
      }
   }
}

// ------------ QtSpinBoxFactory

class QtSpinBoxFactoryPrivate: public EditorFactoryPrivate<QSpinBox>
{
   QtSpinBoxFactory *q_ptr;
   Q_DECLARE_PUBLIC(QtSpinBoxFactory)
 public:

   void slotPropertyChanged(QtProperty *property, int value);
   void slotRangeChanged(QtProperty *property, int min, int max);
   void slotSingleStepChanged(QtProperty *property, int step);
   void slotSetValue(int value);
};

void QtSpinBoxFactoryPrivate::slotPropertyChanged(QtProperty *property, int value)
{
   if (!m_createdEditors.contains(property)) {
      return;
   }

   QListIterator<QSpinBox *> itEditor(m_createdEditors[property]);

   while (itEditor.hasNext()) {
      QSpinBox *editor = itEditor.next();
      if (editor->value() != value) {
         editor->blockSignals(true);
         editor->setValue(value);
         editor->blockSignals(false);
      }
   }
}

void QtSpinBoxFactoryPrivate::slotRangeChanged(QtProperty *property, int min, int max)
{
   if (!m_createdEditors.contains(property)) {
      return;
   }

   QtIntPropertyManager *manager = q_ptr->propertyManager(property);
   if (!manager) {
      return;
   }

   QListIterator<QSpinBox *> itEditor(m_createdEditors[property]);
   while (itEditor.hasNext()) {
      QSpinBox *editor = itEditor.next();
      editor->blockSignals(true);
      editor->setRange(min, max);
      editor->setValue(manager->value(property));
      editor->blockSignals(false);
   }
}

void QtSpinBoxFactoryPrivate::slotSingleStepChanged(QtProperty *property, int step)
{
   if (!m_createdEditors.contains(property)) {
      return;
   }
   QListIterator<QSpinBox *> itEditor(m_createdEditors[property]);
   while (itEditor.hasNext()) {
      QSpinBox *editor = itEditor.next();
      editor->blockSignals(true);
      editor->setSingleStep(step);
      editor->blockSignals(false);
   }
}

void QtSpinBoxFactoryPrivate::slotSetValue(int value)
{
   QObject *object = q_ptr->sender();
   auto  ecend = m_editorToProperty.constEnd();

   for (auto itEditor = m_editorToProperty.constBegin(); itEditor !=  ecend; ++itEditor) {
      if (itEditor.key() == object) {
         QtProperty *property = itEditor.value();
         QtIntPropertyManager *manager = q_ptr->propertyManager(property);
         if (!manager) {
            return;
         }
         manager->setValue(property, value);
         return;
      }
   }
}

QtSpinBoxFactory::QtSpinBoxFactory(QObject *parent)
   : QtAbstractEditorFactory<QtIntPropertyManager>(parent), d_ptr(new QtSpinBoxFactoryPrivate())
{
   d_ptr->q_ptr = this;

}

QtSpinBoxFactory::~QtSpinBoxFactory()
{
   qDeleteAll(d_ptr->m_editorToProperty.keys());
}

void QtSpinBoxFactory::connectPropertyManager(QtIntPropertyManager *manager)
{
   connect(manager, &QtIntPropertyManager::valueChanged,
      this, &QtSpinBoxFactory::slotPropertyChanged);

   connect(manager, &QtIntPropertyManager::rangeChanged,
      this, &QtSpinBoxFactory::slotRangeChanged);

   connect(manager, &QtIntPropertyManager::singleStepChanged,
      this, &QtSpinBoxFactory::slotSingleStepChanged);
}

QWidget *QtSpinBoxFactory::createEditor(QtIntPropertyManager *manager, QtProperty *property,
   QWidget *parent)
{
   QSpinBox *editor = d_ptr->createEditor(property, parent);
   editor->setSingleStep(manager->singleStep(property));
   editor->setRange(manager->minimum(property), manager->maximum(property));
   editor->setValue(manager->value(property));
   editor->setKeyboardTracking(false);

   connect(editor, cs_mp_cast<int>(&QSpinBox::valueChanged), this, &QtSpinBoxFactory::slotSetValue);
   connect(editor, &QSpinBox::destroyed, this, &QtSpinBoxFactory::slotEditorDestroyed);

   return editor;
}

void QtSpinBoxFactory::disconnectPropertyManager(QtIntPropertyManager *manager)
{
   disconnect(manager, &QtIntPropertyManager::valueChanged,
      this, &QtSpinBoxFactory::slotPropertyChanged);

   disconnect(manager, &QtIntPropertyManager::rangeChanged,
      this, &QtSpinBoxFactory::slotRangeChanged);

   disconnect(manager, &QtIntPropertyManager::singleStepChanged,
      this, &QtSpinBoxFactory::slotSingleStepChanged);
}

class QtSliderFactoryPrivate: public EditorFactoryPrivate<QSlider>
{
   QtSliderFactory *q_ptr;
   Q_DECLARE_PUBLIC(QtSliderFactory)

 public:
   void slotPropertyChanged(QtProperty *property, int value);
   void slotRangeChanged(QtProperty *property, int min, int max);
   void slotSingleStepChanged(QtProperty *property, int step);
   void slotSetValue(int value);
};

void QtSliderFactoryPrivate::slotPropertyChanged(QtProperty *property, int value)
{
   if (!m_createdEditors.contains(property)) {
      return;
   }

   QListIterator<QSlider *> itEditor(m_createdEditors[property]);

   while (itEditor.hasNext()) {
      QSlider *editor = itEditor.next();
      editor->blockSignals(true);
      editor->setValue(value);
      editor->blockSignals(false);
   }
}

void QtSliderFactoryPrivate::slotRangeChanged(QtProperty *property, int min, int max)
{
   if (! m_createdEditors.contains(property)) {
      return;
   }

   QtIntPropertyManager *manager = q_ptr->propertyManager(property);
   if (!manager) {
      return;
   }

   QListIterator<QSlider *> itEditor(m_createdEditors[property]);
   while (itEditor.hasNext()) {
      QSlider *editor = itEditor.next();
      editor->blockSignals(true);
      editor->setRange(min, max);
      editor->setValue(manager->value(property));
      editor->blockSignals(false);
   }
}

void QtSliderFactoryPrivate::slotSingleStepChanged(QtProperty *property, int step)
{
   if (!m_createdEditors.contains(property)) {
      return;
   }

   QListIterator<QSlider *> itEditor(m_createdEditors[property]);

   while (itEditor.hasNext()) {
      QSlider *editor = itEditor.next();
      editor->blockSignals(true);
      editor->setSingleStep(step);
      editor->blockSignals(false);
   }
}

void QtSliderFactoryPrivate::slotSetValue(int value)
{
   QObject *object = q_ptr->sender();
   auto ecend = m_editorToProperty.constEnd();

   for (auto itEditor = m_editorToProperty.constBegin(); itEditor != ecend; ++itEditor ) {
      if (itEditor.key() == object) {
         QtProperty *property = itEditor.value();
         QtIntPropertyManager *manager = q_ptr->propertyManager(property);
         if (!manager) {
            return;
         }
         manager->setValue(property, value);
         return;
      }
   }
}

QtSliderFactory::QtSliderFactory(QObject *parent)
   : QtAbstractEditorFactory<QtIntPropertyManager>(parent), d_ptr(new QtSliderFactoryPrivate())
{
   d_ptr->q_ptr = this;

}

QtSliderFactory::~QtSliderFactory()
{
   qDeleteAll(d_ptr->m_editorToProperty.keys());
}

void QtSliderFactory::connectPropertyManager(QtIntPropertyManager *manager)
{
   connect(manager, &QtIntPropertyManager::valueChanged,
      this, &QtSliderFactory::slotPropertyChanged);

   connect(manager, &QtIntPropertyManager::rangeChanged,
      this, &QtSliderFactory::slotRangeChanged);

   connect(manager, &QtIntPropertyManager::singleStepChanged,
      this, &QtSliderFactory::slotSingleStepChanged);
}

QWidget *QtSliderFactory::createEditor(QtIntPropertyManager *manager, QtProperty *property,
   QWidget *parent)
{
   QSlider *editor = new QSlider(Qt::Horizontal, parent);
   d_ptr->initializeEditor(property, editor);
   editor->setSingleStep(manager->singleStep(property));
   editor->setRange(manager->minimum(property), manager->maximum(property));
   editor->setValue(manager->value(property));

   connect(editor, &QSlider::valueChanged, this, &QtSliderFactory::slotSetValue);
   connect(editor, &QSlider::destroyed,    this, &QtSliderFactory::slotEditorDestroyed);

   return editor;
}

void QtSliderFactory::disconnectPropertyManager(QtIntPropertyManager *manager)
{
   disconnect(manager, &QtIntPropertyManager::valueChanged,
      this, &QtSliderFactory::slotPropertyChanged);

   disconnect(manager, &QtIntPropertyManager::rangeChanged,
      this, &QtSliderFactory::slotRangeChanged);

   disconnect(manager, &QtIntPropertyManager::singleStepChanged,
      this, &QtSliderFactory::slotSingleStepChanged);
}

class QtScrollBarFactoryPrivate: public  EditorFactoryPrivate<QScrollBar>
{
   QtScrollBarFactory *q_ptr;
   Q_DECLARE_PUBLIC(QtScrollBarFactory)

 public:
   void slotPropertyChanged(QtProperty *property, int value);
   void slotRangeChanged(QtProperty *property, int min, int max);
   void slotSingleStepChanged(QtProperty *property, int step);
   void slotSetValue(int value);
};

void QtScrollBarFactoryPrivate::slotPropertyChanged(QtProperty *property, int value)
{
   if (!m_createdEditors.contains(property)) {
      return;
   }

   QListIterator<QScrollBar *> itEditor( m_createdEditors[property]);
   while (itEditor.hasNext()) {
      QScrollBar *editor = itEditor.next();
      editor->blockSignals(true);
      editor->setValue(value);
      editor->blockSignals(false);
   }
}

void QtScrollBarFactoryPrivate::slotRangeChanged(QtProperty *property, int min, int max)
{
   if (!m_createdEditors.contains(property)) {
      return;
   }

   QtIntPropertyManager *manager = q_ptr->propertyManager(property);
   if (!manager) {
      return;
   }

   QListIterator<QScrollBar *> itEditor( m_createdEditors[property]);
   while (itEditor.hasNext()) {
      QScrollBar *editor = itEditor.next();
      editor->blockSignals(true);
      editor->setRange(min, max);
      editor->setValue(manager->value(property));
      editor->blockSignals(false);
   }
}

void QtScrollBarFactoryPrivate::slotSingleStepChanged(QtProperty *property, int step)
{
   if (!m_createdEditors.contains(property)) {
      return;
   }
   QListIterator<QScrollBar *> itEditor(m_createdEditors[property]);
   while (itEditor.hasNext()) {
      QScrollBar *editor = itEditor.next();
      editor->blockSignals(true);
      editor->setSingleStep(step);
      editor->blockSignals(false);
   }
}

void QtScrollBarFactoryPrivate::slotSetValue(int value)
{
   QObject *object = q_ptr->sender();
   auto ecend = m_editorToProperty.constEnd();

   for (auto itEditor = m_editorToProperty.constBegin(); itEditor != ecend; ++itEditor)
      if (itEditor.key() == object) {
         QtProperty *property = itEditor.value();
         QtIntPropertyManager *manager = q_ptr->propertyManager(property);
         if (!manager) {
            return;
         }
         manager->setValue(property, value);
         return;
      }
}

QtScrollBarFactory::QtScrollBarFactory(QObject *parent)
   : QtAbstractEditorFactory<QtIntPropertyManager>(parent), d_ptr(new QtScrollBarFactoryPrivate())
{
   d_ptr->q_ptr = this;

}

QtScrollBarFactory::~QtScrollBarFactory()
{
   qDeleteAll(d_ptr->m_editorToProperty.keys());
}

void QtScrollBarFactory::connectPropertyManager(QtIntPropertyManager *manager)
{
   connect(manager, &QtIntPropertyManager::valueChanged,
         this, &QtScrollBarFactory::slotPropertyChanged);

   connect(manager, &QtIntPropertyManager::rangeChanged,
         this, &QtScrollBarFactory::slotRangeChanged);

   connect(manager, &QtIntPropertyManager::singleStepChanged,
         this, &QtScrollBarFactory::slotSingleStepChanged);
}

QWidget *QtScrollBarFactory::createEditor(QtIntPropertyManager *manager, QtProperty *property,
   QWidget *parent)
{
   QScrollBar *editor = new QScrollBar(Qt::Horizontal, parent);
   d_ptr->initializeEditor(property, editor);

   editor->setSingleStep(manager->singleStep(property));
   editor->setRange(manager->minimum(property), manager->maximum(property));
   editor->setValue(manager->value(property));

   connect(editor, &QScrollBar::valueChanged, this, &QtScrollBarFactory::slotSetValue);
   connect(editor, &QScrollBar::destroyed,    this, &QtScrollBarFactory::slotEditorDestroyed);

   return editor;
}

void QtScrollBarFactory::disconnectPropertyManager(QtIntPropertyManager *manager)
{
   disconnect(manager, &QtIntPropertyManager::valueChanged,
         this, &QtScrollBarFactory::slotPropertyChanged);

   disconnect(manager, &QtIntPropertyManager::rangeChanged,
         this, &QtScrollBarFactory::slotRangeChanged);

   disconnect(manager, &QtIntPropertyManager::singleStepChanged,
         this, &QtScrollBarFactory::slotSingleStepChanged);
}

class QtCheckBoxFactoryPrivate: public EditorFactoryPrivate<QtBoolEdit>
{
   QtCheckBoxFactory *q_ptr;
   Q_DECLARE_PUBLIC(QtCheckBoxFactory)

 public:
   void slotPropertyChanged(QtProperty *property, bool value);
   void slotSetValue(bool value);
};

void QtCheckBoxFactoryPrivate::slotPropertyChanged(QtProperty *property, bool value)
{
   if (!m_createdEditors.contains(property)) {
      return;
   }

   QListIterator<QtBoolEdit *> itEditor(m_createdEditors[property]);

   while (itEditor.hasNext()) {
      QtBoolEdit *editor = itEditor.next();
      editor->blockCheckBoxSignals(true);
      editor->setChecked(value);
      editor->blockCheckBoxSignals(false);
   }
}

void QtCheckBoxFactoryPrivate::slotSetValue(bool value)
{
   QObject *object = q_ptr->sender();
   auto ecend = m_editorToProperty.constEnd();

   for (auto itEditor = m_editorToProperty.constBegin(); itEditor != ecend;  ++itEditor)
      if (itEditor.key() == object) {
         QtProperty *property = itEditor.value();
         QtBoolPropertyManager *manager = q_ptr->propertyManager(property);
         if (!manager) {
            return;
         }
         manager->setValue(property, value);
         return;
      }
}

QtCheckBoxFactory::QtCheckBoxFactory(QObject *parent)
   : QtAbstractEditorFactory<QtBoolPropertyManager>(parent), d_ptr(new QtCheckBoxFactoryPrivate())
{
   d_ptr->q_ptr = this;

}

QtCheckBoxFactory::~QtCheckBoxFactory()
{
   qDeleteAll(d_ptr->m_editorToProperty.keys());
}

void QtCheckBoxFactory::connectPropertyManager(QtBoolPropertyManager *manager)
{
   connect(manager, &QtBoolPropertyManager::valueChanged, this, &QtCheckBoxFactory::slotPropertyChanged);
}

QWidget *QtCheckBoxFactory::createEditor(QtBoolPropertyManager *manager, QtProperty *property,
   QWidget *parent)
{
   QtBoolEdit *editor = d_ptr->createEditor(property, parent);
   editor->setChecked(manager->value(property));

   connect(editor, &QtBoolEdit::toggled,   this, &QtCheckBoxFactory::slotSetValue);
   connect(editor, &QtBoolEdit::destroyed, this, &QtCheckBoxFactory::slotEditorDestroyed);

   return editor;
}

void QtCheckBoxFactory::disconnectPropertyManager(QtBoolPropertyManager *manager)
{
   disconnect(manager, &QtBoolPropertyManager::valueChanged, this, &QtCheckBoxFactory::slotPropertyChanged);
}

class QtDoubleSpinBoxFactoryPrivate: public EditorFactoryPrivate<QDoubleSpinBox>
{
   QtDoubleSpinBoxFactory *q_ptr;
   Q_DECLARE_PUBLIC(QtDoubleSpinBoxFactory)

 public:
   void slotPropertyChanged(QtProperty *property, double value);
   void slotRangeChanged(QtProperty *property, double min, double max);
   void slotSingleStepChanged(QtProperty *property, double step);
   void slotDecimalsChanged(QtProperty *property, int prec);
   void slotSetValue(double value);
};

void QtDoubleSpinBoxFactoryPrivate::slotPropertyChanged(QtProperty *property, double value)
{
   QList<QDoubleSpinBox *> editors = m_createdEditors[property];
   QListIterator<QDoubleSpinBox *> itEditor(m_createdEditors[property]);
   while (itEditor.hasNext()) {
      QDoubleSpinBox *editor = itEditor.next();
      if (editor->value() != value) {
         editor->blockSignals(true);
         editor->setValue(value);
         editor->blockSignals(false);
      }
   }
}

void QtDoubleSpinBoxFactoryPrivate::slotRangeChanged(QtProperty *property,
   double min, double max)
{
   if (!m_createdEditors.contains(property)) {
      return;
   }

   QtDoublePropertyManager *manager = q_ptr->propertyManager(property);
   if (!manager) {
      return;
   }

   QList<QDoubleSpinBox *> editors = m_createdEditors[property];
   QListIterator<QDoubleSpinBox *> itEditor(editors);
   while (itEditor.hasNext()) {
      QDoubleSpinBox *editor = itEditor.next();
      editor->blockSignals(true);
      editor->setRange(min, max);
      editor->setValue(manager->value(property));
      editor->blockSignals(false);
   }
}

void QtDoubleSpinBoxFactoryPrivate::slotSingleStepChanged(QtProperty *property, double step)
{
   if (!m_createdEditors.contains(property)) {
      return;
   }

   QtDoublePropertyManager *manager = q_ptr->propertyManager(property);
   if (!manager) {
      return;
   }

   QList<QDoubleSpinBox *> editors = m_createdEditors[property];
   QListIterator<QDoubleSpinBox *> itEditor(editors);
   while (itEditor.hasNext()) {
      QDoubleSpinBox *editor = itEditor.next();
      editor->blockSignals(true);
      editor->setSingleStep(step);
      editor->blockSignals(false);
   }
}

void QtDoubleSpinBoxFactoryPrivate::slotDecimalsChanged(QtProperty *property, int prec)
{
   if (!m_createdEditors.contains(property)) {
      return;
   }

   QtDoublePropertyManager *manager = q_ptr->propertyManager(property);
   if (!manager) {
      return;
   }

   QList<QDoubleSpinBox *> editors = m_createdEditors[property];
   QListIterator<QDoubleSpinBox *> itEditor(editors);
   while (itEditor.hasNext()) {
      QDoubleSpinBox *editor = itEditor.next();
      editor->blockSignals(true);
      editor->setDecimals(prec);
      editor->setValue(manager->value(property));
      editor->blockSignals(false);
   }
}

void QtDoubleSpinBoxFactoryPrivate::slotSetValue(double value)
{
   QObject *object = q_ptr->sender();
   auto itcend = m_editorToProperty.constEnd();

   for (auto itEditor = m_editorToProperty.constBegin(); itEditor != itcend; ++itEditor) {
      if (itEditor.key() == object) {
         QtProperty *property = itEditor.value();
         QtDoublePropertyManager *manager = q_ptr->propertyManager(property);
         if (!manager) {
            return;
         }
         manager->setValue(property, value);
         return;
      }
   }
}

QtDoubleSpinBoxFactory::QtDoubleSpinBoxFactory(QObject *parent)
   : QtAbstractEditorFactory<QtDoublePropertyManager>(parent), d_ptr(new QtDoubleSpinBoxFactoryPrivate())
{
   d_ptr->q_ptr = this;

}

QtDoubleSpinBoxFactory::~QtDoubleSpinBoxFactory()
{
   qDeleteAll(d_ptr->m_editorToProperty.keys());
}

void QtDoubleSpinBoxFactory::connectPropertyManager(QtDoublePropertyManager *manager)
{
   connect(manager, &QtDoublePropertyManager::valueChanged,
      this, &QtDoubleSpinBoxFactory::slotPropertyChanged);

   connect(manager, &QtDoublePropertyManager::rangeChanged,
      this, &QtDoubleSpinBoxFactory::slotRangeChanged);

   connect(manager, &QtDoublePropertyManager::singleStepChanged,
      this, &QtDoubleSpinBoxFactory::slotSingleStepChanged);

   connect(manager, &QtDoublePropertyManager::decimalsChanged,
      this, &QtDoubleSpinBoxFactory::slotDecimalsChanged);
}

QWidget *QtDoubleSpinBoxFactory::createEditor(QtDoublePropertyManager *manager,
   QtProperty *property, QWidget *parent)
{
   QDoubleSpinBox *editor = d_ptr->createEditor(property, parent);
   editor->setSingleStep(manager->singleStep(property));
   editor->setDecimals(manager->decimals(property));
   editor->setRange(manager->minimum(property), manager->maximum(property));
   editor->setValue(manager->value(property));
   editor->setKeyboardTracking(false);

   connect(editor, cs_mp_cast<double>(&QDoubleSpinBox::valueChanged),
         this, &QtDoubleSpinBoxFactory::slotSetValue);

   connect(editor, &QDoubleSpinBox::destroyed,
         this, &QtDoubleSpinBoxFactory::slotEditorDestroyed);

   return editor;
}

void QtDoubleSpinBoxFactory::disconnectPropertyManager(QtDoublePropertyManager *manager)
{
   disconnect(manager, &QtDoublePropertyManager::valueChanged,
      this, &QtDoubleSpinBoxFactory::slotPropertyChanged);

   disconnect(manager, &QtDoublePropertyManager::rangeChanged,
      this, &QtDoubleSpinBoxFactory::slotRangeChanged);

   disconnect(manager, &QtDoublePropertyManager::singleStepChanged,
      this, &QtDoubleSpinBoxFactory::slotSingleStepChanged);

   disconnect(manager, &QtDoublePropertyManager::decimalsChanged,
      this, &QtDoubleSpinBoxFactory::slotDecimalsChanged);
}

class QtLineEditFactoryPrivate: public EditorFactoryPrivate<QLineEdit>
{
   QtLineEditFactory *q_ptr;
   Q_DECLARE_PUBLIC(QtLineEditFactory)

 public:
   void slotPropertyChanged(QtProperty *property, const QString &value);
   void slotRegExpChanged(QtProperty *property, const QRegularExpression &regex);
   void slotSetValue(const QString &value);
};

void QtLineEditFactoryPrivate::slotPropertyChanged(QtProperty *property,
   const QString &value)
{
   if (!m_createdEditors.contains(property)) {
      return;
   }

   QListIterator<QLineEdit *> itEditor( m_createdEditors[property]);
   while (itEditor.hasNext()) {
      QLineEdit *editor = itEditor.next();
      if (editor->text() != value) {
         editor->setText(value);
      }
   }
}

void QtLineEditFactoryPrivate::slotRegExpChanged(QtProperty *property, const QRegularExpression &regex)
{
   if (! m_createdEditors.contains(property)) {
      return;
   }

   QtStringPropertyManager *manager = q_ptr->propertyManager(property);
   if (! manager) {
      return;
   }

   QListIterator<QLineEdit *> itEditor(m_createdEditors[property]);

   while (itEditor.hasNext()) {
      QLineEdit *editor = itEditor.next();
      editor->blockSignals(true);

      const QValidator *oldValidator = editor->validator();
      QValidator *newValidator = nullptr;

      if (regex.isValid()) {
         newValidator = new QRegularExpressionValidator(regex, editor);
      }

      editor->setValidator(newValidator);

      if (oldValidator) {
         delete oldValidator;
      }

      editor->blockSignals(false);
   }
}

void QtLineEditFactoryPrivate::slotSetValue(const QString &value)
{
   QObject *object = q_ptr->sender();
   auto ecend = m_editorToProperty.constEnd();

   for (auto itEditor = m_editorToProperty.constBegin(); itEditor != ecend; ++itEditor)
      if (itEditor.key() == object) {
         QtProperty *property = itEditor.value();
         QtStringPropertyManager *manager = q_ptr->propertyManager(property);
         if (!manager) {
            return;
         }
         manager->setValue(property, value);
         return;
      }
}

QtLineEditFactory::QtLineEditFactory(QObject *parent)
   : QtAbstractEditorFactory<QtStringPropertyManager>(parent), d_ptr(new QtLineEditFactoryPrivate())
{
   d_ptr->q_ptr = this;

}

QtLineEditFactory::~QtLineEditFactory()
{
   qDeleteAll(d_ptr->m_editorToProperty.keys());
}

void QtLineEditFactory::connectPropertyManager(QtStringPropertyManager *manager)
{
   connect(manager, &QtStringPropertyManager::valueChanged,  this, &QtLineEditFactory::slotPropertyChanged);
   connect(manager, &QtStringPropertyManager::regExpChanged, this, &QtLineEditFactory::slotRegExpChanged);
}

QWidget *QtLineEditFactory::createEditor(QtStringPropertyManager *manager,
   QtProperty *property, QWidget *parent)
{

   QLineEdit *editor = d_ptr->createEditor(property, parent);
   QRegularExpression regex = manager->regExp(property);

   if (regex.isValid()) {
      QValidator *validator = new QRegularExpressionValidator(regex, editor);
      editor->setValidator(validator);
   }

   editor->setText(manager->value(property));

   connect(editor, &QLineEdit::textEdited, this, &QtLineEditFactory::slotSetValue);
   connect(editor, &QLineEdit::destroyed,  this, &QtLineEditFactory::slotEditorDestroyed);

   return editor;
}

void QtLineEditFactory::disconnectPropertyManager(QtStringPropertyManager *manager)
{
   disconnect(manager, &QtStringPropertyManager::valueChanged,  this, &QtLineEditFactory::slotPropertyChanged);
   disconnect(manager, &QtStringPropertyManager::regExpChanged, this, &QtLineEditFactory::slotRegExpChanged);
}

class QtDateEditFactoryPrivate: public EditorFactoryPrivate<QDateEdit>
{
   QtDateEditFactory *q_ptr;
   Q_DECLARE_PUBLIC(QtDateEditFactory)

 public:
   void slotPropertyChanged(QtProperty *property, const QDate &value);
   void slotRangeChanged(QtProperty *property, const QDate &min, const QDate &max);
   void slotSetValue(const QDate &value);
};

void QtDateEditFactoryPrivate::slotPropertyChanged(QtProperty *property, const QDate &value)
{
   if (!m_createdEditors.contains(property)) {
      return;
   }

   QListIterator<QDateEdit *> itEditor(m_createdEditors[property]);

   while (itEditor.hasNext()) {
      QDateEdit *editor = itEditor.next();
      editor->blockSignals(true);
      editor->setDate(value);
      editor->blockSignals(false);
   }
}

void QtDateEditFactoryPrivate::slotRangeChanged(QtProperty *property,
   const QDate &min, const QDate &max)
{
   if (!m_createdEditors.contains(property)) {
      return;
   }

   QtDatePropertyManager *manager = q_ptr->propertyManager(property);
   if (!manager) {
      return;
   }

   QListIterator<QDateEdit *> itEditor(m_createdEditors[property]);
   while (itEditor.hasNext()) {
      QDateEdit *editor = itEditor.next();
      editor->blockSignals(true);
      editor->setDateRange(min, max);
      editor->setDate(manager->value(property));
      editor->blockSignals(false);
   }
}

void QtDateEditFactoryPrivate::slotSetValue(const QDate &value)
{
   QObject *object = q_ptr->sender();
   auto  ecend = m_editorToProperty.constEnd();

   for (auto itEditor = m_editorToProperty.constBegin(); itEditor != ecend; ++itEditor)
      if (itEditor.key() == object) {
         QtProperty *property = itEditor.value();
         QtDatePropertyManager *manager = q_ptr->propertyManager(property);
         if (!manager) {
            return;
         }
         manager->setValue(property, value);
         return;
      }
}

QtDateEditFactory::QtDateEditFactory(QObject *parent)
   : QtAbstractEditorFactory<QtDatePropertyManager>(parent), d_ptr(new QtDateEditFactoryPrivate())
{
   d_ptr->q_ptr = this;

}

QtDateEditFactory::~QtDateEditFactory()
{
   qDeleteAll(d_ptr->m_editorToProperty.keys());
}

void QtDateEditFactory::connectPropertyManager(QtDatePropertyManager *manager)
{
   connect(manager, &QtDatePropertyManager::valueChanged,
      this, &QtDateEditFactory::slotPropertyChanged);

   connect(manager, &QtDatePropertyManager::rangeChanged,
      this, &QtDateEditFactory::slotRangeChanged);
}

QWidget *QtDateEditFactory::createEditor(QtDatePropertyManager *manager, QtProperty *property,
   QWidget *parent)
{
   QDateEdit *editor = d_ptr->createEditor(property, parent);
   editor->setDisplayFormat(QtPropertyBrowserUtils::dateFormat());
   editor->setCalendarPopup(true);
   editor->setDateRange(manager->minimum(property), manager->maximum(property));
   editor->setDate(manager->value(property));

   connect(editor, &QDateEdit::dateChanged, this, &QtDateEditFactory::slotSetValue);
   connect(editor, &QDateEdit::destroyed,   this, &QtDateEditFactory::slotEditorDestroyed);

   return editor;
}

void QtDateEditFactory::disconnectPropertyManager(QtDatePropertyManager *manager)
{
   disconnect(manager, &QtDatePropertyManager::valueChanged,
      this, &QtDateEditFactory::slotPropertyChanged);

   disconnect(manager, &QtDatePropertyManager::rangeChanged,
      this, &QtDateEditFactory::slotRangeChanged);
}

class QtTimeEditFactoryPrivate: public EditorFactoryPrivate<QTimeEdit>
{
   QtTimeEditFactory *q_ptr;
   Q_DECLARE_PUBLIC(QtTimeEditFactory)

 public:
   void slotPropertyChanged(QtProperty *property, const QTime &value);
   void slotSetValue(const QTime &value);
};

void QtTimeEditFactoryPrivate::slotPropertyChanged(QtProperty *property, const QTime &value)
{
   if (! m_createdEditors.contains(property)) {
      return;
   }

   QListIterator<QTimeEdit *> itEditor(m_createdEditors[property]);

   while (itEditor.hasNext()) {
      QTimeEdit *editor = itEditor.next();
      editor->blockSignals(true);
      editor->setTime(value);
      editor->blockSignals(false);
   }
}

void QtTimeEditFactoryPrivate::slotSetValue(const QTime &value)
{
   QObject *object = q_ptr->sender();
   auto ecend = m_editorToProperty.constEnd();

   for (auto itEditor = m_editorToProperty.constBegin(); itEditor != ecend; ++itEditor)
      if (itEditor.key() == object) {
         QtProperty *property = itEditor.value();
         QtTimePropertyManager *manager = q_ptr->propertyManager(property);

         if (!manager) {
            return;
         }
         manager->setValue(property, value);
         return;
      }
}

QtTimeEditFactory::QtTimeEditFactory(QObject *parent)
   : QtAbstractEditorFactory<QtTimePropertyManager>(parent), d_ptr(new QtTimeEditFactoryPrivate())
{
   d_ptr->q_ptr = this;

}

QtTimeEditFactory::~QtTimeEditFactory()
{
   qDeleteAll(d_ptr->m_editorToProperty.keys());
}

void QtTimeEditFactory::connectPropertyManager(QtTimePropertyManager *manager)
{
   connect(manager, &QtTimePropertyManager::valueChanged,
      this, &QtTimeEditFactory::slotPropertyChanged);
}

QWidget *QtTimeEditFactory::createEditor(QtTimePropertyManager *manager, QtProperty *property,
   QWidget *parent)
{
   QTimeEdit *editor = d_ptr->createEditor(property, parent);
   editor->setDisplayFormat(QtPropertyBrowserUtils::timeFormat());
   editor->setTime(manager->value(property));

   connect(editor, &QTimeEdit::timeChanged, this, &QtTimeEditFactory::slotSetValue);
   connect(editor, &QTimeEdit::destroyed,   this, &QtTimeEditFactory::slotEditorDestroyed);

   return editor;
}

void QtTimeEditFactory::disconnectPropertyManager(QtTimePropertyManager *manager)
{
   disconnect(manager, &QtTimePropertyManager::valueChanged,
      this, &QtTimeEditFactory::slotPropertyChanged);
}

class QtDateTimeEditFactoryPrivate: public EditorFactoryPrivate<QDateTimeEdit>
{
   QtDateTimeEditFactory *q_ptr;
   Q_DECLARE_PUBLIC(QtDateTimeEditFactory)

 public:
   void slotPropertyChanged(QtProperty *property, const QDateTime &value);
   void slotSetValue(const QDateTime &value);
};

void QtDateTimeEditFactoryPrivate::slotPropertyChanged(QtProperty *property,
   const QDateTime &value)
{
   if (! m_createdEditors.contains(property)) {
      return;
   }

   QListIterator<QDateTimeEdit *> itEditor(m_createdEditors[property]);
   while (itEditor.hasNext()) {
      QDateTimeEdit *editor = itEditor.next();
      editor->blockSignals(true);
      editor->setDateTime(value);
      editor->blockSignals(false);
   }
}

void QtDateTimeEditFactoryPrivate::slotSetValue(const QDateTime &value)
{
   QObject *object = q_ptr->sender();
   auto ecend = m_editorToProperty.constEnd();

   for (auto itEditor = m_editorToProperty.constBegin(); itEditor != ecend;  ++itEditor)
      if (itEditor.key() == object) {
         QtProperty *property = itEditor.value();
         QtDateTimePropertyManager *manager = q_ptr->propertyManager(property);
         if (!manager) {
            return;
         }
         manager->setValue(property, value);
         return;
      }
}

QtDateTimeEditFactory::QtDateTimeEditFactory(QObject *parent)
   : QtAbstractEditorFactory<QtDateTimePropertyManager>(parent), d_ptr(new QtDateTimeEditFactoryPrivate())
{
   d_ptr->q_ptr = this;

}

QtDateTimeEditFactory::~QtDateTimeEditFactory()
{
   qDeleteAll(d_ptr->m_editorToProperty.keys());
}

void QtDateTimeEditFactory::connectPropertyManager(QtDateTimePropertyManager *manager)
{
   connect(manager, &QtDateTimePropertyManager::valueChanged,
      this, &QtDateTimeEditFactory::slotPropertyChanged);
}

QWidget *QtDateTimeEditFactory::createEditor(QtDateTimePropertyManager *manager,
   QtProperty *property, QWidget *parent)
{
   QDateTimeEdit *editor =  d_ptr->createEditor(property, parent);
   editor->setDisplayFormat(QtPropertyBrowserUtils::dateTimeFormat());
   editor->setDateTime(manager->value(property));

   connect(editor, &QDateTimeEdit::dateTimeChanged, this, &QtDateTimeEditFactory::slotSetValue);
   connect(editor, &QDateTimeEdit::destroyed,       this, &QtDateTimeEditFactory::slotEditorDestroyed);

   return editor;
}

void QtDateTimeEditFactory::disconnectPropertyManager(QtDateTimePropertyManager *manager)
{
   disconnect(manager, &QtDateTimePropertyManager::valueChanged,
         this, &QtDateTimeEditFactory::slotPropertyChanged);
}

class QtKeySequenceEditorFactoryPrivate: public EditorFactoryPrivate<QKeySequenceEdit>
{
   QtKeySequenceEditorFactory *q_ptr;
   Q_DECLARE_PUBLIC(QtKeySequenceEditorFactory)

 public:
   void slotPropertyChanged(QtProperty *property, const QKeySequence &value);
   void slotSetValue(const QKeySequence &value);
};

void QtKeySequenceEditorFactoryPrivate::slotPropertyChanged(QtProperty *property, const QKeySequence &value)
{
   if (!m_createdEditors.contains(property)) {
      return;
   }

   QListIterator<QKeySequenceEdit *> itEditor(m_createdEditors[property]);

   while (itEditor.hasNext()) {
      QKeySequenceEdit *editor = itEditor.next();
      editor->blockSignals(true);
      editor->setKeySequence(value);
      editor->blockSignals(false);
   }
}

void QtKeySequenceEditorFactoryPrivate::slotSetValue(const QKeySequence &value)
{
   QObject *object = q_ptr->sender();
   auto ecend = m_editorToProperty.constEnd();

   for (auto itEditor =  m_editorToProperty.constBegin(); itEditor != ecend; ++itEditor)
      if (itEditor.key() == object) {
         QtProperty *property = itEditor.value();
         QtKeySequencePropertyManager *manager = q_ptr->propertyManager(property);
         if (!manager) {
            return;
         }
         manager->setValue(property, value);
         return;
      }
}

QtKeySequenceEditorFactory::QtKeySequenceEditorFactory(QObject *parent)
   : QtAbstractEditorFactory<QtKeySequencePropertyManager>(parent), d_ptr(new QtKeySequenceEditorFactoryPrivate())
{
   d_ptr->q_ptr = this;

}

QtKeySequenceEditorFactory::~QtKeySequenceEditorFactory()
{
   qDeleteAll(d_ptr->m_editorToProperty.keys());
}

void QtKeySequenceEditorFactory::connectPropertyManager(QtKeySequencePropertyManager *manager)
{
   connect(manager, &QtKeySequencePropertyManager::valueChanged,
      this, &QtKeySequenceEditorFactory::slotPropertyChanged);
}

QWidget *QtKeySequenceEditorFactory::createEditor(QtKeySequencePropertyManager *manager,
   QtProperty *property, QWidget *parent)
{
   QKeySequenceEdit *editor = d_ptr->createEditor(property, parent);
   editor->setKeySequence(manager->value(property));

   connect(editor, &QKeySequenceEdit::keySequenceChanged,
      this, &QtKeySequenceEditorFactory::slotSetValue);

   connect(editor, &QKeySequenceEdit::destroyed,
      this, &QtKeySequenceEditorFactory::slotEditorDestroyed);

   return editor;
}

void QtKeySequenceEditorFactory::disconnectPropertyManager(QtKeySequencePropertyManager *manager)
{
   disconnect(manager, &QtKeySequencePropertyManager::valueChanged,
         this, &QtKeySequenceEditorFactory::slotPropertyChanged);
}

class QtCharEdit : public QWidget
{
   CS_OBJECT(QtCharEdit)

 public:
   QtCharEdit(QWidget *parent = nullptr);

   QChar value() const;
   bool eventFilter(QObject *o, QEvent *e);

   CS_SLOT_1(Public, void setValue(const QChar &value))
   CS_SLOT_2(setValue)

   CS_SIGNAL_1(Public, void valueChanged(const QChar &value))
   CS_SIGNAL_2(valueChanged, value)

 protected:
   void focusInEvent(QFocusEvent *e);
   void focusOutEvent(QFocusEvent *e);
   void keyPressEvent(QKeyEvent *e);
   void keyReleaseEvent(QKeyEvent *e);
   bool event(QEvent *e);

 private:
   CS_SLOT_1(Private, void slotClearChar())
   CS_SLOT_2(slotClearChar)

   void handleKeyEvent(QKeyEvent *e);

   QChar m_value;
   QLineEdit *m_lineEdit;
};

QtCharEdit::QtCharEdit(QWidget *parent)
   : QWidget(parent),  m_lineEdit(new QLineEdit(this))
{
   QHBoxLayout *layout = new QHBoxLayout(this);
   layout->addWidget(m_lineEdit);
   layout->setMargin(0);
   m_lineEdit->installEventFilter(this);
   m_lineEdit->setReadOnly(true);
   m_lineEdit->setFocusProxy(this);
   setFocusPolicy(m_lineEdit->focusPolicy());
   setAttribute(Qt::WA_InputMethodEnabled);
}

bool QtCharEdit::eventFilter(QObject *o, QEvent *e)
{
   if (o == m_lineEdit && e->type() == QEvent::ContextMenu) {
      QContextMenuEvent *c = static_cast<QContextMenuEvent *>(e);
      QMenu *menu = m_lineEdit->createStandardContextMenu();
      QList<QAction *> actions = menu->actions();
      QListIterator<QAction *> itAction(actions);
      while (itAction.hasNext()) {
         QAction *action = itAction.next();
         action->setShortcut(QKeySequence());
         QString actionString = action->text();
         const int pos = actionString.lastIndexOf(QLatin1Char('\t'));
         if (pos > 0) {
            actionString = actionString.remove(pos, actionString.length() - pos);
         }
         action->setText(actionString);
      }

      QAction *actionBefore = nullptr;
      if (actions.count() > 0) {
         actionBefore = actions[0];
      }

      QAction *clearAction = new QAction(tr("Clear Char"), menu);
      menu->insertAction(actionBefore, clearAction);
      menu->insertSeparator(actionBefore);
      clearAction->setEnabled(!m_value.isNull());

      connect(clearAction, &QAction::triggered, this, &QtCharEdit::slotClearChar);

      menu->exec(c->globalPos());
      delete menu;
      e->accept();
      return true;
   }

   return QWidget::eventFilter(o, e);
}

void QtCharEdit::slotClearChar()
{
   if (m_value.isNull()) {
      return;
   }
   setValue(QChar());
   emit valueChanged(m_value);
}

void QtCharEdit::handleKeyEvent(QKeyEvent *e)
{
   const int key = e->key();
   switch (key) {
      case Qt::Key_Control:
      case Qt::Key_Shift:
      case Qt::Key_Meta:
      case Qt::Key_Alt:
      case Qt::Key_Super_L:
      case Qt::Key_Return:
         return;
      default:
         break;
   }

   const QString text = e->text();
   if (text.count() != 1) {
      return;
   }

   const QChar c = text.at(0);
   if (!c.isPrint()) {
      return;
   }

   if (m_value == c) {
      return;
   }

   m_value = c;
   const QString str = m_value.isNull() ? QString() : QString(m_value);
   m_lineEdit->setText(str);
   e->accept();
   emit valueChanged(m_value);
}

void QtCharEdit::setValue(const QChar &value)
{
   if (value == m_value) {
      return;
   }

   m_value = value;
   QString str = value.isNull() ? QString() : QString(value);
   m_lineEdit->setText(str);
}

QChar QtCharEdit::value() const
{
   return m_value;
}

void QtCharEdit::focusInEvent(QFocusEvent *e)
{
   m_lineEdit->event(e);
   m_lineEdit->selectAll();
   QWidget::focusInEvent(e);
}

void QtCharEdit::focusOutEvent(QFocusEvent *e)
{
   m_lineEdit->event(e);
   QWidget::focusOutEvent(e);
}

void QtCharEdit::keyPressEvent(QKeyEvent *e)
{
   handleKeyEvent(e);
   e->accept();
}

void QtCharEdit::keyReleaseEvent(QKeyEvent *e)
{
   m_lineEdit->event(e);
}

bool QtCharEdit::event(QEvent *e)
{
   switch (e->type()) {
      case QEvent::Shortcut:
      case QEvent::ShortcutOverride:
      case QEvent::KeyRelease:
         e->accept();
         return true;
      default:
         break;
   }
   return QWidget::event(e);
}

// QtCharEditorFactory

class QtCharEditorFactoryPrivate: public EditorFactoryPrivate<QtCharEdit>
{
   QtCharEditorFactory *q_ptr;
   Q_DECLARE_PUBLIC(QtCharEditorFactory)
 public:

   void slotPropertyChanged(QtProperty *property, const QChar &value);
   void slotSetValue(const QChar &value);

};

void QtCharEditorFactoryPrivate::slotPropertyChanged(QtProperty *property,
   const QChar &value)
{
   if (!m_createdEditors.contains(property)) {
      return;
   }

   QListIterator<QtCharEdit *> itEditor(m_createdEditors[property]);
   while (itEditor.hasNext()) {
      QtCharEdit *editor = itEditor.next();
      editor->blockSignals(true);
      editor->setValue(value);
      editor->blockSignals(false);
   }
}

void QtCharEditorFactoryPrivate::slotSetValue(const QChar &value)
{
   QObject *object = q_ptr->sender();
   auto ecend = m_editorToProperty.constEnd();

   for (auto itEditor = m_editorToProperty.constBegin(); itEditor != ecend;  ++itEditor)
      if (itEditor.key() == object) {
         QtProperty *property = itEditor.value();
         QtCharPropertyManager *manager = q_ptr->propertyManager(property);
         if (!manager) {
            return;
         }
         manager->setValue(property, value);
         return;
      }
}

QtCharEditorFactory::QtCharEditorFactory(QObject *parent)
   : QtAbstractEditorFactory<QtCharPropertyManager>(parent), d_ptr(new QtCharEditorFactoryPrivate())
{
   d_ptr->q_ptr = this;
}

QtCharEditorFactory::~QtCharEditorFactory()
{
   qDeleteAll(d_ptr->m_editorToProperty.keys());
}

void QtCharEditorFactory::connectPropertyManager(QtCharPropertyManager *manager)
{
   connect(manager, &QtCharPropertyManager::valueChanged, this, &QtCharEditorFactory::slotPropertyChanged);
}

QWidget *QtCharEditorFactory::createEditor(QtCharPropertyManager *manager,
   QtProperty *property, QWidget *parent)
{
   QtCharEdit *editor = d_ptr->createEditor(property, parent);
   editor->setValue(manager->value(property));

   connect(editor, &QtCharEdit::valueChanged, this, &QtCharEditorFactory::slotSetValue);
   connect(editor, &QtCharEdit::destroyed,    this, &QtCharEditorFactory::slotEditorDestroyed);

   return editor;
}

void QtCharEditorFactory::disconnectPropertyManager(QtCharPropertyManager *manager)
{
   disconnect(manager, &QtCharPropertyManager::valueChanged, this, &QtCharEditorFactory::slotPropertyChanged);
}

class QtEnumEditorFactoryPrivate: public EditorFactoryPrivate<QComboBox>
{
   QtEnumEditorFactory *q_ptr;
   Q_DECLARE_PUBLIC(QtEnumEditorFactory)

 public:
   void slotPropertyChanged(QtProperty *property, int value);
   void slotEnumNamesChanged(QtProperty *property, const QStringList &);
   void slotEnumIconsChanged(QtProperty *property, const QMap<int, QIcon> &);
   void slotSetValue(int value);
};

void QtEnumEditorFactoryPrivate::slotPropertyChanged(QtProperty *property, int value)
{
   if (! m_createdEditors.contains(property)) {
      return;
   }

   QListIterator<QComboBox *> itEditor(m_createdEditors[property]);

   while (itEditor.hasNext()) {
      QComboBox *editor = itEditor.next();
      editor->blockSignals(true);
      editor->setCurrentIndex(value);
      editor->blockSignals(false);
   }
}

void QtEnumEditorFactoryPrivate::slotEnumNamesChanged(QtProperty *property, const QStringList &enumNames)
{
   if (! m_createdEditors.contains(property)) {
      return;
   }

   QtEnumPropertyManager *manager = q_ptr->propertyManager(property);
   if (! manager) {
      return;
   }

   QMap<int, QIcon> enumIcons = manager->enumIcons(property);

   QListIterator<QComboBox *> itEditor(m_createdEditors[property]);

   while (itEditor.hasNext()) {
      QComboBox *editor = itEditor.next();
      editor->blockSignals(true);
      editor->clear();
      editor->addItems(enumNames);

      const int nameCount = enumNames.count();

      for (int i = 0; i < nameCount; i++) {
         editor->setItemIcon(i, enumIcons.value(i));
      }

      editor->setCurrentIndex(manager->value(property));
      editor->blockSignals(false);
   }
}

void QtEnumEditorFactoryPrivate::slotEnumIconsChanged(QtProperty *property,
   const QMap<int, QIcon> &enumIcons)
{
   if (!m_createdEditors.contains(property)) {
      return;
   }

   QtEnumPropertyManager *manager = q_ptr->propertyManager(property);
   if (!manager) {
      return;
   }

   const QStringList enumNames = manager->enumNames(property);
   QListIterator<QComboBox *> itEditor(m_createdEditors[property]);

   while (itEditor.hasNext()) {
      QComboBox *editor = itEditor.next();
      editor->blockSignals(true);
      const int nameCount = enumNames.count();

      for (int i = 0; i < nameCount; i++) {
         editor->setItemIcon(i, enumIcons.value(i));
      }

      editor->setCurrentIndex(manager->value(property));
      editor->blockSignals(false);
   }
}

void QtEnumEditorFactoryPrivate::slotSetValue(int value)
{
   QObject *object = q_ptr->sender();
   auto ecend = m_editorToProperty.constEnd();

   for (auto itEditor = m_editorToProperty.constBegin(); itEditor != ecend; ++itEditor)
      if (itEditor.key() == object) {
         QtProperty *property = itEditor.value();
         QtEnumPropertyManager *manager = q_ptr->propertyManager(property);
         if (!manager) {
            return;
         }
         manager->setValue(property, value);
         return;
      }
}

QtEnumEditorFactory::QtEnumEditorFactory(QObject *parent)
   : QtAbstractEditorFactory<QtEnumPropertyManager>(parent), d_ptr(new QtEnumEditorFactoryPrivate())
{
   d_ptr->q_ptr = this;
}


QtEnumEditorFactory::~QtEnumEditorFactory()
{
   qDeleteAll(d_ptr->m_editorToProperty.keys());
}

void QtEnumEditorFactory::connectPropertyManager(QtEnumPropertyManager *manager)
{
   connect(manager, &QtEnumPropertyManager::valueChanged,
      this, &QtEnumEditorFactory::slotPropertyChanged);

   connect(manager, &QtEnumPropertyManager::enumNamesChanged,
      this, &QtEnumEditorFactory::slotEnumNamesChanged);
}

QWidget *QtEnumEditorFactory::createEditor(QtEnumPropertyManager *manager, QtProperty *property,
   QWidget *parent)
{
   QComboBox *editor = d_ptr->createEditor(property, parent);
   editor->setSizePolicy(QSizePolicy::Ignored, QSizePolicy::Fixed);
   editor->view()->setTextElideMode(Qt::ElideRight);

   QStringList enumNames = manager->enumNames(property);
   editor->addItems(enumNames);

   QMap<int, QIcon> enumIcons = manager->enumIcons(property);
   const int enumNamesCount  = enumNames.count();

   for (int i = 0; i < enumNamesCount; ++i) {
      editor->setItemIcon(i, enumIcons.value(i));
   }
   editor->setCurrentIndex(manager->value(property));

   connect(editor, cs_mp_cast<int>(&QComboBox::currentIndexChanged),
         this, &QtEnumEditorFactory::slotSetValue);

   connect(editor, &QComboBox::destroyed,
         this, &QtEnumEditorFactory::slotEditorDestroyed);

   return editor;
}

void QtEnumEditorFactory::disconnectPropertyManager(QtEnumPropertyManager *manager)
{
   disconnect(manager, &QtEnumPropertyManager::valueChanged,
         this, &QtEnumEditorFactory::slotPropertyChanged);

   disconnect(manager, &QtEnumPropertyManager::enumNamesChanged,
         this, &QtEnumEditorFactory::slotEnumNamesChanged);
}

Q_GLOBAL_STATIC(QtCursorDatabase, cursorDatabase)

class QtCursorEditorFactoryPrivate
{
   QtCursorEditorFactory *q_ptr;
   Q_DECLARE_PUBLIC(QtCursorEditorFactory)

 public:
   QtCursorEditorFactoryPrivate();

   void slotPropertyChanged(QtProperty *property, const QCursor &cursor);
   void slotEnumChanged(QtProperty *property, int value);
   void slotEditorDestroyed(QObject *object);

   QtEnumEditorFactory *m_enumEditorFactory;
   QtEnumPropertyManager *m_enumPropertyManager;

   QMap<QtProperty *, QtProperty *> m_propertyToEnum;
   QMap<QtProperty *, QtProperty *> m_enumToProperty;
   QMap<QtProperty *, QList<QWidget *>> m_enumToEditors;
   QMap<QWidget *, QtProperty *> m_editorToEnum;
   bool m_updatingEnum;
};

QtCursorEditorFactoryPrivate::QtCursorEditorFactoryPrivate()
   : m_updatingEnum(false)
{

}

void QtCursorEditorFactoryPrivate::slotPropertyChanged(QtProperty *property, const QCursor &cursor)
{
   // update enum property
   QtProperty *enumProp = m_propertyToEnum.value(property);
   if (!enumProp) {
      return;
   }

   m_updatingEnum = true;
   m_enumPropertyManager->setValue(enumProp, cursorDatabase()->cursorToValue(cursor));
   m_updatingEnum = false;
}

void QtCursorEditorFactoryPrivate::slotEnumChanged(QtProperty *property, int value)
{
   if (m_updatingEnum) {
      return;
   }
   // update cursor property
   QtProperty *prop = m_enumToProperty.value(property);
   if (!prop) {
      return;
   }

   QtCursorPropertyManager *cursorManager = q_ptr->propertyManager(prop);

   if (!cursorManager) {
      return;
   }

   cursorManager->setValue(prop, QCursor(cursorDatabase()->valueToCursor(value)));
}

void QtCursorEditorFactoryPrivate::slotEditorDestroyed(QObject *object)
{
   // remove from m_editorToEnum map;
   // remove from m_enumToEditors map;
   // if m_enumToEditors doesn't contains more editors delete enum property;

   auto ecend = m_editorToEnum.constEnd();

   for (auto itEditor = m_editorToEnum.constBegin(); itEditor != ecend; ++itEditor)
      if (itEditor.key() == object) {
         QWidget *editor = itEditor.key();
         QtProperty *enumProp = itEditor.value();
         m_editorToEnum.remove(editor);
         m_enumToEditors[enumProp].removeAll(editor);

         if (m_enumToEditors[enumProp].isEmpty()) {
            m_enumToEditors.remove(enumProp);
            QtProperty *property = m_enumToProperty.value(enumProp);
            m_enumToProperty.remove(enumProp);
            m_propertyToEnum.remove(property);
            delete enumProp;
         }
         return;
      }
}

QtCursorEditorFactory::QtCursorEditorFactory(QObject *parent)
   : QtAbstractEditorFactory<QtCursorPropertyManager>(parent), d_ptr(new QtCursorEditorFactoryPrivate())
{
   d_ptr->q_ptr = this;

   d_ptr->m_enumEditorFactory   = new QtEnumEditorFactory(this);
   d_ptr->m_enumPropertyManager = new QtEnumPropertyManager(this);

   connect(d_ptr->m_enumPropertyManager, &QtEnumPropertyManager::valueChanged,
         this, &QtCursorEditorFactory::slotEnumChanged);

   d_ptr->m_enumEditorFactory->addPropertyManager(d_ptr->m_enumPropertyManager);
}

QtCursorEditorFactory::~QtCursorEditorFactory()
{
}

void QtCursorEditorFactory::connectPropertyManager(QtCursorPropertyManager *manager)
{
   connect(manager, &QtCursorPropertyManager::valueChanged, this, &QtCursorEditorFactory::slotPropertyChanged);
}

QWidget *QtCursorEditorFactory::createEditor(QtCursorPropertyManager *manager, QtProperty *property,
   QWidget *parent)
{
   QtProperty *enumProp = nullptr;

   if (d_ptr->m_propertyToEnum.contains(property)) {
      enumProp = d_ptr->m_propertyToEnum[property];
   } else {
      enumProp = d_ptr->m_enumPropertyManager->addProperty(property->propertyName());
      d_ptr->m_enumPropertyManager->setEnumNames(enumProp, cursorDatabase()->cursorShapeNames());
      d_ptr->m_enumPropertyManager->setEnumIcons(enumProp, cursorDatabase()->cursorShapeIcons());

      d_ptr->m_enumPropertyManager->setValue(enumProp, cursorDatabase()->cursorToValue(manager->value(property)));

      d_ptr->m_propertyToEnum[property] = enumProp;
      d_ptr->m_enumToProperty[enumProp] = property;
   }

   QtAbstractEditorFactoryBase *af = d_ptr->m_enumEditorFactory;

   QWidget *editor = af->createEditor(enumProp, parent);
   d_ptr->m_enumToEditors[enumProp].append(editor);
   d_ptr->m_editorToEnum[editor] = enumProp;

   connect(editor, &QWidget::destroyed, this, &QtCursorEditorFactory::slotEditorDestroyed);

   return editor;
}

void QtCursorEditorFactory::disconnectPropertyManager(QtCursorPropertyManager *manager)
{
   disconnect(manager, &QtCursorPropertyManager::valueChanged, this, &QtCursorEditorFactory::slotPropertyChanged);
}

class QtColorEditWidget : public QWidget
{
   CS_OBJECT(QtColorEditWidget)

 public:
   QtColorEditWidget(QWidget *parent);

   bool eventFilter(QObject *obj, QEvent *ev);

   CS_SLOT_1(Public, void setValue(const QColor &value))
   CS_SLOT_2(setValue)

   CS_SIGNAL_1(Public, void valueChanged(const QColor &value))
   CS_SIGNAL_2(valueChanged, value)

 private:
   CS_SLOT_1(Private, void buttonClicked())
   CS_SLOT_2(buttonClicked)

   QColor m_color;
   QLabel *m_pixmapLabel;
   QLabel *m_label;
   QToolButton *m_button;
};

QtColorEditWidget::QtColorEditWidget(QWidget *parent)
   : QWidget(parent), m_pixmapLabel(new QLabel), m_label(new QLabel), m_button(new QToolButton)
{
   QHBoxLayout *lt = new QHBoxLayout(this);
   setupTreeViewEditorMargin(lt);
   lt->setSpacing(0);
   lt->addWidget(m_pixmapLabel);
   lt->addWidget(m_label);
   lt->addItem(new QSpacerItem(0, 0, QSizePolicy::Expanding, QSizePolicy::Ignored));

   m_button->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Ignored);
   m_button->setFixedWidth(20);
   setFocusProxy(m_button);
   setFocusPolicy(m_button->focusPolicy());
   m_button->setText(tr("..."));
   m_button->installEventFilter(this);

   connect(m_button, &QToolButton::clicked, this, &QtColorEditWidget::buttonClicked);

   lt->addWidget(m_button);
   m_pixmapLabel->setPixmap(QtPropertyBrowserUtils::brushValuePixmap(QBrush(m_color)));
   m_label->setText(QtPropertyBrowserUtils::colorValueText(m_color));
}

void QtColorEditWidget::setValue(const QColor &c)
{
   if (m_color != c) {
      m_color = c;
      m_pixmapLabel->setPixmap(QtPropertyBrowserUtils::brushValuePixmap(QBrush(c)));
      m_label->setText(QtPropertyBrowserUtils::colorValueText(c));
   }
}

void QtColorEditWidget::buttonClicked()
{
   const QColor newColor = QColorDialog::getColor(m_color, this, QString(), QColorDialog::ShowAlphaChannel);
   if (newColor.isValid() && newColor != m_color) {
      setValue(newColor);
      emit valueChanged(m_color);
   }
}

bool QtColorEditWidget::eventFilter(QObject *obj, QEvent *ev)
{
   if (obj == m_button) {
      switch (ev->type()) {
         case QEvent::KeyPress:
         case QEvent::KeyRelease: { // Prevent the QToolButton from handling Enter/Escape meant control the delegate
            switch (static_cast<const QKeyEvent *>(ev)->key()) {
               case Qt::Key_Escape:
               case Qt::Key_Enter:
               case Qt::Key_Return:
                  ev->ignore();
                  return true;
               default:
                  break;
            }
         }
         break;
         default:
            break;
      }
   }
   return QWidget::eventFilter(obj, ev);
}

// QtColorEditorFactoryPrivate

class QtColorEditorFactoryPrivate: public EditorFactoryPrivate<QtColorEditWidget>
{
   QtColorEditorFactory *q_ptr;
   Q_DECLARE_PUBLIC(QtColorEditorFactory)
 public:

   void slotPropertyChanged(QtProperty *property, const QColor &value);
   void slotSetValue(const QColor &value);
};

void QtColorEditorFactoryPrivate::slotPropertyChanged(QtProperty *property,
   const QColor &value)
{
   const PropertyToEditorListMap::iterator it = m_createdEditors.find(property);
   if (it == m_createdEditors.end()) {
      return;
   }
   QListIterator<QtColorEditWidget *> itEditor(it.value());

   while (itEditor.hasNext()) {
      itEditor.next()->setValue(value);
   }
}

void QtColorEditorFactoryPrivate::slotSetValue(const QColor &value)
{
   QObject *object = q_ptr->sender();
   auto ecend = m_editorToProperty.constEnd();

   for (auto itEditor = m_editorToProperty.constBegin(); itEditor != ecend; ++itEditor)
      if (itEditor.key() == object) {
         QtProperty *property = itEditor.value();
         QtColorPropertyManager *manager = q_ptr->propertyManager(property);
         if (!manager) {
            return;
         }
         manager->setValue(property, value);
         return;
      }
}

QtColorEditorFactory::QtColorEditorFactory(QObject *parent)
   : QtAbstractEditorFactory<QtColorPropertyManager>(parent), d_ptr(new QtColorEditorFactoryPrivate())
{
   d_ptr->q_ptr = this;
}

QtColorEditorFactory::~QtColorEditorFactory()
{
   qDeleteAll(d_ptr->m_editorToProperty.keys());
}

void QtColorEditorFactory::connectPropertyManager(QtColorPropertyManager *manager)
{
   connect(manager, &QtColorPropertyManager::valueChanged, this, &QtColorEditorFactory::slotPropertyChanged);
}

QWidget *QtColorEditorFactory::createEditor(QtColorPropertyManager *manager,
   QtProperty *property, QWidget *parent)
{
   QtColorEditWidget *editor = d_ptr->createEditor(property, parent);
   editor->setValue(manager->value(property));

   connect(editor, &QtColorEditWidget::valueChanged, this, &QtColorEditorFactory::slotSetValue);
   connect(editor, &QtColorEditWidget::destroyed,    this, &QtColorEditorFactory::slotEditorDestroyed);

   return editor;
}

void QtColorEditorFactory::disconnectPropertyManager(QtColorPropertyManager *manager)
{
   disconnect(manager, &QtColorPropertyManager::valueChanged, this, &QtColorEditorFactory::slotPropertyChanged);
}

class QtFontEditWidget : public QWidget
{
   CS_OBJECT(QtFontEditWidget)

 public:
   QtFontEditWidget(QWidget *parent);

   bool eventFilter(QObject *obj, QEvent *ev);

   CS_SIGNAL_1(Public, void valueChanged(const QFont &value))
   CS_SIGNAL_2(valueChanged, value)

   CS_SLOT_1(Public, void setValue(const QFont &value))
   CS_SLOT_2(setValue)

 private:
   CS_SLOT_1(Private, void buttonClicked())
   CS_SLOT_2(buttonClicked)

   QFont m_font;
   QLabel *m_pixmapLabel;
   QLabel *m_label;
   QToolButton *m_button;
};

QtFontEditWidget::QtFontEditWidget(QWidget *parent)
   : QWidget(parent), m_pixmapLabel(new QLabel), m_label(new QLabel), m_button(new QToolButton)
{
   QHBoxLayout *lt = new QHBoxLayout(this);
   setupTreeViewEditorMargin(lt);
   lt->setSpacing(0);
   lt->addWidget(m_pixmapLabel);
   lt->addWidget(m_label);
   lt->addItem(new QSpacerItem(0, 0, QSizePolicy::Expanding, QSizePolicy::Ignored));

   m_button->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Ignored);
   m_button->setFixedWidth(20);
   setFocusProxy(m_button);
   setFocusPolicy(m_button->focusPolicy());
   m_button->setText(tr("..."));
   m_button->installEventFilter(this);

   connect(m_button, &QToolButton::clicked, this, &QtFontEditWidget::buttonClicked);

   lt->addWidget(m_button);
   m_pixmapLabel->setPixmap(QtPropertyBrowserUtils::fontValuePixmap(m_font));
   m_label->setText(QtPropertyBrowserUtils::fontValueText(m_font));
}

void QtFontEditWidget::setValue(const QFont &f)
{
   if (m_font != f) {
      m_font = f;
      m_pixmapLabel->setPixmap(QtPropertyBrowserUtils::fontValuePixmap(f));
      m_label->setText(QtPropertyBrowserUtils::fontValueText(f));
   }
}

void QtFontEditWidget::buttonClicked()
{
   bool ok = false;
   QFont newFont = QFontDialog::getFont(&ok, m_font, this, tr("Select Font"));
   if (ok && newFont != m_font) {
      QFont f = m_font;
      // prevent mask for unchanged attributes, don't change other attributes (like kerning, etc...)
      if (m_font.family() != newFont.family()) {
         f.setFamily(newFont.family());
      }
      if (m_font.pointSize() != newFont.pointSize()) {
         f.setPointSize(newFont.pointSize());
      }
      if (m_font.bold() != newFont.bold()) {
         f.setBold(newFont.bold());
      }
      if (m_font.italic() != newFont.italic()) {
         f.setItalic(newFont.italic());
      }
      if (m_font.underline() != newFont.underline()) {
         f.setUnderline(newFont.underline());
      }
      if (m_font.strikeOut() != newFont.strikeOut()) {
         f.setStrikeOut(newFont.strikeOut());
      }
      setValue(f);
      emit valueChanged(m_font);
   }
}

bool QtFontEditWidget::eventFilter(QObject *obj, QEvent *ev)
{
   if (obj == m_button) {
      switch (ev->type()) {
         case QEvent::KeyPress:

         case QEvent::KeyRelease: {
            // Prevent the QToolButton from handling Enter/Escape meant control the delegate

            switch (static_cast<const QKeyEvent *>(ev)->key()) {
               case Qt::Key_Escape:
               case Qt::Key_Enter:
               case Qt::Key_Return:
                  ev->ignore();
                  return true;

               default:
                  break;
            }
         }
         break;

         default:
            break;
      }
   }

   return QWidget::eventFilter(obj, ev);
}

// QtFontEditorFactoryPrivate

class QtFontEditorFactoryPrivate: public EditorFactoryPrivate<QtFontEditWidget>
{
   QtFontEditorFactory *q_ptr;
   Q_DECLARE_PUBLIC(QtFontEditorFactory)

 public:

   void slotPropertyChanged(QtProperty *property, const QFont &value);
   void slotSetValue(const QFont &value);
};

void QtFontEditorFactoryPrivate::slotPropertyChanged(QtProperty *property, const QFont &value)
{
   const PropertyToEditorListMap::iterator it = m_createdEditors.find(property);
   if (it == m_createdEditors.end()) {
      return;
   }
   QListIterator<QtFontEditWidget *> itEditor(it.value());

   while (itEditor.hasNext()) {
      itEditor.next()->setValue(value);
   }
}

void QtFontEditorFactoryPrivate::slotSetValue(const QFont &value)
{
   QObject *object = q_ptr->sender();
   auto ecend = m_editorToProperty.constEnd();

   for (auto itEditor = m_editorToProperty.constBegin(); itEditor != ecend; ++itEditor)
      if (itEditor.key() == object) {
         QtProperty *property = itEditor.value();
         QtFontPropertyManager *manager = q_ptr->propertyManager(property);
         if (!manager) {
            return;
         }
         manager->setValue(property, value);
         return;
      }
}

QtFontEditorFactory::QtFontEditorFactory(QObject *parent) :
   QtAbstractEditorFactory<QtFontPropertyManager>(parent),
   d_ptr(new QtFontEditorFactoryPrivate())
{
   d_ptr->q_ptr = this;
}

QtFontEditorFactory::~QtFontEditorFactory()
{
   qDeleteAll(d_ptr->m_editorToProperty.keys());
}

void QtFontEditorFactory::connectPropertyManager(QtFontPropertyManager *manager)
{
   connect(manager, &QtFontPropertyManager::valueChanged, this, &QtFontEditorFactory::slotPropertyChanged);
}

QWidget *QtFontEditorFactory::createEditor(QtFontPropertyManager *manager,
   QtProperty *property, QWidget *parent)
{
   QtFontEditWidget *editor = d_ptr->createEditor(property, parent);
   editor->setValue(manager->value(property));

   connect(editor, &QtFontEditWidget::valueChanged, this, &QtFontEditorFactory::slotSetValue);
   connect(editor, &QtFontEditWidget::destroyed,    this, &QtFontEditorFactory::slotEditorDestroyed);

   return editor;
}

void QtFontEditorFactory::disconnectPropertyManager(QtFontPropertyManager *manager)
{
   disconnect(manager, &QtFontPropertyManager::valueChanged, this, &QtFontEditorFactory::slotPropertyChanged);
}


// private slots
void QtSpinBoxFactory::slotPropertyChanged(QtProperty *un_named_arg1, int un_named_arg2)
{
   Q_D(QtSpinBoxFactory);
   d->slotPropertyChanged(un_named_arg1, un_named_arg2);
}

void QtSpinBoxFactory::slotRangeChanged(QtProperty *un_named_arg1, int un_named_arg2, int un_named_arg3)
{
   Q_D(QtSpinBoxFactory);
   d->slotRangeChanged(un_named_arg1, un_named_arg3, un_named_arg3);
}

void QtSpinBoxFactory::slotSingleStepChanged(QtProperty *un_named_arg1, int un_named_arg2)
{
   Q_D(QtSpinBoxFactory);
   d->slotSingleStepChanged(un_named_arg1, un_named_arg2);
}

void QtSpinBoxFactory::slotSetValue(int un_named_arg1)
{
   Q_D(QtSpinBoxFactory);
   d->slotSetValue(un_named_arg1);
}

void QtSpinBoxFactory::slotEditorDestroyed(QObject *un_named_arg1)
{
   Q_D(QtSpinBoxFactory);
   d->slotEditorDestroyed(un_named_arg1);
}

void QtSliderFactory::slotPropertyChanged(QtProperty *un_named_arg1, int un_named_arg2)
{
   Q_D(QtSliderFactory);
   d->slotPropertyChanged(un_named_arg1, un_named_arg2);
}

void QtSliderFactory::slotRangeChanged(QtProperty *un_named_arg1, int un_named_arg2, int un_named_arg3)
{
   Q_D(QtSliderFactory);
   d->slotRangeChanged(un_named_arg1, un_named_arg2, un_named_arg2);
}

void QtSliderFactory::slotSingleStepChanged(QtProperty *un_named_arg1, int un_named_arg2)
{
   Q_D(QtSliderFactory);
   d->slotSingleStepChanged(un_named_arg1, un_named_arg2);
}

void QtSliderFactory::slotSetValue(int un_named_arg1)
{
   Q_D(QtSliderFactory);
   d->slotSetValue(un_named_arg1);
}

void QtSliderFactory::slotEditorDestroyed(QObject *un_named_arg1)
{
   Q_D(QtSliderFactory);
   d->slotEditorDestroyed(un_named_arg1);
}

void QtScrollBarFactory::slotPropertyChanged(QtProperty *un_named_arg1, int un_named_arg2)
{
   Q_D(QtScrollBarFactory);
   d->slotPropertyChanged(un_named_arg1, un_named_arg2);
}

void QtScrollBarFactory::slotRangeChanged(QtProperty *un_named_arg1, int un_named_arg2, int un_named_arg3)
{
   Q_D(QtScrollBarFactory);
   d->slotRangeChanged(un_named_arg1, un_named_arg2, un_named_arg3);
}

void QtScrollBarFactory::slotSingleStepChanged(QtProperty *un_named_arg1, int un_named_arg2)
{
   Q_D(QtScrollBarFactory);
   d->slotSingleStepChanged(un_named_arg1, un_named_arg2);
}

void QtScrollBarFactory::slotSetValue(int un_named_arg1)
{
   Q_D(QtScrollBarFactory);
   d->slotSetValue(un_named_arg1);
}

void QtScrollBarFactory::slotEditorDestroyed(QObject *un_named_arg1)
{
   Q_D(QtScrollBarFactory);
   d->slotEditorDestroyed(un_named_arg1);
}

void QtCheckBoxFactory::slotPropertyChanged(QtProperty *un_named_arg1, bool un_named_arg2)
{
   Q_D(QtCheckBoxFactory);
   d->slotPropertyChanged(un_named_arg1, un_named_arg2);
}

void QtCheckBoxFactory::slotSetValue(bool un_named_arg1)
{
   Q_D(QtCheckBoxFactory);
   d->slotSetValue(un_named_arg1);
}

void QtCheckBoxFactory::slotEditorDestroyed(QObject *un_named_arg1)
{
   Q_D(QtCheckBoxFactory);
   d->slotEditorDestroyed(un_named_arg1);
}

void QtDoubleSpinBoxFactory::slotPropertyChanged(QtProperty *un_named_arg1, double un_named_arg2)
{
   Q_D(QtDoubleSpinBoxFactory);
   d->slotPropertyChanged(un_named_arg1, un_named_arg2);
}

void QtDoubleSpinBoxFactory::slotRangeChanged(QtProperty *un_named_arg1, double un_named_arg2, double un_named_arg3)
{
   Q_D(QtDoubleSpinBoxFactory);
   d->slotRangeChanged(un_named_arg1, un_named_arg2, un_named_arg3);
}

void QtDoubleSpinBoxFactory::slotSingleStepChanged(QtProperty *un_named_arg1, double un_named_arg2)
{
   Q_D(QtDoubleSpinBoxFactory);
   d->slotSingleStepChanged(un_named_arg1, un_named_arg2);
}

void QtDoubleSpinBoxFactory::slotDecimalsChanged(QtProperty *un_named_arg1, int un_named_arg2)
{
   Q_D(QtDoubleSpinBoxFactory);
   d->slotDecimalsChanged(un_named_arg1, un_named_arg2);
}

void QtDoubleSpinBoxFactory::slotSetValue(double un_named_arg1)
{
   Q_D(QtDoubleSpinBoxFactory);
   d->slotSetValue(un_named_arg1);
}

void QtDoubleSpinBoxFactory::slotEditorDestroyed(QObject *un_named_arg1)
{
   Q_D(QtDoubleSpinBoxFactory);
   d->slotEditorDestroyed(un_named_arg1);
}

void QtDateEditFactory::slotPropertyChanged(QtProperty *un_named_arg1, const QDate &un_named_arg2)
{
   Q_D(QtDateEditFactory);
   d->slotPropertyChanged(un_named_arg1, un_named_arg2);
}

void QtDateEditFactory::slotRangeChanged(QtProperty *un_named_arg1, const QDate &un_named_arg2, const QDate &un_named_arg3)
{
   Q_D(QtDateEditFactory);
   d->slotRangeChanged(un_named_arg1, un_named_arg2, un_named_arg2);
}

void QtDateEditFactory::slotSetValue(const QDate &un_named_arg1)
{
   Q_D(QtDateEditFactory);
   d->slotSetValue(un_named_arg1);
}

void QtDateEditFactory::slotEditorDestroyed(QObject *un_named_arg1)
{
   Q_D(QtDateEditFactory);
   d->slotEditorDestroyed(un_named_arg1);
}

void QtTimeEditFactory::slotPropertyChanged(QtProperty *un_named_arg1, const QTime &un_named_arg2)
{
   Q_D(QtTimeEditFactory);
   d->slotPropertyChanged(un_named_arg1, un_named_arg2);
}

void QtTimeEditFactory::slotSetValue(const QTime &un_named_arg1)
{
   Q_D(QtTimeEditFactory);
   d->slotSetValue(un_named_arg1);
}

void QtTimeEditFactory::slotEditorDestroyed(QObject *un_named_arg1)
{
   Q_D(QtTimeEditFactory);
   d->slotEditorDestroyed(un_named_arg1);
}

void QtDateTimeEditFactory::slotPropertyChanged(QtProperty *un_named_arg1, const QDateTime &un_named_arg2)
{
   Q_D(QtDateTimeEditFactory);
   d->slotPropertyChanged(un_named_arg1, un_named_arg2);
}

void QtDateTimeEditFactory::slotSetValue(const QDateTime &un_named_arg1)
{
   Q_D(QtDateTimeEditFactory);
   d->slotSetValue(un_named_arg1);
}

void QtDateTimeEditFactory::slotEditorDestroyed(QObject *un_named_arg1)
{
   Q_D(QtDateTimeEditFactory);
   d->slotEditorDestroyed(un_named_arg1);
}


void QtKeySequenceEditorFactory::slotPropertyChanged(QtProperty *un_named_arg1, const QKeySequence &un_named_arg2)
{
   Q_D(QtKeySequenceEditorFactory);
   d->slotPropertyChanged(un_named_arg1, un_named_arg2);
}

void QtKeySequenceEditorFactory::slotSetValue(const QKeySequence &un_named_arg1)
{
   Q_D(QtKeySequenceEditorFactory);
   d->slotSetValue(un_named_arg1);
}

void QtKeySequenceEditorFactory::slotEditorDestroyed(QObject *un_named_arg1)
{
   Q_D(QtKeySequenceEditorFactory);
   d->slotEditorDestroyed(un_named_arg1);
}

void QtCharEditorFactory::slotPropertyChanged(QtProperty *un_named_arg1, const QChar &un_named_arg2)
{
   Q_D(QtCharEditorFactory);
   d->slotPropertyChanged(un_named_arg1, un_named_arg2);
}

void QtCharEditorFactory::slotSetValue(const QChar &un_named_arg1)
{
   Q_D(QtCharEditorFactory);
   d->slotSetValue(un_named_arg1);
}

void QtCharEditorFactory::slotEditorDestroyed(QObject *un_named_arg1)
{
   Q_D(QtCharEditorFactory);
   d->slotEditorDestroyed(un_named_arg1);
}

void QtLineEditFactory::slotPropertyChanged(QtProperty *un_named_arg1, const QString &un_named_arg2)
{
   Q_D(QtLineEditFactory);
   d->slotPropertyChanged(un_named_arg1, un_named_arg2);
}

void QtLineEditFactory::slotRegExpChanged(QtProperty *un_named_arg1, const QRegularExpression &un_named_arg2)
{
   Q_D(QtLineEditFactory);
   d->slotRegExpChanged(un_named_arg1, un_named_arg2);
}

void QtLineEditFactory::slotSetValue(const QString &un_named_arg1)
{
   Q_D(QtLineEditFactory);
   d->slotSetValue(un_named_arg1);
}

void QtLineEditFactory::slotEditorDestroyed(QObject *un_named_arg1)
{
   Q_D(QtLineEditFactory);
   d->slotEditorDestroyed(un_named_arg1);
}

void QtEnumEditorFactory::slotPropertyChanged(QtProperty *un_named_arg1, int un_named_arg2)
{
   Q_D(QtEnumEditorFactory);
   d->slotPropertyChanged(un_named_arg1, un_named_arg2);
}

void QtEnumEditorFactory::slotEnumNamesChanged(QtProperty *un_named_arg1, const QStringList &un_named_arg2)
{
   Q_D(QtEnumEditorFactory);
   d->slotEnumNamesChanged(un_named_arg1, un_named_arg2);
}

void QtEnumEditorFactory::slotEnumIconsChanged(QtProperty *un_named_arg1, const QMap <int, QIcon> &un_named_arg2)
{
   Q_D(QtEnumEditorFactory);
   d->slotEnumIconsChanged(un_named_arg1, un_named_arg2);
}

void QtEnumEditorFactory::slotSetValue(int un_named_arg1)
{
   Q_D(QtEnumEditorFactory);
   d->slotSetValue(un_named_arg1);
}

void QtEnumEditorFactory::slotEditorDestroyed(QObject *un_named_arg1)
{
   Q_D(QtEnumEditorFactory);
   d->slotEditorDestroyed(un_named_arg1);
}

void QtCursorEditorFactory::slotPropertyChanged(QtProperty *un_named_arg1, const QCursor &un_named_arg2)
{
   Q_D(QtCursorEditorFactory);
   d->slotPropertyChanged(un_named_arg1, un_named_arg2);
}

void QtCursorEditorFactory::slotEnumChanged(QtProperty *un_named_arg1, int un_named_arg2)
{
   Q_D(QtCursorEditorFactory);
   d->slotEnumChanged(un_named_arg1, un_named_arg2);
}

void QtCursorEditorFactory::slotEditorDestroyed(QObject *un_named_arg1)
{
   Q_D(QtCursorEditorFactory);
   d->slotEditorDestroyed(un_named_arg1);
}

void QtColorEditorFactory::slotPropertyChanged(QtProperty *un_named_arg1, const QColor &un_named_arg2)
{
   Q_D(QtColorEditorFactory);
   d->slotPropertyChanged(un_named_arg1, un_named_arg2);
}

void QtColorEditorFactory::slotEditorDestroyed(QObject *un_named_arg1)
{
   Q_D(QtColorEditorFactory);
   d->slotEditorDestroyed(un_named_arg1);
}

void QtColorEditorFactory::slotSetValue(const QColor &un_named_arg1)
{
   Q_D(QtColorEditorFactory);
   d->slotSetValue(un_named_arg1);
}

void QtFontEditorFactory::slotPropertyChanged(QtProperty *un_named_arg1, const QFont &un_named_arg2)
{
   Q_D(QtFontEditorFactory);
   d->slotPropertyChanged(un_named_arg1, un_named_arg2);
}

void QtFontEditorFactory::slotEditorDestroyed(QObject *un_named_arg1)
{
   Q_D(QtFontEditorFactory);
   d->slotEditorDestroyed(un_named_arg1);
}

void QtFontEditorFactory::slotSetValue(const QFont &un_named_arg1)
{
   Q_D(QtFontEditorFactory);
   d->slotSetValue(un_named_arg1);
}