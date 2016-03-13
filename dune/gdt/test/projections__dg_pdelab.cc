// This file is part of the dune-gdt project:
//   http://users.dune-project.org/projects/dune-gdt
// Copyright holders: Felix Schindler
// License: BSD 2-Clause License (http://opensource.org/licenses/BSD-2-Clause)

#include <dune/stuff/test/main.hxx>

#include "projections.hh"
#include "spaces/dg/pdelab.hh"

using namespace Dune::GDT::Test;

#if HAVE_DUNE_PDELAB


typedef testing::Types< SPACES_DG_PDELAB(1)
#if HAVE_ALUGRID
                      , SPACES_DG_PDELAB_ALUGRID(1)
#endif
                      > SpaceTypes;

TYPED_TEST_CASE(ProjectionTest, SpaceTypes);
TYPED_TEST(ProjectionTest, produces_correct_results) {
  this->produces_correct_results();
}


#else // HAVE_DUNE_PDELAB


TEST(DISABLED_ProjectionTest, produces_correct_results) {}


#endif // HAVE_DUNE_PDELAB
