// This file is part of the dune-gdt project:
//   https://github.com/dune-community/dune-gdt
// Copyright 2010-2018 dune-gdt developers and contributors. All rights reserved.
// License: Dual licensed as BSD 2-Clause License (http://opensource.org/licenses/BSD-2-Clause)
//      or  GPL-2.0+ (http://opensource.org/licenses/gpl-license)
//          with "runtime exception" (http://www.dune-project.org/license.html)
// Authors:
//   Felix Schindler (2018)
//   René Fritze     (2017 - 2018)
//   Tobias Leibner  (2018)

#ifndef DUNE_GDT_SPACES_PARALLEL_DD_HANDLES_HH
#define DUNE_GDT_SPACES_PARALLEL_DD_HANDLES_HH


#include <dune/gdt/type_traits.hh>
#include <dune/xt/common/fixed_map.hh>
#include <dune/gdt/spaces/mapper/interfaces.hh>

namespace Dune {
namespace GDT {
namespace DD {

static const int magic_number = 666;

template <class VectorType, class ScalarType, class DdContainer, class Descriptor>
class LocalView
{
  static_assert(XT::LA::is_vector<VectorType>::value, "");
  using MacroElement = typename DdContainer::MacroEntityType;

private:
  void resize(size_t size)
  {
    global_indices_.resize(size);
    value_cache_.resize(size);
  }

public:
  using value_type = ScalarType;

  LocalView(VectorType& vector, const DdContainer& dd_container, const Descriptor& descriptor)
    : vector_(vector)
    , dd_container_(dd_container)
    , global_indices_(0)
    , value_cache_(0)
    , descriptor_(descriptor)
  {}

  void bind(const MacroElement& entity)
  {
    const size_t size{descriptor_.size(dd_container_, entity)};
    resize(size);
    const auto& local_space = dd_container_.local_space(entity);
    local_space.mapper().global_indices(entity, global_indices_);
    for (auto i : XT::Common::value_range(size)) {
      assert(i < global_indices_.size());
      const auto global = global_indices_[i];
      assert(global < vector_.size());
      value_cache_[i] = vector_[global];
    }
  }

  //! this needs to exist for communication to allow gather/scatter to define for codim non-zero. even if never called
  template <class OtherEntities>
  void bind(const OtherEntities&)
  {
    DUNE_THROW(NotImplemented, "");
    resize(0);
  }

  void commit()
  {
    assert(value_cache_.size() <= global_indices_.size());
    for (auto i : XT::Common::value_range(value_cache_.size())) {
      const auto global = global_indices_[i];
      vector_[global] = value_cache_[i];
    }
  }

  ScalarType& operator[](const size_t ii)
  {
    return value_cache_[ii];
  }

  const ScalarType& operator[](const size_t ii) const
  {
    return value_cache_[ii];
  }

  size_t size() const
  {
    assert(value_cache_.size() <= global_indices_.size());
    return value_cache_.size();
  }

private:
  VectorType& vector_;
  const DdContainer& dd_container_;
  Dune::DynamicVector<size_t> global_indices_;
  //! must use something that doesn't have specialized data storage for bool
  boost::container::vector<ScalarType> value_cache_;
  const Descriptor& descriptor_;
};

//! We only ever communicate data on Elements, no matter what space
template <typename DDGRID>
static constexpr bool dd_grid_associates_data_with(const DDGRID& /*space*/, int codim)
{
  return codim == 0;
}


//! Communication descriptor for sending one item of type E per DOF.
//! used in Mindatahandle
template <typename E>
struct DofDataCommunicationDescriptor
{
private:
  template <class GV, class MacroEntity>
  static size_t
  local_size(const MapperInterface<GV>& local_mapper, const MacroEntity& e, std::integral_constant<int, 0>)
  {
    //
    return local_mapper.size();
  }

  template <class GV, class MacroEntity, int codim>
  static std::enable_if_t<codim != 0, size_t>
  local_size(const MapperInterface<GV>& /*mapper*/, const MacroEntity& /*e*/, std::integral_constant<int, codim>)
  {
    return 0;
  }

  template <class GV, class MacroEntity>
  static size_t local_size(const MapperInterface<GV>& mapper, const MacroEntity& e)
  {
    return local_size(mapper, e, std::integral_constant<int, MacroEntity::codimension>());
  }

public:
  typedef E DataType;

  // Wrap the grid's communication buffer to enable sending leaf ordering sizes along with the data
  static const bool wrap_buffer = true;

  // export original data type to fix up size information forwarded to standard gather / scatter functors
  typedef E OriginalDataType;

  template <class DdContainer>
  bool contains(const DdContainer& dd_container, int /*dim*/, int codim) const
  {
    return dd_grid_associates_data_with(dd_container, codim);
  }

  template <class DdContainer>
  bool fixedSize(const DdContainer& /*space*/, int /*dim*/, int /*codim*/) const
  {
    // we currently have no adaptive spaces
    return true;
  }

  template <class DdContainer, typename Entity>
  std::size_t size(const DdContainer& dd_container, const Entity& e) const
  {
    //    bool contains = space.grid_view().indexSet().contains(e);
    bool contains = true;
    const auto& local_space = *dd_container.local_spaces_[e];
    return contains ? local_size(local_space.mapper(), e) : 0u;
  }
};

//! Communication descriptor for sending count items of type E per entity with attached DOFs.
//! used in  SharedDOFDataHandle, DisjointPartitioningDataHandle, ghostdata
template <typename E>
struct EntityDataCommunicationDescriptor
{
  typedef E DataType;
  // Data is per entity, so we don't need to send leaf ordering size and thus can avoid wrapping the
  // grid's communication buffer
  static const bool wrap_buffer = false;

  template <class DdContainer>
  bool contains(const DdContainer& dd_container, int /*dim*/, int codim) const
  {
    return dd_grid_associates_data_with(dd_container, codim);
  }

  template <class DdContainer>
  bool fixedSize(const DdContainer& /*space*/, int /*dim*/, int /*codim*/) const
  {
    return true;
  }

  template <class DdContainer, typename Entity>
  std::size_t size(const DdContainer& dd_container, const Entity& e) const
  {
    // TODO hardcoded true
    //      bool contains = space.grid_view().indexSet().contains(e);
    bool contains = true;
    return (dd_grid_associates_data_with(dd_container, Entity::codimension) && contains) ? count_ : 0;
  }

  //! remove default value, force handles to use actual space provided values
  explicit EntityDataCommunicationDescriptor(std::size_t count)
    : count_(count)
  {}

private:
  const std::size_t count_;
};


template <class DdContainer,
          typename VectorType,
          typename GatherScatter,
          typename CommunicationDescriptor = DofDataCommunicationDescriptor<typename VectorType::ScalarType>>
class SpaceDataHandle
  : public Dune::CommDataHandleIF<SpaceDataHandle<DdContainer, VectorType, GatherScatter, CommunicationDescriptor>,
                                  typename CommunicationDescriptor::DataType>
{

public:
  using DataType = typename CommunicationDescriptor::DataType;
  using Codim0EntityType = typename DdContainer::MacroEntityType;

  SpaceDataHandle(const DdContainer& dd_container,
                  VectorType& v,
                  CommunicationDescriptor communication_descriptor,
                  GatherScatter gather_scatter = GatherScatter())
    : dd_container_(dd_container)
    , gather_scatter_(gather_scatter)
    , communication_descriptor_(communication_descriptor)
    , local_view_(v, dd_container, communication_descriptor_)
  {}

  //! returns true if data for this codim should be communicated
  bool contains(int dim, int codim) const
  {
    return communication_descriptor_.contains(dd_container_, dim, codim);
  }

  //!  \brief returns true if size per entity of given dim and codim is a constant
  bool fixedsize(int dim, int codim) const
  {
    return communication_descriptor_.fixedSize(dd_container_, dim, codim);
  }

  /*!  \brief how many objects of type DataType have to be sent for a given entity

    Note: Only the sender side needs to know this size.
  */
  template <class EntityType>
  size_t size(const EntityType& entity) const
  {
    return communication_descriptor_.size(dd_container_, entity);
  }

  /** pack data from user to message buffer
   *  the parent type prescribes a CONST gather method,
   *    because obviously receiving data will never change our local state
   *  this needs to be defined for all codims, but only codims for which
   *  contains is true will ever be called at runtime
   */
  template <typename MessageBuffer, class EntityType>
  void gather(MessageBuffer& buffer, const EntityType& entity) const
  {
    local_view_.bind(entity);
    if (gather_scatter_.gather(buffer, entity, local_view_))
      local_view_.commit();
  }

  /** unpack data from message buffer to user
    n is the number of objects sent by the sender
   *  this needs to be defined for all codims, but only codims for which
   *  contains is true will ever be called at runtime
  */
  template <typename MessageBuffer, class EntityType>
  void scatter(MessageBuffer& buff, const EntityType& e, size_t n)
  {
    local_view_.bind(e);
    if (gather_scatter_.scatter(buff, n, e, local_view_))
      local_view_.commit();
  }


private:
  const DdContainer& dd_container_;
  //! has to be mutable because gather is const
  mutable GatherScatter gather_scatter_;
  CommunicationDescriptor communication_descriptor_;
  mutable DD::LocalView<VectorType, typename VectorType::ScalarType, DdContainer, CommunicationDescriptor> local_view_;
};


template <typename GatherScatter>
class DataGatherScatter
{

public:
  typedef std::size_t size_t;

  template <typename MessageBuffer, typename Entity, typename LocalView>
  bool gather(MessageBuffer& buff, const Entity& /*e*/, const LocalView& local_view) const
  {
    for (std::size_t i = 0; i < local_view.size(); ++i)
      _gather_scatter.gather(buff, local_view[i]);
    return false;
  }

  // default scatter - requires function space structure to be identical on sender and receiver side
  template <typename MessageBuffer, typename Entity, typename LocalView>
  bool scatter(MessageBuffer& buff, size_t n, const Entity& e, LocalView& local_view) const
  {
    if (partitions_.contains(e.partitionType())) {
      if (local_view.size() != n)
        DUNE_THROW(Exception,
                   "size mismatch in GridFunctionSpace data handle, have " << local_view.size() << "DOFs, but received "
                                                                           << n);

      for (std::size_t i = 0; i < local_view.size(); ++i)
        _gather_scatter.scatter(buff, local_view[i]);
      return true;
    } else {
      if (local_view.size() != 0)
        DUNE_THROW(Exception,
                   "expected no DOFs in partition '" << e.partitionType() << "', but have " << local_view.size());

      for (std::size_t i = 0; i < local_view.size(); ++i) {
        typename LocalView::value_type dummy;
        buff.read(dummy);
      }
      return false;
    }
  }

  DataGatherScatter(GatherScatter gather_scatter = GatherScatter())
    : _gather_scatter(gather_scatter)
  {}

private:
  GatherScatter _gather_scatter;
  // this is currently the default since we have no non-overlapping parallel code
  static constexpr const Partitions::All partitions_{};
};


template <typename GatherScatter>
class DataEntityGatherScatter
{

public:
  typedef std::size_t size_t;

  template <typename MessageBuffer, typename Entity, typename LocalView>
  bool gather(MessageBuffer& buff, const Entity& e, const LocalView& local_view) const
  {
    for (std::size_t i = 0; i < local_view.size(); ++i)
      gather_scatter_.gather(buff, e, local_view[i]);
    return false;
  }

  // see documentation in DataGatherScatter for further info on the scatter() implementations
  template <typename MessageBuffer, typename Entity, typename LocalView>
  bool scatter(MessageBuffer& buff, size_t n, const Entity& e, LocalView& local_view) const
  {
    if (local_view.cache().gridFunctionSpace().entitySet().partitions().contains(e.partitionType())) {
      if (local_view.size() != n)
        DUNE_THROW(Exception,
                   "size mismatch in GridFunctionSpace data handle, have " << local_view.size() << "DOFs, but received "
                                                                           << n);

      for (std::size_t i = 0; i < local_view.size(); ++i)
        gather_scatter_.scatter(buff, e, local_view[i]);
      return true;
    } else {
      if (local_view.size() != 0)
        DUNE_THROW(Exception,
                   "expected no DOFs in partition '" << e.partitionType() << "', but have " << local_view.size());

      for (std::size_t i = 0; i < local_view.size(); ++i) {
        typename LocalView::ElementType dummy;
        buff.read(dummy);
      }
      return false;
    }
  }

  DataEntityGatherScatter(GatherScatter gather_scatter = GatherScatter())
    : gather_scatter_(gather_scatter)
  {}

private:
  GatherScatter gather_scatter_;
};

class MinGatherScatter
{
public:
  template <class MessageBuffer, class DataType>
  void gather(MessageBuffer& buff, DataType& data) const
  {
    buff.write(data);
  }

  template <class MessageBuffer, class DataType>
  void scatter(MessageBuffer& buff, DataType& data) const
  {
    DataType x;
    buff.read(x);
    data = std::min(data, x);
  }
};

template <class DdContainer, class VectorType>
class MinDataHandle
  : public SpaceDataHandle<DdContainer,
                           VectorType,
                           DataGatherScatter<MinGatherScatter>,
                           DofDataCommunicationDescriptor<typename VectorType::ScalarType>>
{
  typedef SpaceDataHandle<DdContainer,
                          VectorType,
                          DataGatherScatter<MinGatherScatter>,
                          DofDataCommunicationDescriptor<typename VectorType::ScalarType>>
      BaseType;

public:
  MinDataHandle(const DdContainer& sp, VectorType& v_)
    : BaseType(sp, v_, DofDataCommunicationDescriptor<typename VectorType::ScalarType>())
  {}
};

//! GatherScatter functor for marking ghost DOFs.
/**
 * This data handle will mark all ghost DOFs (more precisely, all DOFs associated
 * with entities not part of either the interior or the border partition).
 *
 * \note In order to work correctly, the data handle must be communicated on the
 * Dune::InteriorBorder_All_Interface.
 */
class GhostGatherScatter
{
public:
  template <typename MessageBuffer, typename Entity, typename LocalView>
  bool gather(MessageBuffer& buff, const Entity& e, LocalView& /*local_view*/) const
  {
    // Figure out where we are...
    const bool ghost = e.partitionType() != Dune::InteriorEntity && e.partitionType() != Dune::BorderEntity;

    // ... and send something (doesn't really matter what, we'll throw it away on the receiving side).
    buff.write(ghost);

    return false;
  }

  template <typename MessageBuffer, typename Entity, typename LocalView>
  bool scatter(MessageBuffer& buff, std::size_t /*n*/, const Entity& e, LocalView& local_view) const
  {
    // Figure out where we are - we have to do this again on the receiving side due to the asymmetric
    // communication interface!
    const bool ghost = e.partitionType() != Dune::InteriorEntity && e.partitionType() != Dune::BorderEntity;

    // drain buffer
    bool dummy;
    buff.read(dummy);

    for (std::size_t i = 0; i < local_view.size(); ++i)
      local_view[i] = ghost;

    return true;
  }
};

//! Data handle for marking ghost DOFs.
/**
 * This data handle will mark all ghost DOFs (more precisely, all DOFs associated
 * with entities not part of either the interior or the border partition).
 *
 * \note In order to work correctly, the data handle must be communicated on the
 * Dune::InteriorBorder_All_Interface.
 */
template <class DdContainer, class VectorType>
class GhostDataHandle
  : public SpaceDataHandle<DdContainer, VectorType, GhostGatherScatter, EntityDataCommunicationDescriptor<bool>>
{
  typedef SpaceDataHandle<DdContainer, VectorType, GhostGatherScatter, EntityDataCommunicationDescriptor<bool>>
      BaseType;

  //  static_assert((std::is_same<typename VectorType::ScalarType, bool>::value),
  //                "GhostDataHandle expects a vector of bool values");

public:
  //! Creates a new GhostDataHandle.
  /**
   * Creates a new GhostDataHandle and by default initializes the result vector
   * with the correct value of false. If you have already done that externally,
   * you can skip the initialization.
   *
   * \param space_         The GridFunctionSpace to operate on.
   * \param v_           The result vector.
   * \param init_vector  Flag to control whether the result vector will be initialized.
   */
  GhostDataHandle(const DdContainer& sp, VectorType& v_, bool init_vector = true)
    // magic_number = sp.mapper().max_local_size()
    : BaseType(sp, v_, EntityDataCommunicationDescriptor<bool>(magic_number))
  {
    if (init_vector)
      v_.set_all(false);
  }
};


//! GatherScatter functor for creating a disjoint DOF partitioning.
/**
 * This functor will associate each DOF with a unique rank, creating a nonoverlapping partitioning
 * of the unknowns. The rank for a DOF is chosen by finding the lowest rank on which the associated
 * grid entity belongs to either the interior or the border partition.
 *
 * \note In order to work correctly, the data handle must be communicated on the
 * Dune::InteriorBorder_All_Interface and the result vector must be initialized with the MPI rank value.
 */
template <typename RankIndex>
class DisjointPartitioningGatherScatter
{

public:
  template <typename MessageBuffer, typename Entity, typename LocalView>
  bool gather(MessageBuffer& buff, const Entity& /*e*/, LocalView& /*local_view*/) const
  {
    // We only gather from interior and border entities, so we can throw in our ownership
    // claim without any further checks.
    buff.write(rank_);

    return true;
  }

  template <typename MessageBuffer, typename Entity, typename LocalView>
  bool scatter(MessageBuffer& buff, std::size_t /*n*/, const Entity& e, LocalView& local_view) const
  {
    // Value used for DOFs with currently unknown rank.
    const RankIndex unknown_rank = std::numeric_limits<RankIndex>::max();

    // We can only own this DOF if it is either on the interior or border partition.
    const bool is_interior_or_border =
        (e.partitionType() == Dune::InteriorEntity || e.partitionType() == Dune::BorderEntity);

    // Receive data.
    RankIndex received_rank;
    buff.read(received_rank);

    for (std::size_t i = 0; i < local_view.size(); ++i) {
      // Get the currently stored owner rank for this DOF.
      RankIndex current_rank = local_view[i];

      // We only gather from interior and border entities, so we need to make sure
      // we relinquish any ownership claims on overlap and ghost entities on the
      // receiving side. We also need to make sure not to overwrite any data already
      // received, so we only blank the rank value if the currently stored value is
      // equal to our own rank.
      if (!is_interior_or_border && current_rank == rank_)
        current_rank = unknown_rank;

      // Assign DOFs to minimum rank value.
      local_view[i] = std::min(current_rank, received_rank);
    }
    return true;
  }

  //! Create a DisjointPartitioningGatherScatter object.
  /**
   * \param rank  The MPI rank of the current process.
   */
  DisjointPartitioningGatherScatter(RankIndex rank)
    : rank_(rank)
  {}

private:
  const RankIndex rank_;
};

//! GatherScatter data handle for creating a disjoint DOF partitioning.
/**
 * This data handle will associate each DOF with a unique rank, creating a nonoverlapping partitioning
 * of the unknowns. The rank for a DOF is chosen by finding the lowest rank on which the associated
 * grid entity belongs to either the interior or the border partition.
 *
 * \note In order to work correctly, the data handle must be communicated on the
 * Dune::InteriorBorder_All_Interface and the result vector must be initialized with the MPI rank value.
 */
template <class DdContainer, class VectorType>
class DisjointPartitioningDataHandle
  : public SpaceDataHandle<DdContainer,
                           VectorType,
                           DisjointPartitioningGatherScatter<typename VectorType::ScalarType>,
                           EntityDataCommunicationDescriptor<typename VectorType::ScalarType>>
{
  typedef SpaceDataHandle<DdContainer,
                          VectorType,
                          DisjointPartitioningGatherScatter<typename VectorType::ScalarType>,
                          EntityDataCommunicationDescriptor<typename VectorType::ScalarType>>
      BaseType;

public:
  //! Creates a new DisjointPartitioningDataHandle.
  /**
   * Creates a new DisjointPartitioningDataHandle and by default initializes the
   * result vector with the current MPI rank. If you have already done that
   * externally, you can skip the initialization.
   *
   * \param space         The GridFunctionSpace to operate on.
   * \param v           The result vector.
   * \param init_vector  Flag to control whether the result vector will be initialized.
   */
  DisjointPartitioningDataHandle(const DdContainer& dd_container, VectorType& v, bool init_vector = true)
    : BaseType(dd_container,
               v,
               // TODO magic_number = space.mapper().max_local_size()
               EntityDataCommunicationDescriptor<typename VectorType::ScalarType>(magic_number),
               DisjointPartitioningGatherScatter<typename VectorType::ScalarType>(
                   dd_container.dd_grid_.global_comm().rank()))
  {
    if (init_vector)
      v.set_all(dd_container.dd_grid_.global_comm().rank());
  }
};


//! GatherScatter functor for marking shared DOFs.
/**
 * This functor will mark all DOFs that exist on multiple processes.
 *
 * \note In order to work correctly, the data handle must be communicated on the
 * Dune::All_All_Interface and the result vector must be initialized with false.
 */
struct SharedDOFGatherScatter
{

  template <typename MessageBuffer, typename Entity, typename LocalView>
  bool gather(MessageBuffer& buff, const Entity& /*e*/, LocalView& local_view) const
  {
    buff.write(local_view.size() > 0);
    return false;
  }

  template <typename MessageBuffer, typename Entity, typename LocalView>
  bool scatter(MessageBuffer& buff, std::size_t /*n*/, const Entity& /*e*/, LocalView& local_view) const
  {
    bool remote_entity_has_dofs;
    buff.read(remote_entity_has_dofs);

    for (std::size_t i = 0; i < local_view.size(); ++i) {
      local_view[i] |= remote_entity_has_dofs;
    }
    return true;
  }
};


//! Data handle for marking shared DOFs.
/**
 * This data handle will mark all DOFs that exist on multiple processes.
 *
 * \note In order to work correctly, the data handle must be communicated on the
 * Dune::All_All_Interface and the result vector must be initialized with false.
 */
template <class DdContainer, class VectorType>
class SharedDOFDataHandle
  : public SpaceDataHandle<DdContainer, VectorType, SharedDOFGatherScatter, EntityDataCommunicationDescriptor<bool>>
{
  typedef SpaceDataHandle<DdContainer, VectorType, SharedDOFGatherScatter, EntityDataCommunicationDescriptor<bool>>
      BaseType;

  //  static_assert((std::is_same<typename VectorType::ScalarType, bool>::value),
  //                "SharedDOFDataHandle expects a vector of bool values");

public:
  //! Creates a new SharedDOFDataHandle.
  /**
   * Creates a new SharedDOFDataHandle and by default initializes the result vector
   * with the correct value of false. If you have already done that externally,
   * you can skip the initialization.
   *
   * \param space         The GridFunctionSpace to operate on.
   * \param v           The result vector.
   * \param init_vector  Flag to control whether the result vector will be initialized.
   */
  // TODO magic_number = space.mapper().max_local_size()
  SharedDOFDataHandle(const DdContainer& dd_container, VectorType& v, bool init_vector = true)
    : BaseType(dd_container, v, EntityDataCommunicationDescriptor<bool>(magic_number))
  {
    if (init_vector)
      v.set_all(false);
  }
};


//! Data handle for collecting set of neighboring MPI ranks.
/**
 * This data handle collects the MPI ranks of all processes that share grid entities
 * with attached DOFs.
 *
 * \note In order to work correctly, the data handle must be communicated on the
 * Dune::All_All_Interface.
 */
template <class DdContainer, typename RankIndex>
class SpaceNeighborDataHandle
  : public Dune::CommDataHandleIF<SpaceNeighborDataHandle<DdContainer, RankIndex>, RankIndex>
{

  // We deliberately avoid using the SpaceDataHandle here, as we don't want to incur the
  // overhead of invoking the whole SpaceType infrastructure.

public:
  typedef RankIndex DataType;

  SpaceNeighborDataHandle(const DdContainer& dd_container, RankIndex rank, std::set<RankIndex>& neighbors)
    : dd_container_(dd_container)
    , rank_(rank)
    , neighbors_(neighbors)
  {}

  bool contains(int dim, int codim) const
  {
    // TODO hardcoded rangedim
    const int range_dim = 1;
    return dim == range_dim && codim == 0;
  }

  bool fixedsize(int /*dim*/, int /*codim*/) const
  {
    // We always send a single value, the MPI rank.
    return true;
  }

  template <typename Entity>
  size_t size(Entity& /*e*/) const
  {
    return 1;
  }

  template <typename MessageBuffer, typename Entity>
  void gather(MessageBuffer& buff, const Entity& /*e*/) const
  {
    buff.write(rank_);
  }

  template <typename MessageBuffer, typename Entity>
  void scatter(MessageBuffer& buff, const Entity& /*e*/, size_t /*n*/)
  {
    RankIndex rank;
    buff.read(rank);
    neighbors_.insert(rank);
  }

private:
  const DdContainer& dd_container_;
  const RankIndex rank_;
  std::set<RankIndex>& neighbors_;
};

} // namespace DD
} // namespace GDT
} // namespace Dune
#endif // DUNE_GDT_SPACES_PARALLEL_DD_HANDLES_HH
