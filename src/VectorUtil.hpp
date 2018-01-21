#ifndef VECTORUTIL_INCLUDED_
#define VECTORUTIL_INCLUDED_

typedef unsigned char byte;

//
// clamp a value between lower and upper limits
//
template <typename T>
T
clamp (T value, T lower, T upper)
{
  if (value < lower)
  {
    return lower;
  }
  if (value > upper)
  {
    return upper;
  }
  return value;
}

//
// type for resolving vector component type promotion
//
template <typename T1, typename T2>
struct PromotionTraits {
  static T1 t1;
  static T2 t2;  
  typedef decltype(t1 + t2) promoted_t;
};



#endif // #ifndef VECTORUTIL_INCLUDED_
