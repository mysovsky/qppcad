#include <qppcad/python/python_plugin.hpp>
#include <qppcad/core/app_state.hpp>
#include <filesystem>

#include <pybind11/pybind11.h>
#include <pybind11/embed.h>
#include <pybind11/stl.h>

namespace py = pybind11;
namespace fs = std::filesystem;

using namespace qpp;
using namespace qpp::cad;

// ----------------------------

std::string join_strings(const std::vector<std::string> & v, const std::string & separ){
  std::string res;
  if (v.empty()) return "";
  res = v[0];
  for (auto i = v.begin()+1; i != v.end(); i++)
    res = res + separ + *i;
  return res;
}

bool compare_string_decomps(const std::vector<std::string> & s1, const std::vector<std::string> & s2, int n){
  if (s1.size()<n || s2.size()<n)
    return false;
  for (int i=0; i<n; i++)
    if (s1[i] != s2[i])
      return false;
  return true;
}

// --------------------------------------------------------------

plugin_param_t::plugin_param_t(basic_types t, const std::string & n, const std::string & d):
  type(t), name(n), description(d)
{}

bool plugin_param_t::fromString(const std::string & s){
  app_state_t *astate = app_state_t::get_inst();
  switch (type){
  case type_qpp_geometry: {
    try{
      auto g = std::get<std::shared_ptr<xgeometry<float, periodic_cell<float> > > >(value);
      astate -> tlog("geometry {} {}",g->name, g->nat());
    }
    catch (const std::bad_variant_access& ex){
      astate -> tlog("value error {}",ex.what());
      return false;
    }
    /*
    if (s == "current"){
      auto [cur_ws, cur_it, gv] = astate -> ws_mgr -> get_sel_tpl_itm<geom_view_t>();
      if (!gv)return false;
      auto g = gv -> m_geom;
      //astate -> tlog("nat: {}", g -> nat() );
      value = g;
    }
    else
      return false;
    */
    return true;
    break;
  }
  case type_float: {
    try {
      value = std::stod(s);
      return true;
    }
    catch (std::invalid_argument & err){
      return false;
    }
    break;
  }
  case type_int: {
    try {
      value = std::stoi(s);
    }
    catch (std::invalid_argument & err){
      return false;
    }
    return true;
    break;
  }
  case type_bool: {
    auto s1 = tolower(s);
    if ( s1 == "true" || s1 == "on" || s1 == "yes")
      value = true;
    else if ( s1 == "false" || s1 == "off" || s1 == "no")
      value = false;
    else
      return false;
    return true;
    break;
  }
  case type_string: {
    value = s;
    return true;
    break;
  }
  case type_array + type_string: {
    auto ss = split(s,", \t");
    //for (std::string & str : ss)
    //  str.erase(std::remove(str.begin(), str.end(), ' '), str.end());
    value = ss;
    return true;
    break;
  }
  case type_array + type_float: {
    auto ss = split(s,", \t");
    std::vector<float> val;
    for (std::string f : ss)
      val.push_back(std::stod(f));
    value = val;
    return true;
    break;
  }
  case type_array + type_int: {
    auto ss = split(s,", \t");
    std::vector<int> val;
    for (std::string f : ss)
      val.push_back(std::stoi(f));
    value = val;
    return true;
    break;
  }
  case type_array + type_bool: {
    auto ss = split(s,", \t");
    std::vector<char> val;
    for (std::string f : ss) {
      auto f1 = tolower(f);
      if ( f1 == "true" || f1 == "on" || f1 == "yes")
	val.push_back(true);
      else if ( f1 == "false" || f1 == "off" || f1 == "no")
	val.push_back(false);
      else
	return false;
    }
    value = val;
    return true;
    break;
  } 
  }
  return true;
}
      
// --------------------------------------------------------------

python_plugin_t::python_plugin_t(const std::string & _name,
				 const std::string & _module_name,
				 const std::string & _path){
  plug_name = _name;
  path = _path;
  module_name = _module_name;
}

void python_plugin_t::load_header(){
  app_state_t *astate = app_state_t::get_inst();
  json data, jparams;
  try{
    std::fstream f(path);
    data = json::parse(f);        
  }
  catch (json::parse_error& ex){
    status = plugin_status_hdr_error;
    error_msg = "Error parsing " + module_name + ":" + ex.what();      
  }

  std::string
    err = extract_json(data, "menu name", plug_menu_name) + 
    extract_json(data, "function", func_call) + 
    extract_json(data, "description", description) +
    extract_json(data, "parameters", jparams);
  if ( err != "" ){
    error_msg = "Type error parsing " + module_name + "\n" + err;
    status = plugin_status_hdr_error;
    return;
  }
  
  //std::cout << "name = " << plug_name << "\nmenu name = " << plug_menu_name << "\nfunction = " << func_call <<
  //"\n--- parameters:\n";
  description = description + "\nDesription of parameters:\n";

  //astate -> tlog("name= {} menu name= {} function= {}\n parameters:\n", plug_name, plug_menu_name, func_call);
  try {
    for (json::iterator it = jparams.begin(); it != jparams.end(); ++it){    
      basic_types t;
      for (int i=0; i<type_data::type_name.size(); i++)
	if ( (*it)["type"] == type_data::type_name[i]){
	  t = basic_types(i);
	  break;
	}
      
      auto p = std::make_shared<plugin_param_t>(t, it.key(), it.value()["description"]);
      p -> pos = it.value()["position"];
      p -> default_sval = it.value()["default"];
      p -> fromString( p -> default_sval );
      p -> browse = "";
      extract_json(it.value(), "browse", p -> browse);
      if ( p -> type != type_string)
	p -> browse = "";

      //astate -> tlog("param name= {} type= {} default= {}", p->name, p->type, p->default_sval);
      
      params.push_back(p);
      description = description + "     " + p -> name + "\n" + p -> description + "\n";
    }
  }
  catch (json::type_error & err){
    error_msg = "Type error parsing " + module_name;
    status = plugin_status_hdr_error;
  }

  std::sort(params.begin(), params.end(),
	    [](std::shared_ptr<plugin_param_t> p1, std::shared_ptr<plugin_param_t> p2){
	      return p1 -> pos < p2 -> pos;
	    });

  /*
  for (auto p : params){
    std::cout << "    " << p->name << " = \n";// << (*it)["default"] << "\n";
    std::cout << "        type = " << p -> type << "\n        " <<
      p -> description << "\n";
  }
  */
  for (auto el = data.begin(); el != data.end(); el++)
    if (el.key() == "parameters") 
      for (auto pm = (*el).begin(); pm != (*el).end(); pm++) 
	json parm = pm.value();		 
  
}

void python_plugin_t::load_module(){
  try {
    mod = py::module::import(module_name.c_str());
  }
  catch (py::error_already_set &err) {
    error_msg = error_msg + "Error importing " + module_name + ":" + err.what();
    status = plugin_status_load_error;
  }
}

py::object python_plugin_t::run(){
  py::list args;
  for (auto p : params){
    //p -> sval = p -> default_sval;
    if ( ! p -> fromString(p -> sval) ){
      status = plugin_status_run_error;
      error_msg = "Value error in parameter " + p -> name;
      return py::cast(error_msg);
    }
    args.append( p -> value );    
  }
  py::object func = mod.attr(func_call.c_str());
  py::object results = func(*args);
  app_state_t::get_inst() -> make_viewport_dirty();
  return results;
}
      
// ----------------------------

plugin_tree_t::plugin_tree_t(const std::string & _name){
  name = _name;
}

void plugin_tree_t::build_nested(std::vector<std::vector<std::string> > & list, int lvl){
  if (list[0].size()==lvl){
    module_name = join_strings(list[0],".");
    return;
  }
  std::vector<std::string> module_name_decomp(list[0].begin(), list[0].begin() + lvl);
  //std::cout << join_strings(module_name_decomp,".") << "\n";
  module_name = join_strings(module_name_decomp,".");
  while (!list.empty()) {
    std::vector<std::vector<std::string> > sublist;
    std::vector<int> idcs = {0};
    for (int i=1; i < list.size(); i++)
      if (compare_string_decomps(list[0], list[i], lvl+1 ))
	idcs.push_back(i);
    for (int i : idcs)
      sublist.push_back(std::vector<std::string>(list[i].begin(), list[i].end()) );

    /*
      std::cout << "n= " << lvl << " " << name << "\n";
      for (auto l:sublist){
      for (auto s:l) std::cout << s << " ";
      std::cout << "\n";     
      }*/
      
    for (int i = idcs.size()-1; i>=0; i--)
      list.erase(list.begin()+idcs[i]);
    auto p = std::make_shared<plugin_tree_t>(sublist[0][lvl]);
    p -> build_nested(sublist, lvl+1);
    nested.push_back(p);
  }
}

void plugin_tree_t::sort(){
  if (nested.empty())
    return;
  std::sort(nested.begin(), nested.end(),
	    [](std::shared_ptr<plugin_tree_t> p1, std::shared_ptr<plugin_tree_t> p2){
	      return p1 -> name < p2 -> name;
	    });
  for (auto p:nested)
    p -> sort();
}

void plugin_tree_t::bind_plugins(const std::vector<std::shared_ptr<python_plugin_t> > & pluglist ){
  if (nested.empty()){
    for (auto p:pluglist)
      if (module_name == p->module_name)
	plugin = p;
  }
  else
    for (auto p:nested)
      p -> bind_plugins(pluglist);
}
  
void plugin_tree_t::print(int offset){

  std::cout << std::string(offset,' ') << name;
  if (nested.empty())
    std::cout << " | " << plugin -> module_name << " " << plugin -> path;
  std::cout << "\n";
  for (auto n:nested) n->print(offset+10);
}


//----------------------------------
  
plugin_manager_t::plugin_manager_t(const std::string & _plugdir):
  plugdir(_plugdir), plug_path(plugdir){

  plug_tree = std::make_shared<plugin_tree_t>("plugins");

  if (!fs::exists(plug_path)) {
    status = plugmgr_plugin_folder_not_found;
    //error_msg = fmt::format("Plugin folder not foud:\n{}\nPlugins will be unavailable.\nCheck Edit->Setting->General.\n", plugdir);
    error_msg = "Plugin folder not foud:\n{}\nPlugins will be unavailable.\nCheck Edit->Setting->General.\n";
    error_descr = error_msg;
    return;
  }

  if (!fs::exists(plug_path/"__init__.py") || !fs::exists(plug_path/"plugins.json")) {
    status = plugmgr_plugins_missing;
    //error_msg = fmt::format("Plugin folder either empty, corrupted or misplaced:\n{}\nPlugins will be unavailable.\nCheck Edit->Setting->General.\n", plugdir);
    error_msg = "Plugin folder either empty, corrupted or misplaced:\n{}\nPlugins will be unavailable.\nCheck Edit->Setting->General.\n";
    error_descr = error_msg;
    return;
  }
}
  
void plugin_manager_t::locate_plugins(){
  std::vector<std::string> plug_dir_decomp(plug_path.begin(), plug_path.end());

  for (const fs::directory_entry& dir_entry :   fs::recursive_directory_iterator(plugdir)){
    std::vector<std::string> entry_path_decomp(dir_entry.path().begin(),
					       dir_entry.path().end());
    std::vector<std::string> relpath_decomp(entry_path_decomp.begin() + plug_dir_decomp.size() - 1,
					    entry_path_decomp.end());
    if (dir_entry.path().extension() == ".json" &&
	dir_entry.path().filename() != "plugins.json" ){
      *(relpath_decomp.end()-1) = dir_entry.path().stem();
      std::shared_ptr<python_plugin_t>  p =
	std::make_shared<python_plugin_t>(dir_entry.path().stem(),
					  join_strings(relpath_decomp,"."),
					  dir_entry.path());
      p -> module_name_decomp = relpath_decomp;
      pluglist.push_back(p);
    }
  }

  std::vector<std::vector<std::string> > pluglist_decomp;
  for (auto plug : pluglist)
    pluglist_decomp.push_back(plug->module_name_decomp);

  plug_tree -> build_nested(pluglist_decomp,1);
  plug_tree -> bind_plugins(pluglist);
  plug_tree -> sort();
}
  
void plugin_manager_t::load_plugins(){
  for (auto plug : pluglist){
    plug -> load_header();
    if (plug -> status == python_plugin_t::plugin_status_hdr_error){
      status = plugmgr_hdr_error;
      error_descr =  error_descr + plug  -> error_msg + "\n";
    }
  }
  if (status == plugmgr_hdr_error){
    error_msg =  error_msg + "There where errors in plugin header files.\nSome plugins will be unavailable.\nCheck python console for details\n";
    error_descr = error_descr  + "There where errors in plugin header files.\nSome plugins will be unavailable.\nCheck python console for details\n";
  }
  py::exec("import sys");
  py::exec("sys.path += ['" + plug_path.parent_path().string() + "']");
  py::exec("import plugins");
  for (auto plug : pluglist){
    if (plug -> status == python_plugin_t::plugin_status_ok){
      std::cout << "Loading " << plug -> module_name << "\n";
      plug -> load_module();
      if (plug -> status == python_plugin_t::plugin_status_load_error){
	status = plugmgr_load_error;
	error_descr = error_descr + plug -> error_msg + "\n";
      }
    }
  }
  if (status == plugmgr_load_error) {
    error_msg = error_msg + "There were errors during plugins import.\nSome plugins will be unavailable.\nCheck python console for details\n";
    error_descr = error_descr + "There were errors during plugins import.\nSome plugins will be unavailable.\nCheck python console for details\n";      
  }
}
      
void plugin_manager_t::init(){
  locate_plugins();
  if (status == plugmgr_ok)
    load_plugins();
  
  app_state_t *astate = app_state_t::get_inst();
  //astate -> tlog("plugins dir: ", plugdir );
}
