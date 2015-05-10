/* 
 * The Biomechanical ToolKit
 * Copyright (c) 2009-2015, Arnaud Barré
 * All rights reserved.
 * 
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 
 *     * Redistributions of source code must retain the above
 *       copyright notice, this list of conditions and the following
 *       disclaimer.
 *     * Redistributions in binary form must reproduce the above
 *       copyright notice, this list of conditions and the following
 *       disclaimer in the documentation and/or other materials
 *       provided with the distribution.
 *     * Neither the name(s) of the copyright holders nor the names
 *       of its contributors may be used to endorse or promote products
 *       derived from this software without specific prior written
 *       permission.
 *  
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
 * FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE
 * COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
 * BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
 * CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN
 * ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */

#ifndef __btklardXpr_h
#define __btklardXpr_h

#include "btkConfigure.h" // Must be included before any Eigen header

#include <Eigen/Core>

namespace btk
{
namespace lard
{
  // ----------------------------------------------------------------------- //
  //                           FORWARD DECLARATIONS
  // ----------------------------------------------------------------------- //

  template <typename Derived> class Data;
  template <int Cols> class Array;
  class Motion;
  template <typename Derived> class Map;
  
  template <typename Xpr, int Cols> class BlockOp;
  template <typename Derived, typename Xpr> class UnaryOp;
  template <typename Derived, typename XprOne, typename XprTwo> class BinaryOp;  
  
  template <typename Xpr> class NormOp;
  template <typename Xpr> class MeanOp;
  template <typename XprOne, typename XprTwo> class DifferenceOp;
  template <typename XprOne, typename XprTwo> class SumOp;
  template <typename Xpr> class ScaleOp;
  template <typename Xpr> class NormalizedOp;
  template <typename XprOne, typename XprTwo> class CrossOp;
  template <typename XprOne, typename XprTwo> class TransformOp;
  template <typename U> class ReplicateOp;
  template <typename Xpr> class InverseOp;
  template <typename Xpr> class EulerAnglesOp;

  // ----------------------------------------------------------------------- //
  //                                TRAITS
  // ----------------------------------------------------------------------- //
  
  enum : int
  {
    // No processing done, the data can be copied directly 
    None = 0x0,
    // Only the values were processed. The generation of the residuals as well as the regeneration of the values for negative residuals must be done
    ValuesOnly = 0x01, 
    // The values and residuals were computed. Still, the regeneration of the values for negative residuals must be done
    Full = 0x02
  };
  
  template <typename T> struct Traits {};
  
  template <typename Derived>
  struct Traits<Data<Derived>>
  {
    using Values = typename Traits<Derived>::Values;
    using Residuals = typename Traits<Derived>::Residuals;
    using Index = typename Traits<Derived>::Index;
    enum : int
    {
      ColsAtCompileTime = Traits<Derived>::ColsAtCompileTime,
      Processing = Traits<Derived>::Processing
    };
  };
  
  template <int Cols>
  struct Traits<Array<Cols>>
  {
    using Values = Eigen::Array<double,Eigen::Dynamic,Cols>;
    using Residuals = Eigen::Array<double,Eigen::Dynamic,1>;
    using Index = typename Values::Index;
    enum : int
    {
      ColsAtCompileTime = Cols,
      Processing = None
    };
  };
  
  template <>
  struct Traits<Motion> : Traits<Array<12>>
  {};
  
  template <typename Derived>
  struct Traits<Map<Derived>>
  {
    using Values = Eigen::Map<typename Traits<Derived>::Values>;
    using Residuals = Eigen::Map<typename Traits<Derived>::Residuals>;
    using Index = typename Values::Index;
    using Scalar = typename Values::Scalar;
    enum : int
    {
      ColsAtCompileTime = Traits<Derived>::ColsAtCompileTime,
      Processing = None
    };
  };
  
  template <typename Derived>
  struct Traits<Map<const Derived>>
  {
    using Values = Eigen::Map<const typename Traits<Derived>::Values>;
    using Residuals = Eigen::Map<const typename Traits<Derived>::Residuals>;
    using Index = typename Values::Index;
    using Scalar = const typename Values::Scalar;
    enum : int
    {
      ColsAtCompileTime = Traits<Derived>::ColsAtCompileTime,
      Processing = None
    };
  };
  
  template <typename Xpr, int Cols>
  struct Traits<BlockOp<Xpr,Cols>>
  {
    using Values = typename Traits<Array<Cols>>::Values;
    using Residuals = typename Traits<Array<Cols>>::Residuals;
    using Index = typename Values::Index;
    enum : int
    {
      ColsAtCompileTime = Cols,
      Processing = Traits<typename std::remove_const<Xpr>::type>::Processing
    };
  };
 
  template <typename Derived, typename Xpr>
  struct Traits<UnaryOp<Derived,Xpr>>
  {
    using Values = typename Traits<Xpr>::Values;
    using Residuals = typename Traits<Xpr>::Residuals;
    using Index = typename Values::Index;
    enum : int
    {
      ColsAtCompileTime = Traits<Xpr>::ColsAtCompileTime,
      Processing = Traits<Derived>::Processing
    };
  };
  
  template <typename Derived, typename XprOne, typename XprTwo>
  struct Traits<BinaryOp<Derived,XprOne,XprTwo>>
  {
    using Values = typename Traits<XprOne>::Values;
    using Residuals = typename Traits<XprOne>::Residuals;
    using Index = typename Values::Index;
    enum : int
    {
      ColsAtCompileTime = Traits<XprTwo>::ColsAtCompileTime, // XprTwo to have the good number of columns after some operation like TransformOp
      Processing = Traits<Derived>::Processing
    };
  };
  
  // NOTE : Traits specialization for each operation is declared before this one
};
};

#include "btklardXpr_returnByValue.h"

namespace btk
{
namespace lard
{ 
  // ----------------------------------------------------------------------- //
  //                                  BASE
  // ----------------------------------------------------------------------- //
  
  template <typename Derived>
  class Base
  {
  public:
    using DerivedType = typename std::remove_const<Derived>::type;
    using Index = typename Traits<DerivedType>::Index;
    enum : int
    {
      ColsAtCompileTime = Traits<DerivedType>::ColsAtCompileTime,
      Processing = Traits<DerivedType>::Processing
    };
    
    operator Derived& () noexcept {return static_cast<Derived&>(*this);};
    operator const Derived& () const noexcept {return static_cast<const Derived&>(*this);};
    operator double () const noexcept {return (static_cast<const Derived&>(*this).derived().residuals().value() >= 0.0) ? static_cast<const Derived&>(*this).derived().values().value() : 0.0;};
    
    // Next methods are defined after the declaration of the operation BlockOp
    template <int Cols> BlockOp<Derived,Cols> block(Index index) noexcept;
    template <int Cols> const BlockOp<const Derived,Cols> block(Index index) const noexcept;
    // Next method is defined after the declaration of the operation MeanOp
    const MeanOp<Derived> mean() const noexcept;
    // Next method is defined after the declaration of the operation NormOp
    const NormOp<Derived> norm() const noexcept;
    // Next method is defined after the declaration of the operation NormalizedOp
    const NormalizedOp<Derived> normalized() const noexcept;
    // Next method is defined after the declaration of the operation CrossOp
    template <typename U> const CrossOp<Derived, U> cross(const Base<U>& other) const noexcept;
    // Next method is defined after the declaration of the operation InverseOp
    const InverseOp<Derived> inverse() const noexcept;
    // Next method is defined after the declaration of the operation ReplicateOp
    const ReplicateOp<Derived> replicate(Index rows) const noexcept;
    // Next method is defined after the declaration of the operation TransformOp
    template <typename U> const TransformOp<Derived,U> transform(const Base<U>& other) const noexcept;    
    // Next method is defined after the declaration of the operation EulerAnglesOp
    const EulerAnglesOp<Derived> eulerAngles(Index a0, Index a1, Index a2) const noexcept;
    
    template <typename U> static inline auto generateResiduals(const U& condition) -> decltype(condition.select(Traits<Array<U::ColsAtCompileTime>>::Residuals::Zero(condition.rows(),1),-1.0)) {return condition.select(Traits<Array<U::ColsAtCompileTime>>::Residuals::Zero(condition.rows(),1),-1.0);};
  };
  
  // ----------------------------------------------------------------------- //
  //                                 DATA
  // ----------------------------------------------------------------------- //
  
  template <typename Derived>
  class Data : public Base<Data<Derived>>
  {
  public:
    using Values = typename Traits<Data<Derived>>::Values;
    using Residuals = typename Traits<Data<Derived>>::Residuals;
    using Index = typename Traits<Data<Derived>>::Index;
    
    Data(const Data& other) = default;
    
    Index rows() const noexcept {return this->m_Values.rows();};
    Index cols() const noexcept {return this->m_Values.cols();};
    
    Values& values() noexcept {return this->m_Values;};
    const Values& values() const noexcept {return this->m_Values;};
    Residuals& residuals() noexcept {return this->m_Residuals;};
    const Residuals& residuals() const noexcept {return this->m_Residuals;};
    
    void setZeroResiduals() noexcept {this->m_Residuals.setZero();};
    
    bool isValid() const noexcept {return (this->m_Values.rows() != 0) && (this->m_Residuals.rows() != 0);};
    bool isOccluded() const noexcept {return (!this->isValid() || (this->m_Residuals < 0.0).all());};
    
    Derived& derived() noexcept {return static_cast<Derived&>(*this);};
    const Derived& derived() const noexcept {return static_cast<const Derived&>(*this);};
    
    BlockOp<Data<Derived>,1> x() noexcept;
    const BlockOp<const Data<Derived>,1> x() const noexcept;
    BlockOp<Data<Derived>,1> y() noexcept;
    const BlockOp<const Data<Derived>,1> y() const noexcept;
    BlockOp<Data<Derived>,1> z() noexcept;
    const BlockOp<const Data<Derived>,1> z() const noexcept;
    
    BlockOp<Data<Derived>,3> u() noexcept;
    const BlockOp<const Data<Derived>,3> u() const noexcept;
    BlockOp<Data<Derived>,3> v() noexcept;
    const BlockOp<const Data<Derived>,3> v() const noexcept;
    BlockOp<Data<Derived>,3> w() noexcept;
    const BlockOp<const Data<Derived>,3> w() const noexcept;
    BlockOp<Data<Derived>,3> o() noexcept;
    const BlockOp<const Data<Derived>,3> o() const noexcept;
    
  protected:
    Data() = delete;
    Data(const Values& values, const Residuals& residuals);
    
    // template <typename U, typename V> static inline typename std::enable_if<std::is_base_of<Eigen::ReturnByValue<V>,V>::value>::type assignValues(U& lhs, const V& rhs) {lhs.resize(rhs.rows(),Eigen::NoChange); rhs.evalTo(lhs);};
    // template <typename U, typename V> static inline typename std::enable_if<!std::is_base_of<Eigen::ReturnByValue<V>,V>::value>::type assignValues(U& lhs, const V& rhs) {lhs.lazyAssign(rhs);};
    
    template <typename U, typename V>
    static inline typename std::enable_if<Traits<V>::Processing == None>::type assign(Data<U>& lhs, const Base<V>& rhs)
    {
      static_assert(Traits<U>::ColsAtCompileTime == Traits<V>::ColsAtCompileTime, "The number of columns is not the same.");
      auto& self = static_cast<U&>(lhs).derived();
      const auto& other = static_cast<const V&>(rhs).derived();
      self.values().lazyAssign(other.values());
      self.residuals().lazyAssign(other.residuals());
    };
    
    template <typename U, typename V>
    static inline typename std::enable_if<Traits<V>::Processing == ValuesOnly>::type assign(Data<U>& lhs, const Base<V>& rhs)
    {
      static_assert(Traits<U>::ColsAtCompileTime == Traits<V>::ColsAtCompileTime, "The number of columns is not the same.");
      auto& self = static_cast<U&>(lhs).derived();
      const auto& other = static_cast<const V&>(rhs).derived();
      self.residuals().lazyAssign(Derived::generateResiduals(other.residuals() >= 0.0));
      self.values().lazyAssign((self.residuals().template replicate<1,Values::ColsAtCompileTime>() >= 0.0).select(other.values(),0.0));
    };
    
    template <typename U, typename V>
    static inline typename std::enable_if<Traits<V>::Processing == Full>::type assign(Data<U>& lhs, const Base<V>& rhs)
    {
      static_assert(Traits<U>::ColsAtCompileTime == Traits<V>::ColsAtCompileTime, "The number of columns is not the same.");
      auto& self = static_cast<U&>(lhs).derived();
      const auto& other = static_cast<const V&>(rhs).derived();
      self.residuals().lazyAssign(other.residuals());
      self.values().lazyAssign((self.residuals().template replicate<1,Values::ColsAtCompileTime>() >= 0.0).select(other.values(),0.0));
    };
    
    Values m_Values;
    Residuals m_Residuals;
  };
  
  // Construct from data
  template <typename Derived>
  inline Data<Derived>::Data(const Values& values, const Residuals& residuals)
  : m_Values(values), m_Residuals(residuals)
  {};
  
  // ----------------------------------------------------------------------- //
  
  template <int Cols>
  class Array : public Data<Array<Cols>>
  {
  public:
    using Values = typename Traits<Array<Cols>>::Values;
    using Residuals = typename Traits<Array<Cols>>::Residuals;
    using Index = typename Traits<Array<Cols>>::Index;
    
    Array();
    Array(Index rows);
    Array(const Values& values);
    Array(const Values& values, const Residuals& residuals);
    template <typename U> Array(const Base<U>& other);
    
    template <typename U> Array& operator= (const Base<U>& other);
  };
  
  // Default ctor
  template <int Cols>
  inline Array<Cols>::Array()
  : Data<Array<Cols>>(Values(),Residuals())
  {};
  
  template <int Cols>
  inline Array<Cols>::Array(Index rows)
  : Data<Array<Cols>>(Values(rows,Cols),Residuals(rows,1))
  {};
  
  template <int Cols>
  inline Array<Cols>::Array(const Values& values)
  : Data<Array<Cols>>(values,Residuals::Zero(values.rows(),1))
  {};

  template <int Cols>
  inline Array<Cols>::Array(const Values& values, const Residuals& residuals)
  : Data<Array<Cols>>(values,residuals)
  {};
  
  // Construct from any Base
  template <int Cols>
  template <typename U>
  inline Array<Cols>::Array(const Base<U>& other)
  : Array()
  {
    // static_assert(Cols == Traits<U>::ColsAtCompileTime, "The number of columns is not the same.");
    Array::assign(*this, other);
    // const U& o = other;
    // this->m_Residuals.lazyAssign(Array::generateResiduals(o.derived().residuals() >= 0.0));
    // this->m_Values.lazyAssign((this->m_Residuals.template replicate<1,Cols>() >= 0.0).select(o.derived().values(),0.0));
  };
  // : Data<Array<Cols>>(static_cast<const U&>(other).derived().values(), static_cast<const U&>(other).derived().residuals())
  // {
  //   static_assert(Cols == Traits<U>::ColsAtCompileTime, "The number of columns is not the same.");
  // };
  
  template <int Cols>
  template <typename U>
  inline Array<Cols>& Array<Cols>::operator= (const Base<U>& other)
  {
    // static_assert(Cols == Traits<U>::ColsAtCompileTime, "The number of columns is not the same.");
    Array::assign(*this, other);
    // const U& o = other;
    // // Array::assignValues(this->m_Values,o.derived().values());
    // this->m_Residuals.lazyAssign(Array::generateResiduals(o.derived().residuals() >= 0.0));
    // this->m_Values.lazyAssign((this->m_Residuals.template replicate<1,Cols>() >= 0.0).select(o.derived().values(),0.0));
    return *this;
  };

  // ----------------------------------------------------------------------- //
  
  template <typename Derived>
  class Map : public Data<Map<Derived>>
  {
  public:
    using Values = typename Traits<Map<Derived>>::Values;
    using Residuals = typename Traits<Map<Derived>>::Residuals;
    using Index = typename Traits<Map<Derived>>::Index;
    using Scalar = typename Traits<Map<Derived>>::Scalar;
    
    Map();
    Map(Index rows, Scalar* values, Scalar* residuals);
    
    Map& operator= (const Map& other) = delete;
    template <typename U> Map& operator= (const Base<U>& other);
  };
  
  // Default ctor
  template <typename Derived>
  inline Map<Derived>::Map()
  : Data<Map<Derived>>(Values(nullptr,0,Values::ColsAtCompileTime),Residuals(nullptr,0,Residuals::ColsAtCompileTime))
  {};
  
  template <typename Derived>
  inline Map<Derived>::Map(Index rows, Scalar* values, Scalar* residuals)
  : Data<Map<Derived>>(Values(values,rows,Derived::Values::ColsAtCompileTime), Residuals(residuals,rows,Derived::Residuals::ColsAtCompileTime))
  {};
  
  template <typename Derived>
  template <typename U>
  inline Map<Derived>& Map<Derived>::operator= (const Base<U>& other)
  {
    // static_assert(Traits<Derived>::ColsAtCompileTime == Traits<U>::ColsAtCompileTime, "The number of columns is not the same.");
    // const U& o = other;
    // Map::assignValues(this->m_Values,o.derived().values());
    // this->m_Residuals.lazyAssign(o.derived().residuals());
    Map::assign(*this, other);
    return *this;
  };
  
  
  // ----------------------------------------------------------------------- //
  //                           CUSTOM TYPES
  // ----------------------------------------------------------------------- //
  
  using Scalar = Array<1>;
  using Vector = Array<3>;
  using Trajectory = Vector;
  
  using MappedScalar = Map<Scalar>;  
  using MappedVector = Map<Vector>;
  using MappedTrajectory = Map<Trajectory>;
  using MappedMotion = Map<Motion>;
  
  class Motion : public Array<12>
  {
  public:
    using Orientations = Array<9>::Values;
    using Positions = Array<3>::Values;
    
    Motion();
    Motion(Index rows);
    template <typename U, typename V, typename W, typename O> Motion(const Base<U>& u, const Base<V>& v, const Base<W>& w, const Base<O>& o);
    template <typename U> Motion(const Base<U>& other);
    
    auto orientations() const noexcept -> decltype(this->m_Values.topLeftCorner(this->rows(),Orientations::ColsAtCompileTime)) {return this->m_Values.topLeftCorner(this->rows(),Orientations::ColsAtCompileTime);};
    auto positions() const noexcept -> decltype(this->m_Values.topRightCorner(this->rows(),Positions::ColsAtCompileTime)) {return this->m_Values.topRightCorner(this->rows(),Positions::ColsAtCompileTime);};
  };
  

  /**
   * Default constructor
   */
  inline Motion::Motion()
  : Array<12>()
  {};
 
  /**
   * Initialize a motion with a number of samples (lines) equals to @a rows 
   */
  inline Motion::Motion(Index rows)
  : Array<12>(rows)
  {};
 
  /**
   * Constructor from vectors (u,v,w,o).
   * Internally, these vectors are joined into an array with 12 components. They represents a compact affine transformation.
   * The storage order is the following u_x, u_y, u_z, v_x, v_x, v_y, v_z, w_x, w_y, w_z, o_x, o_y, o_z.
   */
  template <typename U, typename V, typename W, typename O>
  inline Motion::Motion(const Base<U>& u, const Base<V>& v, const Base<W>& w, const Base<O>& o)
  : Array<12>(static_cast<const U&>(u).rows())
  {
    static_assert(U::ColsAtCompileTime == 3, "The input 'u' must be an array with 3 columns (i.e. a Vector)");
    static_assert(V::ColsAtCompileTime == 3, "The input 'v' must be an array with 3 columns (i.e. a Vector)");
    static_assert(W::ColsAtCompileTime == 3, "The input 'w' must be an array with 3 columns (i.e. a Vector)");
    static_assert(O::ColsAtCompileTime == 3, "The input 'o' must be an array with 3 columns (i.e. a Vector)");
    const U& xpr1 = u;
    const Vector::Values v1 = xpr1.derived().values();
    const Vector::Residuals r1 = xpr1.derived().residuals();
    const V& xpr2 = v;
    const Vector::Values v2 = xpr2.derived().values();
    const Vector::Residuals r2 = xpr2.derived().residuals();
    const W& xpr3 = w;
    const Vector::Values v3 = xpr3.derived().values();
    const Vector::Residuals r3 = xpr3.derived().residuals();
    const O& xpr4 = o;
    const Vector::Values v4 = xpr4.derived().values();
    const Vector::Residuals r4 = xpr4.derived().residuals();
    assert(v1.rows() == v2.rows());
    assert(v2.rows() == v3.rows());
    assert(v3.rows() == v4.rows());
    assert(r1.rows() == r2.rows());
    assert(r2.rows() == r3.rows());
    assert(r3.rows() == r4.rows());
    const Eigen::Array<bool,Eigen::Dynamic,1> cond = (r1 >= 0.0) && (r2 >= 0.0) && (r3 >= 0.0) && (r4 >= 0.0);
    for (Index i = 0 ; i < cond.rows() ; ++i)
    {
      if (cond.coeff(i))
      {
        this->m_Values.block<1,3>(i,0) = v1.row(i);
        this->m_Values.block<1,3>(i,3) = v2.row(i);
        this->m_Values.block<1,3>(i,6) = v3.row(i);
        this->m_Values.block<1,3>(i,9) = v4.row(i);
        this->m_Residuals.coeffRef(i) = 0.0;
      }
      else
      {
        this->m_Values.row(i).setZero();
        this->m_Residuals.coeffRef(i) = -1.0;  
      }
    }
  };
  
  template <typename U>
  Motion::Motion(const Base<U>& other)
  : Array<12>(other)
  {};

  // ----------------------------------------------------------------------- //
  //                              OPERATIONS
  // ----------------------------------------------------------------------- //
  
  template <typename Xpr, int Cols>
  class BlockOp : public Base<BlockOp<Xpr, Cols>>
  {
    static_assert(Xpr::ColsAtCompileTime >= Cols, "The requested number of columns is greater than the number of columns of the array.");
    
    using Index = typename Traits<BlockOp<Xpr,Cols>>::Index;
    
    Xpr& mr_Xpr;
    Index m_Index;
    
  public:
    BlockOp() = delete;
    
    BlockOp(Xpr& x, Index index)
    : mr_Xpr(x), m_Index(index)
    {
      assert(this->mr_Xpr.cols() >= (this->m_Index + Cols));
    };
    
    Index rows() const noexcept {return this->mr_Xpr.rows();};
    Index cols() const noexcept {return Cols;};
    
    const BlockOp& derived() const noexcept {return *this;};
    
    auto values() noexcept -> decltype(this->mr_Xpr.derived().values().block(0,this->m_Index,this->mr_Xpr.derived().values().rows(),Cols))
    {
      return this->mr_Xpr.derived().values().block(0,this->m_Index,this->mr_Xpr.derived().values().rows(),Cols);
    };
    
    auto values() const noexcept -> decltype(this->mr_Xpr.derived().values().block(0,this->m_Index,this->mr_Xpr.derived().values().rows(),Cols))
    {
      return this->mr_Xpr.derived().values().block(0,this->m_Index,this->mr_Xpr.derived().values().rows(),Cols);
    };

    auto residuals() noexcept -> decltype(this->mr_Xpr.derived().residuals().block(0,0,this->mr_Xpr.derived().values().rows(),1))
    {
      return this->mr_Xpr.derived().residuals().block(0,0,this->mr_Xpr.derived().residuals().rows(),1);
    };
    
    auto residuals() const noexcept -> decltype(this->mr_Xpr.derived().residuals().block(0,0,this->mr_Xpr.derived().residuals().rows(),1))
    {
      return this->mr_Xpr.derived().residuals().block(0,0,this->mr_Xpr.derived().residuals().rows(),1);
    };
    
    BlockOp& operator= (const BlockOp& other)
    {
      this->values().lazyAssign(other.values());
      this->residuals().lazyAssign(BlockOp::generateResiduals((this->residuals() >= 0.0) && (other.residuals() >= 0.0)));
      return *this;
    };

    template <typename U, int V> BlockOp& operator= (const BlockOp<U,V>& other)
    {
      static_assert(V == Cols, "The number of columns must be the same.");
      this->values() = other.values();
      this->residuals() = BlockOp::generateResiduals((this->residuals() >= 0.0) && (other.residuals() >= 0.0));
      return *this;
    };
  };
  
  template <typename Derived>
  template <int Cols>
  inline BlockOp<Derived,Cols> Base<Derived>::block(Index index) noexcept
  {
    return BlockOp<Derived,Cols>(*this,index);
  };

  template <typename Derived>
  template <int Cols>
  inline const BlockOp<const Derived,Cols> Base<Derived>::block(Index index) const noexcept
  {
    return BlockOp<const Derived,Cols>(*this,index);
  };
  
  template <typename Derived> inline BlockOp<Data<Derived>,1> Data<Derived>::x() noexcept {return this->template block<1>(0);};
  template <typename Derived> inline const BlockOp<const Data<Derived>,1> Data<Derived>::x() const noexcept {return this->template block<1>(0);};
  template <typename Derived> inline BlockOp<Data<Derived>,1> Data<Derived>::y() noexcept {return this->template block<1>(1);};
  template <typename Derived> inline const BlockOp<const Data<Derived>,1> Data<Derived>::y() const noexcept {return this->template block<1>(1);};
  template <typename Derived> inline BlockOp<Data<Derived>,1> Data<Derived>::z() noexcept {return this->template block<1>(2);};
  template <typename Derived> inline const BlockOp<const Data<Derived>,1> Data<Derived>::z() const noexcept {return this->template block<1>(2);};
  
  template <typename Derived> inline BlockOp<Data<Derived>,3> Data<Derived>::u() noexcept {return this->template block<3>(0);};
  template <typename Derived> inline const BlockOp<const Data<Derived>,3> Data<Derived>::u() const noexcept {return this->template block<3>(0);};
  template <typename Derived> inline BlockOp<Data<Derived>,3> Data<Derived>::v() noexcept {return this->template block<3>(3);};
  template <typename Derived> inline const BlockOp<const Data<Derived>,3> Data<Derived>::v() const noexcept {return this->template block<3>(3);};
  template <typename Derived> inline BlockOp<Data<Derived>,3> Data<Derived>::w() noexcept {return this->template block<3>(6);};
  template <typename Derived> inline const BlockOp<const Data<Derived>,3> Data<Derived>::w() const noexcept {return this->template block<3>(6);};
  template <typename Derived> inline BlockOp<Data<Derived>,3> Data<Derived>::o() noexcept {return this->template block<3>(9);};
  template <typename Derived> inline const BlockOp<const Data<Derived>,3> Data<Derived>::o() const noexcept {return this->template block<3>(9);};
  
  // ----------------------------------------------------------------------- //
  
  template <typename Derived, typename Xpr>
  class UnaryOp : public Base<UnaryOp<Derived, Xpr>>
  {
    using Index = typename Traits<UnaryOp<Derived, Xpr>>::Index;
    
  protected:
    const Xpr& mr_Xpr;
    
  public:
    UnaryOp(const Base<Xpr>& x)
    : mr_Xpr(x)
    {
      assert(this->mr_Xpr.rows() > 0);
    };
    
    Index rows() const noexcept {return static_cast<const Derived&>(*this).rows();};
    Index cols() const noexcept {return Traits<UnaryOp<Derived,Xpr>>::ColsAtCompileTime;};
    
    const Derived& derived() const noexcept {return static_cast<const Derived&>(*this);};
  };
  
  // ----------------------------------------------------------------------- //
  
  template <typename Derived, typename XprOne, typename XprTwo>
  class BinaryOp : public Base<BinaryOp<Derived, XprOne, XprTwo>>
  {
    using Index = typename Traits<BinaryOp<Derived, XprOne, XprTwo>>::Index;
    
  protected:
    const XprOne& mr_Xpr1;
    const XprTwo& mr_Xpr2;
    
  public:
    BinaryOp(const Base<XprOne>& x1, const Base<XprTwo>& x2)
    : mr_Xpr1(x1), mr_Xpr2(x2)
    {
      assert(this->mr_Xpr1.rows() > 0);
      assert(this->mr_Xpr2.rows() > 0);
      assert(this->mr_Xpr1.rows() == this->mr_Xpr2.rows());
    };
    
    Index rows() const noexcept {return static_cast<const Derived&>(*this).rows();};
    Index cols() const noexcept {return Traits<BinaryOp<Derived,XprOne,XprTwo>>::ColsAtCompileTime;};
    
    const Derived& derived() const noexcept {return static_cast<const Derived&>(*this);};
  };
  
  // ----------------------------------------------------------------------- //
  
  template <typename Xpr>
  struct Traits<NormOp<Xpr>>
  {
    enum : int
    {
      Processing = ValuesOnly,
    };
  };
  
  template <typename Xpr>
  struct Traits<UnaryOp<NormOp<Xpr>,Xpr>>
  {
    using Values = typename Traits<Xpr>::Values;
    using Residuals = typename Traits<Xpr>::Residuals;
    using Index = typename Values::Index;
    enum : int
    {
      ColsAtCompileTime = 1,
      Processing = Traits<NormOp<Xpr>>::Processing
    };
  };

  template <typename Xpr>
  class NormOp : public UnaryOp<NormOp<Xpr>,Xpr>
  {
    using Index = typename Traits<UnaryOp<NormOp<Xpr>, Xpr>>::Index;
    
  public:
    NormOp(const Base<Xpr>& x)
    : UnaryOp<NormOp<Xpr>,Xpr>(x)
    {};
    
    Index rows() const noexcept {return this->mr_Xpr.rows();};

    auto values() const noexcept -> decltype(this->mr_Xpr.derived().values().square().rowwise().sum().sqrt())
    {
      return this->mr_Xpr.derived().values().square().rowwise().sum().sqrt();
    };

    auto residuals() const noexcept -> decltype(this->mr_Xpr.derived().residuals())
    {
      return this->mr_Xpr.derived().residuals();
    };
  };
  
  template <typename Derived>
  inline const NormOp<Derived> Base<Derived>::norm() const noexcept
  {
    return NormOp<Derived>(*this);
  };
  
  // ----------------------------------------------------------------------- //
  
  template <typename Xpr>
  struct Traits<MeanOp<Xpr>>
  {
    enum : int
    {
      Processing = Full,
    };
  };

  template <typename Xpr>
  class MeanOp : public UnaryOp<MeanOp<Xpr>,Xpr>
  {
    using Index = typename Traits<UnaryOp<MeanOp<Xpr>, Xpr>>::Index;
    
  public:
    MeanOp(const Base<Xpr>& x)
    : UnaryOp<MeanOp<Xpr>,Xpr>(x)
    {}
      
    Index rows() const noexcept {return this->mr_Xpr.rows();};

    auto values() const noexcept -> decltype((this->mr_Xpr.derived().residuals() >= 0.0).template replicate<1,Xpr::ColsAtCompileTime>().select(this->mr_Xpr.derived().values(), 0.0).colwise().sum() / ((this->mr_Xpr.derived().residuals() >= 0.0).any() ? (this->mr_Xpr.derived().residuals() >= 0.0).count() : 1.0))
    {
      const auto& cond = (this->mr_Xpr.derived().residuals() >= 0.0);
      return cond.template replicate<1,Xpr::ColsAtCompileTime>().select(this->mr_Xpr.derived().values(), 0.0).colwise().sum() / (cond.any() ? cond.count() : 1.0);
    };

    auto residuals() const noexcept -> decltype(MeanOp::generateResiduals((this->mr_Xpr.derived().residuals() >= 0.0).colwise().sum()))
    {
      return MeanOp::generateResiduals((this->mr_Xpr.derived().residuals() >= 0.0).colwise().sum());
    };
  };
  
  template <typename Derived>
  inline const MeanOp<Derived> Base<Derived>::mean() const noexcept
  {
    return MeanOp<Derived>(*this);
  };
    
  // ----------------------------------------------------------------------- //
  
  template <typename XprOne, typename XprTwo>
  struct Traits<DifferenceOp<XprOne,XprTwo>>
  {
    enum : int
    {
      Processing = Full,
    };
  };

  template <typename XprOne, typename XprTwo>
  class DifferenceOp : public BinaryOp<DifferenceOp<XprOne, XprTwo>, XprOne, XprTwo>
  {
    static_assert(XprOne::ColsAtCompileTime == XprTwo::ColsAtCompileTime, "The number of columns must be the same.");
    
    using Index = typename Traits<BinaryOp<DifferenceOp<XprOne, XprTwo>, XprOne, XprTwo>>::Index;
    
  public:
    DifferenceOp(const Base<XprOne>& x1, const Base<XprTwo>& x2)
    : BinaryOp<DifferenceOp<XprOne, XprTwo>, XprOne, XprTwo>(x1,x2)
    {
      assert(this->mr_Xpr1.rows() == this->mr_Xpr2.rows());
    };
    
    Index rows() const noexcept {return this->mr_Xpr1.rows();};

    auto values() const noexcept -> decltype(this->mr_Xpr1.derived().values() - this->mr_Xpr2.derived().values())
    {
      return this->mr_Xpr1.derived().values() - this->mr_Xpr2.derived().values();
    };
  
    auto residuals() const noexcept -> decltype(DifferenceOp::generateResiduals((this->mr_Xpr1.derived().residuals() >= 0.0) && (this->mr_Xpr2.derived().residuals() >= 0.0)))
    {
      return DifferenceOp::generateResiduals((this->mr_Xpr1.derived().residuals() >= 0.0) && (this->mr_Xpr2.derived().residuals() >= 0.0));
    };
  };
  
  // ----------------------------------------------------------------------- //

  template <typename XprOne, typename XprTwo>
  struct Traits<SumOp<XprOne,XprTwo>>
  {
    enum : int
    {
      Processing = Full,
    };
  };

  template <typename XprOne, typename XprTwo>
  class SumOp : public BinaryOp<SumOp<XprOne, XprTwo>, XprOne, XprTwo>
  {
    static_assert(XprOne::ColsAtCompileTime == XprTwo::ColsAtCompileTime, "The number of columns must be the same.");
    
    using Index = typename Traits<BinaryOp<SumOp<XprOne, XprTwo>, XprOne, XprTwo>>::Index;
    
  public:
    SumOp(const Base<XprOne>& x1, const Base<XprTwo>& x2)
    : BinaryOp<SumOp<XprOne, XprTwo>, XprOne, XprTwo>(x1,x2)
    {
      assert(this->mr_Xpr1.rows() == this->mr_Xpr2.rows());
    };
    
    Index rows() const noexcept {return this->mr_Xpr1.rows();};

    auto values() const noexcept -> decltype(this->mr_Xpr1.derived().values() + this->mr_Xpr2.derived().values())
    {
      return this->mr_Xpr1.derived().values() + this->mr_Xpr2.derived().values();
    };
  
    auto residuals() const noexcept -> decltype(SumOp::generateResiduals((this->mr_Xpr1.derived().residuals() >= 0.0) && (this->mr_Xpr2.derived().residuals() >= 0.0)))
    {
      return SumOp::generateResiduals((this->mr_Xpr1.derived().residuals() >= 0.0) && (this->mr_Xpr2.derived().residuals() >= 0.0));
    };
  };
  
  // ----------------------------------------------------------------------- //

  template <typename Xpr>
  struct Traits<ScaleOp<Xpr>>
  {
    enum : int
    {
      Processing = ValuesOnly,
    };
  };

  template <typename Xpr>
  class ScaleOp : public UnaryOp<ScaleOp<Xpr>,Xpr>
  {
    using Index = typename Traits<UnaryOp<ScaleOp<Xpr>, Xpr>>::Index;
    
    double m_Scale;

  public:
    ScaleOp(double scale, const Base<Xpr>& x)
    : UnaryOp<ScaleOp<Xpr>,Xpr>(x), m_Scale(scale)
    {}
      
    Index rows() const noexcept {return this->mr_Xpr.rows();};

    auto values() const noexcept -> decltype(this->m_Scale * this->mr_Xpr.derived().values())
    {
      return this->m_Scale * this->mr_Xpr.derived().values();
    };

    auto residuals() const noexcept -> decltype(this->mr_Xpr.derived().residuals())
    {
      return this->mr_Xpr.derived().residuals();
    };
  };
  
  // ----------------------------------------------------------------------- //
  
  template <typename Xpr>
  struct Traits<NormalizedOp<Xpr>>
  {
    enum : int
    {
      Processing = ValuesOnly,
    };
  };
  
  template <typename Xpr>
  class NormalizedOp : public UnaryOp<NormalizedOp<Xpr>,Xpr>
  {
    using Index = typename Traits<UnaryOp<NormalizedOp<Xpr>, Xpr>>::Index;
    
  public:
    NormalizedOp(const Base<Xpr>& x)
    : UnaryOp<NormalizedOp<Xpr>,Xpr>(x)
    {};
    
    Index rows() const noexcept {return this->mr_Xpr.rows();};

    auto values() const noexcept -> decltype(this->mr_Xpr.derived().values().cwiseQuotient((this->mr_Xpr.derived().values().square().rowwise().sum() >= std::numeric_limits<double>::epsilon()).select(this->mr_Xpr.derived().values().square().rowwise().sum().sqrt(),1.0).replicate(1,3)))
    {
      const auto& xpr = this->mr_Xpr.derived();
      const auto temp = xpr.values().square().rowwise().sum();
      const auto norm = (temp >= std::numeric_limits<double>::epsilon()).select(temp.sqrt(),1.0).replicate(1,xpr.cols());
      return xpr.values().cwiseQuotient(norm);
    };

    auto residuals() const noexcept -> decltype(this->mr_Xpr.derived().residuals())
    {
      return this->mr_Xpr.derived().residuals();
    };
  };
  
  template <typename Derived>
  const NormalizedOp<Derived> Base<Derived>::normalized() const noexcept
  {
    return NormalizedOp<Derived>(*this);
  };
  
  // ----------------------------------------------------------------------- //
  
  template <typename XprOne, typename XprTwo>
  struct Traits<CrossOp<XprOne,XprTwo>>
  {
    enum : int
    {
      Processing = Full,
    };
  };
  
  template <typename XprOne, typename XprTwo>
  class CrossOp : public BinaryOp<CrossOp<XprOne, XprTwo>, XprOne, XprTwo>
  {
    static_assert(XprOne::ColsAtCompileTime == XprTwo::ColsAtCompileTime, "The number of columns must be the same.");
    static_assert(XprOne::ColsAtCompileTime == 3 && XprTwo::ColsAtCompileTime == 3, "The cross operation is only available for array with 3 columns.");
    
    using Index = typename Traits<BinaryOp<CrossOp<XprOne, XprTwo>, XprOne, XprTwo>>::Index;
    
  public:    
    CrossOp(const Base<XprOne>& x1, const Base<XprTwo>& x2)
    : BinaryOp<CrossOp<XprOne, XprTwo>, XprOne, XprTwo>(x1,x2)
    {
      assert(this->mr_Xpr1.rows() == this->mr_Xpr2.rows());
    };
    
    Index rows() const noexcept {return this->mr_Xpr1.rows();};

    auto values() const noexcept -> Eigen::internal::CrossOpValues<decltype(this->mr_Xpr1.derived().values()),decltype(this->mr_Xpr2.derived().values())>
    {
      using V1 = decltype(this->mr_Xpr1.derived().values());
      using V2 = decltype(this->mr_Xpr2.derived().values());
      return Eigen::internal::CrossOpValues<V1,V2>(this->mr_Xpr1.derived().values(), this->mr_Xpr2.derived().values());
    };
  
    auto residuals() const noexcept -> decltype(CrossOp::generateResiduals((this->mr_Xpr1.derived().residuals() >= 0.0) && (this->mr_Xpr2.derived().residuals() >= 0.0)))
    {
      return CrossOp::generateResiduals((this->mr_Xpr1.derived().residuals() >= 0.0) && (this->mr_Xpr2.derived().residuals() >= 0.0));
    };
  };
  
  template <typename Derived>
  template <typename U>
  inline const CrossOp<Derived,U> Base<Derived>::cross(const Base<U>& other) const noexcept
  {
    return CrossOp<Derived,U>(*this,other);
  };

  // ----------------------------------------------------------------------- //
  
  template <typename XprOne, typename XprTwo>
  struct Traits<TransformOp<XprOne,XprTwo>>
  {
    enum : int
    {
      Processing = Full,
    };
  };
  
  template <typename XprOne, typename XprTwo>
  class TransformOp : public BinaryOp<TransformOp<XprOne, XprTwo>, XprOne, XprTwo>
  { 
    using Index = typename Traits<BinaryOp<TransformOp<XprOne, XprTwo>, XprOne, XprTwo>>::Index;
    
  public:
    TransformOp(const Base<XprOne>& x1, const Base<XprTwo>& x2)
    : BinaryOp<TransformOp<XprOne, XprTwo>, XprOne, XprTwo>(x1,x2)
    {};
    
    Index rows() const noexcept {return this->mr_Xpr1.rows();};

    auto values() const noexcept -> Eigen::internal::TransformOpValues<decltype(this->mr_Xpr1.derived().values()),decltype(this->mr_Xpr2.derived().values())>
    {
      using V1 = decltype(this->mr_Xpr1.derived().values());
      using V2 = decltype(this->mr_Xpr2.derived().values()); 
      return Eigen::internal::TransformOpValues<V1,V2>(this->mr_Xpr1.derived().values(), this->mr_Xpr2.derived().values());
    };

    auto residuals() const noexcept -> decltype(TransformOp::generateResiduals((this->mr_Xpr1.derived().residuals() >= 0.0) && (this->mr_Xpr2.derived().residuals() >= 0.0)))
    {
      return TransformOp::generateResiduals((this->mr_Xpr1.derived().residuals() >= 0.0) && (this->mr_Xpr2.derived().residuals() >= 0.0));
    };
  };

  template <typename Derived>
  template <typename U>
  inline const TransformOp<Derived,U> Base<Derived>::transform(const Base<U>& other) const noexcept
  {
    return TransformOp<Derived,U>(*this,other);
  };
  
  // ----------------------------------------------------------------------- //
  
  template <typename Xpr>
  struct Traits<ReplicateOp<Xpr>>
  {
    enum : int
    {
      Processing = None,
    };
  };
  
  template <typename Xpr>
  class ReplicateOp : public UnaryOp<ReplicateOp<Xpr>,Xpr>
  {
    using Index = typename Traits<UnaryOp<ReplicateOp<Xpr>, Xpr>>::Index;
    
    Index m_Rows;
    
  public:
    ReplicateOp(const Base<Xpr>& x, Index rows)
    : UnaryOp<ReplicateOp<Xpr>,Xpr>(x), m_Rows(rows)
    {}
      
    Index rows() const noexcept {return this->mr_Xpr.rows() * this->m_Rows;};

    auto values() const noexcept -> decltype(this->mr_Xpr.values().replicate(this->m_Rows,1))
    {
      return this->mr_Xpr.values().replicate(this->m_Rows,1);
    };

    auto residuals() const noexcept -> decltype(this->mr_Xpr.residuals().replicate(this->m_Rows,1))
    {
      return this->mr_Xpr.residuals().replicate(this->m_Rows,1);
    };
  };
  
  template <typename Derived>
  inline const ReplicateOp<Derived> Base<Derived>::replicate(Index rows) const noexcept
  {
    return ReplicateOp<Derived>(*this,rows);
  };
  
  // ----------------------------------------------------------------------- //
  
  template <typename Xpr>
  struct Traits<InverseOp<Xpr>>
  {
    enum : int
    {
      Processing = ValuesOnly,
    };
  };
  
  template <typename Xpr>
  class InverseOp : public UnaryOp<InverseOp<Xpr>,Xpr>
  {
    using Index = typename Traits<UnaryOp<InverseOp<Xpr>, Xpr>>::Index;
    
    static_assert(Xpr::ColsAtCompileTime == 12, "The inverse operation is only available for array with 12 columns.");
    
  public:
    InverseOp(const Base<Xpr>& x)
    : UnaryOp<InverseOp<Xpr>,Xpr>(x)
    {};
    
    Index rows() const noexcept {return this->mr_Xpr.rows();};

    auto values() const noexcept -> decltype(Eigen::internal::InverseOpValues<decltype(this->mr_Xpr.derived().values())>(this->mr_Xpr.derived().values()))
    {
      using V = decltype(this->mr_Xpr.derived().values());
      return Eigen::internal::InverseOpValues<V>(this->mr_Xpr.derived().values());
    };

    auto residuals() const noexcept -> decltype(this->mr_Xpr.derived().residuals())
    {
      return this->mr_Xpr.derived().residuals();
    };
  };
  
  template <typename Derived>
  inline const InverseOp<Derived> Base<Derived>::inverse() const noexcept
  {
    return InverseOp<Derived>(*this);
  };
  
  // ----------------------------------------------------------------------- //
  
  template <typename Xpr>
  struct Traits<EulerAnglesOp<Xpr>>
  {
    enum : int
    {
      Processing = ValuesOnly,
    };
  };
  
  template <typename Xpr>
  struct Traits<UnaryOp<EulerAnglesOp<Xpr>,Xpr>>
  {
    using Values = typename Traits<Xpr>::Values;
    using Residuals = typename Traits<Xpr>::Residuals;
    using Index = typename Values::Index;
    enum : int
    {
      ColsAtCompileTime = 3,
      Processing = Traits<EulerAnglesOp<Xpr>>::Processing
    };
  }; 
  
  template <typename Xpr>
  class EulerAnglesOp : public UnaryOp<EulerAnglesOp<Xpr>,Xpr>
  {
    static_assert(Xpr::ColsAtCompileTime >= 9, "The Euler angles operation is only available for array with at least 9 columns.");
    
    using Index = typename Traits<UnaryOp<EulerAnglesOp<Xpr>, Xpr>>::Index;
    
    Index m_Axis0;
    Index m_Axis1;
    Index m_Axis2;
    
  public:
    EulerAnglesOp(const Base<Xpr>& x, Index a0, Index a1, Index a2)
    : UnaryOp<EulerAnglesOp<Xpr>,Xpr>(x),
      m_Axis0(a0), m_Axis1(a1), m_Axis2(a2)
    {};
      
    Index rows() const noexcept {return this->mr_Xpr.rows();};
      
    auto values() const noexcept -> decltype(Eigen::internal::EulerAnglesOpValues<decltype(this->mr_Xpr.derived().values())>(this->mr_Xpr.derived().values(), this->m_Axis0, this->m_Axis1, this->m_Axis2))
    {
      using V = decltype(this->mr_Xpr.derived().values());
      return Eigen::internal::EulerAnglesOpValues<V>(this->mr_Xpr.derived().values(), this->m_Axis0, this->m_Axis1, this->m_Axis2);
    };

    auto residuals() const noexcept -> decltype(this->mr_Xpr.derived().residuals())
    {
      return this->mr_Xpr.derived().residuals();
    };
  };
  
  template <typename Derived>
  const EulerAnglesOp<Derived> Base<Derived>::eulerAngles(Index a0, Index a1, Index a2) const noexcept
  {
    return EulerAnglesOp<Derived>(*this,a0,a1,a2);
  };
  
  // ----------------------------------------------------------------------- //

  template <typename XprOne, typename XprTwo>
  const DifferenceOp<XprOne,XprTwo> operator-(const Base<XprOne>& x1, const Base<XprTwo>& x2) noexcept
  {
    return DifferenceOp<XprOne,XprTwo>(x1,x2);
  };
  
  template <typename XprOne, typename XprTwo>
  const SumOp<XprOne,XprTwo> operator+(const Base<XprOne>& x1, const Base<XprTwo>& x2) noexcept
  {
    return SumOp<XprOne,XprTwo>(x1,x2);
  };

  template <typename Xpr>
  const ScaleOp<Xpr> operator* (double x1, const Base<Xpr>& x2)
  {
    return ScaleOp<Xpr>(x1,x2);
  };

  template <typename Xpr>
  const ScaleOp<Xpr> operator* (const Base<Xpr>& x1, double x2)
  {
    return ScaleOp<Xpr>(x2,x1);
  };

  // template <typename Xpr>
  // const Scale<Xpr> operator/ (double x1, const Base<Xpr>& x2)
  // {
  //   return Scale<Xpr>(x1,Inverse<Xpr>(x2));
  // };

  template <typename Xpr>
  const ScaleOp<Xpr> operator/ (const Base<Xpr>& x1, double x2)
  {
    assert(fabs(x2) >= std::numeric_limits<double>::epsilon());
    return ScaleOp<Xpr>(1.0/x2,x1);
  };
};
};

#endif // __btklardXpr_h