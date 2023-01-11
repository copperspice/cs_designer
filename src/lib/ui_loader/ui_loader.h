/***********************************************************************
*
* Copyright (c) 2021-2023 Barbara Geller
* Copyright (c) 2021-2023 Ansel Sermersheim
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

#ifndef QUILOADER_H
#define QUILOADER_H

class QUiLoaderPrivate;

#include <QObject>
#include <QScopedPointer>
#include <QString>

class QWidget;
class QLayout;
class QAction;
class QActionGroup;
class QIODevice;
class QDir;

class QUiLoader : public QObject
{
   CS_OBJECT(QUiLoader)

 public:
   explicit QUiLoader(QObject *parent = nullptr);
   ~QUiLoader() override;

   QStringList pluginPaths() const;
   void clearPluginPaths();
   void addPluginPath(const QString &path);

   QWidget *load(QIODevice *device, QWidget *parentWidget = nullptr);
   QStringList availableWidgets() const;
   QStringList availableLayouts() const;

   virtual QWidget *createWidget(const QString &className, QWidget *parent = nullptr, const QString &name = QString());
   virtual QLayout *createLayout(const QString &className, QObject *parent = nullptr, const QString &name = QString());
   virtual QActionGroup *createActionGroup(QObject *parent = nullptr, const QString &name = QString());
   virtual QAction *createAction(QObject *parent = nullptr, const QString &name = QString());

   void setWorkingDirectory(const QDir &dir);
   QDir workingDirectory() const;

   void setLanguageChangeEnabled(bool enabled);
   bool isLanguageChangeEnabled() const;

   void setTranslationEnabled(bool enabled);
   bool isTranslationEnabled() const;

   QString errorString() const;

 private:
   QUiLoader(const QUiLoader &) = delete;
   QUiLoader &operator=(const QUiLoader &) = delete;

   Q_DECLARE_PRIVATE(QUiLoader)

   QScopedPointer<QUiLoaderPrivate> d_ptr;
};

#endif
