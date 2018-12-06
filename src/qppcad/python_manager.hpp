#ifndef QPP_CAD_PYTHON_MANAGER_H
#define QPP_CAD_PYTHON_MANAGER_H

#include <qppcad/qppcad.hpp>

#pragma push_macro("slots")
#undef slots
#include <pybind11/pybind11.h>
#include <pybind11/embed.h>
#pragma pop_macro("slots")

namespace py = pybind11;

namespace qpp {

  namespace cad {

    class python_manager_t {
      public:
        py::scoped_interpreter guard{};
        py::module core;
        py::module sys;
        py::dict global;
        python_manager_t();
    };

  }

}


#endif