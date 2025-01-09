/***********************************************************************
*
* Copyright (c) 2021-2025 Barbara Geller
* Copyright (c) 2021-2025 Ansel Sermersheim
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

#ifndef SHAREDENUMS_H
#define SHAREDENUMS_H

namespace qdesigner_internal {

// Validation mode of text property line edits
enum TextPropertyValidationMode {
   // Allow for multiline editing using literal "\n".
   ValidationMultiLine,
   // Allow for HTML rich text including multiline editing using literal "\n".
   ValidationRichText,
   // Validate a stylesheet
   ValidationStyleSheet,
   // Single line mode, suppresses newlines
   ValidationSingleLine,
   // Allow only for identifier characters
   ValidationObjectName,
   // Allow only for identifier characters and colons
   ValidationObjectNameScope,
   // URL
   ValidationURL
};

// Container types
enum ContainerType {
   // A container with pages, at least one of which one must always be present (for example, QTabWidget)
   PageContainer,
   // Mdi type container. All pages may be deleted, no concept of page order
   MdiContainer,
   // Wizard container
   WizardContainer
};

enum AuxiliaryItemDataRoles {
   // item->flags while being edited
   ItemFlagsShadowRole = 0x13370551
};
}

#endif
