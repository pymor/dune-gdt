// This file is part of the dune-gdt project:
//   https://github.com/dune-community/dune-gdt
// Copyright 2010-2016 dune-gdt developers and contributors. All rights reserved.
// License: BSD 2-Clause License (http://opensource.org/licenses/BSD-2-Clause)
// Authors:
//   Felix Schindler (2015 - 2016)

#ifndef DUNE_GDT_TESTS_LINEARELLIPTIC_PROBLEMS_MIXEDBOUNDARY_HH
#define DUNE_GDT_TESTS_LINEARELLIPTIC_PROBLEMS_MIXEDBOUNDARY_HH

#if HAVE_ALUGRID
#include <dune/grid/alugrid.hh>
#endif
#include <dune/grid/yaspgrid.hh>

#include <dune/xt/functions/constant.hh>
#include <dune/xt/functions/expression.hh>
#include <dune/xt/grid/boundaryinfo.hh>
#include <dune/xt/grid/gridprovider/cube.hh>

#include <dune/gdt/test/stationary-testcase.hh>

#include "base.hh"

namespace Dune {
namespace GDT {
namespace LinearElliptic {


template <class E, class D, int d, class R, int r = 1>
class MixedBoundaryProblem : public ProblemBase<E, D, d, R, r>
{
  static_assert(AlwaysFalse<E>::value, "Not available for these dimensions!");
};


template <class EntityImp, class DomainFieldImp, class RangeFieldImp>
class MixedBoundaryProblem<EntityImp, DomainFieldImp, 2, RangeFieldImp, 1>
    : public ProblemBase<EntityImp, DomainFieldImp, 2, RangeFieldImp, 1>
{
  typedef ProblemBase<EntityImp, DomainFieldImp, 2, RangeFieldImp, 1> BaseType;
  typedef XT::Functions::ConstantFunction<EntityImp, DomainFieldImp, 2, RangeFieldImp, 1> ScalarConstantFunctionType;
  typedef XT::Functions::ConstantFunction<EntityImp, DomainFieldImp, 2, RangeFieldImp, 2, 2> MatrixConstantFunctionType;
  typedef XT::Functions::ExpressionFunction<EntityImp, DomainFieldImp, 2, RangeFieldImp, 1> ExpressionFunctionType;

public:
  static const size_t default_integration_order = 2;

  static XT::Common::Configuration default_grid_cfg()
  {
    XT::Common::Configuration cfg = XT::Grid::cube_gridprovider_default_config();
    cfg["lower_left"]             = "[0 0]";
    cfg["upper_right"]            = "[1 1]";
    return cfg;
  }

  static XT::Common::Configuration default_boundary_info_cfg()
  {
    XT::Common::Configuration cfg = XT::Grid::normalbased_boundaryinfo_default_config();
    cfg["default"]                = "dirichlet";
    cfg["neumann.0"]              = "[1 0]";
    return cfg;
  }

  MixedBoundaryProblem(const size_t integration_order           = default_integration_order,
                       const XT::Common::Configuration& grd_cfg = default_grid_cfg(),
                       const XT::Common::Configuration& bnd_cfg = default_boundary_info_cfg())
    : BaseType(new ScalarConstantFunctionType(1, "diffusion_factor"),
               new MatrixConstantFunctionType(XT::Functions::internal::unit_matrix<RangeFieldImp, 2>(),
                                              "diffusion_tensor"),
               new ScalarConstantFunctionType(1, "force"),
               new ExpressionFunctionType("x", "0.25 * x[0] * x[1]", integration_order, "dirichlet"),
               new ScalarConstantFunctionType(0.1, "neumann"), grd_cfg, bnd_cfg)
  {
  }
}; // class MixedBoundaryProblem< ..., 1 >


template <class G, class R = double, int r = 1>
class MixedBoundaryTestCase
    : public Test::StationaryTestCase<G, LinearElliptic::MixedBoundaryProblem<typename G::template Codim<0>::Entity,
                                                                              typename G::ctype, G::dimension, R, r>>
{
  typedef typename G::template Codim<0>::Entity E;
  typedef typename G::ctype D;
  static const size_t d = G::dimension;

public:
  typedef LinearElliptic::MixedBoundaryProblem<E, D, d, R, r> ProblemType;

private:
  typedef Test::StationaryTestCase<G, ProblemType> BaseType;
  template <class T, bool anything = true>
  struct Helper
  {
    static_assert(AlwaysFalse<T>::value, "Please add a configuration for this grid type!");
    static XT::Common::Configuration value(XT::Common::Configuration cfg)
    {
      return cfg;
    }
  };

  template <bool anything>
  struct Helper<Dune::YaspGrid<2, Dune::EquidistantOffsetCoordinates<double, 2>>, anything>
  {
    static XT::Common::Configuration value(XT::Common::Configuration cfg)
    {
      cfg["num_elements"] = "[2 2]";
      return cfg;
    }
  };

#if HAVE_ALUGRID
  template <bool anything>
  struct Helper<ALUGrid<2, 2, simplex, conforming>, anything>
  {
    static XT::Common::Configuration value(XT::Common::Configuration cfg)
    {
      cfg["num_elements"]    = "[2 2]";
      cfg["num_refinements"] = "1";
      return cfg;
    }
  };

  template <bool anything>
  struct Helper<ALUGrid<2, 2, simplex, nonconforming>, anything>
  {
    static XT::Common::Configuration value(XT::Common::Configuration cfg)
    {
      cfg["num_elements"] = "[2 2]";
      return cfg;
    }
  };
#endif // HAVE_ALUGRID

  static XT::Common::Configuration grid_cfg()
  {
    auto cfg = ProblemType::default_grid_cfg();
    cfg      = Helper<typename std::decay<G>::type>::value(cfg);
    return cfg;
  }

public:
  using typename BaseType::GridType;

  MixedBoundaryTestCase(const size_t num_refs = 3)
    : BaseType(XT::Grid::make_cube_grid<GridType>(grid_cfg()).grid_ptr(), num_refs)
    , problem_()
  {
  }

  virtual const ProblemType& problem() const override final
  {
    return problem_;
  }

  virtual void print_header(std::ostream& out = std::cout) const override final
  {
    out << "+==========================================================+\n"
        << "|+========================================================+|\n"
        << "||  Testcase: mixed boundary types                        ||\n"
        << "|+--------------------------------------------------------+|\n"
        << "||  domain = [0, 1] x [0, 1]                              ||\n"
        << "||  diffusion = 1                                         ||\n"
        << "||  force     = 1                                         ||\n"
        << "||  neumann   = 0.1       on the right side               ||\n"
        << "||  dirichlet = 1/4 x y   everywhere else                 ||\n"
        << "||  reference solution: discrete solution on finest grid  ||\n"
        << "|+========================================================+|\n"
        << "+==========================================================+" << std::endl;
  }

private:
  const ProblemType problem_;
}; // class MixedBoundaryTestCase


} // namespace LinearElliptic
} // namespace GDT
} // namespace Dune

#endif // DUNE_GDT_TESTS_LINEARELLIPTIC_PROBLEMS_MIXEDBOUNDARY_HH
