// This file is part of the dune-gdt project:
//   https://github.com/dune-community/dune-gdt
// Copyright 2010-2016 dune-gdt developers and contributors. All rights reserved.
// License: BSD 2-Clause License (http://opensource.org/licenses/BSD-2-Clause)
// Authors:
//   Tobias Leibner  (2016)

#include <iostream>

#include "config.h"

#include <dune/common/parallel/mpihelper.hh>

#include <dune/xt/common/string.hh>
#include <dune/xt/common/parallel/threadmanager.hh>

#include <dune/xt/grid/grids.hh>

#include <dune/gdt/momentmodels/moment-approximation.hh>
#include <dune/gdt/momentmodels/basisfunctions.hh>

#include <dune/xt/common/string.hh>
#include <dune/xt/common/parallel/threadmanager.hh>

template <int momentOrder, Dune::GDT::EntropyType entropy>
struct moment_approximation_helper
{
  static void run(const int quadrature_refinements, const std::string testcasename, const std::string filename)
  {
    using namespace Dune;
    using namespace Dune::GDT;

    using BasisfunctionType = RealSphericalHarmonicsMomentBasis<double, double, momentOrder, 3, false, entropy>;

    using GridType = YASP_3D_EQUIDISTANT_OFFSET;
    using GridViewType = typename GridType::LeafGridView;
    using VectorType = typename Dune::XT::LA::Container<double, Dune::XT::LA::default_backend>::VectorType;
    using DiscreteFunctionType = DiscreteFunction<VectorType, GridViewType>;
    auto test = std::make_unique<MomentApproximation<BasisfunctionType, DiscreteFunctionType>>();
    test->run(quadrature_refinements, testcasename, filename);
    moment_approximation_helper<momentOrder - 1, entropy>::run(quadrature_refinements, testcasename, filename);
  }
};

template <Dune::GDT::EntropyType entropy>
struct moment_approximation_helper<0, entropy>
{
  static void
  run(const int /*quadrature_refinements*/, const std::string /*testcasename*/, const std::string /*filename*/)
  {}
};


int main(int argc, char** argv)
{
  using namespace Dune;
  using namespace Dune::GDT;

  MPIHelper::instance(argc, argv);

  std::string testcasename = "GaussOnSphere";
  if (argc == 2)
    testcasename = argv[1];
  else if (argc > 2) {
    std::cerr << "Too many command line arguments, please provide a testcase name only!" << std::endl;
    return 1;
  }

  static constexpr int max_order = 10;
  static constexpr EntropyType entropy = EntropyType::MaxwellBoltzmann;
  const int quadrature_refinements = 5;
  moment_approximation_helper<max_order, entropy>::run(quadrature_refinements, testcasename, testcasename);
}
