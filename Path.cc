#include "Path.hh"
#include "PathVisitor.hh"

#include <Core/Types.hh>
#include <Fsa/Automaton.hh>

namespace Permute {

  Arc::Arc (size_t index, Fsa::StateId source, Fsa::StateId target, double score) :
    PathImpl (score),
    original_index_ (index),
    source_ (source),
    target_ (target)
  {}

  size_t Arc::getIndex () const {
    return original_index_;
  }

  Fsa::StateId Arc::getStart () const {
    return source_;
  }
  Fsa::StateId Arc::getEnd () const {
    return target_;
  }
  void Arc::accept (PathVisitor * visitor) const {
    visitor -> visit (this);
  }

  /**********************************************************************/
  
  PathComposite::PathComposite (const ConstPathRef & left, const ConstPathRef & right, double score) :
    PathImpl (left -> getScore () + right -> getScore () + score),
    left_ (left),
    right_ (right)
  {}
  ConstPathRef PathComposite::getLeft () const {
    return left_;
  }
  ConstPathRef PathComposite::getRight () const {
    return right_;
  }
  Fsa::StateId PathComposite::getStart () const {
    return left_ -> getStart ();
  }
  Fsa::StateId PathComposite::getEnd () const {
    return right_ -> getEnd ();
  }
  void PathComposite::accept (PathVisitor * visitor) const {
    visitor -> visit (this);
  }

  // A KeepNode is a composite path that preserves the order of its two
  // children.  It is normal if its right child is not also a KeepNode.
  class KeepNode : public PathComposite {
  public:
    KeepNode (const ConstPathRef & left, const ConstPathRef & right, double score) :
      PathComposite (left, right, score)
    {}
    virtual Type type () const { return KEEP; }
    virtual bool normal () const { return right_ -> type () != KEEP; }
  };

  // A SwapNode is a composite path that reverses the order of its two
  // children.  It is normal if its right child in the resulting permutation
  // (which is its left child in the tree) is not also a SwapNode.
  class SwapNode : public PathComposite {
  public:
    SwapNode (const ConstPathRef & left, const ConstPathRef & right, double score) :
      PathComposite (left, right, score)
    {}
    virtual Type type () const { return SWAP; }
    virtual bool normal () const { return right_ -> type () != SWAP; }
  };
  
  /**********************************************************************/
  
  // A DummyPath is a path with no symbols used mostly for algorithmic and
  // testing purposes.
  class DummyPath : public Path {
  private:
    Fsa::StateId start_, end_;
    double weight_;
  public:
    DummyPath (Fsa::StateId start = Fsa::InvalidStateId,
	       Fsa::StateId end = Fsa::InvalidStateId,
	       double weight = Core::Type <double>::min) :
      Path (),
      start_ (start),
      end_ (end),
      weight_ (weight)
    {}
    Fsa::StateId getStart () const { return start_; }
    Fsa::StateId getEnd () const { return end_; }
    double getScore () const { return weight_; }
    void accept (PathVisitor * visitor) const {}
  };

  /**********************************************************************/

  // A FinalPath is a path of length zero that applies a state's final weight.
  // Its end state is Fsa::InvalidStateId, which stands in for a super-final
  // state.
  class FinalPath : public PathImpl {
  private:
    Fsa::StateId state_;
  public:
    FinalPath (Fsa::StateId state, double score) :
      PathImpl (score),
      state_ (state)
    {}
    Fsa::StateId getStart () const { return state_; }
    Fsa::StateId getEnd () const { return Fsa::InvalidStateId; }
    void accept (PathVisitor * visitor) const {}
  };

  /**********************************************************************/

  // An AddScorePath is a path with an additional score applied.  It computes
  // it's wrapped path's score at construction time and returns the resulting
  // sum from getScore.
  class AddScorePath : public PathDecorator {
  private:
    double score_;
  public:
    AddScorePath (const ConstPathRef & path, double score) :
      PathDecorator (path),
      score_ (path -> getScore () + score)
    {}
    virtual double getScore () const { return score_; }
  };

  /**********************************************************************/

  ConstPathRef Path::nullPath () {
    static ConstPathRef nullPath_ (new DummyPath);
    return nullPath_;
  }
  
  ConstPathRef Path::lowerBound (Fsa::StateId start) {
    return ConstPathRef (new DummyPath (start, Core::Type <Fsa::StateId>::min));
  }

  ConstPathRef Path::rLowerBound (Fsa::StateId end) {
    return ConstPathRef (new DummyPath (Core::Type <Fsa::StateId>::min, end));
  }

  ConstPathRef Path::upperBound (Fsa::StateId start) {
    return ConstPathRef (new DummyPath (start, Core::Type <Fsa::StateId>::max));
  }

  ConstPathRef Path::rUpperBound (Fsa::StateId end) {
    return ConstPathRef (new DummyPath (Core::Type <Fsa::StateId>::max, end));
  }

  ConstPathRef Path::forSearch (Fsa::StateId start, Fsa::StateId end) {
    return ConstPathRef (new DummyPath (start, end));
  }

  ConstPathRef Path::connect (const ConstPathRef & left, const ConstPathRef & right, double score, bool swap) {
    if (swap) {
      return ConstPathRef (new SwapNode (left, right, score));
    } else {
      return ConstPathRef (new KeepNode (left, right, score));
    }
  }

  ConstPathRef Path::arc (size_t index, Fsa::StateId start, Fsa::StateId end, double score) {
    return ConstPathRef (new Arc (index, start, end, score));
  }

  ConstPathRef Path::epsilon (Fsa::StateId start, Fsa::StateId end, double score) {
    return ConstPathRef (new DummyPath (start, end, score));
  }

  ConstPathRef Path::final (Fsa::StateId state, double score) {
    return ConstPathRef (new FinalPath (state, score));
  }

  ConstPathRef Path::addScore (const ConstPathRef & path, double score) {
    return ConstPathRef (new AddScorePath (path, score));
  }

  /**********************************************************************/

  // One path is less than the other if it start state precedes the other's, or if
  // the start states are identical and its end state precedes the other's.
  bool Path::LessThan::operator () (const ConstPathRef & one, const ConstPathRef & two) const {
    if (one -> getStart () == two -> getStart ()) {
      return one -> getEnd () < two -> getEnd ();
    } else {
      return one -> getStart () < two -> getStart ();
    }
  }

  // From the reverse perspective, end states take precedence over start states.
  bool Path::rLessThan::operator () (const ConstPathRef & one, const ConstPathRef & two) const {
    if (one -> getEnd () == two -> getEnd ()) {
      return one -> getStart () < two -> getStart ();
    } else {
      return one -> getEnd () < two -> getEnd ();
    }
  }

  /**********************************************************************/

  PathImpl::PathImpl (double score) :
    Path (),
    score_ (score)
  {}

  double PathImpl::getScore () const {
    return score_;
  }

  /**********************************************************************/

  PathDecorator::PathDecorator (const ConstPathRef & decorated) :
    Path (),
    decorated_ (decorated)
  {}

  Fsa::StateId PathDecorator::getStart () const {
    return decorated_ -> getStart ();
  }

  Fsa::StateId PathDecorator::getEnd () const {
    return decorated_ -> getEnd ();
  }

  double PathDecorator::getScore () const {
    return decorated_ -> getScore ();
  }

  void PathDecorator::accept (PathVisitor * visitor) const {
    decorated_ -> accept (visitor);
  }
}
