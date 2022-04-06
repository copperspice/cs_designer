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

#include <abstract_formeditor.h>
#include <abstract_introspection.h>

#include <designer_membersheet.h>

#include <QWidget>

static QList<QByteArray> stringListToByteArray(const QStringList &l)
{
   if (l.empty()) {
      return QList<QByteArray>();
   }
   QList<QByteArray> rc;
   const QStringList::const_iterator cend = l.constEnd();
   for (QStringList::const_iterator it = l.constBegin(); it != cend; ++it) {
      rc += it->toUtf8();
   }
   return rc;
}

// Find the form editor in the hierarchy.
// We know that the parent of the sheet is the extension manager
// whose parent is the core

static QDesignerFormEditorInterface *formEditorForObject(QObject *obj)
{
   do {
      if (QDesignerFormEditorInterface *core = dynamic_cast<QDesignerFormEditorInterface *>(obj)) {
         return core;
      }

      obj = obj->parent();

   } while (obj);

   Q_ASSERT(obj);

   return nullptr;
}

// ------------ QDesignerMemberSheetPrivate
class QDesignerMemberSheetPrivate
{
 public:
   explicit QDesignerMemberSheetPrivate(QObject *object, QObject *sheetParent);

   QDesignerFormEditorInterface *m_core;
   const QDesignerMetaObjectInterface *m_meta;

   class Info
   {
    public:
      inline Info() : visible(true) {}

      QString group;
      bool visible;
   };

   Info &ensureInfo(int index);
   QHash<int, Info> m_info;
};

QDesignerMemberSheetPrivate::QDesignerMemberSheetPrivate(QObject *object, QObject *sheetParent) :
   m_core(formEditorForObject(sheetParent)),
   m_meta(m_core->introspection()->metaObject(object))
{
}

QDesignerMemberSheetPrivate::Info &QDesignerMemberSheetPrivate::ensureInfo(int index)
{
   auto it = m_info.find(index);

   if (it == m_info.end()) {
      it = m_info.insert(index, Info());
   }

   return it.value();
}

// --------- QDesignerMemberSheet

QDesignerMemberSheet::QDesignerMemberSheet(QObject *object, QObject *parent) :
   QObject(parent),
   d(new QDesignerMemberSheetPrivate(object, parent))
{
}

QDesignerMemberSheet::~QDesignerMemberSheet()
{
   delete d;
}

int QDesignerMemberSheet::count() const
{
   return d->m_meta->methodCount();
}

int QDesignerMemberSheet::indexOf(const QString &name) const
{
   return d->m_meta->indexOfMethod(name);
}

QString QDesignerMemberSheet::memberName(int index) const
{
   return d->m_meta->method(index)->tag();
}

QString QDesignerMemberSheet::declaredInClass(int index) const
{
   const QString member = d->m_meta->method(index)->signature();

   // Find class whose superclass does not contain the method.
   const QDesignerMetaObjectInterface *meta_obj = d->m_meta;

   for (;;) {
      const QDesignerMetaObjectInterface *tmp = meta_obj->superClass();
      if (tmp == nullptr) {
         break;
      }
      if (tmp->indexOfMethod(member) == -1) {
         break;
      }
      meta_obj = tmp;
   }
   return meta_obj->className();
}

QString QDesignerMemberSheet::memberGroup(int index) const
{
   return d->m_info.value(index).group;
}

void QDesignerMemberSheet::setMemberGroup(int index, const QString &group)
{
   d->ensureInfo(index).group = group;
}

QString QDesignerMemberSheet::signature(int index) const
{
   return d->m_meta->method(index)->normalizedSignature();
}

bool QDesignerMemberSheet::isVisible(int index) const
{
   auto it = d->m_info.constFind(index);

   if (it != d->m_info.constEnd()) {
      return it.value().visible;
   }

   return d->m_meta->method(index)->methodType() == QDesignerMetaMethodInterface::Signal
      || d->m_meta->method(index)->access() == QDesignerMetaMethodInterface::Public;
}

void QDesignerMemberSheet::setVisible(int index, bool visible)
{
   d->ensureInfo(index).visible = visible;
}

bool QDesignerMemberSheet::isSignal(int index) const
{
   return d->m_meta->method(index)->methodType() == QDesignerMetaMethodInterface::Signal;
}

bool QDesignerMemberSheet::isSlot(int index) const
{
   return d->m_meta->method(index)->methodType() == QDesignerMetaMethodInterface::Slot;
}

bool QDesignerMemberSheet::inheritedFromWidget(int index) const
{
   const QString name = d->m_meta->method(index)->signature();
   return declaredInClass(index) == QString("QWidget") || declaredInClass(index) == QString("QObject");
}


QList<QByteArray> QDesignerMemberSheet::parameterTypes(int index) const
{
   return stringListToByteArray(d->m_meta->method(index)->parameterTypes());
}

QList<QByteArray> QDesignerMemberSheet::parameterNames(int index) const
{
   return stringListToByteArray(d->m_meta->method(index)->parameterNames());
}

bool QDesignerMemberSheet::signalMatchesSlot(const QString &signal, const QString &slot)
{
   bool result = true;

   do {
      int signal_idx = signal.indexOf(QLatin1Char('('));
      int slot_idx = slot.indexOf(QLatin1Char('('));
      if (signal_idx == -1 || slot_idx == -1) {
         break;
      }

      ++signal_idx;
      ++slot_idx;

      if (slot.at(slot_idx) == QLatin1Char(')')) {
         break;
      }

      while (signal_idx < signal.size() && slot_idx < slot.size()) {
         const QChar signal_c = signal.at(signal_idx);
         const QChar slot_c = slot.at(slot_idx);

         if (signal_c == QLatin1Char(',') && slot_c == QLatin1Char(')')) {
            break;
         }

         if (signal_c == QLatin1Char(')') && slot_c == QLatin1Char(')')) {
            break;
         }

         if (signal_c != slot_c) {
            result = false;
            break;
         }

         ++signal_idx;
         ++slot_idx;
      }
   } while (false);

   return result;
}

// ------------ QDesignerMemberSheetFactory

QDesignerMemberSheetFactory::QDesignerMemberSheetFactory(QExtensionManager *parent)
   : QExtensionFactory(parent)
{
}

QObject *QDesignerMemberSheetFactory::createExtension(QObject *object, const QString &iid, QObject *parent) const
{
   if (iid == CS_TYPEID(QDesignerMemberSheetExtension)) {
      return new QDesignerMemberSheet(object, parent);
   }

   return nullptr;
}

