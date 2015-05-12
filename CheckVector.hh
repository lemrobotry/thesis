#ifndef _PERMUTE_CHECK_VECTOR_HH
#define _PERMUTE_CHECK_VECTOR_HH

namespace Permute {

  template <class Vector>
  typename Vector::const_reference
  check (const Vector & v, int n) {
    assert (n >= 0);
    assert (n < v.size ());
    return v [n];
  }

  template <class Vector>
  typename Vector::reference
  check (Vector & v, int n) {
    return const_cast <typename Vector::reference>
      (check (static_cast <const Vector> (v), n));
  }
}

#endif//_PERMUTE_CHECK_VECTOR_HH
