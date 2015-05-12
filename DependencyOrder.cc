#include "BeforeScorer.hh"
#include "DependencyOrder.hh"
#include "Iterator.hh"
#include "LinearOrdering.hh"

namespace Permute {

  // Collects the set of children of each parent from a dependency parse.
  class Children {
  private:
    std::vector <std::vector <int> > c_;
  public:
    Children (const std::vector <int> & parents);
    const std::vector <int> & operator [] (int parent) const;
    std::vector <int> & operator [] (int parent);
  };

  Children::Children (const std::vector <int> & parents) :
    c_ (parents.size () + 1)
  {
    for (int child = 0; child < parents.size (); ++ child) {
      operator [] (parents [child]).push_back (child);
    }
  }

  // Parent indices are in [-1,n), so add one to get an index in [0,n+1).
  const std::vector <int> & Children::operator [] (int p) const {
    return c_ [p + 1];
  }
  std::vector <int> & Children::operator [] (int p) {
    return const_cast <std::vector <int> &>
      (static_cast <const Children &> (* this) [p]);
  }

  /**********************************************************************/

  void exhaustive (BeforeCostRef bc, Permutation & pi);
  void greedy (BeforeCostRef bc, Permutation & pi);

  void dependencyOrder (BeforeCostRef bc,
			Permutation & source,
			const std::vector <int> & parents) {
    Children allChildren (parents);
    Permutation target (source);
    Permutation::iterator it = target.begin ();

    // Uses a stack as a substitute for recursion.  Pushes the root node onto
    // the stack.
    std::vector <int> q;
    q.push_back (-1);

    while (! q.empty ()) {
      int p = q.back ();
      q.pop_back ();
      std::vector <int> & children = allChildren [p];
      
      // Assembles the full permutation from the leaves---if a node has no
      // children, then it goes to the next available slot.
      if (children.empty ()) {
	* it ++ = p;
      } else {
	// Creates a subpermutation consisting of these children.  Except for
	// the root node, the parent must be included in the permutation.
	if (p >= 0) {
	  children.push_back (p);
	}
	Permutation child_pi (source, children.begin (), children.end ());
	// Builds a LOP matrix for these children.
	child_pi.identity ();
	// Finds the best ordering of the children.
	greedy (bc, child_pi);
	// Adds the children to the stack in reverse order.
	std::copy (child_pi.rbegin (), child_pi.rend (),
		   std::back_inserter (q));
	// Makes sure that when this parent pops again, it is treated as a leaf.
	children.clear ();
      }
    }

    source = target;
  }

  void exhaustive (BeforeCostRef bc, Permutation & pi) {
    Permutation best = pi;
    double bestScore = bc -> score (best);
    while (std::next_permutation (pi.begin (), pi.end ())) {
      double score = bc -> score (pi);
      if (score > bestScore) {
	best = pi;
	bestScore = score;
      }
    }
    pi = best;
  }

  void greedy (BeforeCostRef bc, Permutation & pi) {
    while (block_lsf (pi, bc, pi.size ()) > 0);
  }
}
