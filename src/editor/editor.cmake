list(APPEND DESIGNER_INCLUDES
   ${CMAKE_CURRENT_SOURCE_DIR}/editor/action_editor/action_editor.h
   ${CMAKE_CURRENT_SOURCE_DIR}/editor/action_editor/action_repository.h
   ${CMAKE_CURRENT_SOURCE_DIR}/editor/action_editor/action_provider.h

   ${CMAKE_CURRENT_SOURCE_DIR}/editor/buddy_editor/buddyeditor.h
   ${CMAKE_CURRENT_SOURCE_DIR}/editor/buddy_editor/buddyeditor_plugin.h
   ${CMAKE_CURRENT_SOURCE_DIR}/editor/buddy_editor/buddyeditor_tool.h

   ${CMAKE_CURRENT_SOURCE_DIR}/editor/form_editor/default_actionprovider.h
   ${CMAKE_CURRENT_SOURCE_DIR}/editor/form_editor/default_container.h
   ${CMAKE_CURRENT_SOURCE_DIR}/editor/form_editor/default_layoutdecoration.h
   ${CMAKE_CURRENT_SOURCE_DIR}/editor/form_editor/deviceprofiledialog.h
   ${CMAKE_CURRENT_SOURCE_DIR}/editor/form_editor/dpi_chooser.h
   ${CMAKE_CURRENT_SOURCE_DIR}/editor/form_editor/embedded_options_page.h
   ${CMAKE_CURRENT_SOURCE_DIR}/editor/form_editor/formeditor.h
   ${CMAKE_CURRENT_SOURCE_DIR}/editor/form_editor/formeditor_optionspage.h
   ${CMAKE_CURRENT_SOURCE_DIR}/editor/form_editor/formwindow.h
   ${CMAKE_CURRENT_SOURCE_DIR}/editor/form_editor/formwindow_dnditem.h
   ${CMAKE_CURRENT_SOURCE_DIR}/editor/form_editor/formwindow_widgetstack.h
   ${CMAKE_CURRENT_SOURCE_DIR}/editor/form_editor/formwindowcursor.h
   ${CMAKE_CURRENT_SOURCE_DIR}/editor/form_editor/formwindowmanager.h
   ${CMAKE_CURRENT_SOURCE_DIR}/editor/form_editor/formwindowsettings.h
   ${CMAKE_CURRENT_SOURCE_DIR}/editor/form_editor/itemview_propertysheet.h
   ${CMAKE_CURRENT_SOURCE_DIR}/editor/form_editor/layout_propertysheet.h
   ${CMAKE_CURRENT_SOURCE_DIR}/editor/form_editor/line_propertysheet.h
   ${CMAKE_CURRENT_SOURCE_DIR}/editor/form_editor/previewactiongroup.h
   ${CMAKE_CURRENT_SOURCE_DIR}/editor/form_editor/qlayoutwidget_propertysheet.h
   ${CMAKE_CURRENT_SOURCE_DIR}/editor/form_editor/qmainwindow_container.h
   ${CMAKE_CURRENT_SOURCE_DIR}/editor/form_editor/qmdiarea_container.h
   ${CMAKE_CURRENT_SOURCE_DIR}/editor/form_editor/qwizard_container.h
   ${CMAKE_CURRENT_SOURCE_DIR}/editor/form_editor/spacer_propertysheet.h
   ${CMAKE_CURRENT_SOURCE_DIR}/editor/form_editor/templateoptionspage.h
   ${CMAKE_CURRENT_SOURCE_DIR}/editor/form_editor/tool_widgeteditor.h
   ${CMAKE_CURRENT_SOURCE_DIR}/editor/form_editor/undostack.h
   ${CMAKE_CURRENT_SOURCE_DIR}/editor/form_editor/widgetselection.h

   ${CMAKE_CURRENT_SOURCE_DIR}/editor/gradient_editor/color_button.h
   ${CMAKE_CURRENT_SOURCE_DIR}/editor/gradient_editor/color_line.h
   ${CMAKE_CURRENT_SOURCE_DIR}/editor/gradient_editor/gradient_dialog.h
   ${CMAKE_CURRENT_SOURCE_DIR}/editor/gradient_editor/gradient_editor.h
   ${CMAKE_CURRENT_SOURCE_DIR}/editor/gradient_editor/gradient_manager.h
   ${CMAKE_CURRENT_SOURCE_DIR}/editor/gradient_editor/gradient_select.h
   ${CMAKE_CURRENT_SOURCE_DIR}/editor/gradient_editor/gradient_utils.h
   ${CMAKE_CURRENT_SOURCE_DIR}/editor/gradient_editor/gradient_view.h
   ${CMAKE_CURRENT_SOURCE_DIR}/editor/gradient_editor/gradient_widget.h
   ${CMAKE_CURRENT_SOURCE_DIR}/editor/gradient_editor/gradientstops_controller.h
   ${CMAKE_CURRENT_SOURCE_DIR}/editor/gradient_editor/gradientstops_model.h
   ${CMAKE_CURRENT_SOURCE_DIR}/editor/gradient_editor/gradientstops_widget.h

   ${CMAKE_CURRENT_SOURCE_DIR}/editor/property_editor/edit_property.h
   ${CMAKE_CURRENT_SOURCE_DIR}/editor/property_editor/edit_property_manager.h
   ${CMAKE_CURRENT_SOURCE_DIR}/editor/property_editor/edit_variant_property.h
   ${CMAKE_CURRENT_SOURCE_DIR}/editor/property_editor/property_view_button.h
   ${CMAKE_CURRENT_SOURCE_DIR}/editor/property_editor/property_view_tree.h
   ${CMAKE_CURRENT_SOURCE_DIR}/editor/property_editor/property_view_groupbox.h
   ${CMAKE_CURRENT_SOURCE_DIR}/editor/property_editor/property_view.h
   ${CMAKE_CURRENT_SOURCE_DIR}/editor/property_editor/property_view_utils.h

   ${CMAKE_CURRENT_SOURCE_DIR}/editor/signalslot_editor/connectdialog_p.h
   ${CMAKE_CURRENT_SOURCE_DIR}/editor/signalslot_editor/signalslot_utils_p.h
   ${CMAKE_CURRENT_SOURCE_DIR}/editor/signalslot_editor/signalsloteditor.h
   ${CMAKE_CURRENT_SOURCE_DIR}/editor/signalslot_editor/signalsloteditor_tool.h
   ${CMAKE_CURRENT_SOURCE_DIR}/editor/signalslot_editor/signalsloteditor_plugin.h
   ${CMAKE_CURRENT_SOURCE_DIR}/editor/signalslot_editor/signalsloteditor_p.h
   ${CMAKE_CURRENT_SOURCE_DIR}/editor/signalslot_editor/signalsloteditorwindow.h

   ${CMAKE_CURRENT_SOURCE_DIR}/editor/taborder_editor/tabordereditor.h
   ${CMAKE_CURRENT_SOURCE_DIR}/editor/taborder_editor/tabordereditor_plugin.h
   ${CMAKE_CURRENT_SOURCE_DIR}/editor/taborder_editor/tabordereditor_tool.h

   ${CMAKE_CURRENT_SOURCE_DIR}/editor/popup_editor/brush_property.h
   ${CMAKE_CURRENT_SOURCE_DIR}/editor/popup_editor/designer_property.h
   ${CMAKE_CURRENT_SOURCE_DIR}/editor/popup_editor/font_property.h
   ${CMAKE_CURRENT_SOURCE_DIR}/editor/popup_editor/inplace_editor.h
   ${CMAKE_CURRENT_SOURCE_DIR}/editor/popup_editor/inplace_widget_helper.h
   ${CMAKE_CURRENT_SOURCE_DIR}/editor/popup_editor/itemlist_editor.h
   ${CMAKE_CURRENT_SOURCE_DIR}/editor/popup_editor/listwidget_editor.h
   ${CMAKE_CURRENT_SOURCE_DIR}/editor/popup_editor/newdynamic_property.h
   ${CMAKE_CURRENT_SOURCE_DIR}/editor/popup_editor/palette_editor.h
   ${CMAKE_CURRENT_SOURCE_DIR}/editor/popup_editor/palette_editor_toolbutton.h
   ${CMAKE_CURRENT_SOURCE_DIR}/editor/popup_editor/property_editor.h
   ${CMAKE_CURRENT_SOURCE_DIR}/editor/popup_editor/stringlist_editor.h
   ${CMAKE_CURRENT_SOURCE_DIR}/editor/popup_editor/stringlist_editor_toolbutton.h
   ${CMAKE_CURRENT_SOURCE_DIR}/editor/popup_editor/stylesheet_editor.h
   ${CMAKE_CURRENT_SOURCE_DIR}/editor/popup_editor/stylesheet_widget.h
   ${CMAKE_CURRENT_SOURCE_DIR}/editor/popup_editor/tablewidget_editor.h
   ${CMAKE_CURRENT_SOURCE_DIR}/editor/popup_editor/treewidget_editor.h

   ${CMAKE_CURRENT_SOURCE_DIR}/editor/text_editor/plaintext_editor.h
   ${CMAKE_CURRENT_SOURCE_DIR}/editor/text_editor/richtext_editor.h
   ${CMAKE_CURRENT_SOURCE_DIR}/editor/text_editor/textproperty_editor.h
)

list(APPEND DESIGNER_SOURCES
   ${CMAKE_CURRENT_SOURCE_DIR}/editor/action_editor/action_editor.cpp
   ${CMAKE_CURRENT_SOURCE_DIR}/editor/action_editor/action_repository.cpp

   ${CMAKE_CURRENT_SOURCE_DIR}/editor/buddy_editor/buddyeditor.cpp
   ${CMAKE_CURRENT_SOURCE_DIR}/editor/buddy_editor/buddyeditor_tool.cpp
   ${CMAKE_CURRENT_SOURCE_DIR}/editor/buddy_editor/buddyeditor_plugin.cpp

   ${CMAKE_CURRENT_SOURCE_DIR}/editor/form_editor/default_actionprovider.cpp
   ${CMAKE_CURRENT_SOURCE_DIR}/editor/form_editor/default_container.cpp
   ${CMAKE_CURRENT_SOURCE_DIR}/editor/form_editor/default_layoutdecoration.cpp
   ${CMAKE_CURRENT_SOURCE_DIR}/editor/form_editor/deviceprofiledialog.cpp
   ${CMAKE_CURRENT_SOURCE_DIR}/editor/form_editor/dpi_chooser.cpp
   ${CMAKE_CURRENT_SOURCE_DIR}/editor/form_editor/embedded_options_page.cpp
   ${CMAKE_CURRENT_SOURCE_DIR}/editor/form_editor/formeditor.cpp
   ${CMAKE_CURRENT_SOURCE_DIR}/editor/form_editor/formeditor_optionspage.cpp
   ${CMAKE_CURRENT_SOURCE_DIR}/editor/form_editor/formwindow.cpp
   ${CMAKE_CURRENT_SOURCE_DIR}/editor/form_editor/formwindow_dnditem.cpp
   ${CMAKE_CURRENT_SOURCE_DIR}/editor/form_editor/formwindow_widgetstack.cpp
   ${CMAKE_CURRENT_SOURCE_DIR}/editor/form_editor/formwindowcursor.cpp
   ${CMAKE_CURRENT_SOURCE_DIR}/editor/form_editor/formwindowmanager.cpp
   ${CMAKE_CURRENT_SOURCE_DIR}/editor/form_editor/formwindowsettings.cpp
   ${CMAKE_CURRENT_SOURCE_DIR}/editor/form_editor/itemview_propertysheet.cpp
   ${CMAKE_CURRENT_SOURCE_DIR}/editor/form_editor/layout_propertysheet.cpp
   ${CMAKE_CURRENT_SOURCE_DIR}/editor/form_editor/line_propertysheet.cpp
   ${CMAKE_CURRENT_SOURCE_DIR}/editor/form_editor/previewactiongroup.cpp
   ${CMAKE_CURRENT_SOURCE_DIR}/editor/form_editor/qlayoutwidget_propertysheet.cpp
   ${CMAKE_CURRENT_SOURCE_DIR}/editor/form_editor/qmainwindow_container.cpp
   ${CMAKE_CURRENT_SOURCE_DIR}/editor/form_editor/qmdiarea_container.cpp
   ${CMAKE_CURRENT_SOURCE_DIR}/editor/form_editor/qwizard_container.cpp
   ${CMAKE_CURRENT_SOURCE_DIR}/editor/form_editor/spacer_propertysheet.cpp
   ${CMAKE_CURRENT_SOURCE_DIR}/editor/form_editor/templateoptionspage.cpp
   ${CMAKE_CURRENT_SOURCE_DIR}/editor/form_editor/tool_widgeteditor.cpp
   ${CMAKE_CURRENT_SOURCE_DIR}/editor/form_editor/undostack.cpp
   ${CMAKE_CURRENT_SOURCE_DIR}/editor/form_editor/widgetselection.cpp

   ${CMAKE_CURRENT_SOURCE_DIR}/editor/gradient_editor/color_button.cpp
   ${CMAKE_CURRENT_SOURCE_DIR}/editor/gradient_editor/color_line.cpp
   ${CMAKE_CURRENT_SOURCE_DIR}/editor/gradient_editor/gradient_dialog.cpp
   ${CMAKE_CURRENT_SOURCE_DIR}/editor/gradient_editor/gradient_editor.cpp
   ${CMAKE_CURRENT_SOURCE_DIR}/editor/gradient_editor/gradient_manager.cpp
   ${CMAKE_CURRENT_SOURCE_DIR}/editor/gradient_editor/gradient_select.cpp
   ${CMAKE_CURRENT_SOURCE_DIR}/editor/gradient_editor/gradient_utils.cpp
   ${CMAKE_CURRENT_SOURCE_DIR}/editor/gradient_editor/gradient_view.cpp
   ${CMAKE_CURRENT_SOURCE_DIR}/editor/gradient_editor/gradient_widget.cpp
   ${CMAKE_CURRENT_SOURCE_DIR}/editor/gradient_editor/gradientstops_controller.cpp
   ${CMAKE_CURRENT_SOURCE_DIR}/editor/gradient_editor/gradientstops_model.cpp
   ${CMAKE_CURRENT_SOURCE_DIR}/editor/gradient_editor/gradientstops_widget.cpp

   ${CMAKE_CURRENT_SOURCE_DIR}/editor/property_editor/edit_property.cpp
   ${CMAKE_CURRENT_SOURCE_DIR}/editor/property_editor/edit_property_manager.cpp
   ${CMAKE_CURRENT_SOURCE_DIR}/editor/property_editor/edit_variant_property.cpp
   ${CMAKE_CURRENT_SOURCE_DIR}/editor/property_editor/property_view_button.cpp
   ${CMAKE_CURRENT_SOURCE_DIR}/editor/property_editor/property_view_tree.cpp
   ${CMAKE_CURRENT_SOURCE_DIR}/editor/property_editor/property_view_groupbox.cpp
   ${CMAKE_CURRENT_SOURCE_DIR}/editor/property_editor/property_view.cpp
   ${CMAKE_CURRENT_SOURCE_DIR}/editor/property_editor/property_view_utils.cpp

   ${CMAKE_CURRENT_SOURCE_DIR}/editor/signalslot_editor/connectdialog.cpp
   ${CMAKE_CURRENT_SOURCE_DIR}/editor/signalslot_editor/signalslot_utils.cpp
   ${CMAKE_CURRENT_SOURCE_DIR}/editor/signalslot_editor/signalsloteditor.cpp
   ${CMAKE_CURRENT_SOURCE_DIR}/editor/signalslot_editor/signalsloteditor_tool.cpp
   ${CMAKE_CURRENT_SOURCE_DIR}/editor/signalslot_editor/signalsloteditor_plugin.cpp
   ${CMAKE_CURRENT_SOURCE_DIR}/editor/signalslot_editor/signalsloteditorwindow.cpp

   ${CMAKE_CURRENT_SOURCE_DIR}/editor/taborder_editor/tabordereditor.cpp
   ${CMAKE_CURRENT_SOURCE_DIR}/editor/taborder_editor/tabordereditor_tool.cpp
   ${CMAKE_CURRENT_SOURCE_DIR}/editor/taborder_editor/tabordereditor_plugin.cpp

   ${CMAKE_CURRENT_SOURCE_DIR}/editor/popup_editor/brush_property.cpp
   ${CMAKE_CURRENT_SOURCE_DIR}/editor/popup_editor/designer_property.cpp
   ${CMAKE_CURRENT_SOURCE_DIR}/editor/popup_editor/font_property.cpp
   ${CMAKE_CURRENT_SOURCE_DIR}/editor/popup_editor/inplace_editor.cpp
   ${CMAKE_CURRENT_SOURCE_DIR}/editor/popup_editor/inplace_widget_helper.cpp
   ${CMAKE_CURRENT_SOURCE_DIR}/editor/popup_editor/itemlist_editor.cpp
   ${CMAKE_CURRENT_SOURCE_DIR}/editor/popup_editor/listwidget_editor.cpp
   ${CMAKE_CURRENT_SOURCE_DIR}/editor/popup_editor/newdynamic_property.cpp
   ${CMAKE_CURRENT_SOURCE_DIR}/editor/popup_editor/palette_editor.cpp
   ${CMAKE_CURRENT_SOURCE_DIR}/editor/popup_editor/palette_editor_toolbutton.cpp
   ${CMAKE_CURRENT_SOURCE_DIR}/editor/popup_editor/property_editor.cpp
   ${CMAKE_CURRENT_SOURCE_DIR}/editor/popup_editor/stringlist_editor.cpp
   ${CMAKE_CURRENT_SOURCE_DIR}/editor/popup_editor/stringlist_editor_toolbutton.cpp
   ${CMAKE_CURRENT_SOURCE_DIR}/editor/popup_editor/stylesheet_editor.cpp
   ${CMAKE_CURRENT_SOURCE_DIR}/editor/popup_editor/tablewidget_editor.cpp
   ${CMAKE_CURRENT_SOURCE_DIR}/editor/popup_editor/treewidget_editor.cpp

   ${CMAKE_CURRENT_SOURCE_DIR}/editor/text_editor/plaintext_editor.cpp
   ${CMAKE_CURRENT_SOURCE_DIR}/editor/text_editor/richtext_editor.cpp
   ${CMAKE_CURRENT_SOURCE_DIR}/editor/text_editor/textproperty_editor.cpp
)
