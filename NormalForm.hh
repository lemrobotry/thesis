// NOTE: This class is not currently in use.  The basic cell implementations now
// include normal form as well.
//
// NormalFormCell eliminates spurious ambiguity from the parse trees in a
// standard way.  PathList enables a collection of paths with the same start and
// end state to stand in for a single path, intended for use with sampling.

#ifndef _PERMUTE_NORMAL_FORM_HH
#define _PERMUTE_NORMAL_FORM_HH

#include "Cell.hh"
#include "Chart.hh"
#include "Path.hh"
#include "PathVisitor.hh"

namespace Permute {

  class NormalFormCell : public CellImpl {
  public:
    NormalFormCell ();
    virtual bool add (ConstPathRef);
  };

  /**********************************************************************/

  class PathList;

  class NormalFormVisitor : public PathVisitor {
  public:
    virtual void visit (const PathList *) = 0;
  };

  /**********************************************************************/

  class PathList : public PathImpl {
  protected:
    ConstPathRef head_;
    ConstPathRef tail_;
  public:
    PathList (ConstPathRef, ConstPathRef);

    virtual Fsa::StateId getStart () const;
    virtual Fsa::StateId getEnd () const;
    virtual void accept (PathVisitor *) const;
    virtual Type type () const;
    virtual bool normal () const;

    void accept (NormalFormVisitor *) const;
    ConstPathRef getHead () const;
    ConstPathRef getTail () const;
  };
}

#endif//_PERMUTE_NORMAL_FORM_HH
