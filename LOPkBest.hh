#ifndef _PERMUTE_LOP_K_BEST_HH
#define _PERMUTE_LOP_K_BEST_HH

#include <Core/PriorityQueue.hh>
#include <functional>
#include <utility>

#include "Path.hh"

namespace Permute {
  namespace LOP {

    class Vertex;

    struct BackPointer {
      Vertex * t;
      int j;
      BackPointer (Vertex * t = 0, int j = 0) :
	t (t),
	j (j)
      {}
      bool hasSuccessor () const;
      BackPointer successor () const;
      const ConstPathRef & path () const;
      bool operator == (const BackPointer & bp) const { return t == bp.t && j == bp.j; }
      size_t hashValue () const { return 5 * size_t (t) + size_t (j); }
    };

    class Derivation {
    public:
      enum Type { LEAF, KEEP, SWAP };
    private:
      std::pair <BackPointer, BackPointer> children_;
      Type type_;
      double derivation_;
      ConstPathRef path_;
    public:
      Derivation ();
      Derivation (const BackPointer &, const BackPointer &, double, Type);
      Derivation (ConstPathRef);
      
      double score () const { return path_ -> getScore (); }
      const ConstPathRef & path () const { return path_; }
      // Adds the successor of each child to the given Vertex.
      void lazyNext (Vertex & v) const;

      bool operator < (const Derivation &) const;
      bool operator == (const Derivation &) const;
      // The hash value doesn't account for the scores because they are implied
      // by the type and back pointers.
      size_t hashValue () const {
	return 2239 * (2239 * size_t (type_)
		       + children_.first.hashValue ())
	  + children_.second.hashValue ();
      }
    };

    typedef __gnu_cxx::identity <Derivation> DerivationIdentity;
    typedef std::less <Derivation> DerivationLess;
    struct DerivationHash {
      size_t operator () (const Derivation & d) const { return d.hashValue (); }
    };

    class Vertex {
    public:
      std::vector <Derivation> derivations_;
      Core::TracedPriorityQueue <Derivation,
				 Derivation,
				 DerivationIdentity,
				 DerivationLess,
				 DerivationHash> candidates_;
      Vertex ();
      // Returns whether there is a kth best derivation, indexed from 0.
      bool lazyKthBest (int k);
      void insert (const Derivation & d);
    };

  }
}

#endif//_PERMUTE_LOP_K_BEST_HH
