list(APPEND DESIGNER_SOURCES
   ${CMAKE_CURRENT_SOURCE_DIR}/../forms/action_editor_new.ui
   ${CMAKE_CURRENT_SOURCE_DIR}/../forms/designer_deviceprofile.ui
   ${CMAKE_CURRENT_SOURCE_DIR}/../forms/designer_form_settings.ui
   ${CMAKE_CURRENT_SOURCE_DIR}/../forms/designer_gridpanel.ui
   ${CMAKE_CURRENT_SOURCE_DIR}/../forms/designer_options.ui
   ${CMAKE_CURRENT_SOURCE_DIR}/../forms/designer_preferences.ui
   ${CMAKE_CURRENT_SOURCE_DIR}/../forms/designer_toolbar.ui
   ${CMAKE_CURRENT_SOURCE_DIR}/../forms/edit_dynamicproperty.ui
   ${CMAKE_CURRENT_SOURCE_DIR}/../forms/edit_itemlist.ui
   ${CMAKE_CURRENT_SOURCE_DIR}/../forms/edit_pageorder.ui
   ${CMAKE_CURRENT_SOURCE_DIR}/../forms/edit_palette.ui
   ${CMAKE_CURRENT_SOURCE_DIR}/../forms/edit_stylesheet.ui
   ${CMAKE_CURRENT_SOURCE_DIR}/../forms/edit_stringlist.ui
   ${CMAKE_CURRENT_SOURCE_DIR}/../forms/edit_tablewidget.ui
   ${CMAKE_CURRENT_SOURCE_DIR}/../forms/edit_treewidget.ui
   ${CMAKE_CURRENT_SOURCE_DIR}/../forms/gradient_dialog.ui
   ${CMAKE_CURRENT_SOURCE_DIR}/../forms/gradient_editor.ui
   ${CMAKE_CURRENT_SOURCE_DIR}/../forms/gradient_select.ui
   ${CMAKE_CURRENT_SOURCE_DIR}/../forms/gradient_view.ui
   ${CMAKE_CURRENT_SOURCE_DIR}/../forms/formlayout_addrow.ui
   ${CMAKE_CURRENT_SOURCE_DIR}/../forms/new_form_widget.ui
   ${CMAKE_CURRENT_SOURCE_DIR}/../forms/plugin_dialog.ui
   ${CMAKE_CURRENT_SOURCE_DIR}/../forms/preview_configuration.ui
   ${CMAKE_CURRENT_SOURCE_DIR}/../forms/preview_widget.ui
   ${CMAKE_CURRENT_SOURCE_DIR}/../forms/resource_editor.ui
   ${CMAKE_CURRENT_SOURCE_DIR}/../forms/richtext_addlink.ui
   ${CMAKE_CURRENT_SOURCE_DIR}/../forms/signal_connect_dialog.ui
   ${CMAKE_CURRENT_SOURCE_DIR}/../forms/signal_select_dialog.ui
   ${CMAKE_CURRENT_SOURCE_DIR}/../forms/signal_slot_dialog.ui
   ${CMAKE_CURRENT_SOURCE_DIR}/../forms/template_options.ui
   ${CMAKE_CURRENT_SOURCE_DIR}/../forms/template_saveform_as.ui

   ${CMAKE_CURRENT_SOURCE_DIR}/../resources/designer_en.ts
   ${CMAKE_CURRENT_SOURCE_DIR}/../resources/designer_de.ts
   ${CMAKE_CURRENT_SOURCE_DIR}/../resources/designer_es.ts
   ${CMAKE_CURRENT_SOURCE_DIR}/../resources/designer_fr.ts
   ${CMAKE_CURRENT_SOURCE_DIR}/../resources/designer_nl.ts
   ${CMAKE_CURRENT_SOURCE_DIR}/../resources/designer_pl.ts
   ${CMAKE_CURRENT_SOURCE_DIR}/../resources/designer_ru.ts
)

include(abstract/abstract.cmake)
include(designer/designer.cmake)
include(dialogs/dialogs.cmake)
include(editor/editor.cmake)
include(formbuilder/formbuilder.cmake)
include(lib/lib.cmake)
include(main/main.cmake)
include(menus/menus.cmake)
include(preview/preview.cmake)

# location for *.qm file, overrides build folder
set(TS_OUTPUT_DIR  ${CMAKE_CURRENT_SOURCE_DIR}/../resources)

# run lrelease to generate translation files
COPPERSPICE_RESOURCES(
   ${CMAKE_CURRENT_SOURCE_DIR}/../resources/designer_en.ts
   ${CMAKE_CURRENT_SOURCE_DIR}/../resources/designer_de.ts
   ${CMAKE_CURRENT_SOURCE_DIR}/../resources/designer_es.ts
   ${CMAKE_CURRENT_SOURCE_DIR}/../resources/designer_fr.ts
   ${CMAKE_CURRENT_SOURCE_DIR}/../resources/designer_nl.ts
   ${CMAKE_CURRENT_SOURCE_DIR}/../resources/designer_pl.ts
   ${CMAKE_CURRENT_SOURCE_DIR}/../resources/designer_ru.ts
)

# run rcc to generate qrc output
COPPERSPICE_RESOURCES(
   ${CMAKE_CURRENT_SOURCE_DIR}/../designer.qrc
)

# run uic to generate source
COPPERSPICE_RESOURCES(
   ${CMAKE_CURRENT_SOURCE_DIR}/../forms/action_editor_new.ui
   ${CMAKE_CURRENT_SOURCE_DIR}/../forms/designer_deviceprofile.ui
   ${CMAKE_CURRENT_SOURCE_DIR}/../forms/designer_form_settings.ui
   ${CMAKE_CURRENT_SOURCE_DIR}/../forms/designer_gridpanel.ui
   ${CMAKE_CURRENT_SOURCE_DIR}/../forms/designer_options.ui
   ${CMAKE_CURRENT_SOURCE_DIR}/../forms/designer_preferences.ui
   ${CMAKE_CURRENT_SOURCE_DIR}/../forms/designer_toolbar.ui
   ${CMAKE_CURRENT_SOURCE_DIR}/../forms/edit_dynamicproperty.ui
   ${CMAKE_CURRENT_SOURCE_DIR}/../forms/edit_itemlist.ui
   ${CMAKE_CURRENT_SOURCE_DIR}/../forms/edit_pageorder.ui
   ${CMAKE_CURRENT_SOURCE_DIR}/../forms/edit_palette.ui
   ${CMAKE_CURRENT_SOURCE_DIR}/../forms/edit_stylesheet.ui
   ${CMAKE_CURRENT_SOURCE_DIR}/../forms/edit_stringlist.ui
   ${CMAKE_CURRENT_SOURCE_DIR}/../forms/edit_tablewidget.ui
   ${CMAKE_CURRENT_SOURCE_DIR}/../forms/edit_treewidget.ui
   ${CMAKE_CURRENT_SOURCE_DIR}/../forms/gradient_dialog.ui
   ${CMAKE_CURRENT_SOURCE_DIR}/../forms/gradient_editor.ui
   ${CMAKE_CURRENT_SOURCE_DIR}/../forms/gradient_select.ui
   ${CMAKE_CURRENT_SOURCE_DIR}/../forms/gradient_view.ui
   ${CMAKE_CURRENT_SOURCE_DIR}/../forms/formlayout_addrow.ui
   ${CMAKE_CURRENT_SOURCE_DIR}/../forms/new_form_widget.ui
   ${CMAKE_CURRENT_SOURCE_DIR}/../forms/plugin_dialog.ui
   ${CMAKE_CURRENT_SOURCE_DIR}/../forms/preview_configuration.ui
   ${CMAKE_CURRENT_SOURCE_DIR}/../forms/preview_widget.ui
   ${CMAKE_CURRENT_SOURCE_DIR}/../forms/resource_editor.ui
   ${CMAKE_CURRENT_SOURCE_DIR}/../forms/richtext_addlink.ui
   ${CMAKE_CURRENT_SOURCE_DIR}/../forms/signal_connect_dialog.ui
   ${CMAKE_CURRENT_SOURCE_DIR}/../forms/signal_select_dialog.ui
   ${CMAKE_CURRENT_SOURCE_DIR}/../forms/signal_slot_dialog.ui
   ${CMAKE_CURRENT_SOURCE_DIR}/../forms/template_options.ui
   ${CMAKE_CURRENT_SOURCE_DIR}/../forms/template_saveform_as.ui
)

add_executable(Designer  ${DESIGNER_SOURCES})

target_include_directories(Designer
   PRIVATE
   ${CMAKE_CURRENT_SOURCE_DIR}/abstract
   ${CMAKE_CURRENT_SOURCE_DIR}/designer
   ${CMAKE_CURRENT_SOURCE_DIR}/dialogs
   ${CMAKE_CURRENT_SOURCE_DIR}/formbuilder
   ${CMAKE_CURRENT_SOURCE_DIR}/main
   ${CMAKE_CURRENT_SOURCE_DIR}/menus
   ${CMAKE_CURRENT_SOURCE_DIR}/preview

   ${CMAKE_CURRENT_SOURCE_DIR}/editor/action_editor
   ${CMAKE_CURRENT_SOURCE_DIR}/editor/buddy_editor
   ${CMAKE_CURRENT_SOURCE_DIR}/editor/form_editor
   ${CMAKE_CURRENT_SOURCE_DIR}/editor/gradient_editor
   ${CMAKE_CURRENT_SOURCE_DIR}/editor/popup_editor
   ${CMAKE_CURRENT_SOURCE_DIR}/editor/property_editor
   ${CMAKE_CURRENT_SOURCE_DIR}/editor/signalslot_editor
   ${CMAKE_CURRENT_SOURCE_DIR}/editor/taborder_editor
   ${CMAKE_CURRENT_SOURCE_DIR}/editor/text_editor

   ${CMAKE_CURRENT_SOURCE_DIR}/lib/extension
   ${CMAKE_CURRENT_SOURCE_DIR}/lib/extra
   ${CMAKE_CURRENT_SOURCE_DIR}/lib/find_widget
   ${CMAKE_CURRENT_SOURCE_DIR}/lib/font_panel
   ${CMAKE_CURRENT_SOURCE_DIR}/lib/object_inspector
   ${CMAKE_CURRENT_SOURCE_DIR}/lib/plugin
   ${CMAKE_CURRENT_SOURCE_DIR}/lib/resource
   ${CMAKE_CURRENT_SOURCE_DIR}/lib/ui_loader
   ${CMAKE_CURRENT_SOURCE_DIR}/lib/util
   ${CMAKE_CURRENT_SOURCE_DIR}/lib/widget_box
)

target_link_libraries(Designer
   CopperSpice::CsCore
   CopperSpice::CsGui
   CopperSpice::CsNetwork
   CopperSpice::CsXml
)

if (CMAKE_SYSTEM_NAME MATCHES "Darwin")
   set_target_properties(Designer PROPERTIES OUTPUT_NAME designer)

elseif(CMAKE_SYSTEM_NAME MATCHES "(Linux|OpenBSD|FreeBSD|NetBSD|DragonFly)")
   set_target_properties(Designer PROPERTIES OUTPUT_NAME designer)

elseif(CMAKE_SYSTEM_NAME MATCHES "Windows")
   # disable console on windows
   target_link_libraries(Designer
      netapi32
      mpr
      -mwindows
   )

endif()

if (CMAKE_SYSTEM_NAME MATCHES "Darwin")
   install(TARGETS Designer DESTINATION ../MacOS)
else()
   install(TARGETS Designer DESTINATION .)
endif()

# installs the CopperSpice libraries
cs_copy_library(CsCore)
cs_copy_library(CsGui)
cs_copy_library(CsNetwork)
cs_copy_library(CsXml)

if (CMAKE_SYSTEM_NAME MATCHES "Darwin")

   set(CPACK_DMG_VOLUME_NAME Designer)
   set(CPACK_DMG_FORMAT UDBZ)

   # installs the platform Gui plugin
   cs_copy_plugins(CsGui ../plugins)
   cs_copy_plugins(CsPrinterDriver ../plugins)

   install(FILES ${CMAKE_CURRENT_SOURCE_DIR}/../resources/designer.icns DESTINATION ./resources)

elseif(CMAKE_SYSTEM_NAME MATCHES "(Linux|OpenBSD|FreeBSD|NetBSD|DragonFly)")

   # installs the platform Gui plugin
   cs_copy_plugins(CsGui)
   cs_copy_plugins(CsPrinterDriver)
   cs_copy_plugins(CsOpenGL)

   install(FILES ${CMAKE_CURRENT_SOURCE_DIR}/../resources/designer.png DESTINATION ./resources)

elseif(CMAKE_SYSTEM_NAME MATCHES "Windows")

   # installs the platform Gui plugin
   cs_copy_plugins(CsGui)
   cs_copy_plugins(CsPrinterDriver)

   install(FILES ${CMAKE_CURRENT_SOURCE_DIR}/../resources/designer.ico DESTINATION ./resources)

endif()

