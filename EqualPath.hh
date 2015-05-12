#ifndef _PERMUTE_EQUAL_PATH_HH
#define _PERMUTE_EQUAL_PATH_HH

#include "Path.hh"

namespace Permute {

  // Tests whether the given paths are equal.
  bool equalPaths (const ConstPathRef & p1, const ConstPathRef & p2);
  
  // Prints a parenthesized representation of the tree contained in the path.
  // At each composite node prints (TYPE LEFT RIGHT) where TYPE is KEEP or SWAP,
  // and LEFT and RIGHT are subtrees contained in the left and right children.
  // At each arc node prints the arc's index.
  std::ostream & operator << (std::ostream &, const ConstPathRef &);
}

#endif//_PERMUTE_EQUAL_PATH_HH
