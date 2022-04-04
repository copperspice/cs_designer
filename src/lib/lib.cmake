list(APPEND DESIGNER_INCLUDES

   ${CMAKE_CURRENT_SOURCE_DIR}/lib/extension/container.h
   ${CMAKE_CURRENT_SOURCE_DIR}/lib/extension/default_extensionfactory.h
   ${CMAKE_CURRENT_SOURCE_DIR}/lib/extension/extension.h
   ${CMAKE_CURRENT_SOURCE_DIR}/lib/extension/extension_manager.h
   ${CMAKE_CURRENT_SOURCE_DIR}/lib/extension/propertysheet.h
   ${CMAKE_CURRENT_SOURCE_DIR}/lib/extension/dynamicpropertysheet.h
   ${CMAKE_CURRENT_SOURCE_DIR}/lib/extension/membersheet.h
   ${CMAKE_CURRENT_SOURCE_DIR}/lib/extension/taskmenu.h
   ${CMAKE_CURRENT_SOURCE_DIR}/lib/extension/extrainfo.h
   ${CMAKE_CURRENT_SOURCE_DIR}/lib/extension/layoutdecoration.h

   ${CMAKE_CURRENT_SOURCE_DIR}/lib/extra/spacer_widget_p.h
   ${CMAKE_CURRENT_SOURCE_DIR}/lib/extra/layoutinfo_p.h
   ${CMAKE_CURRENT_SOURCE_DIR}/lib/extra/layout_p.h
   ${CMAKE_CURRENT_SOURCE_DIR}/lib/extra/connectionedit_p.h
   ${CMAKE_CURRENT_SOURCE_DIR}/lib/extra/metadatabase_p.h
   ${CMAKE_CURRENT_SOURCE_DIR}/lib/extra/qdesigner_formeditorcommand_p.h
   ${CMAKE_CURRENT_SOURCE_DIR}/lib/extra/qdesigner_formwindowcommand_p.h
   ${CMAKE_CURRENT_SOURCE_DIR}/lib/extra/qdesigner_command_p.h
   ${CMAKE_CURRENT_SOURCE_DIR}/lib/extra/morphmenu_p.h
   ${CMAKE_CURRENT_SOURCE_DIR}/lib/extra/qdesigner_command2_p.h
   ${CMAKE_CURRENT_SOURCE_DIR}/lib/extra/qdesigner_formbuilder_p.h
   ${CMAKE_CURRENT_SOURCE_DIR}/lib/extra/qdesigner_taskmenu_p.h
   ${CMAKE_CURRENT_SOURCE_DIR}/lib/extra/formlayoutmenu_p.h
   ${CMAKE_CURRENT_SOURCE_DIR}/lib/extra/qdesigner_widget_p.h
   ${CMAKE_CURRENT_SOURCE_DIR}/lib/extra/qdesigner_propertysheet_p.h
   ${CMAKE_CURRENT_SOURCE_DIR}/lib/extra/qdesigner_membersheet_p.h
   ${CMAKE_CURRENT_SOURCE_DIR}/lib/extra/qdesigner_propertyeditor_p.h
   ${CMAKE_CURRENT_SOURCE_DIR}/lib/extra/qdesigner_objectinspector_p.h
   ${CMAKE_CURRENT_SOURCE_DIR}/lib/extra/invisible_widget_p.h
   ${CMAKE_CURRENT_SOURCE_DIR}/lib/extra/qlayout_widget_p.h
   ${CMAKE_CURRENT_SOURCE_DIR}/lib/extra/sheet_delegate_p.h
   ${CMAKE_CURRENT_SOURCE_DIR}/lib/extra/qdesigner_stackedbox_p.h
   ${CMAKE_CURRENT_SOURCE_DIR}/lib/extra/qdesigner_tabwidget_p.h
   ${CMAKE_CURRENT_SOURCE_DIR}/lib/extra/qdesigner_dockwidget_p.h
   ${CMAKE_CURRENT_SOURCE_DIR}/lib/extra/qdesigner_toolbox_p.h
   ${CMAKE_CURRENT_SOURCE_DIR}/lib/extra/qdesigner_dnditem_p.h
   ${CMAKE_CURRENT_SOURCE_DIR}/lib/extra/widgetfactory_p.h
   ${CMAKE_CURRENT_SOURCE_DIR}/lib/extra/widgetdatabase_p.h
   ${CMAKE_CURRENT_SOURCE_DIR}/lib/extra/qdesigner_promotion_p.h
   ${CMAKE_CURRENT_SOURCE_DIR}/lib/extra/qdesigner_introspection_p.h
   ${CMAKE_CURRENT_SOURCE_DIR}/lib/extra/promotionmodel_p.h
   ${CMAKE_CURRENT_SOURCE_DIR}/lib/extra/qdesigner_promotiondialog_p.h
   ${CMAKE_CURRENT_SOURCE_DIR}/lib/extra/iconloader_p.h
   ${CMAKE_CURRENT_SOURCE_DIR}/lib/extra/qdesigner_toolbar_p.h
   ${CMAKE_CURRENT_SOURCE_DIR}/lib/extra/qdesigner_menubar_p.h
   ${CMAKE_CURRENT_SOURCE_DIR}/lib/extra/qdesigner_menu_p.h
   ${CMAKE_CURRENT_SOURCE_DIR}/lib/extra/csshighlighter_p.h
   ${CMAKE_CURRENT_SOURCE_DIR}/lib/extra/shared_enums_p.h
   ${CMAKE_CURRENT_SOURCE_DIR}/lib/extra/propertylineedit_p.h
   ${CMAKE_CURRENT_SOURCE_DIR}/lib/extra/promotiontaskmenu_p.h
   ${CMAKE_CURRENT_SOURCE_DIR}/lib/extra/gridpanel_p.h
   ${CMAKE_CURRENT_SOURCE_DIR}/lib/extra/grid_p.h
   ${CMAKE_CURRENT_SOURCE_DIR}/lib/extra/formwindowbase_p.h
   ${CMAKE_CURRENT_SOURCE_DIR}/lib/extra/qdesigner_utils_p.h
   ${CMAKE_CURRENT_SOURCE_DIR}/lib/extra/qdesigner_widgetbox_p.h
   ${CMAKE_CURRENT_SOURCE_DIR}/lib/extra/extensionfactory_p.h
   ${CMAKE_CURRENT_SOURCE_DIR}/lib/extra/dialoggui.h
   ${CMAKE_CURRENT_SOURCE_DIR}/lib/extra/deviceprofile_p.h
   ${CMAKE_CURRENT_SOURCE_DIR}/lib/extra/zoomwidget_p.h
   ${CMAKE_CURRENT_SOURCE_DIR}/lib/extra/iconselector_p.h
   ${CMAKE_CURRENT_SOURCE_DIR}/lib/extra/htmlhighlighter_p.h
   ${CMAKE_CURRENT_SOURCE_DIR}/lib/extra/qdesigner_widgetitem_p.h
   ${CMAKE_CURRENT_SOURCE_DIR}/lib/extra/qdesigner_qsettings_p.h
   ${CMAKE_CURRENT_SOURCE_DIR}/lib/extra/qdesigner_formwindowmanager_p.h
   ${CMAKE_CURRENT_SOURCE_DIR}/lib/extra/shared_settings_p.h
   ${CMAKE_CURRENT_SOURCE_DIR}/lib/extra/rcc_support.h

   ${CMAKE_CURRENT_SOURCE_DIR}/lib/find_widget/abstract_findwidget.h
   ${CMAKE_CURRENT_SOURCE_DIR}/lib/find_widget/textedit_findwidget.h
   ${CMAKE_CURRENT_SOURCE_DIR}/lib/find_widget/itemview_findwidget.h
   ${CMAKE_CURRENT_SOURCE_DIR}/lib/font_panel/fontpanel.h

   ${CMAKE_CURRENT_SOURCE_DIR}/lib/object_inspector/object_inspector.h
   ${CMAKE_CURRENT_SOURCE_DIR}/lib/object_inspector/object_inspector_model.h

   ${CMAKE_CURRENT_SOURCE_DIR}/lib/plugin/customwidget.h
   ${CMAKE_CURRENT_SOURCE_DIR}/lib/plugin/plugin_dialog.h
   ${CMAKE_CURRENT_SOURCE_DIR}/lib/plugin/plugin_manager.h

   ${CMAKE_CURRENT_SOURCE_DIR}/lib/resource/resource_editor.h
   ${CMAKE_CURRENT_SOURCE_DIR}/lib/resource/resource_model.h
   ${CMAKE_CURRENT_SOURCE_DIR}/lib/resource/resource_view.h
   ${CMAKE_CURRENT_SOURCE_DIR}/lib/resource/simple_resource.h

   ${CMAKE_CURRENT_SOURCE_DIR}/lib/ui_loader/ui_loader.h
   ${CMAKE_CURRENT_SOURCE_DIR}/lib/ui_loader/ui_loader_p.h

   ${CMAKE_CURRENT_SOURCE_DIR}/lib/util/qlonglong_validator.h
   ${CMAKE_CURRENT_SOURCE_DIR}/lib/util/table_classes.h
   ${CMAKE_CURRENT_SOURCE_DIR}/lib/util/utils.h

   ${CMAKE_CURRENT_SOURCE_DIR}/lib/widget_box/widgetbox.h
   ${CMAKE_CURRENT_SOURCE_DIR}/lib/widget_box/widgetbox_categorylistview.h
   ${CMAKE_CURRENT_SOURCE_DIR}/lib/widget_box/widgetbox_treewidget.h
   ${CMAKE_CURRENT_SOURCE_DIR}/lib/widget_box/widgetbox_dnditem.h
)

list(APPEND DESIGNER_SOURCES
   ${CMAKE_CURRENT_SOURCE_DIR}/lib/extension/default_extensionfactory.cpp
   ${CMAKE_CURRENT_SOURCE_DIR}/lib/extension/extension.cpp
   ${CMAKE_CURRENT_SOURCE_DIR}/lib/extension/extrainfo.cpp
   ${CMAKE_CURRENT_SOURCE_DIR}/lib/extension/extension_manager.cpp

   ${CMAKE_CURRENT_SOURCE_DIR}/lib/extra/spacer_widget.cpp
   ${CMAKE_CURRENT_SOURCE_DIR}/lib/extra/layoutinfo.cpp
   ${CMAKE_CURRENT_SOURCE_DIR}/lib/extra/layout.cpp
   ${CMAKE_CURRENT_SOURCE_DIR}/lib/extra/connectionedit.cpp
   ${CMAKE_CURRENT_SOURCE_DIR}/lib/extra/qdesigner_formwindowcommand.cpp
   ${CMAKE_CURRENT_SOURCE_DIR}/lib/extra/qdesigner_formeditorcommand.cpp
   ${CMAKE_CURRENT_SOURCE_DIR}/lib/extra/qdesigner_command.cpp
   ${CMAKE_CURRENT_SOURCE_DIR}/lib/extra/morphmenu.cpp
   ${CMAKE_CURRENT_SOURCE_DIR}/lib/extra/qdesigner_command2.cpp
   ${CMAKE_CURRENT_SOURCE_DIR}/lib/extra/qdesigner_propertycommand.cpp
   ${CMAKE_CURRENT_SOURCE_DIR}/lib/extra/qdesigner_formbuilder.cpp
   ${CMAKE_CURRENT_SOURCE_DIR}/lib/extra/qdesigner_taskmenu.cpp
   ${CMAKE_CURRENT_SOURCE_DIR}/lib/extra/formlayoutmenu.cpp
   ${CMAKE_CURRENT_SOURCE_DIR}/lib/extra/qdesigner_widget.cpp
   ${CMAKE_CURRENT_SOURCE_DIR}/lib/extra/qdesigner_dockwidget.cpp
   ${CMAKE_CURRENT_SOURCE_DIR}/lib/extra/qdesigner_propertysheet.cpp
   ${CMAKE_CURRENT_SOURCE_DIR}/lib/extra/qdesigner_membersheet.cpp
   ${CMAKE_CURRENT_SOURCE_DIR}/lib/extra/qdesigner_propertyeditor.cpp
   ${CMAKE_CURRENT_SOURCE_DIR}/lib/extra/qdesigner_objectinspector.cpp
   ${CMAKE_CURRENT_SOURCE_DIR}/lib/extra/qdesigner_dnditem.cpp
   ${CMAKE_CURRENT_SOURCE_DIR}/lib/extra/invisible_widget.cpp
   ${CMAKE_CURRENT_SOURCE_DIR}/lib/extra/qlayout_widget.cpp
   ${CMAKE_CURRENT_SOURCE_DIR}/lib/extra/sheet_delegate.cpp
   ${CMAKE_CURRENT_SOURCE_DIR}/lib/extra/metadatabase.cpp
   ${CMAKE_CURRENT_SOURCE_DIR}/lib/extra/qdesigner_stackedbox.cpp
   ${CMAKE_CURRENT_SOURCE_DIR}/lib/extra/qdesigner_tabwidget.cpp
   ${CMAKE_CURRENT_SOURCE_DIR}/lib/extra/qdesigner_toolbox.cpp
   ${CMAKE_CURRENT_SOURCE_DIR}/lib/extra/widgetfactory.cpp
   ${CMAKE_CURRENT_SOURCE_DIR}/lib/extra/widgetdatabase.cpp
   ${CMAKE_CURRENT_SOURCE_DIR}/lib/extra/qdesigner_promotion.cpp
   ${CMAKE_CURRENT_SOURCE_DIR}/lib/extra/qdesigner_introspection.cpp
   ${CMAKE_CURRENT_SOURCE_DIR}/lib/extra/promotionmodel.cpp
   ${CMAKE_CURRENT_SOURCE_DIR}/lib/extra/qdesigner_promotiondialog.cpp
   ${CMAKE_CURRENT_SOURCE_DIR}/lib/extra/qdesigner_toolbar.cpp
   ${CMAKE_CURRENT_SOURCE_DIR}/lib/extra/qdesigner_menubar.cpp
   ${CMAKE_CURRENT_SOURCE_DIR}/lib/extra/qdesigner_menu.cpp
   ${CMAKE_CURRENT_SOURCE_DIR}/lib/extra/csshighlighter.cpp
   ${CMAKE_CURRENT_SOURCE_DIR}/lib/extra/propertylineedit.cpp
   ${CMAKE_CURRENT_SOURCE_DIR}/lib/extra/promotiontaskmenu.cpp
   ${CMAKE_CURRENT_SOURCE_DIR}/lib/extra/gridpanel.cpp
   ${CMAKE_CURRENT_SOURCE_DIR}/lib/extra/grid.cpp
   ${CMAKE_CURRENT_SOURCE_DIR}/lib/extra/formwindowbase.cpp
   ${CMAKE_CURRENT_SOURCE_DIR}/lib/extra/qdesigner_utils.cpp
   ${CMAKE_CURRENT_SOURCE_DIR}/lib/extra/qdesigner_widgetbox.cpp
   ${CMAKE_CURRENT_SOURCE_DIR}/lib/extra/iconloader.cpp
   ${CMAKE_CURRENT_SOURCE_DIR}/lib/extra/dialoggui.cpp
   ${CMAKE_CURRENT_SOURCE_DIR}/lib/extra/deviceprofile.cpp
   ${CMAKE_CURRENT_SOURCE_DIR}/lib/extra/zoomwidget.cpp
   ${CMAKE_CURRENT_SOURCE_DIR}/lib/extra/iconselector.cpp
   ${CMAKE_CURRENT_SOURCE_DIR}/lib/extra/htmlhighlighter.cpp
   ${CMAKE_CURRENT_SOURCE_DIR}/lib/extra/qdesigner_widgetitem.cpp
   ${CMAKE_CURRENT_SOURCE_DIR}/lib/extra/qdesigner_qsettings.cpp
   ${CMAKE_CURRENT_SOURCE_DIR}/lib/extra/qdesigner_formwindowmanager.cpp
   ${CMAKE_CURRENT_SOURCE_DIR}/lib/extra/shared_settings.cpp
   ${CMAKE_CURRENT_SOURCE_DIR}/lib/extra/rcc_support.cpp

   ${CMAKE_CURRENT_SOURCE_DIR}/lib/find_widget/abstract_findwidget.cpp
   ${CMAKE_CURRENT_SOURCE_DIR}/lib/find_widget/textedit_findwidget.cpp
   ${CMAKE_CURRENT_SOURCE_DIR}/lib/find_widget/itemview_findwidget.cpp
   ${CMAKE_CURRENT_SOURCE_DIR}/lib/font_panel/fontpanel.cpp

   ${CMAKE_CURRENT_SOURCE_DIR}/lib/object_inspector/object_inspector.cpp
   ${CMAKE_CURRENT_SOURCE_DIR}/lib/object_inspector/object_inspector_model.cpp

   ${CMAKE_CURRENT_SOURCE_DIR}/lib/plugin/plugin_dialog.cpp
   ${CMAKE_CURRENT_SOURCE_DIR}/lib/plugin/plugin_manager.cpp

   ${CMAKE_CURRENT_SOURCE_DIR}/lib/resource/resource_editor.cpp
   ${CMAKE_CURRENT_SOURCE_DIR}/lib/resource/resource_model.cpp
   ${CMAKE_CURRENT_SOURCE_DIR}/lib/resource/resource_view.cpp
   ${CMAKE_CURRENT_SOURCE_DIR}/lib/resource/simple_resource.cpp

   ${CMAKE_CURRENT_SOURCE_DIR}/lib/ui_loader/ui_loader.cpp

   ${CMAKE_CURRENT_SOURCE_DIR}/lib/util/qlonglong_validator.cpp
   ${CMAKE_CURRENT_SOURCE_DIR}/lib/util/utils.cpp

   ${CMAKE_CURRENT_SOURCE_DIR}/lib/widget_box/widgetbox.cpp
   ${CMAKE_CURRENT_SOURCE_DIR}/lib/widget_box/widgetbox_categorylistview.cpp
   ${CMAKE_CURRENT_SOURCE_DIR}/lib/widget_box/widgetbox_treewidget.cpp
   ${CMAKE_CURRENT_SOURCE_DIR}/lib/widget_box/widgetbox_dnditem.cpp
)
