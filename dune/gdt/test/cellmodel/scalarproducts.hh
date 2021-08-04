// This file is part of the dune-gdt project:
//   https://github.com/dune-community/dune-gdt
// Copyright 2010-2018 dune-gdt developers and contributors. All rights reserved.
// License: Dual licensed as BSD 2-Clause License (http://opensource.org/licenses/BSD-2-Clause)
//      or  GPL-2.0+ (http://opensource.org/licenses/gpl-license)
//          with "runtime exception" (http://www.dune-project.org/license.html)
// Authors:
//   Tobias Leibner (2019)

#ifndef DUNE_GDT_TEST_CELLMODEL_SCALARPRODUCTS_HH
#define DUNE_GDT_TEST_CELLMODEL_SCALARPRODUCTS_HH

#include <dune/istl/scalarproducts.hh>
#include <dune/xt/la/container/vector-view.hh>

namespace Dune {


template <class VectorType, class MatrixType>
class MassMatrixScalarProduct : public ScalarProduct<VectorType>
{
public:
  //! export types
  using domain_type = VectorType;
  using field_type = typename VectorType::field_type;
  using real_type = typename FieldTraits<field_type>::real_type;

  MassMatrixScalarProduct(const MatrixType& M)
    : M_(M)
    , tmp_vec_(M_.rows())
  {}

  /*! \brief Dot product of two vectors. In the complex case, the first argument is conjugated.
     It is assumed that the vectors are consistent on the interior+border
     partition.
   */
  virtual field_type dot(const VectorType& x, const VectorType& y) const final
  {
    M_.mv(x, tmp_vec_);
    return tmp_vec_.dot(y);
  }

  /*! \brief Norm of a right-hand side vector.
     The vector must be consistent on the interior+border partition
   */
  virtual real_type norm(const VectorType& x) const final
  {
    return std::sqrt(dot(x, x));
  }

  //! Category of the scalar product (see SolverCategory::Category)
  virtual SolverCategory::Category category() const final
  {
    return SolverCategory::sequential;
  }

private:
  const MatrixType& M_;
  mutable VectorType tmp_vec_;
};


template <class VectorType, class MatrixType>
class PfieldScalarProduct : public ScalarProduct<VectorType>
{
public:
  //! export types
  using domain_type = VectorType;
  using field_type = typename VectorType::field_type;
  using real_type = typename FieldTraits<field_type>::real_type;
  using ConstVectorViewType = XT::LA::ConstVectorView<VectorType>;

  PfieldScalarProduct(const MatrixType& M)
    : M_(M)
    , size_phi_(M_.rows())
    , tmp_vec_(size_phi_, 0., 0)
    , tmp_vec2_(size_phi_, 0., 0)
  {}

  virtual field_type dot(const VectorType& x, const VectorType& y) const final
  {
    const ConstVectorViewType y_phi(y, 0, size_phi_);
    const ConstVectorViewType y_phinat(y, size_phi_, 2 * size_phi_);
    const ConstVectorViewType y_mu(y, 2 * size_phi_, 3 * size_phi_);
    auto& x_phi = tmp_vec_;
    for (size_t ii = 0; ii < size_phi_; ++ii)
      x_phi[ii] = x[ii];
    M_.mv(x_phi, tmp_vec2_);
    field_type ret = y_phi.dot(tmp_vec2_);
    auto& x_phinat = tmp_vec_;
    for (size_t ii = 0; ii < size_phi_; ++ii)
      x_phinat[ii] = x[size_phi_ + ii];
    M_.mv(x_phinat, tmp_vec2_);
    ret += y_phinat.dot(tmp_vec2_);
    auto& x_mu = tmp_vec_;
    for (size_t ii = 0; ii < size_phi_; ++ii)
      x_mu[ii] = x[2 * size_phi_ + ii];
    M_.mv(x_mu, tmp_vec2_);
    ret += y_mu.dot(tmp_vec2_);
    return ret;
  }

  /*! \brief Norm of a right-hand side vector.
     The vector must be consistent on the interior+border partition
   */
  virtual real_type norm(const VectorType& x) const final
  {
    return std::sqrt(dot(x, x));
  }

  //! Category of the scalar product (see SolverCategory::Category)
  virtual SolverCategory::Category category() const final
  {
    return SolverCategory::sequential;
  }

private:
  const MatrixType& M_;
  const size_t size_phi_;
  mutable VectorType tmp_vec_;
  mutable VectorType tmp_vec2_;
};


template <class VectorType, class MatrixType>
class OfieldScalarProduct : public ScalarProduct<VectorType>
{
public:
  //! export types
  using domain_type = VectorType;
  using field_type = typename VectorType::field_type;
  using real_type = typename FieldTraits<field_type>::real_type;
  using ConstVectorViewType = XT::LA::ConstVectorView<VectorType>;

  OfieldScalarProduct(const MatrixType& M)
    : M_(M)
    , size_P_(M_.rows())
    , tmp_vec_(size_P_, 0., 0)
    , tmp_vec2_(size_P_, 0., 0)
  {}

  virtual field_type dot(const VectorType& x, const VectorType& y) const final
  {
    const ConstVectorViewType y_P(y, 0, size_P_);
    const ConstVectorViewType y_Pnat(y, size_P_, 2 * size_P_);
    auto& x_P = tmp_vec_;
    for (size_t ii = 0; ii < size_P_; ++ii)
      x_P[ii] = x[ii];
    M_.mv(x_P, tmp_vec2_);
    field_type ret = y_P.dot(tmp_vec2_);
    auto& x_Pnat = tmp_vec_;
    for (size_t ii = 0; ii < size_P_; ++ii)
      x_Pnat[ii] = x[size_P_ + ii];
    M_.mv(x_Pnat, tmp_vec2_);
    ret += y_Pnat.dot(tmp_vec2_);
    return ret;
  }

  /*! \brief Norm of a right-hand side vector.
     The vector must be consistent on the interior+border partition
   */
  virtual real_type norm(const VectorType& x) const final
  {
    return std::sqrt(dot(x, x));
  }

  //! Category of the scalar product (see SolverCategory::Category)
  virtual SolverCategory::Category category() const final
  {
    return SolverCategory::sequential;
  }

private:
  const MatrixType& M_;
  const size_t size_P_;
  mutable VectorType tmp_vec_;
  mutable VectorType tmp_vec2_;
};


} // namespace Dune

#endif // DUNE_GDT_TEST_CELLMODEL_SCALARPRODUCTS_HH
