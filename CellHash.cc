#include "CellHash.hh"

namespace Permute {

  CellHash::Cursor hash_key (Fsa::StateId start, Fsa::StateId end, Path::Type type) {
    return 2239 * start + 4 * end + type;
  }
  CellHash::Cursor hash_key (ConstPathRef path) {
    return hash_key (path -> getStart (), path -> getEnd (), path -> type ());
  }
  bool hash_equal (ConstPathRef path, Fsa::StateId start, Fsa::StateId end, Path::Type type) {
    return (path -> getStart () == start
	    && path -> getEnd () == end
	    && path -> type () == type);
  }
  bool hash_equal (ConstPathRef p1, ConstPathRef p2) {
    return (p1 -> getStart () == p2 -> getStart ()
	    && p1 -> getEnd () == p2 -> getEnd ()
	    && p1 -> type () == p2 -> type ());
  }

  CellHash::Cursor l_hash_key (Fsa::StateId start, Path::Type type) {
    return 2239 * start + type;
  }
  CellHash::Cursor l_hash_key (ConstPathRef path) {
    return l_hash_key (path -> getStart (), path -> type ());
  }
  bool l_hash_equal (ConstPathRef path, Fsa::StateId start, Path::Type type) {
    return (path -> getStart () == start
	    && path -> type () == type);
  }

  /**********************************************************************/

  CellHash::CellHash () {
    bins_.resize (10, Core::Type <CellHash::Cursor>::max);
    lbins_.resize (10, Core::Type <CellHash::Cursor>::max);
  }

  // Resets the bins_ vector and grows it to the requested size.  Iterates over
  // elements_ and computes the new hash_key for each, linking together elements
  // with identical keys.
  void CellHash::resize (CellHash::Cursor size) {
    std::fill (bins_.begin (), bins_.end (), Core::Type <CellHash::Cursor>::max);
    bins_.grow (size, Core::Type <CellHash::Cursor>::max);
    size = bins_.size ();
    for (std::vector <CellHash::Element>::iterator i = elements_.begin (); i != elements_.end (); ++ i) {
      CellHash::Cursor key = hash_key (i -> path_) % size;
      i -> next_ = bins_ [key];
      bins_ [key] = i - elements_.begin ();
    }
  }
  // Resets the lbins_ vector and grows it to the requested size.  Iterates over
  // elements_ and computes the new l_hash_key for each, linking together
  // elements with identical keys.
  void CellHash::lresize (CellHash::Cursor size) {
    std::fill (lbins_.begin (), lbins_.end (), Core::Type <CellHash::Cursor>::max);
    lbins_.grow (size, Core::Type <CellHash::Cursor>::max);
    size = lbins_.size ();
    for (std::vector <CellHash::Element>::iterator i = elements_.begin (); i != elements_.end (); ++ i) {
      CellHash::Cursor key = l_hash_key (i -> path_) % size;
      i -> lnext_ = lbins_ [key];
      lbins_ [key] = i - elements_.begin ();
    }
  }

  bool CellHash::empty () const {
    return elements_.empty ();
  }

  size_t CellHash::size () const {
    return elements_.size ();
  }

  void CellHash::clear () {
    std::fill (bins_.begin (), bins_.end (), Core::Type <CellHash::Cursor>::max);
    std::fill (lbins_.begin (), lbins_.end (), Core::Type <CellHash::Cursor>::max);
    elements_.clear ();
  }

  // Inserts the given path and returns false unless it is hash_equal to a path
  // that already exists, in which case does nothing and returns true.
  bool CellHash::insert (ConstPathRef path) {
    CellHash::Cursor key = hash_key (path),
      i = bins_ [key % bins_.size ()];
    for (; (i != Core::Type <CellHash::Cursor>::max) && (! hash_equal (elements_ [i].path_, path));
	 i = elements_[i].next_);
    if (i != Core::Type <CellHash::Cursor>::max) {
      return true;
    } else {
      if (elements_.size () > 2 * bins_.size ()) {
	resize (2 * bins_.size () - 1);
      }
      if (elements_.size () > lbins_.size () * lbins_.size ()) {
	lresize (2 * lbins_.size () - 1);
      }
      i = elements_.size ();
      key = key % bins_.size ();
      CellHash::Cursor lkey = l_hash_key (path) % lbins_.size ();
      elements_.push_back (Element (bins_ [key], lbins_ [lkey], path));
      bins_ [key] = lbins_ [lkey] = i;
      return false;
    }
  }

  // Returns an iterator pointing to the first element with the given (start,
  // end) states and type, if it exists, or the end of the vector if not.
  CellHash::iterator CellHash::find (Fsa::StateId start, Fsa::StateId end, Path::Type type) {
    CellHash::Cursor i = bins_ [hash_key (start, end, type) % bins_.size ()];
    for (; (i != Core::Type <CellHash::Cursor>::max) && (! hash_equal (elements_ [i].path_, start, end, type));
	 i = elements_ [i].next_);
    if (i == Core::Type <CellHash::Cursor>::max) {
      return CellHash::iterator (elements_.end ());
    } else {
      return CellHash::iterator (elements_.begin () + i);
    }
  }

  // Returns a const_iterator pointing to the first element with the given
  // (start, end) states and type, if it exists, or the end of the vector if
  // not.
  CellHash::const_iterator CellHash::find (Fsa::StateId start, Fsa::StateId end, Path::Type type) const {
    CellHash::Cursor i = bins_ [hash_key (start, end, type) % bins_.size ()];
    for (; (i != Core::Type <CellHash::Cursor>::max) && (! hash_equal (elements_ [i].path_, start, end, type));
	 i = elements_ [i].next_);
    if (i == Core::Type <CellHash::Cursor>::max) {
      return CellHash::const_iterator (elements_.end ());
    } else {
      return CellHash::const_iterator (elements_.begin () + i);
    }
  }

  CellHash::const_iterator CellHash::begin () const {
    return CellHash::const_iterator (elements_.begin ());
  }

  CellHash::const_iterator CellHash::end () const {
    return CellHash::const_iterator (elements_.end ());
  }

  // Returns a const_iterator pointing to the first element with the given start
  // state and type, if it exists, or the end of the vector if not.
  CellHash::const_iterator CellHash::lower_bound (Fsa::StateId start, Path::Type type) const {
    CellHash::Cursor i = lbins_ [l_hash_key (start, type) % lbins_.size ()];
    for (; (i != Core::Type <CellHash::Cursor>::max) && (! l_hash_equal (elements_[i].path_, start, type));
	 i = elements_ [i].lnext_);
    if (i == Core::Type <CellHash::Cursor>::max) {
      return CellHash::end ();
    } else {
      return CellHash::const_iterator (elements_.begin () + i);
    }
  }

  // Returns a const_iterator pointing to the next element with the given start
  // state and type, if it exists, or the end of the vector if not, given an
  // existing iterator.
  CellHash::const_iterator CellHash::next (CellHash::const_iterator iter, Fsa::StateId start, Path::Type type) const {
    CellHash::Cursor i = iter -> lnext_;
    for (; (i != Core::Type <CellHash::Cursor>::max) && (! l_hash_equal (elements_[i].path_, start, type));
	 i = elements_[i].lnext_);
    if (i == Core::Type <CellHash::Cursor>::max) {
      return CellHash::end ();
    } else {
      return CellHash::const_iterator (elements_.begin () + i);
    }
  }
}
