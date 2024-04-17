#ifndef QPPCAD_PYTHON_PLUGIN_H
#define QPPCAD_PYTHON_PLUGIN_H

#pragma push_macro("slots")
#undef slots
#include <pybind11/pybind11.h>
#include <pybind11/embed.h>
#pragma pop_macro("slots")

#include <QStringList>
#include <filesystem>
#include <map>

namespace fs = std::filesystem;

namespace py = pybind11;

namespace qpp {

  namespace cad {

    class python_plugin_t{

    public:

      enum plugin_status : int{
	plugin_status_ok                = 0,
	plugin_status_hdr_error         = 1,
	plugin_status_module_not_found  = 2,
	plugin_status_load_error        = 3,
	plugin_status_run_error         = 4
      };
      
      std::string plug_name;
      std::string parent_module;
      std::string module_name;
      std::string plug_menu_name;
      std::string func_call;
      std::string func_param;
      fs::path folder;
      
      plugin_status status{plugin_status_ok};
      
      python_plugin_t(const std::string & _folder,
		      const std::string & _parent_module,
		      const std::string & _name);

      void load_header();
      
      void load_module();
      
      void run();
      
    };

    //----------------------------------

    struct plugin_tree_t;

    struct plugin_tree_t{

      std::shared_ptr<python_plugin_t> plugin;
      bool is_folder {false};
      std::map<std::string, std::shared_ptr<plugin_tree_t> > nested;

      plugin_tree_t();

      void list_all(const std::string & prefix, const std::string & separ,
		    std::vector<std::string> & list,
		    std::vector< std::shared_ptr<python_plugin_t> > & plugins);

    };

    // ----------------------------------

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

      //std::vector<std::string> hdr_names;
      //std::vector<std::shared_ptr<python_plugin_t> > plugins;

      std::shared_ptr<plugin_tree_t> plugins;

      plugmgr_status status{plugmgr_ok};
      std::string error_msg{""};
      
      plugin_manager_t();

      void init();
      
      void locate_plugins();

      void load_plugins();
      
    };

    
  } // namespace qpp::cad

} // namespace qpp

#endif
