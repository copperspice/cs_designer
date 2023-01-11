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

#include <designer.h>

#include <QLabel>

int main(int argc, char *argv[])
{
   QDesigner app(argc, argv);

   auto reply = app.initSystem();

   switch (reply) {
      case QDesigner::ParseArgumentsSuccess:
         break;

      case QDesigner::ParseArgumentsError:
         return 1;

      case QDesigner::ParseArgumentsHelpRequested:
         return 0;
   }

   app.setQuitOnLastWindowClosed(false);

   return app.exec();
}
