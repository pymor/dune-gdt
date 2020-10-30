// This file is part of the dune-gdt project:
//   https://github.com/dune-community/dune-gdt
// Copyright 2010-2016 dune-gdt developers and contributors. All rights reserved.
// License: BSD 2-Clause License (http://opensource.org/licenses/BSD-2-Clause)
// Authors:
//   Tobias Leibner  (2016)

// This one has to come first (includes the config.h)!
#include <dune/xt/test/main.hxx>

#if HAVE_DUNE_XT_DATA

#  include <dune/gdt/test/momentmodels/kinetictransport/testcases.hh>
#  include <dune/gdt/test/momentmodels/entropic-coords-mn-discretization.hh>

using Yasp1 = Dune::YaspGrid<1, Dune::EquidistantOffsetCoordinates<double, 1>>;
using Yasp2 = Dune::YaspGrid<2, Dune::EquidistantOffsetCoordinates<double, 2>>;
using Yasp3 = Dune::YaspGrid<3, Dune::EquidistantOffsetCoordinates<double, 3>>;

using YaspGridTestCasesAll = testing::Types<
    // first order tests
    Dune::GDT::PlaneSourceMnTestCase<Yasp1, Dune::GDT::LegendreMomentBasis<double, double, 7>, false, true>,
    Dune::GDT::PlaneSourceMnTestCase<Yasp1, Dune::GDT::HatFunctionMomentBasis<double, 1, double, 8, 1, 1>, false, true>,
    Dune::GDT::SourceBeamMnTestCase<Yasp1, Dune::GDT::HatFunctionMomentBasis<double, 1, double, 8, 1, 1>, false, true>,
    Dune::GDT::PlaneSourceMnTestCase<Yasp1, Dune::GDT::PartialMomentBasis<double, 1, double, 8, 1, 1>, false, true>,
    Dune::GDT::SourceBeamMnTestCase<Yasp1, Dune::GDT::PartialMomentBasis<double, 1, double, 8, 1, 1>, false, true>,
#  if 0 //! DXT_DISABLE_LARGE_TESTS
    Dune::GDT::SourceBeamMnTestCase<Yasp1, Dune::GDT::LegendreMomentBasis<double, double, 7>, false, true>,
    Dune::GDT::
        PointSourceMnTestCase<Yasp3, Dune::GDT::RealSphericalHarmonicsMomentBasis<double, double, 2, 3>, false, true>,
    Dune::GDT::
        CheckerboardMnTestCase<Yasp3, Dune::GDT::HatFunctionMomentBasis<double, 3, double, 0, 1, 3>, false, true>,
    Dune::GDT::ShadowMnTestCase<Yasp3, Dune::GDT::HatFunctionMomentBasis<double, 3, double, 0, 1, 3>, false, true>,
    Dune::GDT::PointSourceMnTestCase<Yasp3, Dune::GDT::HatFunctionMomentBasis<double, 3, double, 0, 1, 3>, false, true>,
    Dune::GDT::PointSourceMnTestCase<Yasp3, Dune::GDT::PartialMomentBasis<double, 3, double, 0, 1, 3>, false, true>,
#  endif
    // tests with reconstruction
    Dune::GDT::PlaneSourceMnTestCase<Yasp1, Dune::GDT::LegendreMomentBasis<double, double, 7>, true, true>,
    Dune::GDT::PlaneSourceMnTestCase<Yasp1, Dune::GDT::HatFunctionMomentBasis<double, 1, double, 8, 1, 1>, true, true>,
    Dune::GDT::SourceBeamMnTestCase<Yasp1, Dune::GDT::HatFunctionMomentBasis<double, 1, double, 8, 1, 1>, true, true>,
    Dune::GDT::PlaneSourceMnTestCase<Yasp1, Dune::GDT::PartialMomentBasis<double, 1, double, 8, 1, 1>, true, true>,
    Dune::GDT::SourceBeamMnTestCase<Yasp1, Dune::GDT::PartialMomentBasis<double, 1, double, 8, 1, 1>, true, true>
#  if 0 // !DXT_DISABLE_LARGE_TESTS
    ,
    Dune::GDT::SourceBeamMnTestCase<Yasp1, Dune::GDT::LegendreMomentBasis<double, double, 7>, true, true>,
    Dune::GDT::
        PointSourceMnTestCase<Yasp3, Dune::GDT::RealSphericalHarmonicsMomentBasis<double, double, 2, 3>, true, true>,
    Dune::GDT::PointSourceMnTestCase<Yasp3, Dune::GDT::HatFunctionMomentBasis<double, 3, double, 0, 1, 3>, true, true>,
    Dune::GDT::PointSourceMnTestCase<Yasp3, Dune::GDT::PartialMomentBasis<double, 3, double, 0, 1, 3>, true, true>
#  endif // !DXT_DISABLE_LARGE_TESTS
    >;

TYPED_TEST_SUITE(HyperbolicEntropicCoordsMnTest, YaspGridTestCasesAll);
TYPED_TEST(HyperbolicEntropicCoordsMnTest, check)
{
  this->run();
}

#else // HAVE_DUNE_XT_DATA

GTEST_TEST(HyperbolicEntropicCoordsMnTest, YaspGridTestCasesAll)
{
  std::cerr << "Test disabled, missing dune-xt-data!" << std::endl;
}

#endif // HAVE_DUNE_XT_DATA
