// Replaces map-style storage of paths in cells with hashing.  CellHash stores
// its elements in a vector, and holds two hash vectors, one for looking up
// (start, end) state pairs, and one for looking up start states only.  Each
// element stores not just its path, but also the next in the chain with the
// same (start, end) pair, and the next with the same start state.  This
// facilitates iteration over both sets of paths.

#ifndef _PERMUTE_CELL_HASH_HH
#define _PERMUTE_CELL_HASH_HH

#include <Fsa/Vector.hh>
#include "Path.hh"

namespace Permute {
  class CellHash {
  public:
    typedef u32 Cursor;
  private:
    Fsa::Vector <Cursor> bins_;
    Fsa::Vector <Cursor> lbins_;
    class Element {
    public:
      Cursor next_;
      Cursor lnext_;
      ConstPathRef path_;
    public:
      Element (Cursor next, Cursor lnext, ConstPathRef path) :
	next_ (next),
	lnext_ (lnext),
	path_ (path)
      {}
    };
    std::vector <Element> elements_;
  public:
    class iterator : public std::vector <Element>::iterator {
    public:
      iterator (std::vector <Element>::iterator i) :
	std::vector <Element>::iterator (i)
      {}
      ConstPathRef & operator * () const {
	return std::vector <Element>::iterator::operator * ().path_;
      }
    };
    class const_iterator : public std::vector <Element>::const_iterator {
    public:
      const_iterator (std::vector <Element>::const_iterator i) :
	std::vector <Element>::const_iterator (i)
      {}
      ConstPathRef operator * () const {
	return std::vector <Element>::const_iterator::operator * ().path_;
      }
    };
  public:
    CellHash ();
    bool empty () const;
    size_t size () const;
    void clear ();
    bool insert (ConstPathRef);
    iterator find (Fsa::StateId, Fsa::StateId, Path::Type type);
    const_iterator find (Fsa::StateId, Fsa::StateId, Path::Type type) const;
    const_iterator begin () const;
    const_iterator end () const;
    const_iterator lower_bound (Fsa::StateId, Path::Type type) const;
    const_iterator next (const_iterator, Fsa::StateId, Path::Type type) const;
  private:
    void resize (Cursor);
    void lresize (Cursor);
  };
}

#endif//_PERMUTE_CELL_HASH_HH
