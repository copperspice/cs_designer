list(APPEND DESIGNER_INCLUDES

   ${CMAKE_CURRENT_SOURCE_DIR}/lib/extension/container.h
   ${CMAKE_CURRENT_SOURCE_DIR}/lib/extension/default_extensionfactory.h
   ${CMAKE_CURRENT_SOURCE_DIR}/lib/extension/dynamicpropertysheet.h
   ${CMAKE_CURRENT_SOURCE_DIR}/lib/extension/extension.h
   ${CMAKE_CURRENT_SOURCE_DIR}/lib/extension/extrainfo.h
   ${CMAKE_CURRENT_SOURCE_DIR}/lib/extension/extension_manager.h
   ${CMAKE_CURRENT_SOURCE_DIR}/lib/extension/layoutdecoration.h
   ${CMAKE_CURRENT_SOURCE_DIR}/lib/extension/membersheet.h
   ${CMAKE_CURRENT_SOURCE_DIR}/lib/extension/propertysheet.h
   ${CMAKE_CURRENT_SOURCE_DIR}/lib/extension/taskmenu.h

   ${CMAKE_CURRENT_SOURCE_DIR}/lib/extra/connection_edit.h
   ${CMAKE_CURRENT_SOURCE_DIR}/lib/extra/csshighlighter_p.h
   ${CMAKE_CURRENT_SOURCE_DIR}/lib/extra/deviceprofile_p.h
   ${CMAKE_CURRENT_SOURCE_DIR}/lib/extra/dialoggui.h
   ${CMAKE_CURRENT_SOURCE_DIR}/lib/extra/extensionfactory_p.h
   ${CMAKE_CURRENT_SOURCE_DIR}/lib/extra/formlayoutmenu_p.h
   ${CMAKE_CURRENT_SOURCE_DIR}/lib/extra/formwindowbase_p.h
   ${CMAKE_CURRENT_SOURCE_DIR}/lib/extra/grid_p.h
   ${CMAKE_CURRENT_SOURCE_DIR}/lib/extra/gridpanel_p.h
   ${CMAKE_CURRENT_SOURCE_DIR}/lib/extra/htmlhighlighter_p.h
   ${CMAKE_CURRENT_SOURCE_DIR}/lib/extra/iconloader_p.h
   ${CMAKE_CURRENT_SOURCE_DIR}/lib/extra/iconselector_p.h
   ${CMAKE_CURRENT_SOURCE_DIR}/lib/extra/invisible_widget_p.h
   ${CMAKE_CURRENT_SOURCE_DIR}/lib/extra/layout.h
   ${CMAKE_CURRENT_SOURCE_DIR}/lib/extra/layout_info.h
   ${CMAKE_CURRENT_SOURCE_DIR}/lib/extra/layout_widget.h
   ${CMAKE_CURRENT_SOURCE_DIR}/lib/extra/metadatabase_p.h
   ${CMAKE_CURRENT_SOURCE_DIR}/lib/extra/morphmenu_p.h
   ${CMAKE_CURRENT_SOURCE_DIR}/lib/extra/promotion_model.h
   ${CMAKE_CURRENT_SOURCE_DIR}/lib/extra/promotion_taskmenu.h
   ${CMAKE_CURRENT_SOURCE_DIR}/lib/extra/propertylineedit_p.h
   ${CMAKE_CURRENT_SOURCE_DIR}/lib/extra/rcc_support.h
   ${CMAKE_CURRENT_SOURCE_DIR}/lib/extra/shared_enums_p.h
   ${CMAKE_CURRENT_SOURCE_DIR}/lib/extra/shared_settings_p.h
   ${CMAKE_CURRENT_SOURCE_DIR}/lib/extra/sheet_delegate_p.h
   ${CMAKE_CURRENT_SOURCE_DIR}/lib/extra/spacer_widget_p.h
   ${CMAKE_CURRENT_SOURCE_DIR}/lib/extra/widgetdatabase_p.h
   ${CMAKE_CURRENT_SOURCE_DIR}/lib/extra/widgetfactory.h
   ${CMAKE_CURRENT_SOURCE_DIR}/lib/extra/zoomwidget_p.h

   ${CMAKE_CURRENT_SOURCE_DIR}/lib/find_widget/abstract_findwidget.h
   ${CMAKE_CURRENT_SOURCE_DIR}/lib/find_widget/itemview_findwidget.h
   ${CMAKE_CURRENT_SOURCE_DIR}/lib/find_widget/textedit_findwidget.h

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

   ${CMAKE_CURRENT_SOURCE_DIR}/lib/util/table_classes.h
   ${CMAKE_CURRENT_SOURCE_DIR}/lib/util/utils.h
   ${CMAKE_CURRENT_SOURCE_DIR}/lib/util/validator_longlong.h

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

   ${CMAKE_CURRENT_SOURCE_DIR}/lib/extra/connection_edit.cpp
   ${CMAKE_CURRENT_SOURCE_DIR}/lib/extra/csshighlighter.cpp
   ${CMAKE_CURRENT_SOURCE_DIR}/lib/extra/deviceprofile.cpp
   ${CMAKE_CURRENT_SOURCE_DIR}/lib/extra/dialoggui.cpp
   ${CMAKE_CURRENT_SOURCE_DIR}/lib/extra/formlayoutmenu.cpp
   ${CMAKE_CURRENT_SOURCE_DIR}/lib/extra/formwindowbase.cpp
   ${CMAKE_CURRENT_SOURCE_DIR}/lib/extra/grid.cpp
   ${CMAKE_CURRENT_SOURCE_DIR}/lib/extra/gridpanel.cpp
   ${CMAKE_CURRENT_SOURCE_DIR}/lib/extra/htmlhighlighter.cpp
   ${CMAKE_CURRENT_SOURCE_DIR}/lib/extra/iconloader.cpp
   ${CMAKE_CURRENT_SOURCE_DIR}/lib/extra/iconselector.cpp
   ${CMAKE_CURRENT_SOURCE_DIR}/lib/extra/invisible_widget.cpp
   ${CMAKE_CURRENT_SOURCE_DIR}/lib/extra/layout.cpp
   ${CMAKE_CURRENT_SOURCE_DIR}/lib/extra/layout_info.cpp
   ${CMAKE_CURRENT_SOURCE_DIR}/lib/extra/layout_widget.cpp
   ${CMAKE_CURRENT_SOURCE_DIR}/lib/extra/metadatabase.cpp
   ${CMAKE_CURRENT_SOURCE_DIR}/lib/extra/morphmenu.cpp
   ${CMAKE_CURRENT_SOURCE_DIR}/lib/extra/promotion_model.cpp
   ${CMAKE_CURRENT_SOURCE_DIR}/lib/extra/promotion_taskmenu.cpp
   ${CMAKE_CURRENT_SOURCE_DIR}/lib/extra/propertylineedit.cpp
   ${CMAKE_CURRENT_SOURCE_DIR}/lib/extra/rcc_support.cpp
   ${CMAKE_CURRENT_SOURCE_DIR}/lib/extra/shared_settings.cpp
   ${CMAKE_CURRENT_SOURCE_DIR}/lib/extra/sheet_delegate.cpp
   ${CMAKE_CURRENT_SOURCE_DIR}/lib/extra/spacer_widget.cpp
   ${CMAKE_CURRENT_SOURCE_DIR}/lib/extra/widgetdatabase.cpp
   ${CMAKE_CURRENT_SOURCE_DIR}/lib/extra/widgetfactory.cpp
   ${CMAKE_CURRENT_SOURCE_DIR}/lib/extra/zoomwidget.cpp

   ${CMAKE_CURRENT_SOURCE_DIR}/lib/find_widget/abstract_findwidget.cpp
   ${CMAKE_CURRENT_SOURCE_DIR}/lib/find_widget/itemview_findwidget.cpp
   ${CMAKE_CURRENT_SOURCE_DIR}/lib/find_widget/textedit_findwidget.cpp

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

   ${CMAKE_CURRENT_SOURCE_DIR}/lib/util/utils.cpp
   ${CMAKE_CURRENT_SOURCE_DIR}/lib/util/validator_longlong.cpp

   ${CMAKE_CURRENT_SOURCE_DIR}/lib/widget_box/widgetbox.cpp
   ${CMAKE_CURRENT_SOURCE_DIR}/lib/widget_box/widgetbox_categorylistview.cpp
   ${CMAKE_CURRENT_SOURCE_DIR}/lib/widget_box/widgetbox_treewidget.cpp
   ${CMAKE_CURRENT_SOURCE_DIR}/lib/widget_box/widgetbox_dnditem.cpp
)
