// This file is part of the dune-gdt project:
//   https://github.com/dune-community/dune-gdt
// Copyright 2010-2018 dune-gdt developers and contributors. All rights reserved.
// License: Dual licensed as BSD 2-Clause License (http://opensource.org/licenses/BSD-2-Clause)
//      or  GPL-2.0+ (http://opensource.org/licenses/gpl-license)
//          with "runtime exception" (http://www.dune-project.org/license.html)
// Authors:
//   Felix Schindler (2020)

#include "config.h"

#include <dune/pybindxi/pybind11.h>
#include <dune/pybindxi/stl.h>

#include <dune/xt/grid/type_traits.hh>
#include <dune/xt/grid/grids.hh>

#include <dune/gdt/local/functionals/integrals.hh>

#include <python/dune/xt/common/configuration.hh>
#include <python/dune/xt/common/fvector.hh>
#include <python/dune/xt/grid/grids.bindings.hh>


namespace Dune {
namespace GDT {
namespace bindings {


template <class G, class E, size_t r = 1, size_t rC = 1, class RF = double, class F = double>
class LocalElementIntegralFunctional
{
  static const size_t d = G::dimension;

public:
  using type = GDT::LocalElementIntegralFunctional<E, r, rC, RF, F>;
  using base_type = GDT::LocalElementFunctionalInterface<E, r, rC, RF, F>;
  using bound_type = pybind11::class_<type, base_type>;

  static bound_type bind(pybind11::module& m,
                         const std::string& layer_id = "",
                         const std::string& grid_id = XT::Grid::bindings::grid_name<G>::value(),
                         const std::string& class_id = "local_element_integral_functional")
  {
    namespace py = pybind11;
    using namespace pybind11::literals;

    std::string class_name = class_id;
    class_name += "_" + grid_id;
    if (!layer_id.empty())
      class_name += "_" + layer_id;
    std::string test_string = "";
    test_string += "_" + XT::Common::to_string(r) + "d";
    if (rC > 1)
      test_string += "x" + XT::Common::to_string(rC) + "d";
    if (!std::is_same<RF, double>::value)
      test_string += "_" + XT::Common::Typename<RF>::value(/*fail_wo_typeid=*/true);
    test_string += "_test_basis";
    class_name += test_string;
    class_name += "_to_scalar";
    if (!std::is_same<F, double>::value)
      class_name += "_" + XT::Common::Typename<F>::value(/*fail_wo_typeid=*/true);
    const auto ClassName = XT::Common::to_camel_case(class_name);
    bound_type c(m, ClassName.c_str(), class_id.c_str());
    c.def(py::init<const typename type::IntegrandType&, const int>(),
          "unary_element_integrand"_a,
          "over_integrate"_a = 0);

    // factory
    m.def(
        XT::Common::to_camel_case(class_id).c_str(),
        [](const typename type::IntegrandType& unary_element_integrand, const int over_integrate) {
          return new type(unary_element_integrand, over_integrate);
        },
        "unary_element_integrand"_a,
        "over_integrate"_a = 0);

    return c;
  } // ... bind(...)
}; // class LocalElementIntegralFunctional


} // namespace bindings
} // namespace GDT
} // namespace Dune


template <class GridTypes = Dune::XT::Grid::AvailableGridTypes>
struct LocalElementIntegralFunctional_for_all_grids
{
  using G = typename GridTypes::head_type;
  using GV = typename G::LeafGridView;
  using E = Dune::XT::Grid::extract_entity_t<GV>;
  static const constexpr size_t d = G::dimension;
  using F = double;

  static void bind(pybind11::module& m)
  {
    using Dune::GDT::bindings::LocalElementIntegralFunctional;

    LocalElementIntegralFunctional<G, E>::bind(m);
    if (d > 1) {
      LocalElementIntegralFunctional<G, E, d, 1, F, F>::bind(m);
      LocalElementIntegralFunctional<G, E, d, d, F, F>::bind(m);
    }
    // add your extra dimensions here
    // ...
    LocalElementIntegralFunctional_for_all_grids<typename GridTypes::tail_type>::bind(m);
  }
};

template <>
struct LocalElementIntegralFunctional_for_all_grids<boost::tuples::null_type>
{
  static void bind(pybind11::module& /*m*/) {}
};


PYBIND11_MODULE(_local_functionals_element_integrals, m)
{
  namespace py = pybind11;
  using namespace Dune;
  using namespace Dune::XT;
  using namespace Dune::GDT;

  py::module::import("dune.xt.common");
  py::module::import("dune.xt.la");
  py::module::import("dune.xt.grid");
  py::module::import("dune.xt.functions");

  py::module::import("dune.gdt._local_functionals_element_interface");

  LocalElementIntegralFunctional_for_all_grids<XT::Grid::AvailableGridTypes>::bind(m);
}
