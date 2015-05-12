#include "BestCell.hh"

namespace Permute {
  BestCell::BestCell () :
    CellImpl ()
  {}

  // Replaces the current path with the given path if its score is better.
  bool BestCell::add (ConstPathRef path) {
    assert (path -> normal ());
    if (paths_.empty ()) {
      paths_.insert (path);
      return true;
    } else if (path -> getScore () > (* paths_.begin ()) -> getScore ()) {
      paths_.clear ();
      paths_.insert (path);
      return true;
    }
    return false;
  }
}
