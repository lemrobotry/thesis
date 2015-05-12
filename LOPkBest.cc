#include "LOPkBest.hh"

namespace Permute {
  namespace LOP {

    Vertex::Vertex () :
      derivations_ (),
      candidates_ ()
    {}

    // Adapted from kBestChart::lazyKthBest.  There is no need to check
    // initialization, because that is the responsibility of the parser.  There
    // is no need to explicitly check for a leaf node, because those will have
    // empty candidates lists.  The rest is identical.
    bool Vertex::lazyKthBest (int k) {
      while (derivations_.size () <= k) {
	if (! derivations_.empty ()) {
	  derivations_.back ().lazyNext (* this);
	}
	if (! candidates_.empty ()) {
	  derivations_.push_back (candidates_.top ());
	  candidates_.pop ();
	} else {
	  return false;
	}
      }
      return true;
    }

    void Vertex::insert (const Derivation & d) {
      candidates_.insertOrRelax (d);
    }

    ////////////////////////////////////////////////////////////////////////////////

    // Utility function called by Derivation::Derivation.
    ConstPathRef connect (const ConstPathRef & left,
			  const ConstPathRef & right,
			  double score,
			  Derivation::Type type) {
      if (type == Derivation::KEEP) {
	return Path::connect (left, right, score, false);
      } else {
	return Path::connect (right, left, score, true);
      }
    }

    Derivation::Derivation () :
      type_ (LEAF),
      derivation_ (0.0)
    {}

    Derivation::Derivation (const BackPointer & left,
			    const BackPointer & right,
			    double derivation,
			    Type type) :
      children_ (std::make_pair (left, right)),
      type_ (type),
      derivation_ (derivation),
      path_ (connect (left.path (), right.path (), derivation, type))
    {}
      
    Derivation::Derivation (ConstPathRef path) :
      type_ (LEAF),
      derivation_ (0.0),
      path_ (path)
    {}

    // Inserts the pair of successors of this Derivation's children into the
    // given Vertex.  If this is a leaf Derivation, does nothing.
    void Derivation::lazyNext (Vertex & v) const {
      if (type_ != LEAF) {
	if (children_.first.hasSuccessor ()) {
	  v.insert (Derivation (children_.first.successor (),
				children_.second,
				derivation_, type_));
	}
	if (children_.second.hasSuccessor ()) {
	  v.insert (Derivation (children_.first,
				children_.second.successor (),
				derivation_, type_));
	}
      }
    }

    // One derivation is better than another if it has a HIGHER score.  Because
    // there may be ties, we must also include other properties.  The order is
    // defined by the following tuple (total, type, first child, second child).
    bool Derivation::operator < (const Derivation & d) const {
      if (score () == d.score ()) {
	if (type_ == d.type_) {
	  if (children_.first.j == d.children_.first.j) {
	    return children_.second.j < d.children_.second.j;
	  } else {
	    return children_.first.j < d.children_.first.j;
	  }
	} else {
	  return type_ < d.type_;
	}
      } else {
	return score () > d.score ();
      }
    }

    // Two derivations are equal if they have the same type and children.  The
    // other properties are derived from these.
    bool Derivation::operator == (const Derivation & d) const {
      return type_ == d.type_
	&& children_.first == d.children_.first
	&& children_.second == d.children_.second;
    }

    ////////////////////////////////////////////////////////////////////////////////

    bool BackPointer::hasSuccessor () const {
      return t -> lazyKthBest (j + 1);
    }

    BackPointer BackPointer::successor () const {
      return BackPointer (t, j + 1);
    }

    const ConstPathRef & BackPointer::path () const {
      return t -> derivations_ [j].path ();
    }
  }
}
