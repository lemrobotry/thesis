#include "FullCell.hh"

namespace Permute {
  FullCell::FullCell () :
    CellImpl ()
  {}

  // Inserts the given path if it has a new (start, end, type) tuple, or if its
  // score is better than the existing path with the same tuple.
  bool FullCell::add (ConstPathRef path) {
    assert (path -> normal ());
    CellHash::iterator existing = paths_.find (path -> getStart (), path -> getEnd (), path -> type ());
    if (existing == paths_.end ()) {
      paths_.insert (path);
      return true;
    } else if (path -> getScore () > (* existing) -> getScore ()) {
      * existing = path;
      return true;
    }
    return false;
  }
}
