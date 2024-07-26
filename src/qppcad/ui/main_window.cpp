#include <qppcad/ui/main_window.hpp>
#include <qppcad/core/app_state.hpp>
#include <qppcad/core/workspace.hpp>

#include <qppcad/ui/app_settings_widget.hpp>
#include <qppcad/ui/add_new_ws_item_widget.hpp>
#include <qppcad/ui/toolbar_element.hpp>
#include <qppcad/ui/ptable_rich_widget.hpp>
#include <qppcad/ui/toolbar_element.hpp>
#include <qppcad/ui/create_arrow_array.hpp>

#include <qppcad/ws_item/ws_item_behaviour_manager.hpp>
#include <qppcad/ws_item/geom_view/geom_view.hpp>
#include <qppcad/ws_item/geom_view/geom_view_anim_subsys.hpp>
#include <qppcad/ws_item/geom_view/geom_view_measurement_subsys.hpp>
#include <qppcad/ws_item/geom_view/geom_view_labels_subsys.hpp>
#include <qppcad/ws_item/arrow_primitive/arrow_primitive.hpp>
#include <qppcad/python/python_simple_query.hpp>
#include <qppcad/python/python_plugin.hpp>
#include <qppcad/python/plugin_param_editor.hpp>

#include <QDateTime>
#include <QColorDialog>

using namespace qpp;
using namespace qpp::cad;

main_window_t::main_window_t(QWidget *parent) : QMainWindow(parent) {

  main_widget = new QWidget;
  app_state_t* astate = app_state_t::get_inst();

  setAcceptDrops(true);
  setCentralWidget(main_widget);
  setMinimumHeight(astate->size_guide.main_window_h());
  setMinimumWidth(astate->size_guide.main_window_w());

  init_base_shortcuts();
  init_menus();
  build_bhv_menus_and_actions();
  build_bhv_tools_menus();
  control_bhv_menus_activity();
  control_bhv_tools_menus_activity();
  init_widgets();
  //astate -> plug_mgr -> load_plugins();
  init_layouts();
  build_bhv_toolpanel();

  connect(astate->astate_evd,
          &app_state_event_disp_t::wss_changed_signal,
          this,
          &main_window_t::wss_changed_slot);

  connect(astate->astate_evd,
          &app_state_event_disp_t::cur_ws_changed_signal,
          this,
          &main_window_t::cur_ws_changed);

  connect(astate->astate_evd,
          &app_state_event_disp_t::cur_ws_selected_item_changed_signal,
          this,
          &main_window_t::cur_ws_selected_item_changed);

  connect(astate->astate_evd,
          &app_state_event_disp_t::cur_ws_edit_type_changed_signal,
          this,
          &main_window_t::cur_ws_edit_type_changed);

  connect(astate->astate_evd,
          &app_state_event_disp_t::cur_ws_selected_atoms_list_selection_changed_signal,
          this,
          &main_window_t::cur_ws_selected_atoms_list_selection_changed);

  connect(astate->astate_evd,
          &app_state_event_disp_t::new_file_loaded_signal,
          this,
          &main_window_t::rebuild_recent_files_menu);

  wss_changed_slot();
  cur_ws_changed();
  cur_ws_edit_type_changed();
  cur_ws_selected_atoms_list_selection_changed();

}

main_window_t::~main_window_t() {

}

void main_window_t::init_base_shortcuts() {

  sc_toggle_console = new QShortcut(this);
  sc_toggle_console->setKey(Qt::Key_QuoteLeft);
  connect(sc_toggle_console,
          &QShortcut::activated,
          this,
          &main_window_t::action_toggle_console);

  sc_enter_immersive_mode = new QShortcut(this);
  sc_enter_immersive_mode->setKey(Qt::Key_F12);
  connect(sc_enter_immersive_mode,
          &QShortcut::activated,
          this,
          &main_window_t::toggle_immersive_mode);

}

// asm -----------------


std::shared_ptr<ws_item_t> construct_from_geom1(
    workspace_t &ws,
    std::shared_ptr<xgeometry<float, periodic_cell<float> > > geom,
    const std::string &name) {

    auto new_item = ws.m_owner->m_bhv_mgr->fbr_ws_item_by_type(geom_view_t::get_type_static());
    if (!new_item) return nullptr;

    auto as_gv = new_item->cast_as<geom_view_t>();
    if(!as_gv) return nullptr;

    if (as_gv->m_geom) {
        as_gv->m_geom->remove_observer(*as_gv->m_ext_obs);
        as_gv->m_geom->remove_observer(*as_gv->m_tws_tr);
      }

    as_gv->m_geom = geom;
    as_gv -> hardcoded_xfields = false;
    as_gv->m_ext_obs->geom = as_gv->m_geom.get();
    as_gv->m_tws_tr->geom = as_gv->m_geom.get();

    as_gv->m_tws_tr->do_action(act_lock | act_clear_all);
    as_gv->m_geom->add_observer(*as_gv->m_ext_obs);
    as_gv->m_geom->add_observer(*as_gv->m_tws_tr);
    as_gv->m_tws_tr->do_action(act_unlock | act_rebuild_tree);
    as_gv->m_tws_tr->do_action(act_rebuild_ntable);
    as_gv->m_name = name;
    ws.add_item_to_ws(new_item);

    return new_item;

}


bool consider_ws_add(py::object & obj){
  auto astate = app_state_t::get_inst();
  std::stringstream ss;
  py::object tp = py::eval("type");
  ss <<  py::cast<std::string>(py::str(tp(obj)));
  if (ss.str() == "<class 'pyqpp.xgeometry_f'>"){    
    auto g = py::cast<std::shared_ptr<xgeometry<float, periodic_cell<float> > > >(obj);
    construct_from_geom1(*(astate -> ws_mgr -> get_cur_ws()), g, g->name );
    return true;
  }
  return false;
}

std::string use_plugin_results(py::object & results){
  if (results == py::none())
    return "";
  std::string s;
  if (py::isinstance<py::tuple>(results)){
    auto tuple_res = py::cast<py::tuple>(results);
    for (auto hnd : tuple_res){
      auto obj = py::cast<py::object>(hnd);
      if (!consider_ws_add(obj))
	s = s + py::cast<std::string>(py::str(obj));
    }
  }
  else {
    if (!consider_ws_add(results))
      s = py::cast<std::string>(py::str(results));
  }
  return s;  
}

// asm -----------------

void build_plugins_menu(QMenu * menu, std::shared_ptr<plugin_tree_t> p){
  app_state_t* astate = app_state_t::get_inst();
  for (auto ptr : p -> nested){
    std::string s = ptr -> name;
    if (ptr -> nested.empty()){
      auto plugin_act = new QAction(nullptr);
      std::string ms = ptr -> plugin ->  plug_menu_name;
      plugin_act -> setText( ms!="" ? ms.c_str() : s.c_str());
      menu -> addAction(plugin_act);
      if (ptr -> plugin -> status != plugin_manager_t::plugmgr_ok)
	QObject::connect(plugin_act,
			 &QAction::triggered,
			 [s,ptr]() {
			   //astate -> plug_mgr -> load_plugins();
			   //ptr -> plugin -> run();
			   QMessageBox::warning(nullptr,
						s.c_str(),
						ptr -> plugin -> error_msg.c_str());
			 
		       });
      else {
	QObject::connect(plugin_act,
			 &QAction::triggered,
			 [s,ptr,astate]() {
			   //astate -> plug_mgr -> load_plugins();
			   plugin_param_editor_t  parm_dialog(ptr -> plugin);
			   parm_dialog.exec();
			   if (parm_dialog.result()==QDialog::Accepted)
			     try{
			       auto results = ptr -> plugin -> run();
			       std::string res = use_plugin_results(results);
			       if (res!="")
				 QMessageBox::information(nullptr,
							  s.c_str(),
							  res.c_str());
			     }
			     catch (py::error_already_set &err) {
			       astate -> make_viewport_dirty();
			       QMessageBox::warning(nullptr,
						    s.c_str(),
						    err.what());			     
			     }
			 });
	
      }
      
    }
    else {
      auto submenu = menu -> addMenu(QString(s.c_str()));
      build_plugins_menu(submenu,ptr);
    }
  }
}

// ---------------------- asm

void main_window_t::init_menus() {

  app_state_t* astate = app_state_t::get_inst();
  
  file_menu  = menuBar()->addMenu(tr("&File"));

  file_menu_new_ws = new QAction(nullptr);
  file_menu_new_ws->setText(tr("New workspace"));
  file_menu_new_ws->setShortcut(QKeySequence(tr("Ctrl+n")));
  file_menu->addAction(file_menu_new_ws);
  connect(file_menu_new_ws,
          &QAction::triggered,
          this,
          &main_window_t::create_new_ws);

  file_menu_close_ws = new QAction(nullptr);
  file_menu_close_ws->setText(tr("Close workspace"));
  file_menu->addAction(file_menu_close_ws);
  connect(file_menu_close_ws,
          &QAction::triggered,
          this,
          &main_window_t::close_cur_ws);

  file_menu_close_all_ws = new QAction(nullptr);
  file_menu_close_all_ws->setText(tr("Close all workspaces"));
  file_menu->addAction(file_menu_close_all_ws);
  connect(file_menu_close_all_ws,
          &QAction::triggered,
          this,
          &main_window_t::close_all_ws);

  file_menu_open_ws = new QAction(nullptr);
  file_menu_open_ws->setText(tr("Open workspace"));
  file_menu_open_ws->setShortcut(QKeySequence(tr("Ctrl+o")));
  file_menu->addAction(file_menu_open_ws);
  connect(file_menu_open_ws,
          &QAction::triggered,
          this,
          &main_window_t::open_ws);

  file_menu_save_ws = new QAction(nullptr);
  file_menu_save_ws->setText(tr("Save workspace"));
  file_menu_save_ws->setShortcut(QKeySequence(tr("Ctrl+s")));
  file_menu->addSeparator();
  file_menu->addAction(file_menu_save_ws);
  connect(file_menu_save_ws,
          &QAction::triggered,
          this,
          &main_window_t::save_ws);

  file_menu_save_ws_as = new QAction(nullptr);
  file_menu_save_ws_as->setText(tr("Save workspace as"));
  file_menu->addAction(file_menu_save_ws_as);
  connect(file_menu_save_ws_as,
          &QAction::triggered,
          this,
          &main_window_t::save_ws_as);

  file_menu->addSeparator();
  file_menu_import_as_new_ws = file_menu->addMenu(tr("Import as new workspace"));
  file_menu_import_to_cur_ws = file_menu->addMenu(tr("Import to current workspace"));
  file_menu_export_sel_as = file_menu->addMenu(tr("Export selected item"));

  file_menu_recent_files = file_menu->addMenu(tr("Recent files"));
  for (int i = 0; i < qpp::cad::max_recent_files; i++) {

      QAction *recent_act = new QAction(file_menu_recent_files);
      file_menu_recent_entries[i] = recent_act;
      file_menu_recent_files->addAction(recent_act);
      connect(recent_act,
              &QAction::triggered,
              this,
              &main_window_t::recent_files_clicked);

    }

  file_menu_close_app = new QAction(nullptr);
  file_menu_close_app->setText(tr("Close"));
  file_menu_close_app->setShortcut(QKeySequence(tr("Ctrl+q")));
  file_menu->addSeparator();
  file_menu->addAction(file_menu_close_app);
  connect(file_menu_close_app, &QAction::triggered, this,
          &main_window_t::slot_shortcut_terminate_app);

  edit_menu  = menuBar()->addMenu(tr("&Edit"));
  edit_menu_undo = new QAction(nullptr);
  edit_menu_undo->setText(tr("Undo"));
  edit_menu_undo->setEnabled(false);
  edit_menu_undo->setShortcut(QKeySequence(tr("Ctrl+u")));
  edit_menu->addAction(edit_menu_undo);

  edit_menu_redo = new QAction(nullptr);
  edit_menu_redo->setText(tr("Redo"));
  edit_menu_redo->setEnabled(false);
  edit_menu_redo->setShortcut(QKeySequence(tr("Ctrl+r")));
  edit_menu->addAction(edit_menu_redo);

  edit_menu->addSeparator();
  edit_menu_ptable_widget = new QAction(nullptr);
  edit_menu_ptable_widget->setText(tr("Periodic table"));
  edit_menu->addAction(edit_menu_ptable_widget);
  connect(edit_menu_ptable_widget,
          &QAction::triggered,
          []() {
                ptable_rich_widget_t ptable_rich_wdgt; ptable_rich_wdgt.exec();
                app_state_t::get_inst()->make_viewport_dirty();
          });

  edit_menu->addSeparator();

  //Selection menu
  edit_menu_selection = edit_menu->addMenu(tr("Content selection"));
  edit_menu_selection_select_all = new QAction(nullptr);
  edit_menu_selection_select_all->setText(tr("Select all"));
  edit_menu_selection_select_all->setShortcut(QKeySequence(tr("ctrl+a")));
  edit_menu_selection->addAction(edit_menu_selection_select_all);
  connect(edit_menu_selection_select_all,
          &QAction::triggered,
          this,
          &main_window_t::action_select_all_content);

  edit_menu_selection_unselect_all = new QAction(nullptr);
  edit_menu_selection_unselect_all->setText(tr("Unselect all"));
  edit_menu_selection_unselect_all->setShortcut(QKeySequence(tr("shift+a")));
  edit_menu_selection->addAction(edit_menu_selection_unselect_all);
  connect(edit_menu_selection_unselect_all,
          &QAction::triggered,
          this,
          &main_window_t::action_unselect_all_content);

  edit_menu_selection_invert = new QAction(nullptr);
  edit_menu_selection_invert->setText(tr("Invert selection"));
  edit_menu_selection_invert->setShortcut(QKeySequence(tr("ctrl+i")));
  edit_menu_selection->addAction(edit_menu_selection_invert);
  connect(edit_menu_selection_invert,
          &QAction::triggered,
          this,
          &main_window_t::action_invert_selected_content);
  //End of selection menu

  edit_menu_switch_ws_edit_mode = new QAction(nullptr);
  edit_menu_switch_ws_edit_mode->setText(tr("Switch edit mode"));
  edit_menu_switch_ws_edit_mode->setShortcut(Qt::Key::Key_Tab);
  connect(edit_menu_switch_ws_edit_mode,
          &QAction::triggered,
          this,
          &main_window_t::toggle_ws_edit_mode);

  edit_menu->addAction(edit_menu_switch_ws_edit_mode);

  // tools menu
  tools_menu = menuBar()->addMenu(tr("&Tools"));
  // end tools menu

  // workspace menu
  ws_menu = menuBar()->addMenu(tr("&Workspace"));

  ws_menu_rename_ws = new QAction(nullptr);
  ws_menu_rename_ws->setText(tr("Rename workspace"));
  ws_menu->addAction(ws_menu_rename_ws);
  connect(ws_menu_rename_ws,
          &QAction::triggered,
          this,
          &main_window_t::rename_cur_ws);

  // asm
  ws_create_new_item = ws_menu->addMenu(tr("&Create new..."));

  ws_create_geometry = new QAction(nullptr);
  ws_create_geometry -> setText(tr("Geometry"));
  ws_create_new_item -> addAction(ws_create_geometry);  
  connect(ws_create_geometry,
          &QAction::triggered,
          this,
          &main_window_t::action_select_all_content);

  ws_create_symmetry = new QAction(nullptr);
  ws_create_symmetry -> setText(tr("Symmetry group"));
  ws_create_new_item -> addAction(ws_create_symmetry);
  connect(ws_create_symmetry,
          &QAction::triggered,
          this,
          &main_window_t::action_select_all_content);

  ws_create_gprimitive = new QAction(nullptr);
  ws_create_gprimitive -> setText(tr("Graphic primitive"));
  ws_create_new_item -> addAction(ws_create_gprimitive);
  connect(ws_create_gprimitive,
          &QAction::triggered,
          this,
          &main_window_t::action_select_all_content);

  ws_create_arrow_array = new QAction(nullptr);
  ws_create_arrow_array -> setText(tr("Arrow array"));
  ws_create_new_item -> addAction(ws_create_arrow_array);
  connect(ws_create_arrow_array,
          &QAction::triggered,
          this,
          &main_window_t::action_create_arrow_array);
  
  ws_menu_bg_color = new QAction(nullptr);
  ws_menu_bg_color->setText(tr("Change background"));
  ws_menu->addAction(ws_menu_bg_color);
  connect(ws_menu_bg_color,
          &QAction::triggered,
          this,
          &main_window_t::change_cur_ws_bg);

  ws_copy_cam = new QAction(nullptr);
  ws_copy_cam->setText(tr("Copy camera to all"));
  ws_menu->addAction(ws_copy_cam);
  connect(ws_copy_cam,
          &QAction::triggered,
          this,
          &main_window_t::cam_copy_to_all);
  // end workspace menu

  // view menu
  view_menu = menuBar()->addMenu(tr("&View"));

  view_menu_show_oi = new QAction(nullptr);
  view_menu_show_oi->setText(tr("Show object inspector"));
  view_menu_show_oi->setCheckable(true);
  view_menu->addAction(view_menu_show_oi);
  connect(view_menu_show_oi,
          &QAction::toggled,
          this,
          &main_window_t::show_obj_insp_state_changed);

  view_menu_show_gizmo = new QAction(nullptr);
  view_menu_show_gizmo->setText(tr("Show axes"));
  view_menu_show_gizmo->setCheckable(true);
  view_menu->addAction(view_menu_show_gizmo);
  connect(view_menu_show_gizmo,
          &QAction::toggled,
          this,
          &main_window_t::show_gizmo_state_changed);

  view_menu_toggle_fullscreen = new QAction(nullptr);
  view_menu_toggle_fullscreen->setText(tr("Toggle Fullscreen"));
  view_menu_toggle_fullscreen->setCheckable(true);
  view_menu_toggle_fullscreen->setShortcut(Qt::Key::Key_F11);
  view_menu->addAction(view_menu_toggle_fullscreen);
  connect(view_menu_toggle_fullscreen,
          &QAction::toggled,
          this,
          &main_window_t::toggle_fullscreen);

  view_menu_settings = new QAction(nullptr);
  view_menu_settings->setEnabled(true);
  view_menu_settings->setText(tr("Settings"));
  connect(view_menu_settings,
          &QAction::triggered,[](){app_settings_widget_t aset;aset.exec();});

  view_menu_ws_settings = new QAction(nullptr);
  view_menu_ws_settings->setEnabled(true);
  view_menu_ws_settings->setText(tr("Workspace settings"));
  // asm
  edit_menu->addAction(view_menu_settings);
  view_menu->addAction(view_menu_ws_settings);

  view_menu_log_wdgt = new QAction(nullptr);
  view_menu_log_wdgt->setText(tr("Show log"));
  view_menu_log_wdgt->setCheckable(true);
  connect(view_menu_log_wdgt,
          &QAction::toggled,
          this,
          &main_window_t::show_log_wdgt_state_changed);
  view_menu->addAction(view_menu_log_wdgt);

  view_menu_console = new QAction(nullptr);
  view_menu_console->setText(tr("Console"));
  view_menu_console->setShortcut(QKeySequence(tr("~")));
  view_menu_console->setShortcutContext(Qt::ShortcutContext::ApplicationShortcut);
  connect(view_menu_console,
          &QAction::triggered,
          this,
          &main_window_t::action_toggle_console);
  view_menu->addAction(view_menu_console);

  view_menu_debug = view_menu->addMenu(tr("Debug"));

  view_menu_toggle_debug_info = new QAction(nullptr);
  view_menu_toggle_debug_info->setText(tr("Show frame info"));
  view_menu_toggle_debug_info->setCheckable(true);
  view_menu_debug->addAction(view_menu_toggle_debug_info);
  connect(view_menu_toggle_debug_info,
          &QAction::toggled,
          [](bool checked) {
            app_state_t* astate = app_state_t::get_inst();
            astate->m_show_debug_frame_stats = checked;
            astate->make_viewport_dirty();
          });

  view_menu_toggle_debug_tws_tree = new QAction(nullptr);
  view_menu_toggle_debug_tws_tree->setText(tr("Render tws-tree"));
  view_menu_toggle_debug_tws_tree->setCheckable(true);
  view_menu_debug->addAction(view_menu_toggle_debug_tws_tree);
  connect(view_menu_toggle_debug_tws_tree,
          &QAction::toggled,
          [](bool checked) {
            app_state_t* astate = app_state_t::get_inst();
            astate->m_debug_show_tws_tree = checked;
            astate->make_viewport_dirty();
          });

  view_menu_toggle_sel_deque = new QAction(nullptr);
  view_menu_toggle_sel_deque->setText(tr("Show selection deque"));
  view_menu_toggle_sel_deque->setCheckable(true);
  view_menu_debug->addAction(view_menu_toggle_sel_deque);
  connect(view_menu_toggle_debug_tws_tree,
          &QAction::toggled,
          [](bool checked) {
            app_state_t* astate = app_state_t::get_inst();
            astate->m_debug_show_sel_deque = checked;
            astate->make_viewport_dirty();
          });

  // end of view menu

  // asm
  // plugins menu

  //astate -> tlog("plugmgr status: {}\n", astate -> plug_mgr -> status);
  if (astate -> plug_mgr -> status == plugin_manager_t::plugmgr_plugin_folder_not_found ||
      astate -> plug_mgr -> status == plugin_manager_t::plugmgr_plugins_missing){
    //astate -> tlog("plugin status:\n {}\n", astate -> plug_mgr -> error_msg);
    QMessageBox::warning(nullptr,
			 QString("Plugin error!"),
			 QString(astate -> plug_mgr -> error_msg.c_str()));    
  }
  else {
    //astate -> plug_mgr -> load_plugins();
    if (astate -> plug_mgr -> status != plugin_manager_t::plugmgr_ok)
      QMessageBox::warning(nullptr,
			   QString("Plugin error!"),
			   QString(astate -> plug_mgr -> error_msg.c_str()));      
    
    plugins_menu = menuBar()->addMenu(tr("&Plugins"));

    build_plugins_menu(plugins_menu, astate -> plug_mgr -> plug_tree);
    
    /*
    for (const auto & plugin : (astate -> plug_mgr -> hdr_names)){
	auto plugin_act = new QAction(nullptr);
	plugin_act -> setText(plugin.c_str());
	plugins_menu -> addAction(plugin_act);
    
	connect(plugin_act,
		&QAction::triggered,
		[plugin]() {
		  QMessageBox::warning(nullptr,
				       plugin.c_str(),
				       plugin.c_str());
		});
		}*/
    auto plugins_manage = new QAction(nullptr);
    plugins_manage -> setText(tr("Manage plugins ..."));
    plugins_manage -> setEnabled(false);
    plugins_menu -> addAction(plugins_manage);
    connect(plugins_manage,
	    &QAction::triggered,
	    this,
	    &main_window_t::action_plugin_manager);
    
  }
  // end of plugins menu
  
  // help menu
  help_menu  = menuBar()->addMenu(tr("&Help"));
  help_menu_about = new QAction(nullptr);
  help_menu_about->setText(tr("About"));
  help_menu->addAction(help_menu_about);
  connect(help_menu_about,
          &QAction::triggered,
          []() {
            QMessageBox::about(nullptr, "Fenzihua",
                               QObject::tr(
                                 "Site : <a href='https://github.com/mysovsky/fenzihua'>"
                                 "https://github.com/mysovsky/fenzihua.git"
                                 "</a><br>"
                                 "Git rev : %1<br>"
                                 "Build time : %2<br>"
                                 "Qt version : %3<br>"
                                 "Python : %4")
                               .arg(QString::fromStdString(build_info_helper::get_git_version()))
                               .arg(QString::fromStdString(build_info_helper::get_build_date()))
                               .arg(QString::fromStdString(build_info_helper::get_qt_version()))
                               .arg(QString::fromStdString(build_info_helper::get_python_version()))
                               );
          });

  // end of help menu

}

void main_window_t::init_widgets() {

  app_state_t* astate = app_state_t::get_inst();

  tool_panel_widget = new QWidget(nullptr);
  tool_panel_widget->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
  tool_panel_widget->setFixedHeight(astate->size_guide.tool_panel_h_exact());
  tool_panel_widget->setProperty("s_class", "tp_generic");
  tool_panel_widget->setObjectName("tool_panel_widget_e");

  tp_print_screen = new QPushButton(nullptr);
  tp_print_screen->setProperty("s_class", "tp_cb");
  tp_print_screen->setFixedWidth(astate->size_guide.tool_panel_ctrl_w());
  tp_print_screen->setFixedHeight(astate->size_guide.tool_panel_ctrl_h());
  tp_print_screen->setIconSize(QSize(astate->size_guide.tool_panel_icon_size(),
                                     astate->size_guide.tool_panel_icon_size()));
  tp_print_screen->setToolTip(tr("Save screenshot to current work dir"));
  tp_print_screen->setIcon(QIcon("://images/outline-camera_enhance-24px.svg"));
  connect(tp_print_screen,
          &QPushButton::pressed,
          this,
          &main_window_t::make_screenshot);

  tp_edit_mode = new QButtonGroup(nullptr);
  tp_edit_mode->setExclusive(true);
  connect(tp_edit_mode,
          static_cast<void(QButtonGroup::*)(int)>(&QButtonGroup::buttonClicked),
          this,
          &main_window_t::ws_edit_mode_selector_button_clicked);

  tp_edit_mode_item = new QPushButton(nullptr);
  tp_edit_mode_item->setFixedWidth(astate->size_guide.tool_panel_ctrl_w());
  tp_edit_mode_item->setFixedHeight(astate->size_guide.tool_panel_ctrl_h());
  tp_edit_mode_item->setIconSize(QSize(astate->size_guide.tool_panel_icon_size(),
                                       astate->size_guide.tool_panel_icon_size()));
  tp_edit_mode_item->setToolTip(tr("Set current workspace's edit mode to <edit item>"));
  tp_edit_mode_item->setIcon(QIcon("://images/edit0.svg"));
  tp_edit_mode_item->setCheckable(true);

  tp_edit_mode_content= new QPushButton(nullptr);
  tp_edit_mode_content->setFixedWidth(astate->size_guide.tool_panel_ctrl_w());
  tp_edit_mode_content->setFixedHeight(astate->size_guide.tool_panel_ctrl_h());
  tp_edit_mode_content->setIconSize(QSize(astate->size_guide.tool_panel_icon_size(),
                                          astate->size_guide.tool_panel_icon_size()));
  tp_edit_mode_content->setToolTip(tr("Set current workspace's edit mode to <edit content>"));
  tp_edit_mode_content->setIcon(QIcon("://images/edit1.svg"));
  tp_edit_mode_content->setCheckable(true);

  tp_edit_mode->addButton(tp_edit_mode_item, 0);
  tp_edit_mode->addButton(tp_edit_mode_content, 1);

  tp_edit_mode_start = new QFrame(nullptr);
  tp_edit_mode_start->setFrameShape(QFrame::VLine);
  tp_edit_mode_start->setFrameShadow(QFrame::Sunken);
  tp_edit_mode_start->setFixedWidth(3);
  tp_edit_mode_start->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Maximum);

  tp_edit_mode_end = new QFrame(nullptr);
  tp_edit_mode_end->setFrameShape(QFrame::VLine);
  tp_edit_mode_end->setFrameShadow(QFrame::Sunken);
  tp_edit_mode_end->setFixedWidth(3);
  tp_edit_mode_end->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Maximum);
  connect(tp_edit_mode,
          static_cast<void(QButtonGroup::*)(int)>(&QButtonGroup::buttonClicked),
          this,
          &main_window_t::ws_edit_mode_selector_button_clicked);

  tp_camera_tool = new QToolButton(nullptr);
  tp_camera_tool->setIcon(QIcon("://images/outline-videocam-24px.svg"));
  tp_camera_tool->setIconSize(QSize(astate->size_guide.tool_panel_icon_size(),
                                    astate->size_guide.tool_panel_icon_size()));

  tp_camera_tool->setPopupMode(QToolButton::InstantPopup);
  tp_camera_tool->setArrowType(Qt::NoArrow);
  tp_camera_tool->setFixedWidth(astate->size_guide.tool_panel_ctrl_w());
  tp_camera_tool->setMinimumHeight(astate->size_guide.tool_panel_ctrl_h());
  connect(tp_camera_tool,
          &QToolButton::triggered,
          this,
          &main_window_t::tp_camera_tool_button_triggered);

  tp_scenic_rot_cam = new QPushButton(nullptr);
  tp_scenic_rot_cam->setToolTip(tr("Orbit the camera"));
  tp_scenic_rot_cam->setIcon(QIcon("://images/baseline-3d_rotation_x-24px.svg"));
  tp_scenic_rot_cam->setFixedWidth(astate->size_guide.tool_panel_ctrl_w());
  tp_scenic_rot_cam->setFixedHeight(astate->size_guide.tool_panel_ctrl_h());
  tp_scenic_rot_cam->setIconSize(QSize(astate->size_guide.tool_panel_icon_size(),
                                       astate->size_guide.tool_panel_icon_size()));
  tp_scenic_rot_cam->setCheckable(true);
  connect(tp_scenic_rot_cam,
          &QPushButton::toggled,
          this,
          &main_window_t::tp_scenic_rotation_toggle);

  tp_camera_tool_act_x = new qextended_action(nullptr);
  tp_camera_tool_act_x->m_joined_data[0] = cam_tv_e::tv_x;
  tp_camera_tool_act_x->setText(tr( "x - axis"));
  tp_camera_tool->addAction(tp_camera_tool_act_x);

  tp_camera_tool_act_y = new qextended_action(nullptr);
  tp_camera_tool_act_y->m_joined_data[0] = cam_tv_e::tv_y;
  tp_camera_tool_act_y->setText(tr( "y - axis"));
  tp_camera_tool->addAction(tp_camera_tool_act_y);

  tp_camera_tool_act_z = new qextended_action(nullptr);
  tp_camera_tool_act_z->m_joined_data[0] = cam_tv_e::tv_z;
  tp_camera_tool_act_z->setText(tr( "z - axis"));
  tp_camera_tool->addAction(tp_camera_tool_act_z);

  tp_camera_tool_act_cc = new qextended_action(nullptr);
  tp_camera_tool_act_cc->m_joined_data[0] = cam_tv_e::tv_cc;
  tp_camera_tool_act_cc->setText(tr("cart. c."));
  tp_camera_tool->addAction(tp_camera_tool_act_cc);

  tp_camera_tool_act_a = new qextended_action(nullptr);
  tp_camera_tool_act_a->m_joined_data[0] = cam_tv_e::tv_a;
  tp_camera_tool_act_a->setText(tr( "a - axis"));
  tp_camera_tool->addAction(tp_camera_tool_act_a);

  tp_camera_tool_act_b = new qextended_action(nullptr);
  tp_camera_tool_act_b->m_joined_data[0] = cam_tv_e::tv_b;
  tp_camera_tool_act_b->setText(tr( "b - axis"));
  tp_camera_tool->addAction(tp_camera_tool_act_b);

  tp_camera_tool_act_c = new qextended_action(nullptr);
  tp_camera_tool_act_c->m_joined_data[0] = cam_tv_e::tv_c;
  tp_camera_tool_act_c->setText(tr( "c - axis"));
  tp_camera_tool->addAction(tp_camera_tool_act_c);

  tp_utility_frame_end = new QFrame(nullptr);
  tp_utility_frame_end->setFrameShape(QFrame::StyledPanel);
  tp_utility_frame_end->setFixedWidth(3);
  tp_utility_frame_end->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Maximum);

  tp_anim_fast_forward = new QPushButton(nullptr);
  tp_anim_fast_forward->setFixedWidth(astate->size_guide.tool_panel_ctrl_w());
  tp_anim_fast_forward->setFixedHeight(astate->size_guide.tool_panel_ctrl_h());
  tp_anim_fast_forward->setIconSize(QSize(astate->size_guide.tool_panel_icon_size(),
                                          astate->size_guide.tool_panel_icon_size()));
  tp_anim_fast_forward->setToolTip(tr("Fast-forward to last frame of animated geometry"));
  tp_anim_fast_forward->setIcon(QIcon("://images/outline-fast_forward-24px.svg"));
  connect(tp_anim_fast_forward,
          &QPushButton::clicked,
          this,
          &main_window_t::tp_fast_forward_anim_clicked);

  tp_measure_dist = new QPushButton(nullptr);
  tp_measure_dist->setFixedWidth(astate->size_guide.tool_panel_ctrl_w());
  tp_measure_dist->setFixedHeight(astate->size_guide.tool_panel_ctrl_h());
  tp_measure_dist->setCheckable(true);
  tp_measure_dist->setIconSize(QSize(astate->size_guide.tool_panel_icon_size(),
                                     astate->size_guide.tool_panel_icon_size()));
  tp_measure_dist->setToolTip(tr("Measure distance between pair of atoms"));
  tp_measure_dist->setIcon(QIcon("://images/dist.svg"));
  connect(tp_measure_dist,
          &QPushButton::toggled,
          this,
          &main_window_t::tp_dist_button_clicked);

  tp_toggle_atom_override = new QPushButton(nullptr);
  tp_toggle_atom_override->setFixedWidth(astate->size_guide.tool_panel_ctrl_w());
  tp_toggle_atom_override->setFixedHeight(astate->size_guide.tool_panel_ctrl_h());
  tp_toggle_atom_override->setCheckable(true);
  tp_toggle_atom_override->setIconSize(QSize(astate->size_guide.tool_panel_icon_size(),
                                             astate->size_guide.tool_panel_icon_size()));
  tp_toggle_atom_override->setToolTip(tr("Toggle atom override"));
  tp_toggle_atom_override->setIcon(QIcon("://images/outline-my_location-24px.svg"));
  connect(tp_toggle_atom_override,
          &QPushButton::toggled,
          this,
          &main_window_t::tp_toggle_atom_override_button_clicked);

  tp_force_sel_lbl_vis = new QPushButton(nullptr);
  tp_force_sel_lbl_vis->setFixedWidth(astate->size_guide.tool_panel_ctrl_w());
  tp_force_sel_lbl_vis->setFixedHeight(astate->size_guide.tool_panel_ctrl_h());
  tp_force_sel_lbl_vis->setCheckable(true);
  tp_force_sel_lbl_vis->setIconSize(QSize(astate->size_guide.tool_panel_icon_size(),
                                          astate->size_guide.tool_panel_icon_size()));
  tp_force_sel_lbl_vis->setToolTip(tr("Force labels selective visibility"));
  tp_force_sel_lbl_vis->setIcon(QIcon("://images/outline-font_download-24px.svg"));
  connect(tp_force_sel_lbl_vis,
          &QPushButton::toggled,
          this,
          &main_window_t::tp_force_sel_lbl_vis_button_clicked);

  tp_measure_angle = new QPushButton(nullptr);
  tp_measure_angle->setFixedWidth(astate->size_guide.tool_panel_ctrl_w());
  tp_measure_angle->setFixedHeight(astate->size_guide.tool_panel_ctrl_h());
  tp_measure_angle->setCheckable(true);
  tp_measure_angle->setIconSize(QSize(astate->size_guide.tool_panel_icon_size(),
                                      astate->size_guide.tool_panel_icon_size()));
  tp_measure_angle->setToolTip(tr("Measure angle between atoms triple"));
  tp_measure_angle->setIcon(QIcon("://images/angle.svg"));
  connect(tp_measure_angle,
          &QPushButton::toggled,
          this,
          &main_window_t::tp_angle_button_clicked);

  tp_add_cube = new QPushButton(nullptr);
  tp_add_cube->setFixedWidth(astate->size_guide.tool_panel_ctrl_w());
  tp_add_cube->setFixedHeight(astate->size_guide.tool_panel_ctrl_h());
  tp_add_cube->setToolTip(tr("Embed cube between two atoms"));
  tp_add_cube->setIcon(QIcon("://images/add_cube.svg"));
  tp_add_cube->setIconSize(QSize(astate->size_guide.tool_panel_icon_size(),
                                 astate->size_guide.tool_panel_icon_size()));
  connect(tp_add_cube,
          &QPushButton::clicked,
          this,
          &main_window_t::tp_add_cube_clicked);

  tp_add_arrow = new QPushButton(nullptr);
  tp_add_arrow->setFixedWidth(astate->size_guide.tool_panel_ctrl_w());
  tp_add_arrow->setFixedHeight(astate->size_guide.tool_panel_ctrl_h());
  tp_add_arrow->setToolTip(tr("Embed arrow between two atoms"));
  tp_add_arrow->setIcon(QIcon("://images/add_arrow.svg"));
  tp_add_arrow->setIconSize(QSize(astate->size_guide.tool_panel_icon_size(),
                                  astate->size_guide.tool_panel_icon_size()));
  connect(tp_add_arrow,
          &QPushButton::clicked,
          this,
          &main_window_t::tp_add_arrow_clicked);

  tp_add_point_sym_group = new QPushButton(nullptr);
  tp_add_point_sym_group->setFixedWidth(astate->size_guide.tool_panel_ctrl_w());
  tp_add_point_sym_group->setFixedHeight(astate->size_guide.tool_panel_ctrl_h());
  tp_add_point_sym_group->setToolTip(tr("Construct point symmetry group"));
  tp_add_point_sym_group->setIcon(QIcon("://images/add_psg.svg"));
  tp_add_point_sym_group->setIconSize(QSize(astate->size_guide.tool_panel_icon_size(),
                                            astate->size_guide.tool_panel_icon_size()));
  connect(tp_add_point_sym_group,
          &QPushButton::clicked,
          this,
          &main_window_t::tp_add_point_sym_group_clicked);

  ws_viewer_widget = new ws_viewer_widget_t(nullptr);

  obj_insp_widget = new object_inspector_widget_t(nullptr);
  obj_insp_widget->setFixedWidth(astate->size_guide.obj_insp_w());

  py_console_widget = new python_console_widget_t(nullptr);

  extended_editor_compositor = new ws_item_extended_editor_compositor_t(nullptr);

}

void main_window_t::init_layouts() {

  main_layout = new QVBoxLayout;
  ws_tabbar_wdgt = new ws_tabbar_t(nullptr);
  log_wdgt = new log_widget_t(nullptr);

  main_widget->setLayout(main_layout);
  main_layout->addWidget(ws_tabbar_wdgt);
  main_layout->addWidget(tool_panel_widget);
  main_layout->setContentsMargins(0,0,0,0);
  main_layout->setSpacing(0);

  ws_tabbar_wdgt->raise();

  splitter_ws_viewer_extended_editor = new QSplitter(Qt::Horizontal);
  splitter_ws_viewer_extended_editor->addWidget(ws_viewer_widget);
  splitter_ws_viewer_extended_editor->addWidget(extended_editor_compositor);
  splitter_ws_viewer_extended_editor->setHandleWidth(0);

  splitter_ws_viewer_extended_editor->setContentsMargins(0,0,0,0);
  splitter_ws_viewer_extended_editor->setCollapsible(1, false);
  splitter_ws_viewer_extended_editor->setCollapsible(0, false);
  extended_editor_compositor->hide();

  splitter_py_console_log_widget = new QSplitter(Qt::Horizontal);
  splitter_py_console_log_widget->addWidget(py_console_widget);
  splitter_py_console_log_widget->addWidget(log_wdgt);
  splitter_py_console_log_widget->setHandleWidth(1);
  splitter_py_console_log_widget->setCollapsible(0, false);
  splitter_py_console_log_widget->setCollapsible(1, false);

  splitter_ws_viewer_py_console_log = new QSplitter(Qt::Vertical);
  splitter_ws_viewer_py_console_log->addWidget(splitter_ws_viewer_extended_editor);
  splitter_ws_viewer_py_console_log->addWidget(splitter_py_console_log_widget);
  splitter_ws_viewer_py_console_log->setHandleWidth(0);
  splitter_ws_viewer_py_console_log->setSizes(QList<int>({1, 3}));
  splitter_ws_viewer_py_console_log->setContentsMargins(0,0,0,0);
  py_console_widget->hide();
  log_wdgt->hide();
  splitter_ws_viewer_py_console_log->setCollapsible(0, false);
  splitter_ws_viewer_py_console_log->setCollapsible(1, false);

  layout_ws_viewer_obj_insp = new QSplitter(Qt::Horizontal);
  layout_ws_viewer_obj_insp->addWidget(splitter_ws_viewer_py_console_log);
  layout_ws_viewer_obj_insp->addWidget(obj_insp_widget);
  layout_ws_viewer_obj_insp->setContentsMargins(0,0,0,0);
  layout_ws_viewer_obj_insp->setCollapsible(1, false);

  layout_ws_viewer_obj_insp->setHandleWidth(0);
  main_layout->addWidget(layout_ws_viewer_obj_insp);

  tool_panel_layout = new QHBoxLayout;
  tool_panel_widget->setLayout(tool_panel_layout);
  tool_panel_layout->setContentsMargins(5,0,0,0);

  tool_panel_layout->addWidget(tp_edit_mode_start, 0, Qt::AlignLeft);
  tool_panel_layout->addWidget(tp_edit_mode_item, 0, Qt::AlignLeft);
  tool_panel_layout->addWidget(tp_edit_mode_content, 0, Qt::AlignLeft);
  tool_panel_layout->addWidget(tp_edit_mode_end, 0, Qt::AlignLeft);

  tool_panel_layout->addWidget(tp_print_screen, 0, Qt::AlignLeft);
  tool_panel_layout->addWidget(tp_scenic_rot_cam, 0, Qt::AlignLeft);
  tool_panel_layout->addWidget(tp_camera_tool, 0, Qt::AlignLeft);
  tool_panel_layout->addWidget(tp_utility_frame_end, 0, Qt::AlignLeft);

  tool_panel_layout->addWidget(tp_anim_fast_forward, 0, Qt::AlignLeft);
  tool_panel_layout->addWidget(tp_measure_dist, 0, Qt::AlignLeft);
  tool_panel_layout->addWidget(tp_measure_angle, 0, Qt::AlignLeft);
  tool_panel_layout->addWidget(tp_force_sel_lbl_vis, 0, Qt::AlignLeft);
  tool_panel_layout->addWidget(tp_toggle_atom_override, 0, Qt::AlignLeft);

  tool_panel_layout->addWidget(tp_add_cube, 0, Qt::AlignLeft);
  tool_panel_layout->addWidget(tp_add_arrow, 0, Qt::AlignLeft);
  tool_panel_layout->addWidget(tp_add_point_sym_group, 0, Qt::AlignLeft);

  tool_panel_layout->addStretch(1);
  //tool_panel_widget->stackUnder(ws_viewer_widget);
  ws_viewer_widget->lower();

}

void main_window_t::dragEnterEvent(QDragEnterEvent *event) {
  event->acceptProposedAction();
}

void main_window_t::dragMoveEvent(QDragMoveEvent *event) {
  event->acceptProposedAction();
}

void main_window_t::dragLeaveEvent(QDragLeaveEvent *event) {
  event->accept();
}

void main_window_t::dropEvent(QDropEvent *event) {

  const QMimeData *mimeData = event->mimeData();

  if (mimeData && mimeData->hasUrls()) {

      app_state_t* astate = app_state_t::get_inst();
      QList<QUrl> urlList = mimeData->urls();

      for (int i = 0; i < urlList.size(); i++) {

          QString native_path = urlList.at(i).toLocalFile();
          std::string native_path_str = native_path.toStdString();
          //astate->get_inst()->log(fmt::format("DRAG EN DROP EVENT {} {}", i, native_path_str));
          astate->ws_mgr->load_from_file_autodeduce(native_path_str);

        }

    }

}

void main_window_t::resizeEvent(QResizeEvent *event) {

  //app_state_t::get_inst()->log(fmt::format("main_window::resizeEvent(width={}, height={})",
  //                                           event->size().width(),
  //                                         event->size().height()));

  QMainWindow::resizeEvent(event);

}

void main_window_t::wss_changed_slot() {

  app_state_t* astate = app_state_t::get_inst();

  control_bhv_menus_activity();


  if (astate->ws_mgr->has_wss()) {

      if (!astate->m_immersive_mode) {
          ws_tabbar_wdgt->setVisible(true);
          tool_panel_widget->setVisible(true);
        }

      file_menu_close_ws->setEnabled(true);
      ws_menu_rename_ws->setEnabled(true);
      view_menu_show_gizmo->setEnabled(true);
      view_menu_show_gizmo->setVisible(true);
      ws_copy_cam->setEnabled(true);
      ws_menu_bg_color->setEnabled(true);
      //obj_insp_widget->setVisible()

    } else {

      obj_insp_widget->setVisible(false);
      tool_panel_widget->setVisible(false);
      ws_tabbar_wdgt->setVisible(false);
      file_menu_close_ws->setEnabled(false);
      ws_menu_rename_ws->setEnabled(false);
      view_menu_show_gizmo->setEnabled(false);
      view_menu_show_gizmo->setVisible(false);
      ws_copy_cam->setEnabled(false);
      ws_menu_bg_color->setEnabled(false);

    }

  //astate->log(fmt::format("main_window::workspaces_changed_slot(), total ws = {}",
  //                          astate->ws_mgr->m_ws.size()));

}

void main_window_t::ws_selector_selection_changed(int index) {

  app_state_t* astate = app_state_t::get_inst();

  if (astate->ws_mgr->has_wss()) {

      auto current = astate->ws_mgr->get_cur_id();
      //astate->log(fmt::format("ws_selector_selection_changed index: {}, ws_cur_id: {}",
      //index, *current));
      if (current) {
          astate->ws_mgr->set_cur_id(opt<size_t>(index));
          astate->make_viewport_dirty();
        }

    }

}

void main_window_t::show_obj_insp_state_changed(bool checked) {

  app_state_t* astate = app_state_t::get_inst();

  auto [ok, cur_ws] = astate->ws_mgr->get_sel_tuple_ws(error_ctx_ignore);

  if (!ok) {
      obj_insp_widget->hide();
      return;
    }

  if (checked) {
      obj_insp_widget->show();
      cur_ws->m_show_obj_insp = true;
    } else {
      obj_insp_widget->hide();
      cur_ws->m_show_obj_insp = false;
    }

}

void main_window_t::show_gizmo_state_changed(bool checked) {

  app_state_t* astate = app_state_t::get_inst();

  auto [ok, cur_ws] = astate->ws_mgr->get_sel_tuple_ws(error_ctx_ignore);

  if (ok) {
      cur_ws->m_gizmo->m_is_visible = checked;
      astate->make_viewport_dirty();
    }

}

void main_window_t::show_log_wdgt_state_changed(bool checked) {

  log_wdgt->setVisible(checked);

}

void main_window_t::create_new_ws() {

  app_state_t* astate = app_state_t::get_inst();
  astate->ws_mgr->create_new_ws(true);
  wss_changed_slot();
  astate->make_viewport_dirty();

}

void main_window_t::open_ws() {

  app_state_t* astate = app_state_t::get_inst();
  QString file_name = QFileDialog::getOpenFileName(this,
                                                   "Open qpp::cad workspace",
                                                   astate->m_last_dir,
                                                   "*.json");

  if (file_name != "") {

      astate->ws_mgr->load_from_file(file_name.toStdString(), true);
      wss_changed_slot();

    }

}

void main_window_t::save_ws() {

  app_state_t* astate = app_state_t::get_inst();

  stop_update_cycle();

  auto [ok, cur_ws] = astate->ws_mgr->get_sel_tuple_ws(error_ctx_mbox);

  if (ok) {

      QFileInfo check_file(QString::fromStdString(cur_ws->m_fs_path));

      if (check_file.exists() && check_file.isFile() && cur_ws->m_fs_path != "" &&
          !cur_ws->m_is_ws_imported) {

          cur_ws->m_is_ws_imported = false;
          cur_ws->save_ws_to_json(cur_ws->m_fs_path);

        } else {

          QString file_name = QFileDialog::getSaveFileName(this,
                                                           "Save qpp::cad workspace",
                                                           astate->m_last_dir,
                                                           "*.json");

          if (file_name != "") {

              if (!file_name.endsWith(".json")) file_name += ".json";
              cur_ws->save_ws_to_json(file_name.toStdString());
              cur_ws->m_is_ws_imported = false;
              cur_ws->m_fs_path = file_name.toStdString();

            }

        }
    }

  start_update_cycle();

  cur_ws_changed();

}

void main_window_t::save_ws_as() {

  app_state_t* astate = app_state_t::get_inst();

  stop_update_cycle();

  auto [ok, cur_ws] = astate->ws_mgr->get_sel_tuple_ws(error_ctx_mbox);

  if (ok) {

      QString file_name = QFileDialog::getSaveFileName(this,
                                                       "Save qpp::cad workspace",
                                                       astate->m_last_dir,
                                                       "*.json");

      if (file_name != "") {

          cur_ws->save_ws_to_json(file_name.toStdString());
          cur_ws->m_is_ws_imported = false;
          cur_ws->m_fs_path = file_name.toStdString();

        }

    }

  cur_ws_changed();

  start_update_cycle();

}

void main_window_t::close_cur_ws() {

  app_state_t* astate = app_state_t::get_inst();
  auto [ok, cur_ws] = astate->ws_mgr->get_sel_tuple_ws(error_ctx_mbox);

  if (ok) {

      QMessageBox::StandardButton reply;
      reply = QMessageBox::question(this, tr("Workspace -> Close"),
                                    tr("Do you really want to close the workspace?"),
                                    QMessageBox::Yes | QMessageBox::No);

      if (reply == QMessageBox::Yes) {
          cur_ws->m_marked_for_deletion = true;
        }

    }

}

void main_window_t::close_all_ws() {

  app_state_t* astate = app_state_t::get_inst();

  QMessageBox::StandardButton reply;
  reply = QMessageBox::question(this, tr("Workspaces -> Close"),
                                tr("Do you really want to close all workspaces?"),
                                QMessageBox::Yes | QMessageBox::No);

  if (reply == QMessageBox::Yes)
    for (auto ws : astate->ws_mgr->m_ws) ws->m_marked_for_deletion = true;

}

void main_window_t::rename_cur_ws() {

  app_state_t* astate = app_state_t::get_inst();
  auto [ok, cur_ws] = astate->ws_mgr->get_sel_tuple_ws(error_ctx_mbox);

  if (ok) {

      bool ok_q;
      QString text = QInputDialog::getText(this, tr("Workspace -> Rename"),
                                           tr("User name:"), QLineEdit::Normal,
                                           QString::fromStdString(cur_ws->m_ws_name), &ok_q);
      if (ok_q && text != "") {
          cur_ws->m_ws_name = text.toStdString();
          astate->astate_evd->wss_changed();
        }

    }

}

void main_window_t::change_cur_ws_bg() {

  app_state_t* astate = app_state_t::get_inst();

  auto [ok, cur_ws] = astate->ws_mgr->get_sel_tuple_ws(error_ctx_mbox);

  if (ok) {

      QColor _stored_color = QColor::fromRgbF(
                               cur_ws->m_background_color[0],
                               cur_ws->m_background_color[1],
                               cur_ws->m_background_color[2]
                             );

      const QColor clr = QColorDialog::getColor(_stored_color, this,
                                                "Select workspace`s background color");
      if (clr.isValid()) {

          cur_ws->m_background_color = vector3<float> {
              static_cast<float>(clr.redF()),
              static_cast<float>(clr.greenF()),
              static_cast<float>(clr.blueF())
          };

          astate->make_viewport_dirty();

        }

    }

}

void main_window_t::cam_copy_to_all() {

  app_state_t* astate = app_state_t::get_inst();
  auto [ok, cur_ws] = astate->ws_mgr->get_sel_tuple_ws(error_ctx_mbox);
  if (!ok && !cur_ws) return;

  for (size_t i = 0; i < astate->ws_mgr->m_ws.size(); i++)
    if (cur_ws != astate->ws_mgr->m_ws[i])
      astate->ws_mgr->m_ws[i]->copy_camera(cur_ws);

}

void main_window_t::cur_ws_changed() {

  app_state_t* astate = app_state_t::get_inst();

  auto [ok, cur_ws] = astate->ws_mgr->get_sel_tuple_ws(error_ctx_ignore);

  if (ok) {

      tp_scenic_rot_cam->blockSignals(true);
      tp_scenic_rot_cam->setChecked(cur_ws->m_scenic_rotation);
      tp_scenic_rot_cam->blockSignals(false);

      view_menu_show_oi->setChecked(cur_ws->m_show_obj_insp);
      view_menu_show_oi->setVisible(true);
      view_menu_show_gizmo->setChecked(cur_ws->m_gizmo->m_is_visible);

      std::string title_text = fmt::format("qpp::cad [ws_name: {}] - [path: {}]",
                                           cur_ws->m_ws_name, cur_ws->m_fs_path);
      this->setWindowTitle(QString::fromStdString(title_text));

  } else {

    view_menu_show_oi->setChecked(false);
    view_menu_show_oi->setVisible(false);
    this->setWindowTitle("qpp::cad");

  }

  wss_changed_slot();
  cur_ws_properties_changed();
  cur_ws_selected_item_changed();

}

void main_window_t::cur_ws_selected_item_changed() {

  app_state_t* astate = app_state_t::get_inst();

  bool show_cam_button{false};
  bool show_fast_forward_button{false};

  auto [cur_ws, cur_itm, as_al] = astate->ws_mgr->get_sel_tpl_itm<geom_view_t>();

  if (cur_itm) {

    if (cur_itm->get_flags() & ws_item_flags_cam_target_view) show_cam_button = true;

    if (as_al) {

        bool al_is_3d = as_al->m_geom->DIM == 3;
        //tp_camera_tool_act_a->setVisible(al_is_3d)
        tp_camera_tool_act_a->setVisible(al_is_3d);
        tp_camera_tool_act_b->setVisible(al_is_3d);
        tp_camera_tool_act_c->setVisible(al_is_3d);
        tp_camera_tool_act_x->setVisible(!al_is_3d);
        tp_camera_tool_act_y->setVisible(!al_is_3d);
        tp_camera_tool_act_z->setVisible(!al_is_3d);
        tp_camera_tool_act_cc->setVisible(!al_is_3d);

        show_fast_forward_button = as_al->m_anim->animable();

      }

  }

  tp_camera_tool->setVisible(show_cam_button);
  tp_anim_fast_forward->setVisible(show_fast_forward_button);

  cur_ws_edit_type_changed();
  control_bhv_menus_activity();
  control_bhv_tools_menus_activity();

}

void main_window_t::cur_ws_properties_changed() {

  app_state_t* astate = app_state_t::get_inst();

  auto [ok, cur_ws] = astate->ws_mgr->get_sel_tuple_ws(error_ctx_ignore);

  if (ok) {

    bool check_t = cur_ws->m_edit_type == ws_edit_e::edit_item;
    tp_edit_mode_item->blockSignals(true);
    tp_edit_mode_content->blockSignals(true);
    tp_edit_mode_item->setChecked(check_t);
    tp_edit_mode_content->setChecked(!check_t);
    tp_edit_mode_item->blockSignals(false);
    tp_edit_mode_content->blockSignals(false);

  }

  astate->make_viewport_dirty();

}

void main_window_t::cur_ws_edit_type_changed() {

  cur_ws_properties_changed();
  cur_ws_selected_atoms_list_selection_changed();

}

void main_window_t::cur_ws_selected_atoms_list_selection_changed() {

  app_state_t* astate = app_state_t::get_inst();
  auto [cur_ws, cur_item, as_al, ok] = astate->ws_mgr->get_sel_tpl_itmc<geom_view_t>();

  bool need_to_hide_al_cntls{true};
  bool need_to_hide_force_sel_lbl_vis{true};
  bool need_to_hide_atom_override{true};
  bool need_to_hide_add_primitives{true};
  bool need_to_hide_make_psg{true};

  if (ok) {

    /* detect selective labels */
    need_to_hide_force_sel_lbl_vis =
        cur_ws->m_edit_type == ws_edit_e::edit_item || as_al->m_atom_idx_sel.empty();

    if (!need_to_hide_force_sel_lbl_vis) {

        tp_force_sel_lbl_vis->show();

        tp_force_sel_lbl_vis->blockSignals(true);
        tp_force_sel_lbl_vis->setChecked(
              !as_al->any_of_sel_xfield_equal<bool>(xgeom_label_show, false)
              );
        tp_force_sel_lbl_vis->blockSignals(false);
        /* end of detect selective labels */

      }

    need_to_hide_make_psg = need_to_hide_force_sel_lbl_vis;
    tp_add_point_sym_group->show();

    /* detect atom override */
    if (!(as_al->m_atom_idx_sel.empty() || cur_ws->m_edit_type == ws_edit_e::edit_item)) {

        need_to_hide_atom_override = false;

        tp_toggle_atom_override->show();
        tp_toggle_atom_override->blockSignals(true);

        tp_toggle_atom_override->setChecked(
                !as_al->any_of_sel_xfield_equal<bool>(xgeom_override, false)
              );

        tp_toggle_atom_override->blockSignals(false);

      }
    /* end of detect atom override */

    /* add cube or arrow between 2 atoms */
    if (as_al->m_atom_idx_sel.size() == 2 && cur_ws->m_edit_type == ws_edit_e::edit_content) {

      tp_add_arrow->show();
      tp_add_cube->show();
      need_to_hide_add_primitives = false;

      tp_measure_angle->hide();
      tp_measure_dist->show();
      need_to_hide_al_cntls = false;
      auto it1 = as_al->m_atom_idx_sel.begin();
      auto it2 = ++(as_al->m_atom_idx_sel.begin());

      auto cur_sel =
          as_al->m_measure->is_bond_msr_exists(it1->m_atm, it2->m_atm, it1->m_idx, it2->m_idx);

      tp_measure_dist->blockSignals(true);
      tp_measure_dist->setChecked(cur_sel != std::nullopt);
      tp_measure_dist->blockSignals(false);

    }
    /* end of add cube or arrow between 2 atoms */

    /* angle between 3 atoms */
    if (as_al->m_atom_idx_sel.size() == 3 &&
        cur_ws->m_edit_type == ws_edit_e::edit_content &&
        as_al->m_atom_ord_sel.size() == 3) {

      need_to_hide_al_cntls = false;

      auto cur_sel = as_al->m_measure->is_angle_msr_exists(as_al->m_atom_ord_sel[0].m_atm,
                                                           as_al->m_atom_ord_sel[1].m_atm,
                                                           as_al->m_atom_ord_sel[2].m_atm,
                                                           as_al->m_atom_ord_sel[0].m_idx,
                                                           as_al->m_atom_ord_sel[1].m_idx,
                                                           as_al->m_atom_ord_sel[2].m_idx);

      tp_measure_dist->hide();
      tp_measure_angle->show();
      tp_measure_angle->blockSignals(true);
      tp_measure_angle->setChecked(cur_sel != std::nullopt);
      tp_measure_angle->blockSignals(false);

    }
    /* end of angle between 3 atoms */

  }

  if (need_to_hide_add_primitives) {

      tp_add_arrow->hide();
      tp_add_cube->hide();

    }

  if (need_to_hide_al_cntls) {

    tp_measure_dist->hide();
    tp_measure_angle->hide();
    tp_add_arrow->hide();
    tp_add_cube->hide();

  }

  if (need_to_hide_force_sel_lbl_vis) {
    tp_force_sel_lbl_vis->hide();
  }

  if (need_to_hide_atom_override) {
    tp_toggle_atom_override->hide();
  }

  if (need_to_hide_make_psg) {
      tp_add_point_sym_group->hide();
    }

}

void main_window_t::tp_dist_button_clicked(bool checked) {

  app_state_t* astate = app_state_t::get_inst();
  auto [cur_ws, cur_item, as_al, ok] = astate->ws_mgr->get_sel_tpl_itmc<geom_view_t>();
  if (!ok) return;

  if (as_al->m_atom_idx_sel.size() == 2 && cur_ws->m_edit_type == ws_edit_e::edit_content) {

    tp_measure_dist->show();

    auto it1 = as_al->m_atom_idx_sel.begin();
    auto it2 = ++(as_al->m_atom_idx_sel.begin());

    auto cur_sel = as_al->m_measure->is_bond_msr_exists(it1->m_atm,
                                                        it2->m_atm,
                                                        it1->m_idx,
                                                        it2->m_idx);

    if (checked) as_al->m_measure->add_bond_msr(it1->m_atm,
                                                it2->m_atm,
                                                it1->m_idx,
                                                it2->m_idx);

    else as_al->m_measure->rm_bond_msr(*cur_sel);

  }


  astate->make_viewport_dirty();

}

void main_window_t::tp_angle_button_clicked(bool checked) {

  app_state_t* astate = app_state_t::get_inst();
  auto [cur_ws, cur_item, as_al, ok] = astate->ws_mgr->get_sel_tpl_itmc<geom_view_t>();
  if (!ok) return;

  if (as_al->m_atom_idx_sel.size() == 3 && cur_ws->m_edit_type == ws_edit_e::edit_content) {

    tp_measure_angle->show();

    auto cur_sel = as_al->m_measure->is_angle_msr_exists(as_al->m_atom_ord_sel[0].m_atm,
                                                         as_al->m_atom_ord_sel[1].m_atm,
                                                         as_al->m_atom_ord_sel[2].m_atm,
                                                         as_al->m_atom_ord_sel[0].m_idx,
                                                         as_al->m_atom_ord_sel[1].m_idx,
                                                         as_al->m_atom_ord_sel[2].m_idx);

    if (checked) as_al->m_measure->add_angle_msr(as_al->m_atom_ord_sel[0].m_atm,
                                                 as_al->m_atom_ord_sel[1].m_atm,
                                                 as_al->m_atom_ord_sel[2].m_atm,
                                                 as_al->m_atom_ord_sel[0].m_idx,
                                                 as_al->m_atom_ord_sel[1].m_idx,
                                                 as_al->m_atom_ord_sel[2].m_idx);

    else as_al->m_measure->rm_angle_msr(*cur_sel);

  }

  astate->make_viewport_dirty();

}

void main_window_t::ws_edit_mode_selector_button_clicked(int id) {

  app_state_t* astate = app_state_t::get_inst();
  auto [ok, cur_ws] = astate->ws_mgr->get_sel_tuple_ws(error_ctx_mbox);

  if (ok) {
      if (id == 0) cur_ws->m_edit_type = ws_edit_e::edit_item;
      else cur_ws->m_edit_type = ws_edit_e::edit_content;
    }

  astate->astate_evd->cur_ws_edit_type_changed();
  cur_ws_properties_changed();

}

void main_window_t::tp_force_sel_lbl_vis_button_clicked(bool checked) {

  app_state_t* astate = app_state_t::get_inst();
  auto [cur_ws, cur_item, as_al, ok] = astate->ws_mgr->get_sel_tpl_itmc<geom_view_t>();
  if (!ok) return;

  if (as_al && cur_ws->m_edit_type == ws_edit_e::edit_content)
    for (auto &rec : as_al->m_atom_idx_sel)
      as_al->m_geom->xfield<bool>(xgeom_label_show, rec.m_atm) = checked;

  // if selective labels rendering unchecked - force it and select some random style
  if (!as_al->m_atom_idx_sel.empty() && !as_al->m_labels->m_selective_lbl) {

        as_al->m_labels->m_selective_lbl = true;
        as_al->m_labels->m_style = geom_labels_style_e::show_id_type;
        astate->astate_evd->cur_ws_selected_item_need_to_update_obj_insp();

    }

  astate->make_viewport_dirty();

}

void main_window_t::tp_toggle_atom_override_button_clicked(bool checked) {

  app_state_t* astate = app_state_t::get_inst();

  auto [cur_ws, cur_item, as_al, ok] = astate->ws_mgr->get_sel_tpl_itmc<geom_view_t>();
  if (!ok || cur_ws->m_edit_type == ws_edit_e::edit_item) return;

  for (auto &rec : as_al->m_atom_idx_sel)
    if (rec.m_atm < as_al->m_geom->nat()) {

      as_al->m_geom->xfield<bool>(xgeom_override, rec.m_atm) = checked;

      if (as_al->m_geom->xfield<float>(xgeom_atom_r, rec.m_atm) < 0.01f) {

          auto ap_idx = ptable::number_by_symbol(as_al->m_geom->atom(rec.m_atm));
          float _rad = 1.0f;
          if (ap_idx) _rad = ptable::get_inst()->arecs[*ap_idx - 1].m_radius;
          as_al->m_geom->xfield<float>(xgeom_atom_r, rec.m_atm) = _rad;

        }

    }

  astate->astate_evd->cur_ws_selected_item_need_to_update_obj_insp();
  astate->make_viewport_dirty();

}

void main_window_t::tp_camera_tool_button_triggered(QAction *action) {

  if (!action) return;

  qextended_action *ext_act = qobject_cast<qextended_action*>(action);
  if (!ext_act) return;

  app_state_t* astate = app_state_t::get_inst();

  auto [cur_ws, cur_item, as_al, ok] = astate->ws_mgr->get_sel_tpl_itmc<geom_view_t>();
  if (!ok) return;

  cam_tv_e _tv = static_cast<cam_tv_e>(ext_act->m_joined_data[0]);

  if (as_al) as_al->apply_target_view(_tv);

  astate->make_viewport_dirty();

}

void main_window_t::tp_fast_forward_anim_clicked() {

  app_state_t* astate = app_state_t::get_inst();
  auto [cur_ws, cur_itm, as_al] = astate->ws_mgr->get_sel_tpl_itm<geom_view_t>();

  if (!as_al || !as_al->m_anim->animable()) return;

  as_al->m_anim->m_cur_anim = as_al->m_anim->get_total_anims()-1;
  as_al->m_anim->update_current_frame_to_end();

  astate->make_viewport_dirty();
  astate->astate_evd->cur_ws_selected_item_need_to_update_obj_insp();

}

void main_window_t::tp_add_arrow_clicked() {

 simple_query::embed_arrow();

}

void main_window_t::tp_add_cube_clicked() {

 simple_query::embed_cube();

}

void main_window_t::tp_add_point_sym_group_clicked() {

  simple_query::make_psg_view(0.1);

}

void main_window_t::tp_scenic_rotation_toggle() {

  app_state_t* astate = app_state_t::get_inst();
  auto [ok, cur_ws] = astate->ws_mgr->get_sel_tuple_ws();
  cur_ws->m_scenic_rotation = !cur_ws->m_scenic_rotation;

}

void main_window_t::apply_camera_view_change(cam_tv_e target_view) {

}

void main_window_t::toggle_ws_edit_mode() {

  app_state_t* astate = app_state_t::get_inst();

  auto [ok, cur_ws] = astate->ws_mgr->get_sel_tuple_ws();

  if (ok) {
      cur_ws->toggle_edit_mode();
      cur_ws_changed();
    }

}

void main_window_t::toggle_fullscreen(bool checked) {

  if (checked) {

      setWindowState(windowState() | Qt::WindowFullScreen);

    } else {

      setWindowFlags(Qt::CustomizeWindowHint |
                     Qt::WindowStaysOnTopHint |
                     Qt::WindowMinimizeButtonHint |
                     Qt::WindowMaximizeButtonHint |
                     Qt::WindowCloseButtonHint);
      show();

    }

}

void main_window_t::toggle_immersive_mode() {

  app_state_t* astate = app_state_t::get_inst();

//  if (!astate->ws_mgr->has_wss()) return;

  astate->m_immersive_mode = !astate->m_immersive_mode;

  if (astate->m_immersive_mode) {

      menuBar()->hide();
      tool_panel_widget->hide();
      ws_tabbar_wdgt->hide();
      obj_insp_widget->hide();
      view_menu_toggle_fullscreen->setChecked(true);

    } else {

      tool_panel_widget->show();
      ws_tabbar_wdgt->show();
      obj_insp_widget->show();
      menuBar()->show();
      view_menu_toggle_fullscreen->setChecked(false);

    }

}

void main_window_t::start_update_cycle() {

  if (ws_viewer_widget && ws_viewer_widget->m_update_timer) {
      ws_viewer_widget->m_update_timer->start();
    }

}

void main_window_t::stop_update_cycle() {

  if (ws_viewer_widget && ws_viewer_widget->m_update_timer) {

      p_elapsed_time_in_event_loop =  ws_viewer_widget->m_update_timer->remainingTime();
      ws_viewer_widget->m_update_timer->stop();
      ws_viewer_widget->m_update_timer->setInterval(p_elapsed_time_in_event_loop);

    }

}

void main_window_t::action_select_all_content() {

  app_state_t* astate = app_state_t::get_inst();
  auto [cur_ws, cur_it, al] = astate->ws_mgr->get_sel_tpl_itm<geom_view_t>();
  if (al) al->sel_atoms(true);

}

void main_window_t::action_unselect_all_content() {

  app_state_t* astate = app_state_t::get_inst();
  auto [cur_ws, cur_it, al] = astate->ws_mgr->get_sel_tpl_itm<geom_view_t>();
  if (al) al->sel_atoms(false);

}

void main_window_t::action_invert_selected_content() {

  app_state_t* astate = app_state_t::get_inst();
  auto [cur_ws, cur_it, al] = astate->ws_mgr->get_sel_tpl_itm<geom_view_t>();
  if (al) al->inv_sel_atoms();

}

void main_window_t::action_toggle_console() {

  app_state_t* astate = app_state_t::get_inst();

  if (py_console_widget->isVisible()) {
      py_console_widget->hide();
      astate->m_show_console = true;
    } else {
      py_console_widget->show();
      py_console_widget->py_tedit->setFocus();
      astate->m_show_console = false;
    }

}

void main_window_t::rebuild_recent_files_menu() {

  if (!file_menu_recent_files) return;

  for (auto &elem : file_menu_recent_entries)
    elem->setVisible(false);

  app_state_t* astate = app_state_t::get_inst();

  for (int i = 0; i < astate->m_recent_files.size(); i++) {

      file_menu_recent_entries[i]->setVisible(true);
      std::string ff_name = astate->ws_mgr->m_bhv_mgr->get_ff_full_name(
                              astate->m_recent_files[i].m_ff_id);

      std::string rec_menu_entry;
      if (!astate->m_recent_files[i].m_native)
        rec_menu_entry = fmt::format("{}) {} ({})",
                                     i,
                                     astate->m_recent_files[i].m_file_name,
                                     ff_name);
      else
        rec_menu_entry = fmt::format("{}) {} (qpp::cad workspace)",
                                     i,
                                     astate->m_recent_files[i].m_file_name);

      file_menu_recent_entries[i]->setText(QString::fromStdString(rec_menu_entry));

    }

}

void main_window_t::recent_files_clicked() {

  int idx = -1;
  app_state_t* astate = app_state_t::get_inst();

  QObject* obj = sender();
  for (int i = 0 ; i < file_menu_recent_entries.size(); i++)
    if (file_menu_recent_entries[i]->isVisible() && file_menu_recent_entries[i] == obj) {
        idx = i;
      }

  if (idx != -1 && idx < astate->m_recent_files.size()) {

      auto &rec_idx = astate->m_recent_files[idx];

      if (rec_idx.m_native)
        astate->ws_mgr->load_from_file(rec_idx.m_file_name, false);
      else {
          auto bhv_id = astate->ws_mgr->m_bhv_mgr->get_io_bhv_by_file_format(rec_idx.m_ff_id);
          if (bhv_id) astate->ws_mgr->import_from_file(rec_idx.m_file_name, *bhv_id, true);
          else astate->ws_mgr->load_from_file_autodeduce(rec_idx.m_file_name);
        }

    }

}

void main_window_t::build_bhv_menus_and_actions() {

  app_state_t* astate = app_state_t::get_inst();
  ws_item_behaviour_manager_t *bhv_mgr = astate->ws_mgr->m_bhv_mgr.get();

  if (!bhv_mgr) return;

  //init groups for IMPORTED TO WS
  for (auto &ff_grp : bhv_mgr->m_file_format_groups) {

      //spawn top-level menus
      QMenu *new_menu_as_new_ws =
          file_menu_import_as_new_ws->addMenu(QString::fromStdString(ff_grp.second.m_full_name));

      QMenu *new_menu_to_cur_ws =
          file_menu_import_to_cur_ws->addMenu(QString::fromStdString(ff_grp.second.m_full_name));

      QMenu *new_menu_export_selected =
          file_menu_export_sel_as->addMenu(QString::fromStdString(ff_grp.second.m_full_name));

      file_menu_import_as_new_ws_menus.emplace(ff_grp.first, new_menu_as_new_ws);
      file_menu_import_to_cur_ws_menus.emplace(ff_grp.first, new_menu_to_cur_ws);
      file_menu_export_sel_as_menus.emplace(ff_grp.first, new_menu_export_selected);

      //iterate over file formats from group
      for (auto &ff : ff_grp.second.m_ffs_lookup)

        for (size_t i = 0; i < bhv_mgr->m_ws_item_io.size(); i++) {

            //deduce import to current ws
            if (bhv_mgr->m_ws_item_io[i]->m_accepted_file_format == ff &&
                bhv_mgr->m_ws_item_io[i]->can_load() &&
                bhv_mgr->m_ws_item_io[i]->m_menu_occupier &&
                bhv_mgr->m_ws_item_io[i]->m_can_be_imported_to_ws) {

                qextended_action *new_act = new qextended_action((nullptr));
                new_act->m_joined_data[0] = i;
                connect(new_act, &QAction::triggered,
                        this, &main_window_t::action_bhv_import_to_cur_workspace);
                new_act->setText(
                      QString::fromStdString(bhv_mgr->m_file_formats[ff].m_full_name));
                new_menu_to_cur_ws->addAction(new_act);

              }

            //deduce import as new ws
            if (bhv_mgr->m_ws_item_io[i]->m_accepted_file_format == ff &&
                bhv_mgr->m_ws_item_io[i]->can_load() &&
                bhv_mgr->m_ws_item_io[i]->m_menu_occupier &&
                bhv_mgr->m_ws_item_io[i]->m_can_be_imported_as_new_ws) {

                qextended_action *new_act = new qextended_action((nullptr));
                new_act->m_joined_data[0] = i;
                connect(new_act, &QAction::triggered,
                        this, &main_window_t::action_bhv_import_as_new_workspace);
                new_act->setText(
                      QString::fromStdString(bhv_mgr->m_file_formats[ff].m_full_name));
                new_menu_as_new_ws->addAction(new_act);

              }

            //deduce save selected item
            if (bhv_mgr->m_ws_item_io[i]->m_accepted_file_format == ff &&
                bhv_mgr->m_ws_item_io[i]->can_save() &&
                bhv_mgr->m_ws_item_io[i]->m_menu_occupier) {

                qextended_action *new_act = new qextended_action((nullptr));
                new_act->m_joined_data[0] = i;
                connect(new_act, &QAction::triggered,
                        this, &main_window_t::action_bhv_export_selected);
                new_act->setText(
                      QString::fromStdString(bhv_mgr->m_file_formats[ff].m_full_name));
                new_menu_export_selected->addAction(new_act);
                file_menu_export_sel_as_acts.push_back(new_act);

              }

          }

    }

  //TODO: make lookup for bhv

}

void main_window_t::build_bhv_tools_menus() {

  app_state_t* astate = app_state_t::get_inst();

  //construct tools groups
  for (auto &ffg : astate->ws_mgr->m_bhv_mgr->m_tools_groups) {
      QMenu *new_menu = tools_menu->addMenu(QString::fromStdString(ffg.second.m_full_name));
      tools_menu_groups.emplace(ffg.first, new_menu);
    }

  //construct tools actions
  for (auto &ff : astate->ws_mgr->m_bhv_mgr->m_tools_info) {

      //construct tool's action
      qextended_action *new_act = new qextended_action((nullptr));
      new_act->m_joined_data[0] = ff.first;
      connect(new_act, &QAction::triggered,
              this, &main_window_t::action_bhv_tools_menus_clicked);
      new_act->setText(QString::fromStdString(ff.second.m_full_name));
      //locate menu for group
      auto it_g = tools_menu_groups.find(ff.second.m_group_hash);
      if (it_g != tools_menu_groups.end()) it_g->second->addAction(new_act);
      else tools_menu->addAction(new_act);

      tools_menu_actions.push_back(new_act);

    }

}

void main_window_t::build_bhv_toolpanel() {

  app_state_t *astate = app_state_t::get_inst();
  ws_item_behaviour_manager_t *bhv_mgr = astate->ws_mgr->m_bhv_mgr.get();

  m_toolbar_elements.reserve(bhv_mgr->m_toolbar_elements_info.size());

  for (auto &tb_info : bhv_mgr->m_toolbar_elements_info) {

      auto new_tb = tb_info.second.m_fabric();
      new_tb->init_element(tool_panel_widget);
      m_toolbar_elements.push_back(new_tb);

    }

}

void main_window_t::action_bhv_tools_menus_clicked() {

  app_state_t* astate = app_state_t::get_inst();
  ws_item_behaviour_manager_t *bhv_mgr = astate->ws_mgr->m_bhv_mgr.get();

  qextended_action *ext_act = qobject_cast<qextended_action*>(sender());
  if (!ext_act) return;

  size_t t_hash = ext_act->m_joined_data[0];

  auto it_t = bhv_mgr->m_tools_info.find(t_hash);

  std::shared_ptr<workspace_t> cur_ws = astate->ws_mgr->get_cur_ws();
  std::shared_ptr<ws_item_t> cur_it{nullptr};
  if (cur_ws) cur_it = cur_ws->get_selected_sp();

  if (it_t != bhv_mgr->m_tools_info.end())
    bhv_mgr->exec_tool(cur_it.get(), t_hash);

}

void main_window_t::control_bhv_tools_menus_activity() {

  app_state_t* astate = app_state_t::get_inst();
  ws_item_behaviour_manager_t *bhv_mgr = astate->ws_mgr->m_bhv_mgr.get();

  std::shared_ptr<workspace_t> cur_ws = astate->ws_mgr->get_cur_ws();
  std::shared_ptr<ws_item_t> cur_it{nullptr};
  if (cur_ws) cur_it = cur_ws->get_selected_sp();

  for (auto elem : tools_menu_actions) {

      elem->setEnabled(false);

      if (cur_ws) {

          auto it_t = bhv_mgr->m_tools_info.find(elem->m_joined_data[0]);
          if (it_t != bhv_mgr->m_tools_info.end()) {

              if (!it_t->second.m_item_required) elem->setEnabled(true);
              if (cur_it && it_t->second.m_item_required &&
                  it_t->second.m_accepted_type == cur_it->get_type()) elem->setEnabled(true);
            }

        }

    }

}

void main_window_t::action_bhv_import_to_cur_workspace() {

  app_state_t* astate = app_state_t::get_inst();
  ws_item_behaviour_manager_t *bhv_mgr = astate->ws_mgr->m_bhv_mgr.get();

  qextended_action *ext_act = qobject_cast<qextended_action*>(sender());
  if (!ext_act) return;

  size_t b_id = ext_act->m_joined_data[0];

  //check that bhv is valid
  if (b_id < bhv_mgr->m_ws_item_io.size() &&
      bhv_mgr->m_ws_item_io[b_id]->can_load() &&
      bhv_mgr->m_ws_item_io[b_id]->m_can_be_imported_to_ws) {

      std::string file_name =
          QFileDialog::getOpenFileName(this,
                                       "Import to current workspace",
                                       astate->m_last_dir,
                                       "*").toStdString();
      if (!file_name.empty())
        astate->ws_mgr->import_from_file(file_name, b_id, false);

    }

}

void main_window_t::action_bhv_import_as_new_workspace() {

  app_state_t* astate = app_state_t::get_inst();
  ws_item_behaviour_manager_t *bhv_mgr = astate->ws_mgr->m_bhv_mgr.get();

  qextended_action *ext_act = qobject_cast<qextended_action*>(sender());
  if (!ext_act) return;

  size_t b_id = ext_act->m_joined_data[0];

  //check that bhv is valid
  if (b_id < bhv_mgr->m_ws_item_io.size() &&
      bhv_mgr->m_ws_item_io[b_id]->can_load() &&
      bhv_mgr->m_ws_item_io[b_id]->m_can_be_imported_as_new_ws) {

      // astate->log(fmt::format("{}", b_id));
      std::string file_name = QFileDialog::getOpenFileName(this,
                                                           "Import as new workspace",
                                                           astate->m_last_dir,
                                                           "*").toStdString();
      if (!file_name.empty())
        astate->ws_mgr->import_from_file(file_name, b_id);

    }

}

void main_window_t::action_bhv_export_selected() {

  app_state_t* astate = app_state_t::get_inst();
  ws_item_behaviour_manager_t *bhv_mgr = astate->ws_mgr->m_bhv_mgr.get();

  qextended_action *ext_act = qobject_cast<qextended_action*>(sender());
  if (!ext_act) return;

  auto [cur_ws, cur_it, ok] = astate->ws_mgr->get_sel_tpl_itm_nc();

  if (!ok) return;

  size_t b_id = ext_act->m_joined_data[0];

  //check that bhv is valid
  if (b_id < bhv_mgr->m_ws_item_io.size() &&
      bhv_mgr->m_ws_item_io[b_id]->can_save() &&
      bhv_mgr->m_ws_item_io[b_id]->m_accepted_type == cur_it->get_type()) {

      QString qfile_name = QFileDialog::getSaveFileName(this,
                                                        "Export selected",
                                                        astate->m_last_dir
                                                        );

      if (qfile_name.size() != 0) {

          QFileInfo file_nfo(qfile_name);
          astate->m_last_dir = file_nfo.absoluteDir().canonicalPath();
          std::string file_name = qfile_name.toStdString();
          astate->ws_mgr->save_ws_item_to_file(file_name, cur_it, b_id);

        }

    }

}

void main_window_t::control_bhv_menus_activity() {

  app_state_t* astate = app_state_t::get_inst();

  if (!astate->ws_mgr) return;

  ws_item_behaviour_manager_t *bhv_mgr = astate->ws_mgr->m_bhv_mgr.get();

  auto [cur_ws, cur_it, ok] = astate->ws_mgr->get_sel_tpl_itm_nc();

  if (cur_ws) file_menu_import_to_cur_ws->setEnabled(true);
  else file_menu_import_to_cur_ws->setEnabled(false);

  if (!ok) {
    //
    file_menu_export_sel_as->setEnabled(false);
  } else {

    if (cur_it) {

        file_menu_export_sel_as->setEnabled(true);

        for (auto &exp_act : file_menu_export_sel_as_acts) {

            size_t bhv_id = exp_act->m_joined_data[0];

            if (bhv_mgr->m_ws_item_io[bhv_id]->can_save() &&
                bhv_mgr->m_ws_item_io[bhv_id]->m_accepted_type == cur_it->get_type()) {
                //need_to_enable_export_menu = true;
                exp_act->setEnabled(true);
                //save item dialog
              } else {
                exp_act->setEnabled(false);
              }

          }

      }

  } // not ok

}

void main_window_t::make_screenshot() {

  app_state_t* astate = app_state_t::get_inst();
  auto [ok, cur_ws] = astate->ws_mgr->get_sel_tuple_ws(error_ctx_mbox);

  if (!ok) return;

  astate->m_ignore_scanline = true;

  QDateTime date = QDateTime::currentDateTime();
  QString date_s = date.toString("dd_MM_yyyy_hh_mm_ss");
  ws_viewer_widget->grabFramebuffer().save(tr("%1_screenshot.png").arg(date_s));

  astate->m_ignore_scanline = false;
  astate->make_viewport_dirty();

}

void main_window_t::slot_shortcut_terminate_app() {

  QApplication::quit();
  
}


void main_window_t::action_create_arrow_array() {

  app_state_t* astate = app_state_t::get_inst();

  if (!astate->ws_mgr->has_wss()) return;
  
  create_arrow_array_widget_t  add_dialog;
  add_dialog.exec(); 
  
}

void main_window_t::action_plugin_manager(){

}

