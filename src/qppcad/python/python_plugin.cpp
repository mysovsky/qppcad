#include <qppcad/python/python_plugin.hpp>
#include <qppcad/core/app_state.hpp>
#include <filesystem>

namespace fs = std::filesystem;

using namespace qpp;
using namespace qpp::cad;

python_plugin_t::python_plugin_t(const std::string & _folder,
				 const std::string & _parent_module,
				 const std::string & _name){
  plug_name = _name;
  folder = fs::path(_folder);
  parent_module = _parent_module;
  module_name = _parent_module + "." + plug_name;
}

void python_plugin_t::load_header(){
  
}
      
void python_plugin_t::load_module()
{
  //py::module plug_module = py::module::import(module_name.c_str());
  std::string cmd = "import " + module_name;
  app_state_t *astate = app_state_t::get_inst();
  astate -> py_mgr -> execute(cmd);
}     

void python_plugin_t::run(){

  // stub
  func_param = "ws.get_sel_itm().geom";
  
  std::string cmmd  = module_name + "." + func_call + "(" + func_param + ")";
  app_state_t *astate = app_state_t::get_inst();
  astate -> py_mgr -> execute(cmmd);
  
  //py::exec(cmmd, py::globals());
  //app_state_t *astate = app_state_t::get_inst();
  // astate -> log("plu plu");
}

// ----------------------------------------------------------

plugin_tree_t::plugin_tree_t(){}

void plugin_tree_t::list_all(const std::string & prefix, const std::string & separ,
			     std::vector<std::string> & list,
			     std::vector< std::shared_ptr<python_plugin_t> > & plugins){
  for (auto pair : nested){
    std::string s = pair.first;
    auto ptr = pair.second;
    if (!ptr){
      list.push_back(prefix+separ+s);
      plugins.push_back(plugin);
    }
    else
      ptr -> list_all(prefix+separ+s, separ, list, plugins);
  }
}

// ----------------------------------------------------------

plugin_manager_t::plugin_manager_t(){
}

std::shared_ptr<plugin_tree_t> plugins_in_folder(const fs::path & p){

  app_state_t *astate = app_state_t::get_inst();
  astate -> tlog("plugins_in_folder {}", p.string());
  
  auto res = std::make_shared<plugin_tree_t>();
  res -> is_folder = true;

  for (const auto & entry : fs::directory_iterator(p)){
    if (entry.path().extension() == ".json"){
      auto s = entry.path().stem();
      if (s!="plugins") {
	res -> nested.insert(std::pair<std::string, std::shared_ptr<plugin_tree_t> >(s, nullptr) );
	res -> is_folder = false;
	res -> plugin = std::make_shared<python_plugin_t>("","",s);
      }
    }
    if (fs::is_directory(entry.path())){
      auto s = entry.path().filename();
      auto sbd = plugins_in_folder(entry.path());
      if (sbd -> nested.size() >0)
	res -> nested.insert(std::pair<std::string, std::shared_ptr<plugin_tree_t> >(s,sbd));      
    }
  }

  return res;
}


void plugin_manager_t::locate_plugins(){
  app_state_t *astate = app_state_t::get_inst();
  fs::path plugdir = astate -> m_plugins_dir;
  if (!fs::exists(plugdir)) {
    status = plugmgr_plugin_folder_not_found;
    error_msg = fmt::format("Plugin folder not foud:\n{}\nPlugins will be unavailable.\nCheck Edit->Setting->General.\n", astate -> m_plugins_dir);
    return;
  }
  if (!fs::exists(plugdir/"__init__.py") || !fs::exists(plugdir/"plugins.json")) {
    status = plugmgr_plugins_missing;
    error_msg = fmt::format("Plugin folder either empty, corrupted or misplaced:\n{}\nPlugins will be unavailable.\nCheck Edit->Setting->General.\n", astate -> m_plugins_dir);
    return;
  }
  /*
  for (const auto & entry : fs::directory_iterator(plugdir))
    if (entry.path().extension() == ".json"){
      auto s = entry.path().stem();
      if (s!="plugins")
	hdr_names.push_back(s);
    }
  */
  plugins = plugins_in_folder(plugdir);
}


void plugin_manager_t::load_plugins(){
  app_state_t *astate = app_state_t::get_inst();
  fs::path plugdir = astate -> m_plugins_dir;

  /*
  astate -> py_mgr -> execute("import sys");
  astate -> py_mgr -> execute("sys.path += [" + plugdir.parent_path().string() + "]");
  astate -> py_mgr -> execute("import plugins");
  */
  try {
    py::exec("import sys");
    py::exec("sys.path += ['" + plugdir.parent_path().string() + "']");
    py::exec("import plugins");
    
    std::vector<std::string> list;
    {
      std::vector< std::shared_ptr<python_plugin_t> > nodes;      
      astate -> tlog("Import plugins");
      plugins -> list_all("plugins", ".", list, nodes);
    }
    std::vector<std::string> filenames;
    std::vector< std::shared_ptr<python_plugin_t> >  nodes;
    plugins -> list_all(astate -> m_plugins_dir, "/", filenames, nodes);

    /*
    for (int i=0; i<list.size(); i++){
      nodes[i] -> module_name = list[i];
      nodes[i] -> folder = filenames[i];
      nodes[i] -> plug_name = nodes[i] -> folder.filename();
    }
    */
    /*    
    for (int i=0; i<list.size(); i++){
      astate -> tlog("{} {} {}",  nodes[i] -> plug_name, nodes[i] -> folder, nodes[i] -> module_name);
    }
    */
    for (int i=0; i<list.size(); i++){
      astate -> tlog("{} {}",  list[i], filenames[i]);
    }
    
    /*
    for (auto plug : list){
      py::exec("import " + plug);
      astate -> tlog("Importing: {}", plug);
    }
    */
    
    astate -> tlog("Import plugins successful");
  } catch (py::error_already_set &err) {
    astate -> tlog("Import plugins errors:{}", err.what());
    status = plugmgr_load_error;
    error_msg = "There where some errors during plugins import.\nSome plugins will be unavailable.\nCheck python console for details";
  }
}


void plugin_manager_t::init(){  
  locate_plugins();
  if (status == plugmgr_ok)
    load_plugins();
}
