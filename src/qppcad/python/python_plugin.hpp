#ifndef QPPCAD_PYTHON_PLUGIN_H
#define QPPCAD_PYTHON_PLUGIN_H

#pragma push_macro("slots")
#undef slots
#include <pybind11/pybind11.h>
#include <pybind11/embed.h>
#pragma pop_macro("slots")

#include <QStringList>
#include <filesystem>
#include <variant>
#include <map>
#include <data/types.hpp>

#include <qppcad/core/json_adapter.hpp>
#include <qppcad/ws_item/geom_view/geom_view.hpp>

//#include <nlohmann/json.hpp>
//using json = nlohmann::json;

namespace fs = std::filesystem;

namespace py = pybind11;

namespace qpp {

  namespace cad {

    typedef std::variant<double, float, int, bool, std::string,
			 std::vector<double>, std::vector<float>, std::vector<int>,
			 std::vector<char>,  std::vector<std::string >,
			 std::shared_ptr<xgeometry<float, periodic_cell<float> > > > plugin_param_value_t;
    
    // --------------------------------------------------------------
    
    struct plugin_param_t{
      
      basic_types type;
      std::string name;
      std::string description;
      int pos;
      std::string browse;

      plugin_param_value_t value;
      std::string default_sval, sval;

      plugin_param_t(basic_types t, const std::string & n, const std::string & d);

      bool fromString(const std::string & s);
      
    };


    // --------------------------------------------------------------

    class python_plugin_t{

      py::module mod;
        
    public:

      //constexpr static std::vector<std::string> parm_types =
      // {"bool", "int", "float", "vector3(int)", "vector3(float)" };

      enum plugin_status : int{
	plugin_status_ok                = 0,
	plugin_status_hdr_error         = 1,
	plugin_status_module_not_found  = 2,
	plugin_status_load_error        = 3,
	plugin_status_run_error         = 4
      };
      
      std::string plug_name;
      std::string module_name;
      std::vector<std::string> module_name_decomp;
      fs::path path;
      std::string plug_menu_name;
      std::string description;
  
      std::string func_call;
      std::vector<std::shared_ptr<plugin_param_t> > params;
      
      plugin_status status{plugin_status_ok};
      std::string error_msg;

      python_plugin_t(const std::string & _name,
		      const std::string & _module_name,
		      const std::string & _path);

      void load_header();
      
      void load_module();
      
      py::object run();
      
    };

    // ----------------------------
    
    struct plugin_tree_t;

    struct plugin_tree_t {
      std::string name, module_name;
      std::vector<std::shared_ptr<plugin_tree_t> > nested;
      std::shared_ptr<python_plugin_t> plugin {nullptr};

      plugin_tree_t(const std::string & _name);

      void build_nested(std::vector<std::vector<std::string> > & list, int lvl);

      void sort();

      void bind_plugins(const std::vector<std::shared_ptr<python_plugin_t> > & pluglist );
  
      void print(int offset=0);
    };

    //----------------------------------

    class plugin_manager_t{

    public:

      enum plugmgr_status : int{
	plugmgr_ok                        = 0,
	plugmgr_plugin_folder_not_found   = 1,
	plugmgr_plugins_missing           = 2, 
	plugmgr_hdr_error                 = 3,
	plugmgr_module_not_found          = 4,
	plugmgr_load_error                = 5,
	plugmgr_run_error                 = 6	
      };
  
      std::string plugdir;
      fs::path plug_path;
      std::vector<std::shared_ptr<python_plugin_t> > pluglist;
      std::shared_ptr<plugin_tree_t> plug_tree;

      plugmgr_status status{plugmgr_ok};
      std::string error_msg{""};
      std::string error_descr{""};
  
      plugin_manager_t(const std::string & _plugdir);
      void locate_plugins();
      void load_plugins();
      void init();
    };

    // ---------------------------------------------------------------------------
    
    template<class T>
    std::string extract_json(json & data, const std::string & key, T & variable){
      try {    
	variable = data[key];
	return "";
      }
      catch (json::type_error & err){
	return err.what();
      }
    }

    
  } // namespace qpp::cad

} // namespace qpp

#endif
