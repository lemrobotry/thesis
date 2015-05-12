#include "Log.hh"
#include "NormalForm.hh"

namespace Permute {

  PathList::PathList (ConstPathRef head, ConstPathRef tail) :
    PathImpl (Log::add (head -> getScore (), tail -> getScore ())),
    head_ (head),
    tail_ (tail)
  {}
  Fsa::StateId PathList::getStart () const {
    return head_ -> getStart ();
  }
  Fsa::StateId PathList::getEnd () const {
    return head_ -> getEnd ();
  }
  void PathList::accept (PathVisitor * visitor) const {
    head_ -> accept (visitor);
  }
  Path::Type PathList::type () const {
    return head_ -> type ();
  }
  bool PathList::normal () const {
    return head_ -> normal ();
  }
  void PathList::accept (NormalFormVisitor * visitor) const {
    visitor -> visit (this);
  }
  ConstPathRef PathList::getHead () const {
    return head_;
  }
  ConstPathRef PathList::getTail () const {
    return tail_;
  }

  ConstPathRef cons (ConstPathRef head, ConstPathRef tail) {
    return ConstPathRef (new PathList (head, tail));
  }

  /**********************************************************************/
  
  NormalFormCell::NormalFormCell () :
    CellImpl ()
  {}

  bool NormalFormCell::add (ConstPathRef path) {
    if (! path -> normal ()) {
      return false;
    } else {
      CellHash::iterator existing = paths_.find (path -> getStart (), path -> getEnd (), path -> type ());
      if (existing == paths_.end ()) {
	paths_.insert (path);
      } else {
	ConstPathRef tail = * existing;
	* existing = cons (path, tail);
      }
      return true;
    }
  }
}
