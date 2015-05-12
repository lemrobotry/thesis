#ifndef _PERMUTE_PATH_HH
#define _PERMUTE_PATH_HH

#include <Core/ReferenceCounting.hh>
#include <Fsa/Automaton.hh>

namespace Permute {
  class PathVisitor;

  class Path;
  typedef Core::Reference <const Path> ConstPathRef;

  /**********************************************************************/
  
  // Path is an abstract base class representing a type with a start state, an
  // end state, and a score.  For use with normal-form parsing, a path also has
  // a type, and can compute whether it is normal.
  //
  // Path has numerous static methods for constructing different kinds of
  // paths.
  class Path : public Core::ReferenceCounted {
  public:
    typedef enum {
      NEITHER = 0,
      KEEP,
      SWAP
    } Type;
    Path () : Core::ReferenceCounted () {}
    virtual ~Path () {};
    virtual Fsa::StateId getStart () const = 0;
    virtual Fsa::StateId getEnd () const = 0;
    virtual double getScore () const = 0;
    virtual void accept (PathVisitor *) const = 0;
    virtual Type type () const { return NEITHER; }
    virtual bool normal () const { return true; }

    static ConstPathRef nullPath ();
    static ConstPathRef lowerBound (Fsa::StateId start);
    static ConstPathRef rLowerBound (Fsa::StateId end);
    static ConstPathRef upperBound (Fsa::StateId start);
    static ConstPathRef rUpperBound (Fsa::StateId end);
    static ConstPathRef forSearch (Fsa::StateId start, Fsa::StateId end);
    static ConstPathRef connect (const ConstPathRef &, const ConstPathRef &, double, bool);
    static ConstPathRef arc (size_t, Fsa::StateId, Fsa::StateId, double);
    static ConstPathRef epsilon (Fsa::StateId, Fsa::StateId, double);
    static ConstPathRef final (Fsa::StateId, double);
    static ConstPathRef addScore (const ConstPathRef &, double);

    class LessThan {
    public:
      bool operator () (const ConstPathRef &, const ConstPathRef &) const;
    };
    class rLessThan {
    public:
      bool operator () (const ConstPathRef &, const ConstPathRef &) const;
    };
  };

  /**********************************************************************/

  // A still abstract specification of the Path interface.  PathImpl contains a
  // score, but still has no start or end state.
  class PathImpl : public Path {
  protected:
    double score_;
  public:
    PathImpl (double);
    virtual double getScore () const;
  };

  /**********************************************************************/

  // Decorates a Path and implements the Path interface.
  class PathDecorator : public Path {
  protected:
    ConstPathRef decorated_;
    PathDecorator (const ConstPathRef &);
  public:
    virtual Fsa::StateId getStart () const;
    virtual Fsa::StateId getEnd () const;
    virtual double getScore () const;
    virtual void accept (PathVisitor *) const;
  };

  /**********************************************************************/

  // Arcs are leaf nodes in path trees.  Holds source and target states from a
  // finite-state automaton, plus an original index into the permutation.
  class Arc : public PathImpl {
  private:
    size_t original_index_;
    Fsa::StateId source_;
    Fsa::StateId target_;
  public:
    Arc (size_t, Fsa::StateId, Fsa::StateId, double);

    size_t getIndex () const;

    virtual Fsa::StateId getStart () const;
    virtual Fsa::StateId getEnd () const;
    virtual void accept (PathVisitor *) const;
  };

  /**********************************************************************/

  // PathComposites are tree nodes in path trees.  Holds two paths, the left and
  // right children.  Does not implement type and normal, which are reserved for
  // the specializations to Keep and Swap nodes. 
  class PathComposite : public PathImpl {
  protected:
    ConstPathRef left_, right_;
  public:
    PathComposite (const ConstPathRef &, const ConstPathRef &, double);
    ConstPathRef getLeft () const;
    ConstPathRef getRight () const;

    virtual Fsa::StateId getStart () const;
    virtual Fsa::StateId getEnd () const;
    virtual void accept (PathVisitor *) const;
    virtual Type type () const = 0;
    virtual bool normal () const = 0;
  };
}

#endif//_PERMUTE_PATH_HH
