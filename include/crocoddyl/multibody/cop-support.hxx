///////////////////////////////////////////////////////////////////////////////
// BSD 3-Clause License
//
// Copyright (C) 2021, University of Edinburgh
// Copyright note valid unless otherwise stated in individual files.
// All rights reserved.
///////////////////////////////////////////////////////////////////////////////

#include <math.h>
#include <iostream>

namespace crocoddyl {

template <typename Scalar>
CoPSupportTpl<Scalar>::CoPSupportTpl()
    : R_(Matrix3s::Identity()),
      nsurf_(Vector3s::UnitZ()),
      box_(std::numeric_limits<Scalar>::max(), std::numeric_limits<Scalar>::max()) {
  A_.setZero();
  ub_.setZero();
  lb_.setZero();

  // Update the inequality matrix and bounds
  update();
}

template <typename Scalar>
CoPSupportTpl<Scalar>::CoPSupportTpl(const Matrix3s& R, const Vector2s& box)
    : R_(R), nsurf_(R_.transpose() * Vector3s::UnitZ()), box_(box) {
  A_.setZero();
  ub_.setZero();
  lb_.setZero();

  // Update the inequality matrix and bounds
  update();
}

template <typename Scalar>
CoPSupportTpl<Scalar>::CoPSupportTpl(const Vector3s& nsurf, const Vector2s& box)
    : R_(Quaternions::FromTwoVectors(nsurf, Vector3s::UnitZ()).toRotationMatrix()), nsurf_(nsurf), box_(box) {
  A_.setZero();
  ub_.setZero();
  lb_.setZero();

  // Update the inequality matrix and bounds
  update();
}

template <typename Scalar>
CoPSupportTpl<Scalar>::CoPSupportTpl(const WrenchConeTpl<Scalar>& support)
    : A_(support.get_A()),
      ub_(support.get_ub()),
      lb_(support.get_lb()),
      R_(support.get_R()),
      nsurf_(support.get_nsurf()),
      box_(support.get_box()) {}

template <typename Scalar>
CoPSupportTpl<Scalar>::~CoPSupportTpl() {}

template <typename Scalar>
void CoPSupportTpl<Scalar>::update() {
  // Initialize the matrix and bounds
  A_.setZero();
  ub_.setZero();
  lb_.setOnes();
  lb_ *= -std::numeric_limits<Scalar>::max();

  // CoP information
  // This matrix is defined as
  // [0 0 -W  1  0;
  //  0 0 -W -1  0;
  //  0 0 -L  0  1;
  //  0 0 -L  0 -1]
  const Scalar L = box_(0) / Scalar(2.);
  const Scalar W = box_(1) / Scalar(2.);
  A_.row(0) << -W * R_.col(2).transpose(), R_.col(0).transpose();
  A_.row(1) << -W * R_.col(2).transpose(), -R_.col(0).transpose();
  A_.row(2) << -L * R_.col(2).transpose(), R_.col(1).transpose();
  A_.row(3) << -L * R_.col(2).transpose(), -R_.col(1).transpose();
}

template <typename Scalar>
const typename MathBaseTpl<Scalar>::Matrix46s& CoPSupportTpl<Scalar>::get_A() const {
  return A_;
}

template <typename Scalar>
const typename MathBaseTpl<Scalar>::Vector4s& CoPSupportTpl<Scalar>::get_ub() const {
  return ub_;
}

template <typename Scalar>
const typename MathBaseTpl<Scalar>::Vector4s& CoPSupportTpl<Scalar>::get_lb() const {
  return lb_;
}

template <typename Scalar>
const typename MathBaseTpl<Scalar>::Vector2s& CoPSupportTpl<Scalar>::get_box() const {
  return box_;
}

template <typename Scalar>
const typename MathBaseTpl<Scalar>::Matrix3s& CoPSupportTpl<Scalar>::get_R() const {
  return R_;
}

template <typename Scalar>
const typename MathBaseTpl<Scalar>::Vector3s& CoPSupportTpl<Scalar>::get_nsurf() const {
  return nsurf_;
}

template <typename Scalar>
void CoPSupportTpl<Scalar>::set_R(const Matrix3s& R) {
  R_ = R;
  nsurf_ = R_.transpose() * Vector3s::UnitZ();
}

template <typename Scalar>
void CoPSupportTpl<Scalar>::set_nsurf(const Vector3s& nsurf) {
  nsurf_ = nsurf;
  // Sanity checks
  if (!nsurf.isUnitary()) {
    nsurf_ /= nsurf.norm();
    std::cerr << "Warning: normal is not an unitary vector, then we normalized it" << std::endl;
  }
  R_ = Quaternions::FromTwoVectors(nsurf_, Vector3s::UnitZ()).toRotationMatrix();
}

template <typename Scalar>
void CoPSupportTpl<Scalar>::set_box(const Vector2s& box) {
  box_ = box;
  if (box_(0) < Scalar(0.)) {
    box_(0) = std::numeric_limits<Scalar>::max();
    std::cerr << "Warning: box(0) has to be a positive value, set to max. float" << std::endl;
  }
  if (box_(1) < Scalar(0.)) {
    box_(1) = std::numeric_limits<Scalar>::max();
    std::cerr << "Warning: box(0) has to be a positive value, set to max. float" << std::endl;
  }
}

template <typename Scalar>
std::ostream& operator<<(std::ostream& os, const CoPSupportTpl<Scalar>& X) {
  os << "         R: " << X.get_R() << std::endl;
  os << "   (nsurf): " << X.get_nsurf().transpose() << std::endl;
  os << "       box: " << X.get_box().transpose() << std::endl;
  return os;
}

}  // namespace crocoddyl
