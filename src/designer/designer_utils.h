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

#ifndef QDESIGNER_UTILS_H
#define QDESIGNER_UTILS_H

#include <abstract_formwindow.h>
#include <utils.h>

#include <QDebug>
#include <QIcon>
#include <QMainWindow>
#include <QMap>
#include <QPixmap>
#include <QSharedDataPointer>
#include <QVariant>

namespace qdesigner_internal {

class QDesignerFormWindowCommand;
class DesignerIconCache;
class FormWindowBase;
class PropertySheetIconValueData;

void reloadIconResources(DesignerIconCache *iconCache, QObject *object);

template <class IntType>
class MetaEnum
{
 public:
   MetaEnum(const QString &name, const QString &scope, const QString &separator);
   MetaEnum() { }

   void addKey(IntType value, const QString &name);

   QString valueToKey(IntType value, bool *ok = nullptr) const;
   IntType keyToValue(const QString &key, bool *ok = nullptr) const;

   const QString &name() const      {
      return m_name;
   }

   const QString &scope() const     {
      return m_scope;
   }

   const QString &separator() const {
      return m_separator;
   }

   const QStringList &keys() const {
      return m_keys;
   }

   const QMap<QString, IntType> &keyToValueMap() const {
      return m_keyToValueMap;
   }

 protected:
   void appendQualifiedName(const QString &key, QString &target) const;

 private:
   QString m_name;
   QString m_scope;
   QString m_separator;

   QMap<QString, IntType> m_keyToValueMap;
   QStringList m_keys;
};

template <class IntType>
MetaEnum<IntType>::MetaEnum(const QString &name, const QString &scope, const QString &separator)
   : m_name(name), m_scope(scope), m_separator(separator)
{
}

template <class IntType>
void MetaEnum<IntType>::addKey(IntType value, const QString &name)
{
   m_keyToValueMap.insert(name, value);
   m_keys.append(name);
}

template <class IntType>
QString MetaEnum<IntType>::valueToKey(IntType value, bool *ok) const
{
   const QString rc = m_keyToValueMap.key(value);

   if (ok) {
      *ok = !rc.isEmpty();
   }

   return rc;
}

template <class IntType>
IntType MetaEnum<IntType>::keyToValue(const QString &key, bool *ok) const
{
   QStringView strView = key;

   if (! m_scope.isEmpty() && strView.startsWith(m_scope)) {
      strView = strView.mid(m_scope.size() + m_separator.size());
   }

   IntType retval = 0;
   bool found     = false;

   auto iter = m_keyToValueMap.find(strView);

   if (iter != m_keyToValueMap.constEnd()) {
      retval = iter.value();
      found  = true;
   }

   if (ok != nullptr) {
      *ok = found;
   }

   return retval;
}

template <class IntType>
void MetaEnum<IntType>::appendQualifiedName(const QString &key, QString &target) const
{
   if (!m_scope.isEmpty()) {
      target += m_scope;
      target += m_separator;
   }
   target += key;
}

// -------------- DesignerMetaEnum: Meta type for enumerations

class DesignerMetaEnum : public MetaEnum<int>
{
 public:
   DesignerMetaEnum(const QString &name, const QString &scope, const QString &separator);

   DesignerMetaEnum()
   {
   }

   enum SerializationMode { FullyQualified, NameOnly };
   QString toString(int value, SerializationMode sm, bool *ok = nullptr) const;

   void messageToStringFailed(int value) const;
   void messageParseFailed(const QString &s) const;

   // parse a string
   int parseEnum(const QString &s, bool *ok = nullptr) const {
      return keyToValue(s, ok);
   }
};

// -------------- DesignerMetaFlags: Meta type for flags

class DesignerMetaFlags : public MetaEnum<uint>
{
 public:
   DesignerMetaFlags(const QString &name, const QString &scope, const QString &separator);

   DesignerMetaFlags()
   {
   }

   enum SerializationMode {
      FullyQualified,
      NameOnly
   };

   QString toString(int value, SerializationMode sm) const;
   QStringList flags(int value) const;

   void messageParseFailed(const QString &s) const;

   // parse a string (ignore scopes)
   int parseFlags(const QString &s, bool *ok = nullptr) const;
};

// -------------- EnumValue: Returned by the property sheet for enums

struct PropertySheetEnumValue {
   PropertySheetEnumValue(int v, const DesignerMetaEnum &me);
   PropertySheetEnumValue();

   int value;
   DesignerMetaEnum metaEnum;
};

// -------------- FlagValue: Returned by the property sheet for flags

struct PropertySheetFlagValue {
   PropertySheetFlagValue(int v, const DesignerMetaFlags &mf);
   PropertySheetFlagValue();

   int value;
   DesignerMetaFlags metaFlags;
};

// -------------- PixmapValue: Returned by the property sheet for pixmaps
class PropertySheetPixmapValue
{
 public:
   PropertySheetPixmapValue(const QString &path);
   PropertySheetPixmapValue();

   bool operator==(const PropertySheetPixmapValue &other) const {
      return compare(other) == 0;
   }

   bool operator!=(const PropertySheetPixmapValue &other) const {
      return compare(other) != 0;
   }

   bool operator<(const PropertySheetPixmapValue &other) const  {
      return compare(other) <  0;
   }

   // Check where a pixmap comes from
   enum PixmapSource { LanguageResourcePixmap, ResourcePixmap, FilePixmap };
   static PixmapSource getPixmapSource(QDesignerFormEditorInterface *core, const QString &path);

   PixmapSource pixmapSource(QDesignerFormEditorInterface *core) const {
      return getPixmapSource(core, m_path);
   }

   QString path() const;
   void setPath(const QString &path); // passing the empty path resets the pixmap

   int compare(const PropertySheetPixmapValue &other) const;

 private:
   QString m_path;
};

// -------------- IconValue: Returned by the property sheet for icons

class PropertySheetIconValue
{
 public:
   PropertySheetIconValue(const PropertySheetPixmapValue &pixmap);
   PropertySheetIconValue();
   ~PropertySheetIconValue();

   PropertySheetIconValue(const PropertySheetIconValue &);
   PropertySheetIconValue &operator=(const PropertySheetIconValue &);

   bool operator==(const PropertySheetIconValue &other) const {
      return equals(other);
   }
   bool operator!=(const PropertySheetIconValue &other) const {
      return !equals(other);
   }
   bool operator<(const PropertySheetIconValue &other) const;

   bool isEmpty() const;

   QString theme() const;
   void setTheme(const QString &);

   PropertySheetPixmapValue pixmap(QIcon::Mode mode, QIcon::State state) const;
   void setPixmap(QIcon::Mode mode, QIcon::State state, const PropertySheetPixmapValue &path); // passing the empty path resets the pixmap

   uint mask() const;
   uint compare(const PropertySheetIconValue &other) const;
   void assign(const PropertySheetIconValue &other, uint mask);

   // Convenience accessors to get themed/unthemed icons.
   PropertySheetIconValue themed() const;
   PropertySheetIconValue unthemed() const;

   typedef QPair<QIcon::Mode, QIcon::State> ModeStateKey;
   typedef QMap<ModeStateKey, PropertySheetPixmapValue> ModeStateToPixmapMap;

   const ModeStateToPixmapMap &paths() const;

 private:
   bool equals(const PropertySheetIconValue &rhs) const;
   QSharedDataPointer<PropertySheetIconValueData> m_data;
};

QDebug operator<<(QDebug, const PropertySheetIconValue &);

class DesignerPixmapCache : public QObject
{
   CS_OBJECT(DesignerPixmapCache)

 public:
   DesignerPixmapCache(QObject *parent = nullptr);
   QPixmap pixmap(const PropertySheetPixmapValue &value) const;
   void clear();

   CS_SIGNAL_1(Public, void reloaded())
   CS_SIGNAL_2(reloaded)

 private:
   mutable QMap<PropertySheetPixmapValue, QPixmap> m_cache;
   friend class FormWindowBase;
};

class DesignerIconCache : public QObject
{
   CS_OBJECT(DesignerIconCache)

 public:
   explicit DesignerIconCache(DesignerPixmapCache *pixmapCache, QObject *parent = nullptr);
   QIcon icon(const PropertySheetIconValue &value) const;
   void clear();

   CS_SIGNAL_1(Public, void reloaded())
   CS_SIGNAL_2(reloaded)

 private:
   mutable QMap<PropertySheetIconValue, QIcon> m_cache;
   DesignerPixmapCache *m_pixmapCache;
   friend class FormWindowBase;
};

// -------------- PropertySheetTranslatableData: Base class for translatable properties.
class PropertySheetTranslatableData
{
 protected:
   PropertySheetTranslatableData(bool translatable = true,
      const QString &disambiguation = QString(),
      const QString &comment = QString());
   bool equals(const PropertySheetTranslatableData &rhs) const;

 public:
   bool translatable() const                {
      return m_translatable;
   }
   void setTranslatable(bool translatable)  {
      m_translatable = translatable;
   }
   QString disambiguation() const           {
      return m_disambiguation;
   }
   void setDisambiguation(const QString &d) {
      m_disambiguation = d;
   }
   QString comment() const                  {
      return m_comment;
   }
   void setComment(const QString &comment)  {
      m_comment = comment;
   }

 private:
   bool m_translatable;
   QString m_disambiguation;
   QString m_comment;
};

// -------------- StringValue: Returned by the property sheet for strings
class PropertySheetStringValue : public PropertySheetTranslatableData
{
 public:
   PropertySheetStringValue(const QString &value = QString(), bool translatable = true,
      const QString &disambiguation = QString(), const QString &comment = QString());

   bool operator==(const PropertySheetStringValue &other) const {
      return equals(other);
   }
   bool operator!=(const PropertySheetStringValue &other) const {
      return !equals(other);
   }

   QString value() const;
   void setValue(const QString &value);

 private:
   bool equals(const PropertySheetStringValue &rhs) const;

   QString m_value;
};

// -------------- StringValue: Returned by the property sheet for string lists
class PropertySheetStringListValue : public PropertySheetTranslatableData
{
 public:
   PropertySheetStringListValue(const QStringList &value = QStringList(),
      bool translatable = true,
      const QString &disambiguation = QString(),
      const QString &comment = QString());

   bool operator==(const PropertySheetStringListValue &other) const {
      return equals(other);
   }
   bool operator!=(const PropertySheetStringListValue &other) const {
      return !equals(other);
   }

   QStringList value() const;
   void setValue(const QStringList &value);

 private:
   bool equals(const PropertySheetStringListValue &rhs) const;

   QStringList m_value;
};

// -------------- StringValue: Returned by the property sheet for strings
class PropertySheetKeySequenceValue : public PropertySheetTranslatableData
{
 public:
   PropertySheetKeySequenceValue(const QKeySequence &value = QKeySequence(),
      bool translatable = true,
      const QString &disambiguation = QString(),
      const QString &comment = QString());
   PropertySheetKeySequenceValue(const QKeySequence::StandardKey &standardKey,
      bool translatable = true,
      const QString &disambiguation = QString(),
      const QString &comment = QString());

   bool operator==(const PropertySheetKeySequenceValue &other) const {
      return equals(other);
   }
   bool operator!=(const PropertySheetKeySequenceValue &other) const {
      return !equals(other);
   }

   QKeySequence value() const;
   void setValue(const QKeySequence &value);
   QKeySequence::StandardKey standardKey() const;
   void setStandardKey(const QKeySequence::StandardKey &standardKey);
   bool isStandardKey() const;

 private:
   bool equals(const PropertySheetKeySequenceValue &rhs) const;

   QKeySequence m_value;
   QKeySequence::StandardKey m_standardKey;
};

}  // end namespace qdesigner_internal


CS_DECLARE_METATYPE(qdesigner_internal::PropertySheetEnumValue)
CS_DECLARE_METATYPE(qdesigner_internal::PropertySheetFlagValue)
CS_DECLARE_METATYPE(qdesigner_internal::PropertySheetPixmapValue)
CS_DECLARE_METATYPE(qdesigner_internal::PropertySheetIconValue)
CS_DECLARE_METATYPE(qdesigner_internal::PropertySheetStringValue)
CS_DECLARE_METATYPE(qdesigner_internal::PropertySheetStringListValue)
CS_DECLARE_METATYPE(qdesigner_internal::PropertySheetKeySequenceValue)


namespace qdesigner_internal {

// Create a command to change a text property (that is, create a reset property command if the text is empty)
QDesignerFormWindowCommand *createTextPropertyCommand(const QString &propertyName, const QString &text, QObject *object,
   QDesignerFormWindowInterface *fw);

// Returns preferred task menu action for managed widget
QAction *preferredEditAction(QDesignerFormEditorInterface *core, QWidget *managedWidget);

// Convenience to run UIC
bool runUIC(const QString &fileName, QByteArray &ba, QString &errorMessage);

QString refactorClassName(const QString &name);

/* UpdateBlocker: Blocks the updates of the widget passed on while in scope.
 * Does nothing if the incoming widget already has updatesEnabled==false
 * which is important to avoid side-effects when putting it into QStackedLayout. */

class UpdateBlocker
{
   Q_DISABLE_COPY(UpdateBlocker)

 public:
   UpdateBlocker(QWidget *w);
   ~UpdateBlocker();

 private:
   QWidget *m_widget;
   const bool m_enabled;
};

namespace Utils {

inline int valueOf(const QVariant &data, bool *ok = nullptr)
{
   if (data.canConvert<PropertySheetEnumValue>()) {

      if (ok) {
         *ok = true;
      }

      return data.value<PropertySheetEnumValue>().value;

   } else if (data.canConvert<PropertySheetFlagValue>()) {

      if (ok) {
         *ok = true;
      }

      return data.value<PropertySheetFlagValue>().value;
   }

   return data.toInt(ok);
}

inline bool isObjectAncestorOf(QObject *ancestor, QObject *child)
{
   QObject *obj = child;

   while (obj != nullptr) {
      if (obj == ancestor) {
         return true;
      }

      obj = obj->parent();
   }

   return false;
}

inline bool isCentralWidget(QDesignerFormWindowInterface *fw, QWidget *widget)
{
   if (! fw || ! widget) {
      return false;
   }

   if (widget == fw->mainContainer()) {
      return true;
   }

   // ### generalize for other containers
   if (QMainWindow *mw = dynamic_cast<QMainWindow *>(fw->mainContainer())) {
      return mw->centralWidget() == widget;
   }

   return false;
}

}   // end namespace - Utils

}   // end namespace - qdesigner_internal

#endif
