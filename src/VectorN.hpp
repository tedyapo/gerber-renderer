#ifndef VECTORN_HPP_INCLUDED_
#define VECTORN_HPP_INCLUDED_

#include <cmath>
#include <iostream>

#include "VectorUtil.hpp"

template <typename base_t, unsigned N>
struct VectorN;

template <typename base_t, unsigned N>
std::ostream& operator<< (std::ostream &out, VectorN<base_t, N> const& v);

template <typename base_t, unsigned N>
std::istream& operator>> (std::istream &in, VectorN<base_t, N> & v);

//
// implementations here for inlining
//
template <typename base_t, unsigned N>
class VectorN
{
public:
  static_assert(N >= 2, "VectorN<base_t, N> must have N >=2");

  VectorN(base_t const &val = 0)
  {
    for (unsigned i=0; i<N; ++i)
    {
      v[i] = val;
    }
  }

  VectorN(base_t const& v0, base_t const& v1)
  {
    static_assert(N == 2, "VectorN<base_t, 2>() must have N == 2");
    v[0] = v0;
    v[1] = v1;
  }

  VectorN(base_t const& v0, base_t const& v1, base_t const& v2)
  {
    static_assert(N == 3, "VectorN<base_t, 3>() must have N == 3");
    v[0] = v0;
    v[1] = v1;
    v[2] = v2;
  }

  VectorN(base_t const& v0, base_t const& v1,
          base_t const& v2, base_t const& v3)
  {
    static_assert(N == 4, "VectorN<base_t, 4>() must have N == 4");
    v[0] = v0;
    v[1] = v1;
    v[2] = v2;
    v[3] = v3;
  }

  VectorN(base_t const *val)
  {
    for (unsigned i=0;i<N;i++){
      v[i] = val[i];
    }
  }

  // accessors for xy(z)(w) points
  base_t &x(){return v[0];}
  base_t x() const {return v[0];}
  base_t &y(){return v[1];}
  base_t y() const {return v[1];}
  base_t &z()
  {
    static_assert(N >= 3, "VectorN<base_t, N>.z() must have N >= 3");
    return v[2];
  }
  base_t z() const
  {
    static_assert(N >= 3, "VectorN<base_t, N>.z() must have N >= 3");
    return v[2];
  }
  base_t &w()
  {
    static_assert(N >= 4, "VectorN<base_t, N>.w() must have N >= 4");
    return v[3];
  }
  base_t w() const
  {
    static_assert(N >= 4, "VectorN<base_t, N>.w() must have N >= 4");
    return v[3];
  }
  
  // accessors for polar/spherical coordinates
  base_t &rho(){return v[0];}
  base_t rho() const {return v[0];}
  base_t &theta(){return v[1];}
  base_t theta() const {return v[1];}
  base_t &phi()
  {
    static_assert(N >= 3, "VectorN<base_t, N>.phi() must have N >= 3");
    return v[2];
  }
  base_t phi() const
  {
    static_assert(N >= 3, "VectorN<base_t, N>.phi() must have N >= 3");
    return v[2];
  }

  // accessors for rgb(a) points
  base_t &r(){return v[0];}
  base_t r() const {return v[0];}
  base_t &g(){return v[1];}
  base_t g() const {return v[1];}
  base_t &b()
  {
    static_assert(N >= 3, "VectorN<base_t, N>.b() must have N >= 3");
    return v[2];
  }
  base_t b() const
  {
    static_assert(N >= 3, "VectorN<base_t, N>.b() must have N >= 3");
    return v[2];
  }
  base_t &a()
  { 
    static_assert(N >= 4, "VectorN<base_t, N>.a() must have N >= 4");
    return v[3];
  }
  base_t a() const
  {
    static_assert(N >= 4, "VectorN<base_t, N>.a() must have N >= 4");
    return v[3];
  }

  //
  // conversion from a different base_t VectorN
  //
  template<typename T>
  VectorN(VectorN<T, N> const& a)
  {
    for (unsigned i=0; i<N; i++){
      v[i] = base_t(a.v[i]);
    }
  }

  //
  // assign to all components from a base_t
  //
  VectorN<base_t, N>& operator=(const base_t &val)
  {
    v[0] = v[1] = v[2] = base_t(val);
    return *this;
  }
    
  //
  // test equality with another VectorN
  //   NB: should never be used with floating-point types
  //
  bool operator== (const VectorN<base_t, N> &a) const 
  {
    for (unsigned i=0; i<N; i++){
      if (!(v[i] == a.v[i])){
        return false;
      } 
    }
    return true;
  }

  //
  // test inequality with another VectorN
  //   NB: should never be used with floating-point types
  //
  bool operator!= (const VectorN<base_t, N> &a) const
  {
    return !operator==(a);
  }

  //
  // output to ostream (space delimited)
  //
  friend
  std::ostream& operator<< <>(std::ostream &out, VectorN<base_t, N> const& v);

  //
  // input from istream
  //
  friend
  std::istream& operator>> <>(std::istream &in, VectorN<base_t, N> & v);

  //
  // return a component reference by index
  //   NB: no bounds checking here
  base_t &operator()(int i)
  {
    return v[i];
  }

  //
  // return a component value by index
  //   NB: no bounds checking here
  base_t operator()(int i) const
  {
    return v[i];
  }

  //
  // element-wise unary negation of VectorN
  //
  VectorN<base_t, N> operator-()
  {
    VectorN<base_t, N> temp;
    for (unsigned i=0; i<N; i++)
    {
      temp.v[i] = -v[i];
    }
    return temp;    
  }

#if 0
  // !!! todo: get this to work
  MatrixN<base_t, N> outer(const VectorN<base_t, N> &a) const {
    MatrixN<base_t, N> mat;
    for (unsigned r=0; r<N; r++){
      for (unsigned c=0; c<N; c++){
        mat(r,c) = v[r] * a[c];
      }
    }
    return mat;
  }
#endif

  //
  //  length (magnitude) of VectorN
  //
  base_t length() const
  {
    base_t l = 0;
    for (unsigned i=0; i<N; ++i)
    {
      l += v[i]*v[i];
    }
    return base_t(std::sqrt(l));
  }

  //
  // normalize to unit length
  //
  base_t normalize()
  {
    base_t l = 1./length();
    // NB: this compare is problematic for floating-point types
    //  !!! replace with epsilon test
    if (l != base_t(0))
    {
      for (unsigned i=0; i<N; ++i)
      {
        v[i] *= l;
      }
    }
    return l;
  }
  
  //
  // apply a scalar function to each component (idependently)
  //
  VectorN<base_t, N> &apply(base_t (*func)(base_t))
  {
    for (unsigned i=0; i<N; i++){
      v[i] = func(v[i]);
    }
    return *this;
  }

  //
  // return minimum element
  //
  base_t min() const
  {
    base_t m = v[0];
    for (unsigned i=1;i<N;i++){
      if (v[i] < m) m = v[i];
    }
    return m;
  }

  //
  // return maximum element
  //
  base_t max() const {
    base_t m = v[0];
    for (unsigned i=1;i<N;i++){
      if (v[i] > m) m = v[i];
    }
    return m;
  }

  //
  // swizzle functions
  //
  // instantiate all permutations like:
  //  v.xyz()
  //  v.yxz()
  //  v.argb()
  //   etc
#include "VectorNSwizzle.hpp"

  template<typename T, unsigned M> friend class VectorN;  
//  template<typename T, unsigned N> friend class MatrixN;

private:
  base_t v[N];
};

//
// element-wise minimum of two VectorN's
//
template <typename T, unsigned N>
VectorN<T, N> min(const VectorN<T, N> &a, const VectorN<T, N> &b)
{
  VectorN<T, N> result;
  for (unsigned i=0; i<N; ++i)
  {
    result(i) = std::min(a(i), b(i));
  }
  return result;
}

//
// element-wise maximum of two VectorN's
//
template <typename T, unsigned N>
VectorN<T, N> max(const VectorN<T, N> &a, const VectorN<T, N> &b)
{
  VectorN<T, N> result;
  for (unsigned i=0; i<N; ++i)
  {
    result(i) = std::max(a(i), b(i));
  }
  return result;
}

//
// add two VectorN's with promotion (V + V)
//
template <typename T1, typename T2, unsigned N>
VectorN<typename PromotionTraits<T1, T2>::promoted_t, N>
operator+(const VectorN<T1, N> &a, const VectorN<T2, N> &b)
{
  typedef typename PromotionTraits<T1, T2>::promoted_t promoted_t;
  VectorN<promoted_t, N> result;
  for (unsigned i=0;i<N;i++)
  {
    result(i) = ( promoted_t(a(i)) + 
                  promoted_t(b(i)) );
  }
  return result;
}

//
// add VectorN to exisiting VectorN (V += V)
//
template <typename T1, typename T2, unsigned N>
VectorN<T1, N>&
operator+=(VectorN<T1, N> &a, const VectorN<T2, N> &b)
{
  for (unsigned i=0;i<N;i++)
  {
    a(i) += T1(b(i));
  }
  return a;
}

//
// add a VectorN and scalar (V + s)
//
template <typename T1, unsigned N>
VectorN<T1, N>
operator+(const VectorN<T1, N> &a, const T1 &b)
{
  VectorN<T1, N> result;
  for (unsigned i=0;i<N;i++)
  {
    result(i) = a(i) + b;
  }
  return result;
}

//
// add a scalar and VectorN (s + V)
//
template <typename T1, unsigned N>
VectorN<T1, N>
operator+(const T1 &a, const VectorN<T1, N> &b)
{
  VectorN<T1, N> result;
  for (unsigned i=0;i<N;i++)
  {
    result(i) = a + b(i);
  }
  return result;
}

//
// add a scalar to existing Vector ( V += s)
//
template <typename T1, typename T2, unsigned N>
VectorN<T1, N>&
operator+=(VectorN<T1, N> &a, const T2 &b)
{
  for (unsigned i=0;i<N;i++)
  {
    a(i) += T1(b);
  }
  return a;
}

//
// subtract two VectorN types with promotion (V - V)
//
template <typename T1, typename T2, unsigned N>
VectorN<typename PromotionTraits<T1, T2>::promoted_t, N>
operator-(const VectorN<T1, N> &a, const VectorN<T2, N> &b)
{
  typedef typename PromotionTraits<T1, T2>::promoted_t promoted_t;
  VectorN<promoted_t, N> result;
  for (unsigned i=0;i<N;i++)
  {
    result(i) = ( promoted_t(a(i)) - 
                  promoted_t(b(i)) );
  }
  return result;
}

//
// subtract VectorN from exisiting VectorN (V -= V)
//
template <typename T1, typename T2, unsigned N>
VectorN<T1, N>&
operator-=(VectorN<T1, N> &a, const VectorN<T2, N> &b)
{
  for (unsigned i=0;i<N;i++)
  {
    a(i) -= T1(b(i));
  }
  return a;
}

//
// subtract scalar from VectorN with promotion (V - s)
//
template <typename T1, typename T2, unsigned N>
VectorN<typename PromotionTraits<T1, T2>::promoted_t, N>
operator-(const VectorN<T1, N> &a, const T2 &b)
{
  typedef typename PromotionTraits<T1, T2>::promoted_t promoted_t;
  VectorN<promoted_t, N> result;
  for (unsigned i=0;i<N;i++)
  {
    result(i) = ( promoted_t(a(i)) - 
                  promoted_t(b) );
  }
  return result;
}

//
// subtract VectorN from scalar with promotion (s - V)
//
template <typename T1, typename T2, unsigned N>
VectorN<typename PromotionTraits<T1, T2>::promoted_t, N>
operator-(const T1 &a, const VectorN<T2, N> &b)
{
  typedef typename PromotionTraits<T1, T2>::promoted_t promoted_t;
  VectorN<promoted_t, N> result;
  for (unsigned i=0;i<N;i++)
  {
    result(i) = ( promoted_t(a) - 
                  promoted_t(b(i)) );
  }
  return result;
}

//
// subtract scalar from existing VectorN (V -= s)
//
template <typename T1, typename T2, unsigned N>
VectorN<T1, N>&
operator-=(VectorN<T1, N> &a, const T2 &b)
{
  for (unsigned i=0;i<N;i++)
  {
    a(i) -= T1(b);
  }
  return a;
}

//
// element-wise multiply two VectorN's with promotion (V * V)
//   NB: equivalent to .* MATLAB operator
//
template <typename T1, typename T2, unsigned N>
VectorN<typename PromotionTraits<T1, T2>::promoted_t, N>
operator*(const VectorN<T1, N> &a, const VectorN<T2, N> &b)
{
  typedef typename PromotionTraits<T1, T2>::promoted_t promoted_t;
  VectorN<promoted_t, N> result;
  for (unsigned i=0;i<N;i++)
  {
    result(i) = ( promoted_t(a(i)) * 
                  promoted_t(b(i)) );
  }
  return result;
}

//
// element-wise multiply VectorN with existing VectorN (V *= V)
//
template <typename T1, typename T2, unsigned N>
VectorN<T1, N>&
operator*=(VectorN<T1, N> &a, const VectorN<T2, N> &b)
{
  for (unsigned i=0;i<N;i++)
  {
    a(i) *= T1(b(i));
  }
  return a;
}

//
// multiply VectorN with scalar (V * s)
//
template <typename T1, unsigned N>
VectorN<T1, N>
operator*(const VectorN<T1, N> &a, const T1 &b)
{
  VectorN<T1, N> result;
  for (unsigned i=0;i<N;i++)
  {
    result(i) = a(i) * b;
  }
  return result;
}

//
// multiply scalar with VectorN (s * V)
//
template <typename T1, unsigned N>
VectorN<T1, N>
operator* (const T1 &a, const VectorN<T1, N> &b)
{
  VectorN<T1, N> result;
  for (unsigned i=0;i<N;i++)
  {
    result(i) = a * b(i);
  }
  return result;
}

//
// multiply existing VectorN by scalar (V *= s)
//
template <typename T1, typename T2, unsigned N>
VectorN<T1, N>&
operator*=(VectorN<T1, N> &a, const T2 &b)
{
  for (unsigned i=0;i<N;i++)
  {
    a(i) *= T1(b);
  }
  return a;
}

//
// element-wise division of two VectorN's with promotion
//  NB: equivalent to ./ MATLAB operator
//
template <typename T1, typename T2, unsigned N>
VectorN<typename PromotionTraits<T1, T2>::promoted_t, N>
operator/(const VectorN<T1, N> &a, const VectorN<T2, N> &b)
{
  typedef typename PromotionTraits<T1, T2>::promoted_t promoted_t;
  VectorN<promoted_t, N> result;
  for (unsigned i=0;i<N;i++)
  {
    result(i) = ( promoted_t(a(i)) / 
                  promoted_t(b(i)) );
  }
  return result;
}

//
// element-wise division of existing VectorN by VectorN
//
template <typename T1, typename T2, unsigned N>
VectorN<T1, N>&
operator/=(VectorN<T1, N> &a, const VectorN<T2, N> &b)
{
  for (unsigned i=0;i<N;i++)
  {
    a(i) /= T1(b(i));
  }
  return a;
}

//
// element-wise division of scalar by VectorN (s / V)
//
template <typename T1, unsigned N>
VectorN<T1, N>
operator/(const T1 &a, const VectorN<T1, N> &b)
{
  VectorN<T1, N> result;
  for (unsigned i=0;i<N;i++)
  {
    result(i) = a / b(i);
  }
  return result;
}

//
// element-wise division of VectorN by scalar (V / s)
//
template <typename T1, unsigned N>
VectorN<T1, N>
operator/(const VectorN<T1, N> &a, const T1 &b)
{
  VectorN<T1, N> result;
  for (unsigned i=0;i<N;i++){
    result(i) = a(i) / b;
  }
  return result;
}

//
// element-wise division of existing VectorN by scalar (V /= s)
//
template <typename T1, unsigned N>
VectorN<T1, N>&
operator/=(VectorN<T1, N> &a, const T1 &b)
{
  for (unsigned i=0;i<N;i++)
  {
    a(i) /= b;
  }
  return a;
}

// todo: add traits for formatting various base_t's
//       especially eggregious are char types which print as chars vs. numeric
template<typename base_t, unsigned N>
std::ostream&
operator<< (std::ostream &out, VectorN<base_t, N> const& v)
{
  for (unsigned i=0; i<N; ++i)
  {
    out << v(i) << " ";
  }
  return out;
}

template<typename base_t, unsigned N>
std::istream&
operator>> (std::istream &in, VectorN<base_t, N> &v)
{
  for (unsigned i=0; i<N; ++i)
  {
    in >> v(i);
  }
  return in;
}

//
// dot product of two VectorN's
//
template<typename base_t, unsigned N>
base_t
dot(const VectorN<base_t, N> &a, const VectorN<base_t, N> &b)
{
  base_t sum = base_t(0);
  for (unsigned i=0; i<N; i++){
    sum += a(i) * b(i);
  }
  return sum;
}

//
// cross product of two Vector3's
//
template<typename base_t>
VectorN<base_t, 3>
cross(VectorN<base_t, 3> const& a, VectorN<base_t, 3> const& b)
{
  return VectorN<base_t, 3> (a.y()*b.z() - b.y()*a.z(),
                             b.x()*a.z() - a.x()*b.z(),
                             a.x()*b.y() - b.x()*a.y());
}

// typedefs for commonly-used types
typedef VectorN<float, 2>  v2f;
typedef VectorN<double, 2> v2d;
typedef VectorN<int, 2>    v2i;

typedef VectorN<float, 3>  v3f;
typedef VectorN<double, 3> v3d;
typedef VectorN<int, 3>    v3i;

typedef VectorN<float, 4>  v4f;
typedef VectorN<double, 4> v4d;
typedef VectorN<int, 4>    v4i;

#endif // #ifndef VECTORN_HPP_INCLUDED_
