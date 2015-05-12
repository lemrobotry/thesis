#include <algorithm>
#include "Cell.hh"
#include "kBestChart.hh"
#include "Each.hh"

namespace Permute {

  // Outputs the following information about a vertex:
  //
  //   ((b,e) ->/<- (s,t)) |D| |C|
  //
  // where (b,e) is the span of the permutation covered, the arrow indicates
  // keep or swap, (s,t) is the source and target states of the path, and D and
  // C  are the set of derivations and candidates, respectively.
  std::ostream & operator << (std::ostream & out, const Vertex & v) {
    out << "(("
	<< v.begin << ", " << v.end << ")"
	<< (v.swap ? " <- " : " -> ") << "("
	<< v.source << ", " << v.target << "))"
	<< " |D| = " << v.derivations.size ()
	<< " |C| = " << v.candidates.size ()
	<< (v.initialized ? " init" : " un");
    return out;
  }

  // If the derivation is a leaf node, simply outputs the source and target
  // states of the path:
  //
  //   Dbp: (s, t) 
  //
  // Otherwise, outputs the three positions indicating the pair of spans the
  // derivation joins, the pair of indices indicating the ranks of the child
  // derivations, a type (KEEP or SWAP), and the source and target states of the
  // path:
  //
  //   Dbp: (i, j, k) [r_1, r_2] KEEP/SWAP (s, t)
  std::ostream & operator << (std::ostream & out, const Dbp & ej) {
    if (ej.t [0] == 0) {
      out << "Dbp: (" << ej.path -> getStart () << ", " << ej.path -> getEnd () << ")";
    } else {
      out << "Dbp: (";
      if (ej.swap) {
	out << ej.t [0] -> end << ", " << ej.t [0] -> begin << ", " << ej.t [1] -> begin;
      } else {
	out << ej.t [0] -> begin << ", " << ej.t [0] -> end << ", " << ej.t [1] -> end;
      }
      out << ") "
	  << "[" << ej.j [0] << ", " << ej.j [1] << "] "
	  << (ej.swap ? "SWAP " : "KEEP ")
	  << "(" << ej.path -> getStart () << ", " << ej.path -> getEnd () << ")";
    }
    return out;
  }

  // For each cell in the Chart, prints the coordinates of the cell and the
  // number of paths in the cell:
  //
  //   [i, j]: |P|
  std::ostream & operator << (std::ostream & out, ChartRef chart) {
    int N = chart -> getLength ();
    for (int i = 0; i < N; ++ i) {
      for (int j = i + 1; j <= N; ++ j) {
	ConstCellRef cell = chart -> getConstCell (i, j);
	out << "[" << i << ", " << j << "]: " << cell -> size () << std::endl;
      }
    }
    return out;
  }

  /**********************************************************************/

  kBestChart::kBestChart (ChartRef chart, ParseControllerRef controller, ScorerRef scorer) :
    chart_ (chart),
    controller_ (controller),
    scorer_ (scorer),
    vertices_ ()
  {}

  kBestChart::~kBestChart () {
    for (Vertices::const_iterator i = vertices_.begin (); i != vertices_.end (); ++ i) {
      Vertex * v = (* i);
      delete v;
    }
  }

  void kBestChart::permute () {
    Chart::permute (chart_, controller_, scorer_);
  }

  // Creates a vertex called top with (start, end) pair (0, N).  Iterates over
  // (source, target, type) tuples in the top cell of the chart.  For each such
  // path, calls getCandidates on the top vertex to add the best derivations for
  // that (source, target, type) tuple to the candidates queue.
  //
  // Next, calls lazyKthBest with i = k - 1 to compute the k best derivations
  // with any (source, target, type) tuple.
  //
  // Finally, iterates over the derivations and puts their paths in a vector to
  // return.
  kBestChart::Paths kBestChart::best (int k) {
    // Resets the vertex table.
    for (Vertices::const_iterator i = vertices_.begin (); i != vertices_.end (); ++ i) {
      Vertex * v = (* i);
      delete v;
    }
    vertices_.clear ();
    // The top cell requires special handling because there are multiple
    // possible final states, in general.  For now, simply calls getCandidates
    // multiple times with different target states.
    Vertex top (0, chart_ -> getLength ());
    ConstCellRef topCell = chart_ -> getConstCell (top.begin, top.end);
    for (Cell::PathIterator path = topCell -> begin (); path != topCell -> end (); ++ path) {
      top.source = (* path) -> getStart ();
      top.target = (* path) -> getEnd ();
      top.swap = ((* path) -> type () == Path::SWAP);
      getCandidates (top, k);
    }
    // Finds the best paths.
    lazyKthBest (top, k - 1, k);
    // Extracts the best paths.
    Paths paths;
    for (std::vector <Dbp>::const_iterator d = top.derivations.begin (); d != top.derivations.end (); ++ d) {
      paths.push_back (d -> path);
    }
    // Returns the best paths.
    return paths;
  }

  // Tries to extend the given vertex to contain an ith (counting from zero)
  // derivation, with a maximum of k.
  //
  // If the vertex has not been initialized with a candidates queue, calls
  // getCandidates with the vertex.
  //
  // If the vertex represents a leaf node---no expansion is possible---simply
  // returns whether the vertex has more than i possible derivations.
  //
  // Otherwise, calls lazyNext on the most recent derivation and pops the next
  // derivation off the candidates queue until the ith derivation has been
  // computed.
  //
  // Returns true if the vertex has an ith derivation, and false if not.
  bool kBestChart::lazyKthBest (const Vertex & v, int i, int k) {
    if (! v.initialized) {
      getCandidates (v, k);
    }
    // Leaf nodes cannot be expanded.
    if (v.end - v.begin == 1) {
      return v.derivations.size () > i;
    }
    while (v.derivations.size () <= i) {
      if (v.derivations.size () > 0) {
	lazyNext (v, v.derivations.back (), k);
      }
      if (! v.candidates.empty ()) {
	v.derivations.push_back (v.candidates.top ());
	v.candidates.pop ();
      } else {
	return false;
      }
    }
    return true;
  }

  // Extends the given derivation by extending each of its children and putting
  // both new derivations into the given vertex's candidates queue.
  //
  // For both the first and second child of the given derivation, calls
  // lazyKthBest to ensure that child has computed the necessary number of
  // items, then inserts the Dbp derived from the given one by adding one to the
  // appropriate index.
  void kBestChart::lazyNext (const Vertex & v, const Dbp & ej, int k) {
    for (int i = 0; i < 2; ++ i) {
      if (lazyKthBest (* (ej.t [i]), ej.j [i] + 1, k)) {
	v.candidates.insertOrRelax (Dbp (ej, i));
      }
    }
  }

  // Iterates over paths of a given type in the left cell with the given source
  // state.  Looks for paths in the right cell of a given type whose source
  // state matches the left path's target state and whose target state matches
  // the given target state.
  //
  // For each such right path, calls lazyKthBest on both the left and right
  // corresponding vertices with i = 0 to ensure that at least one derivation
  // exists, then adds the resulting derivation to the given list of Dbps.
  void kBestChart::getCandidatesHelper (std::vector <Dbp> & temp, int k,
					ConstCellRef left,
					ConstCellRef right,
					Fsa::StateId lsource,
					Fsa::StateId rtarget,
					Path::Type ltype,
					Path::Type rtype,
					size_t lbegin, size_t lend,
					size_t rbegin, size_t rend,
					double score,
					bool swap) {
    for (Cell::PathIterator lpath = left -> begin (lsource, ltype);
	 lpath != left -> end ();
	 lpath = left -> next (lpath, lsource, ltype)) {
      Fsa::StateId via = (* lpath) -> getEnd ();
      Cell::PathIterator rpath = right -> find (via, rtarget, rtype);
      if (rpath != right -> end ()) {
	// Get vertices (lbegin, lend, lsource, via) and (rbegin, rend, via, rtarget)
	const Vertex
	  * lv = getVertex (lbegin, lend, (* lpath) -> type () == Path::SWAP, lsource, via),
	  * rv = getVertex (rbegin, rend, (* rpath) -> type () == Path::SWAP, via, rtarget);
	assert (lazyKthBest (* lv, 0, k));
	assert (lazyKthBest (* rv, 0, k));
	temp.push_back (Dbp (lv, rv, score, swap));
      }
    }
  }					

  // Initializes the given vertex's candidates queue.
  //
  // First, sets the vertex to initialized.
  //
  // If the vertex corresponds to a leaf node, simply extracts a set of
  // derivations from the chart's cell, initialized using
  // Dbp::Dbp(ConstPathRef).  Sorts the derivations to make them a k-best list.
  //
  // Otherwise, iterates over all split positions allowed by the ParseController
  // and populates a vector with all possible derivation types using
  // getCandidatesHelper.
  //
  // If the vertex swaps, then normal form requires that its right child (left in
  // the tree) is not a swap node, but allows either type for the left child.
  //
  // If the vertex keeps it children in order, then normal form requires that
  // its right child is not a keep node, but allows either type for the left
  // child.
  void kBestChart::getCandidates (const Vertex & v, int k) {
    v.initialized = true;
    // Base Case
    if (v.end - v.begin == 1) {
      ConstCellRef cell = chart_ -> getConstCell (v.begin, v.end);
      for (Cell::PathIterator path = cell -> begin (v.source, Path::NEITHER);
	   path != cell -> end ();
	   path = cell -> next (path, v.source, Path::NEITHER)) {
	v.derivations.push_back (Dbp (* path));
      }
      std::sort (v.derivations.begin (), v.derivations.end ());
      return;
    }
    // Recursive Case
    std::vector <Dbp> temp;
    for (ParseController::iterator middle = controller_ -> begin (v.begin, v.end);
	 middle != controller_ -> end (v.begin, v.end); ++ middle) {
      ConstCellRef
	left = chart_ -> getConstCell (v.begin, middle),
	right = chart_ -> getConstCell (middle, v.end);
      if (v.swap) {
	// Swap
	Path::Type rtype = (middle - v.begin == 1) ? Path::NEITHER : Path::KEEP;
	double score = scorer_ -> score (v.end, middle, v.begin);
	if (v.end - middle == 1) {
	  getCandidatesHelper (temp, k, right, left, v.source, v.target, Path::NEITHER, rtype,
			       middle, v.end, v.begin, middle, score, true);
	} else {
	  getCandidatesHelper (temp, k, right, left, v.source, v.target, Path::KEEP, rtype,
			       middle, v.end, v.begin, middle, score, true);
	  getCandidatesHelper (temp, k, right, left, v.source, v.target, Path::SWAP, rtype,
			       middle, v.end, v.begin, middle, score, true);
	}
      } else {
	// Keep
	Path::Type rtype = (v.end - middle == 1) ? Path::NEITHER : Path::SWAP;
	double score = scorer_ -> score (v.begin, middle, v.end);
	if (middle - v.begin == 1) {
	  getCandidatesHelper (temp, k, left, right, v.source, v.target, Path::NEITHER, rtype,
			       v.begin, middle, middle, v.end, score, false);
	} else {
	  getCandidatesHelper (temp, k, left, right, v.source, v.target, Path::KEEP, rtype,
			       v.begin, middle, middle, v.end, score, false);
	  getCandidatesHelper (temp, k, left, right, v.source, v.target, Path::SWAP, rtype,
			       v.begin, middle, middle, v.end, score, false);
	}
      }
    }
    // Quick select
    std::nth_element (temp.begin (), temp.begin () + k, temp.end ());
    std::for_each (temp.begin (), std::min (temp.begin () + k, temp.end ()),
		   each_fun (& v.candidates,
			     & Core::TracedPriorityQueue <Dbp, Dbp, DbpIdentity, DbpLess, DbpHash>::insert));
//     for (std::vector <Dbp>::const_iterator i = temp.begin ();
// 	 (i != temp.end ()) && (i < temp.begin () + k);
// 	 ++ i) {
//       v.candidates.insert (* i);
//     }
  }

  // Gets an existing vertex with the given 5-tuple from the hash table or
  // creates a new one if none exists.
  const Vertex * kBestChart::getVertex (int begin, int end, bool swap, Fsa::StateId source, Fsa::StateId target) {
    Vertex * v = new Vertex (begin, end, swap, source, target);
    InsertPair p = vertices_.insertExisting (v);
    assert (! p.second || vertices_ [p.first] -> initialized);
    if (p.second) {
      delete v;
    }
    return vertices_ [p.first];
  }

  /**********************************************************************/

  Dbp::Dbp () :
    constit (0.0),
    swap (false)
  {
    for (int i = 0; i < 2; ++ i) {
      t [i] = 0;
      j [i] = 0;
    }
  }

  // Constructs a first derivation (j[0] == j[1] == 0) from the given pair of
  // vertices with the given constituent score and swap value.  This
  // derivation's path connects the best paths of the two children.
  Dbp::Dbp (const Vertex * t1, const Vertex * t2, double constit, bool swap) :
    constit (constit),
    swap (swap)
  {
    t [0] = t1;
    t [1] = t2;
    j [0] = j [1] = 0;
    path = Path::connect (t1 -> derivations [0].path,
			  t2 -> derivations [0].path,
			  constit,
			  swap);
    assert (path -> normal ());
  }

  // Constructs a leaf derivation from a path.  The Vertex array consists of
  // null pointers.
  Dbp::Dbp (ConstPathRef path) :
    path (path),
    constit (0.0),
    swap (false)
  {
    t [0] = t [1] = 0;
    j [0] = j [1] = 0;
  }

  // Constructs a new derivation from a given one by taking the next derivation
  // from the child indicated by the given index.  This derivation's path
  // connects the appropriate paths from the children.
  Dbp::Dbp (const Dbp & dbp, int index) :
    constit (dbp.constit),
    swap (dbp.swap)
  {
    for (int i = 0; i < 2; ++ i) {
      t [i] = dbp.t [i];
      j [i] = dbp.j [i] + (i == index ? 1 : 0);
    }
    path = Path::connect (t [0] -> derivations [j [0]].path,
			  t [1] -> derivations [j [1]].path,
			  constit,
			  swap);
    assert (path -> normal ());
  }

  // One Dbp is better than another if its path has a higher score.  Because
  // sometimes paths may have identical scores, we must also compare the j
  // values of Dbps in this case.
  bool Dbp::operator < (const Dbp & other) const {
    if (path -> getScore () == other.path -> getScore ()) {
      if (j [0] == other.j [0]) {
	return j [1] < other.j [1];
      } else {
	return j [0] < other.j [0];
      }
    } else {
      return path -> getScore () > other.path -> getScore ();
    }
  }

  // Checks only those attributes not derived from the others.
  bool Dbp::operator == (const Dbp & other) const {
    return t [0] == other.t [0]
      && t [1] == other.t [1]
      && j [0] == other.j [0]
      && j [1] == other.j [1]
      && swap == other.swap;
  }
}
