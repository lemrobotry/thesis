#ifndef _PERMUTE_K_BEST_CHART_HH
#define _PERMUTE_K_BEST_CHART_HH

#include <Core/PriorityQueue.hh>
#include <Fsa/Hash.hh>
#include "Chart.hh"
#include "Path.hh"

namespace Permute {

  class Vertex;

  // A derivation with back-pointers (Huang and Chiang, 2005) \hat{D} of v is a
  // tuple <e,j> such that e \in BS(v), and j \in {1,2,...,k}^{|e|}.  There is a
  // one-to-one correspondence ~ between dbps of v and derivations of v.
  //
  // Dbp represents the hyperarc e \in BS(v) as a pair of Vertex t[0] and t[1],
  // and j[0] and j[1] are the ranks of the derivations at those vertices.
  //
  // It also holds a path---the Permute internal representation of the
  // derivation---, a constituent score, and a Boolean indicating whether to
  // keep or swap the order of the children.
  class Dbp {
  public:
    const Vertex * t [2];
    int j [2];

    ConstPathRef path;
    double constit;
    bool swap;

    Dbp ();
    Dbp (const Vertex *, const Vertex *, double, bool);
    Dbp (ConstPathRef path);
    // Copy a Dbp but increment one index.
    Dbp (const Dbp &, int);
    bool operator < (const Dbp &) const;
    bool operator == (const Dbp &) const;
  };

  struct DbpIdentity {
    const Dbp & operator () (const Dbp & dbp) const { return dbp; }
  };
  struct DbpLess {
    bool operator () (const Dbp & one, const Dbp & two) const { return (one < two); }
  };
  struct DbpHash {
    size_t operator () (const Dbp & dbp) const {
      size_t h = size_t (dbp.swap);
      for (int i = 0; i < 2; ++ i) {
	h = 2239 * h + 5 * size_t (dbp.t [i]) + size_t (dbp.j [i]);
      }
      return h;
    }
  };

  /**********************************************************************/

  // A Vertex corresponds to a particular (begin, end, swap, source, target).
  // It holds a list of Dbps corresponding to derivations already expanded, and
  // a priority queue of candidates. 
  class Vertex {
  public:
    int begin, end;
    bool swap;
    Fsa::StateId source, target;
    mutable std::vector <Dbp> derivations;
    mutable Core::TracedPriorityQueue <Dbp, Dbp, DbpIdentity, DbpLess, DbpHash> candidates;
    mutable bool initialized;

    Vertex (int begin, int end, bool swap = false, Fsa::StateId source = Fsa::InvalidStateId, Fsa::StateId target = Fsa::InvalidStateId) :
      begin (begin),
      end (end),
      swap (swap),
      source (source),
      target (target),
      derivations (),
      candidates (),
      initialized (false)
    {}
  };

  struct VertexEqual {
    bool operator () (const Vertex * one, const Vertex * two) const {
      return one -> begin == two -> begin
	&& one -> end == two -> end
	&& one -> swap == two -> swap
	&& one -> source == two -> source
	&& one -> target == two -> target;
    }
  };
  struct VertexHash {
    size_t operator () (const Vertex * v) const {
      return 5 * (2239 * (5 * v -> begin + v -> end) + v -> source) + v -> target + v -> swap;
    }
  };

  /**********************************************************************/

  // A kBestChart wraps a Chart.  It provides a permute method, which simply
  // dispatches to Chart::permute, and a best method, which extracts as many
  // paths as requested.
  class kBestChart {
  private:
    ChartRef chart_;
    ParseControllerRef controller_;
    ScorerRef scorer_;
  public:
    typedef Fsa::Hash <Vertex *, VertexHash, VertexEqual> Vertices;
    typedef std::pair <Vertices::Cursor, bool> InsertPair;
    typedef std::vector <ConstPathRef> Paths;
  private:
    Vertices vertices_;
  private:
    bool lazyKthBest (const Vertex &, int, int);
    void lazyNext (const Vertex &, const Dbp &, int);
    void getCandidates (const Vertex &, int);
    void getCandidatesHelper (std::vector <Dbp> &, int, ConstCellRef, ConstCellRef, Fsa::StateId, Fsa::StateId, Path::Type, Path::Type, size_t, size_t, size_t, size_t, double, bool);
    const Vertex * getVertex (int, int, bool, Fsa::StateId, Fsa::StateId);
  public:
    kBestChart (ChartRef, ParseControllerRef, ScorerRef);
    ~ kBestChart ();
    void permute ();
    Paths best (int);
  };
}

#endif//_PERMUTE_K_BEST_CHART_HH
