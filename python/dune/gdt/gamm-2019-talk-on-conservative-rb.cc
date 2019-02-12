// This file is part of the dune-gdt project:
//   https://github.com/dune-community/dune-gdt
// Copyright 2010-2018 dune-gdt developers and contributors. All rights reserved.
// License: Dual licensed as BSD 2-Clause License (http://opensource.org/licenses/BSD-2-Clause)
//      or  GPL-2.0+ (http://opensource.org/licenses/gpl-license)
//          with "runtime exception" (http://www.dune-project.org/license.html)
// Authors:
//   Felix Schindler (2018)

#include "config.h"

#include <mutex>

#include <dune/pybindxi/pybind11.h>
#include <dune/pybindxi/stl.h>
#include <python/dune/xt/common/bindings.hh>
#include <python/dune/xt/common/fvector.hh>
#include <python/dune/xt/common/exceptions.bindings.hh>

#include <dune/xt/la/container/istl.hh>
#include <dune/xt/grid/grids.hh>
#include <dune/xt/grid/gridprovider/cube.hh>
#include <dune/xt/functions/constant.hh>
#include <dune/xt/functions/interfaces/function.hh>

#include <dune/gdt/discretefunction/default.hh>
#include <dune/gdt/functionals/vector-based.hh>
#include <dune/gdt/local/bilinear-forms/integrals.hh>
#include <dune/gdt/local/functionals/integrals.hh>
#include <dune/gdt/local/integrands/conversion.hh>
#include <dune/gdt/local/integrands/elliptic.hh>
#include <dune/gdt/local/integrands/elliptic-ipdg.hh>
#include <dune/gdt/local/integrands/product.hh>
#include <dune/gdt/operators/matrix-based.hh>
#include <dune/gdt/operators/ipdg-flux-reconstruction.hh>
#include <dune/gdt/spaces/l2/discontinuous-lagrange.hh>
#include <dune/gdt/spaces/hdiv/raviart-thomas.hh>

using namespace Dune;
using namespace Dune::GDT;

using G = ALU_2D_SIMPLEX_CONFORMING;
using GP = XT::Grid::GridProvider<G>;
using GV = typename G::LeafGridView;
using E = XT::Grid::extract_entity_t<GV>;
using I = XT::Grid::extract_intersection_t<GV>;
static const constexpr size_t d = G::dimension;

using M = XT::LA::IstlRowMajorSparseMatrix<double>;
using V = XT::LA::IstlDenseVector<double>;

using DG = DiscontinuousLagrangeSpace<GV>;
using RTN = RaviartThomasSpace<GV>;
using ScalarDF = DiscreteFunction<V, GV>;
using VectorDF = DiscreteFunction<V, GV, d>;


PYBIND11_MODULE(gamm_2019_talk_on_conservative_rb, m)
{
  namespace py = pybind11;
  using namespace pybind11::literals;

  py::module::import("dune.xt.common");
  py::module::import("dune.xt.la");
  py::module::import("dune.xt.grid");
  py::module::import("dune.xt.functions");

  Dune::XT::Common::bindings::addbind_exceptions(m);
  Dune::XT::Common::bindings::add_initialization(m, "dune.gdt");

  py::class_<GP> grid_provider(m, "GridProvider", "GridProvider");
  grid_provider.def(py::init([](const FieldVector<double, d>& lower_left,
                                const FieldVector<double, d>& upper_right,
                                const std::array<unsigned int, d>& num_elements) {
                      return new GP(XT::Grid::make_cube_grid<G>(lower_left, upper_right, num_elements));
                    }),
                    "lower_left"_a,
                    "upper_right"_a,
                    "num_elements"_a);
  grid_provider.def_property_readonly("num_elements", [](GP& self) { return self.leaf_view().indexSet().size(0); });
  grid_provider.def("refine", [](GP& self, const int num_refinements) { self.global_refine(num_refinements); });

  py::class_<DG> dg_space(m, "DiscontinuousLagrangeSpace", "DiscontinuousLagrangeSpace");
  dg_space.def(py::init([](GP& grid_provider, const int order) { return new DG(grid_provider.leaf_view(), order); }),
               "grid_provider"_a,
               "order"_a = 1);
  dg_space.def_property_readonly("dimDomain", [](DG& /*self*/) { return d; });
  dg_space.def_property_readonly("num_DoFs", [](DG& self) { return self.mapper().size(); });

  py::class_<RTN> rtn_space(m, "RaviartThomasSpace", "RaviartThomasSpace");
  rtn_space.def(py::init([](GP& grid_provider, const int order) { return new RTN(grid_provider.leaf_view(), order); }),
                "grid_provider"_a,
                "order"_a = 1);
  rtn_space.def_property_readonly("dimDomain", [](RTN& /*self*/) { return d; });
  rtn_space.def_property_readonly("num_DoFs", [](RTN& self) { return self.mapper().size(); });

  py::class_<ScalarDF> scalar_discrete_function(m, "ScalarDiscreteFunction", "ScalarDiscreteFunction");
  scalar_discrete_function.def(
      py::init([](DG& space, V& vec, const std::string& name) { return new ScalarDF(space, vec, name); }),
      "dg_space"_a,
      "DoF_vector"_a,
      "name"_a);
  scalar_discrete_function.def(
      "visualize", [](ScalarDF& self, const std::string filename) { self.visualize(filename); }, "filename"_a);

  py::class_<VectorDF> vector_discrete_function(m, "VectorDiscreteFunction", "VectorDiscreteFunction");
  vector_discrete_function.def(
      py::init([](RTN& space, V& vec, const std::string& name) { return new VectorDF(space, vec, name); }),
      "rtn_space"_a,
      "DoF_vector"_a,
      "name"_a);
  vector_discrete_function.def(
      "visualize", [](VectorDF& self, const std::string filename) { self.visualize(filename); }, "filename"_a);

  m.def("make_discrete_function",
        [](DG& dg_space, V& vec, const std::string& name) { return ScalarDF(dg_space, vec, name); },
        "dg_space"_a,
        "DoF_vector"_a,
        "name"_a);
  m.def("make_discrete_function",
        [](RTN& rtn_space, V& vec, const std::string& name) { return VectorDF(rtn_space, vec, name); },
        "rtn_space"_a,
        "DoF_vector"_a,
        "name"_a);

  m.def("assemble_SWIPDG_matrix",
        [](DG& space, XT::Functions::FunctionInterface<d>& diffusion_factor, const bool parallel) {
          const XT::Functions::ConstantFunction<d, d, d> diffusion_tensor(
              XT::Common::FieldMatrix<double, 2, d>({{1., 0.}, {0., 1.}}));
          auto op = make_matrix_operator<M>(space, Stencil::element_and_intersection);
          op.append(LocalElementIntegralBilinearForm<E>(LocalEllipticIntegrand<E>(
              diffusion_factor.as_grid_function<E>(), diffusion_tensor.as_grid_function<E>())));
          op.append(LocalIntersectionIntegralBilinearForm<I>(
                        LocalEllipticIpdgIntegrands::
                            Inner<I, double, LocalEllipticIpdgIntegrands::Method::swipdg_affine_factor>(
                                diffusion_factor.as_grid_function<E>(), diffusion_tensor.as_grid_function<E>())),
                    {},
                    XT::Grid::ApplyOn::InnerIntersectionsOnce<GV>());
          op.append(LocalIntersectionIntegralBilinearForm<I>(
                        LocalEllipticIpdgIntegrands::
                            DirichletBoundaryLhs<I, double, LocalEllipticIpdgIntegrands::Method::swipdg_affine_factor>(
                                diffusion_factor.as_grid_function<E>(), diffusion_tensor.as_grid_function<E>())),
                    {},
                    XT::Grid::ApplyOn::BoundaryIntersections<GV>());
          op.assemble(parallel);
          return std::move(std::make_unique<M>(std::move(op.matrix())));
        },
        py::call_guard<py::gil_scoped_release>(),
        "dg_space"_a,
        "diffusion_factor"_a,
        "parallel"_a = true);

  m.def("assemble_L2_vector",
        [](DG& space, XT::Functions::FunctionInterface<d>& force, const bool parallel) {
          auto func = make_vector_functional<V>(space);
          func.append(LocalElementIntegralFunctional<E>(
              local_binary_to_unary_element_integrand(LocalElementProductIntegrand<E>(), force.as_grid_function<E>())));
          func.assemble(parallel);
          return std::move(std::make_unique<V>(std::move(func.vector())));
        },
        py::call_guard<py::gil_scoped_release>(),
        "dg_space"_a,
        "force"_a,
        "parallel"_a = true);

  m.def(
      "assemble_DG_product_matrix",
      [](DG& space, const bool parallel) {
        const XT::Functions::ConstantFunction<d> diffusion_factor(1.);
        const XT::Functions::ConstantFunction<d, d, d> diffusion_tensor(
            XT::Common::FieldMatrix<double, 2, d>({{1., 0.}, {0., 1.}}));
        auto op = make_matrix_operator<M>(space, Stencil::element_and_intersection);
        op.append(LocalElementIntegralBilinearForm<E>(
            LocalEllipticIntegrand<E>(diffusion_factor.as_grid_function<E>(), diffusion_tensor.as_grid_function<E>())));
        op.append(LocalIntersectionIntegralBilinearForm<I>(
                      LocalEllipticIpdgIntegrands::
                          InnerOnlyPenalty<I, double, LocalEllipticIpdgIntegrands::Method::swipdg_affine_factor>(
                              diffusion_factor.as_grid_function<E>(), diffusion_tensor.as_grid_function<E>())),
                  {},
                  XT::Grid::ApplyOn::InnerIntersectionsOnce<GV>());
        op.append(LocalIntersectionIntegralBilinearForm<I>(LocalEllipticIpdgIntegrands::DirichletBoundaryLhsOnlyPenalty<
                                                           I,
                                                           double,
                                                           LocalEllipticIpdgIntegrands::Method::swipdg_affine_factor>(
                      diffusion_factor.as_grid_function<E>(), diffusion_tensor.as_grid_function<E>())),
                  {},
                  XT::Grid::ApplyOn::BoundaryIntersections<GV>());
        op.assemble(parallel);
        return std::move(std::make_unique<M>(std::move(op.matrix())));
      },
      py::call_guard<py::gil_scoped_release>(),
      "dg_space"_a,
      "parallel"_a = true);

  m.def("compute_flux_reconstruction",
        [](GP& grid, DG& dg_space, RTN& rtn_space, XT::Functions::FunctionInterface<d>& diffusion_factor, V& dg_vec) {
          const XT::Functions::ConstantFunction<d, d, d> diffusion_tensor(
              XT::Common::FieldMatrix<double, 2, d>({{1., 0.}, {0., 1.}}));
          auto op =
              make_ipdg_flux_reconstruction_operator<M, LocalEllipticIpdgIntegrands::Method::swipdg_affine_factor>(
                  grid.leaf_view(),
                  dg_space,
                  rtn_space,
                  diffusion_factor.template as_grid_function<E>(),
                  diffusion_tensor.template as_grid_function<E>());
          auto rtn_vec = op.apply(dg_vec);
          return std::move(std::make_unique<V>(std::move(rtn_vec)));
        },
        py::call_guard<py::gil_scoped_release>(),
        "grid"_a,
        "dg_space"_a,
        "rtn_space"_a,
        "diffusion_factor"_a,
        "dg_DoF_vector"_a);

  m.def("assemble_Hdiv_product_matrix",
        [](RTN& rtn_space, const bool parallel) {
          auto op = make_matrix_operator<M>(rtn_space, Stencil::element_and_intersection);
          op.append(LocalElementIntegralBilinearForm<E, d>(LocalElementProductIntegrand<E, d>()));
          op.append(LocalElementIntegralBilinearForm<E, d>(
              [](const auto& test_basis, const auto& ansatz_basis, const auto& /*param*/) {
                return std::max(test_basis.order() - 1, 0) + std::max(ansatz_basis.order() - 1, 0);
              },
              [](const auto& test_basis,
                 const auto& ansatz_basis,
                 const auto& point_in_reference_element,
                 auto& result,
                 const auto& /*param*/) {
                auto test_grads = test_basis.jacobians_of_set(point_in_reference_element);
                auto ansatz_grads = ansatz_basis.jacobians_of_set(point_in_reference_element);
                auto divergence = [](const auto& grad) {
                  double div = 0.;
                  for (size_t dd = 0; dd < d; ++dd)
                    div += grad[dd][dd];
                  return div;
                };
                for (size_t ii = 0; ii < test_basis.size(); ++ii)
                  for (size_t jj = 0; jj < ansatz_basis.size(); ++jj)
                    for (size_t dd = 0; dd < d; ++dd)
                      result[ii][jj] = divergence(test_grads[ii]) * divergence(ansatz_grads[jj]);
              }));
          op.assemble(parallel);
          return std::move(std::make_unique<M>(std::move(op.matrix())));
        },
        py::call_guard<py::gil_scoped_release>(),
        "dg_space"_a,
        "parallel"_a = true);

  m.def("visualize",
        [](GP& grid, XT::Functions::FunctionInterface<d>& func, const std::string& filename) {
          func.visualize(grid.leaf_view(), filename);
        },
        py::call_guard<py::gil_scoped_release>(),
        "grid"_a,
        "function"_a,
        "filename"_a);

  m.def("compute_local_conservation_error",
        [](GP& grid, VectorDF& flux, XT::Functions::FunctionInterface<d>& rhs, const bool parallel) {
          const auto& grid_rhs = rhs.template as_grid_function<E>();
          double error = 0.;
          std::mutex eror_mutex;
          auto walker = XT::Grid::make_walker(grid.leaf_view());
          walker.append([](/*prepare nothing*/) {},
                        [&](const auto& element) {
                          auto local_flux = flux.local_function();
                          local_flux->bind(element);
                          auto local_rhs = grid_rhs.local_function();
                          local_rhs->bind(element);
                          auto local_error =
                              LocalElementIntegralFunctional<E>(
                                  [&](const auto&, const auto&) {
                                    return std::max(std::max(local_flux->order() - 1, 0), local_rhs->order());
                                  },
                                  [&](const auto&, const auto& xx, auto& result, const auto&) {
                                    auto flux_grads = local_flux->jacobian(xx);
                                    auto rhs_value = local_rhs->evaluate(xx);
                                    auto divergence = [](const auto& grad) {
                                      double div = 0.;
                                      for (size_t dd = 0; dd < d; ++dd)
                                        div += grad[dd][dd];
                                      return div;
                                    };
                                    result[0] = divergence(flux_grads) - rhs_value;
                                  },
                                  {},
                                  /*over_integrate=*/3)
                                  .apply(*local_rhs)[0];
                          std::lock_guard<std::mutex> lock(eror_mutex);
                          error += std::abs(local_error);
                        },
                        [](/*finalize nothing*/) {});
          walker.walk(parallel);
          return error;
        },
        py::call_guard<py::gil_scoped_release>(),
        "grid"_a,
        "flux"_a,
        "rhs"_a,
        "parallel"_a = true);
} // PYBIND11_PLUGIN(...)
