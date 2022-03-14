list(APPEND DESIGNER_INCLUDES
   ${CMAKE_CURRENT_SOURCE_DIR}/dialogs/appfont_dialog.h
   ${CMAKE_CURRENT_SOURCE_DIR}/dialogs/code_dialog.h
   ${CMAKE_CURRENT_SOURCE_DIR}/dialogs/designer_enums.h
   ${CMAKE_CURRENT_SOURCE_DIR}/dialogs/newaction_dialog.h
   ${CMAKE_CURRENT_SOURCE_DIR}/dialogs/newform.h
   ${CMAKE_CURRENT_SOURCE_DIR}/dialogs/newform_widget.h
   ${CMAKE_CURRENT_SOURCE_DIR}/dialogs/order_dialog.h
   ${CMAKE_CURRENT_SOURCE_DIR}/dialogs/preferences_dialog.h
   ${CMAKE_CURRENT_SOURCE_DIR}/dialogs/toolbar_dialog.h
   ${CMAKE_CURRENT_SOURCE_DIR}/dialogs/saveform_as_template.h
   ${CMAKE_CURRENT_SOURCE_DIR}/dialogs/signalslot_dialog.h
)

list(APPEND DESIGNER_SOURCES
   ${CMAKE_CURRENT_SOURCE_DIR}/dialogs/appfont_dialog.cpp
   ${CMAKE_CURRENT_SOURCE_DIR}/dialogs/code_dialog.cpp
   ${CMAKE_CURRENT_SOURCE_DIR}/dialogs/newaction_dialog.cpp
   ${CMAKE_CURRENT_SOURCE_DIR}/dialogs/newform.cpp
   ${CMAKE_CURRENT_SOURCE_DIR}/dialogs/newform_widget.cpp
   ${CMAKE_CURRENT_SOURCE_DIR}/dialogs/order_dialog.cpp
   ${CMAKE_CURRENT_SOURCE_DIR}/dialogs/preferences_dialog.cpp
   ${CMAKE_CURRENT_SOURCE_DIR}/dialogs/toolbar_dialog.cpp
   ${CMAKE_CURRENT_SOURCE_DIR}/dialogs/saveform_as_template.cpp
   ${CMAKE_CURRENT_SOURCE_DIR}/dialogs/signalslot_dialog.cpp
)
