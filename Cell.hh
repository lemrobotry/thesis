#ifndef _PERMUTE_CELL_HH
#define _PERMUTE_CELL_HH

#include <Core/ReferenceCounting.hh>
#include "CellHash.hh"
#include "Path.hh"

namespace Permute {
  class Cell;
  typedef Core::Ref <Cell> CellRef;
  typedef Core::Ref <const Cell> ConstCellRef;

  // Cell is an abstract base class that specifies the interface for a
  // collection of paths covering the same span in a permutation.  It provides
  // iteration over all paths of a given type covering the span (using begin and
  // end), all paths with the same start state (using begin and next), and paths
  // with a given start and end state (using find).
  //
  // The add and build methods construct cells over wider spans from existing
  // cells, and getBestPath returns the best path in the cell.
  class Cell : public Core::ReferenceCounted {
  public:
    typedef CellHash::const_iterator PathIterator;

    virtual ~Cell () {};
    virtual bool empty () const = 0;
    virtual size_t size () const = 0;
    virtual void clear () = 0;

    virtual PathIterator begin () const = 0;
    virtual PathIterator end () const = 0;
    virtual PathIterator begin (Fsa::StateId start, Path::Type) const = 0;
    virtual PathIterator next (PathIterator, Fsa::StateId, Path::Type) const = 0;
    
    virtual PathIterator find (Fsa::StateId, Fsa::StateId, Path::Type) const = 0;

    virtual bool add (ConstPathRef) = 0;
    static void build (CellRef, ConstCellRef, ConstCellRef, double, bool, Path::Type);
    static ConstPathRef getBestPath (ConstCellRef);
  };

  // CellImpl implements the Cell interface using an instance of CellHash to
  // store and retrieve the paths.
  class CellImpl : public Cell {
  protected:
    CellHash paths_;
  public:
    CellImpl ();
    virtual bool empty () const;
    virtual size_t size () const;
    virtual void clear ();
    virtual PathIterator begin () const;
    virtual PathIterator end () const;
    virtual PathIterator begin (Fsa::StateId, Path::Type) const;
    virtual PathIterator next (PathIterator, Fsa::StateId, Path::Type) const;
    virtual PathIterator find (Fsa::StateId, Fsa::StateId, Path::Type) const;
    virtual bool add (ConstPathRef) = 0;
  };

  // CellDecorator implements the Cell interface using a slave instance of
  // Cell.  All the methods pass unchanged in this superclass so that subclasses
  // can implement only those methods they specialize.
  class CellDecorator : public Cell {
  protected:
    CellRef decorated_;
  public:
    CellDecorator (CellRef);
    virtual bool empty () const;
    virtual size_t size () const;
    virtual void clear ();
    virtual PathIterator begin () const;
    virtual PathIterator end () const;
    virtual PathIterator begin (Fsa::StateId, Path::Type) const;
    virtual PathIterator next (PathIterator, Fsa::StateId, Path::Type) const;
    virtual PathIterator find (Fsa::StateId, Fsa::StateId, Path::Type) const;
    virtual bool add (ConstPathRef);
  };
}

#endif//_PERMUTE_CELL_HH
