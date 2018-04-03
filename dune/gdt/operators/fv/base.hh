// This file is part of the dune-gdt project:
//   https://github.com/dune-community/dune-gdt
// Copyright 2010-2018 dune-gdt developers and contributors. All rights reserved.
// License: Dual licensed as BSD 2-Clause License (http://opensource.org/licenses/BSD-2-Clause)
//      or  GPL-2.0+ (http://opensource.org/licenses/gpl-license)
//          with "runtime exception" (http://www.dune-project.org/license.html)
// Authors:
//   Rene Milk      (2018)
//   Tobias Leibner (2017)

#ifndef DUNE_GDT_OPERATORS_FV_BASE_HH
#define DUNE_GDT_OPERATORS_FV_BASE_HH

#include <type_traits>

#include <dune/geometry/quadraturerules.hh>

#include <dune/xt/grid/walker/apply-on.hh>

#include <dune/gdt/local/fluxes/interfaces.hh>
#include <dune/gdt/local/operators/fv.hh>
#include <dune/gdt/operators/base.hh>
#include <dune/gdt/type_traits.hh>

#include "boundary.hh"

namespace Dune {
namespace GDT {
namespace internal {


template <class AnalyticalFluxImp, class BoundaryValueImp>
class AdvectionTraitsBase
{
  static_assert(XT::Functions::is_localizable_flux_function<AnalyticalFluxImp>::value,
                "AnalyticalFluxImp has to be derived from LocalNumericalCouplingFluxInterface!");
  static_assert(is_localizable_boundary_value<BoundaryValueImp>::value,
                "BoundaryValueImp has to be derived from LocalizableBoundaryValueInterface!");

public:
  typedef AnalyticalFluxImp AnalyticalFluxType;
  typedef BoundaryValueImp BoundaryValueType;
  static const size_t dimDomain = AnalyticalFluxType::dimDomain;
  static const size_t dimRange = AnalyticalFluxType::dimRange;
  static const size_t dimRangeCols = 1;
  typedef typename BoundaryValueType::DomainFieldType DomainFieldType;
  typedef typename BoundaryValueType::RangeFieldType RangeFieldType;
  typedef RangeFieldType FieldType;
  typedef typename BoundaryValueType::DomainType DomainType;
  typedef typename AnalyticalFluxType::PartialURangeType JacobianType;
}; // class AdvectionTraitsBase


} // namespace internal


template <class AnalyticalFluxImp,
          class NumericalCouplingFluxImp,
          class NumericalBoundaryFluxImp,
          class BoundaryValueImp,
          class SourceImp,
          class RangeImp>
class AdvectionLocalizableDefault
    : public Dune::GDT::LocalizableOperatorBase<typename RangeImp::SpaceType::GridLayerType, SourceImp, RangeImp>
{
  typedef Dune::GDT::LocalizableOperatorBase<typename RangeImp::SpaceType::GridLayerType, SourceImp, RangeImp> BaseType;

  static_assert(XT::Functions::is_localizable_flux_function<AnalyticalFluxImp>::value,
                "AnalyticalFluxImp has to be derived from LocalNumericalCouplingFluxInterface!");
  static_assert(is_local_numerical_coupling_flux<NumericalCouplingFluxImp>::value,
                "NumericalCouplingFluxImp has to be derived from LocalNumericalCouplingFluxInterface!");
  static_assert(is_local_numerical_boundary_flux<NumericalBoundaryFluxImp>::value,
                "NumericalBoundaryFluxImp has to be derived from LocalNumericalBoundaryFluxInterface!");
  static_assert(is_localizable_boundary_value<BoundaryValueImp>::value,
                "BoundaryValueImp has to be derived from LocalizableBoundaryValueInterface!");
  static_assert(is_discrete_function<SourceImp>::value, "SourceImp has to be derived from DiscreteFunction!");
  static_assert(is_discrete_function<RangeImp>::value, "RangeImp has to be derived from DiscreteFunction!");

public:
  typedef AnalyticalFluxImp AnalyticalFluxType;
  typedef NumericalCouplingFluxImp NumericalCouplingFluxType;
  typedef NumericalBoundaryFluxImp NumericalBoundaryFluxType;
  typedef BoundaryValueImp BoundaryValueType;
  typedef SourceImp SourceType;
  typedef RangeImp RangeType;
  typedef typename SourceType::RangeFieldType RangeFieldType;
  typedef typename RangeType::SpaceType::GridLayerType GridLayerType;
  static const size_t dimDomain = GridLayerType::dimension;
  typedef typename Dune::GDT::LocalCouplingFvOperator<NumericalCouplingFluxType> LocalCouplingOperatorType;
  typedef typename Dune::GDT::LocalBoundaryFvOperator<NumericalBoundaryFluxType> LocalBoundaryOperatorType;

  template <class QuadratureRuleType, class... LocalOperatorArgTypes>
  AdvectionLocalizableDefault(const AnalyticalFluxType& analytical_flux,
                              const BoundaryValueType& boundary_values,
                              const SourceType& source,
                              RangeType& range,
                              const XT::Common::Parameter& param,
                              const QuadratureRuleType& quadrature_rule,
                              LocalOperatorArgTypes&&... local_operator_args)
    : BaseType(range.space().grid_layer(), source, range)
    , local_operator_(
          quadrature_rule, analytical_flux, param, std::forward<LocalOperatorArgTypes>(local_operator_args)...)
    , local_boundary_operator_(quadrature_rule,
                               analytical_flux,
                               boundary_values,
                               param,
                               std::forward<LocalOperatorArgTypes>(local_operator_args)...)
  {
    this->append(
        local_operator_,
        new XT::Grid::ApplyOn::PartitionSetInnerIntersectionsPrimally<GridLayerType, Dune::Partitions::Interior>());
    this->append(local_operator_, new XT::Grid::ApplyOn::PeriodicIntersectionsPrimally<GridLayerType>());
    this->append(local_boundary_operator_, new XT::Grid::ApplyOn::NonPeriodicBoundaryIntersections<GridLayerType>());
  }

private:
  const LocalCouplingOperatorType local_operator_;
  const LocalBoundaryOperatorType local_boundary_operator_;
}; // class AdvectionLocalizableDefault


template <class Traits>
class AdvectionOperatorBase
{
public:
  typedef typename Traits::AnalyticalFluxType AnalyticalFluxType;
  typedef typename Traits::BoundaryValueType BoundaryValueType;
  typedef typename Traits::DomainFieldType DomainFieldType;
  typedef typename Traits::DomainType DomainType;
  typedef typename Traits::RangeFieldType RangeFieldType;
  static const size_t dimDomain = Traits::dimDomain;
  static const size_t dimRange = Traits::dimRange;
  static const size_t dimRangeCols = Traits::dimRangeCols;
  typedef typename Traits::NumericalCouplingFluxType NumericalCouplingFluxType;
  typedef typename Traits::NumericalBoundaryFluxType NumericalBoundaryFluxType;
  typedef Dune::QuadratureRule<DomainFieldType, 1> Quadrature1dType;
  typedef Dune::QuadratureRule<DomainFieldType, dimDomain - 1> IntersectionQuadratureType;

public:
  AdvectionOperatorBase(const AnalyticalFluxType& analytical_flux,
                        const BoundaryValueType& boundary_values,
                        const IntersectionQuadratureType& intersection_quadrature = midpoint_quadrature())
    : analytical_flux_(analytical_flux)
    , boundary_values_(boundary_values)
    , intersection_quadrature_(intersection_quadrature)
  {
  }

  template <class SourceType, class RangeType, class... LocalOperatorArgs>
  void apply(SourceType& source,
             RangeType& range,
             const XT::Common::Parameter& param,
             LocalOperatorArgs&&... local_operator_args) const
  {
    AdvectionLocalizableDefault<AnalyticalFluxType,
                                NumericalCouplingFluxType,
                                NumericalBoundaryFluxType,
                                BoundaryValueType,
                                SourceType,
                                RangeType>
    localizable_operator(analytical_flux_,
                         boundary_values_,
                         source,
                         range,
                         param,
                         intersection_quadrature_,
                         std::forward<LocalOperatorArgs>(local_operator_args)...);
    localizable_operator.apply(true);
  }

  static const Quadrature1dType& default_1d_quadrature()
  {
    return midpoint_quadrature();
  }

  //  const AnalyticalFluxType& analytical_flux() const
  //  {
  //    return analytical_flux_;
  //  }

private:
  static const IntersectionQuadratureType& midpoint_quadrature()
  {
    static auto midpoint_quadrature_ = product_quadrature_helper<>::get(quadrature_helper_1d<>::get());
    return midpoint_quadrature_;
  }

  template <size_t reconstructionOrder = 0, class anything = void>
  struct quadrature_helper_1d
  {
    static Quadrature1dType get()
    {
      return Dune::QuadratureRules<DomainFieldType, 1>::rule(Dune::GeometryType(Dune::GeometryType::BasicType::cube, 1),
                                                             2 * reconstructionOrder);
    }
  };

  template <class anything>
  struct quadrature_helper_1d<1, anything>
  {
    static Quadrature1dType get()
    {
      Quadrature1dType quadrature;
      quadrature.push_back(Dune::QuadraturePoint<DomainFieldType, 1>(0.5, 1.));
      //      quadrature.push_back(Dune::QuadraturePoint<DomainFieldType, 1>(0.5 * (1. - 1. / std::sqrt(3)), 0.5));
      //      quadrature.push_back(Dune::QuadraturePoint<DomainFieldType, 1>(0.5 * (1. + 1. / std::sqrt(3)), 0.5));
      return quadrature;
    }
  };

  template <size_t domainDim = dimDomain, class anything = void>
  struct product_quadrature_helper;

  template <class anything>
  struct product_quadrature_helper<1, anything>
  {
    static Dune::QuadratureRule<DomainFieldType, dimDomain - 1> get(const Quadrature1dType& /*quadrature_1d*/)
    {
      Dune::QuadratureRule<DomainFieldType, dimDomain - 1> ret;
      ret.push_back(Dune::QuadraturePoint<DomainFieldType, 0>({0}, 1));
      return ret;
    }
  };

  template <class anything>
  struct product_quadrature_helper<2, anything>
  {
    static Dune::QuadratureRule<DomainFieldType, dimDomain - 1> get(const Quadrature1dType& quadrature_1d)
    {
      return quadrature_1d;
    }
  };

  template <class anything>
  struct product_quadrature_helper<3, anything>
  {
    static Dune::QuadratureRule<DomainFieldType, dimDomain - 1> get(const Quadrature1dType& quadrature_1d)
    {
      Dune::QuadratureRule<DomainFieldType, dimDomain - 1> ret;
      for (size_t ii = 0; ii < quadrature_1d.size(); ++ii)
        for (size_t jj = 0; jj < quadrature_1d.size(); ++jj)
          ret.push_back(Dune::QuadraturePoint<DomainFieldType, dimDomain - 1>(
              {quadrature_1d[ii].position()[0], quadrature_1d[jj].position()[0]},
              quadrature_1d[ii].weight() * quadrature_1d[jj].weight()));
      return ret;
    }
  };

  const AnalyticalFluxType& analytical_flux_;
  const BoundaryValueType& boundary_values_;
  const IntersectionQuadratureType& intersection_quadrature_;
}; // class AdvectionOperatorBase<...>


} // namespace GDT
} // namespace Dune

#endif // DUNE_GDT_OPERATORS_FV_BASE_HH
