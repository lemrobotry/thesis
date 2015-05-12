#include <iterator>

#include <Core/Utility.hh>
#include <Fsa/Static.hh>

#include "Iterator.hh"
#include "Permutation.hh"
#include "PathVisitor.hh"

namespace Permute {
  // Extracts the permutation along a path.  Each arc visited holds an index,
  // to which the next position in the permutation is set.
  class PathExtractLabels : public PathVisitor {
  private:
    Permutation::iterator iter_;
    bool changed_;
  public:
    PathExtractLabels (Permutation::iterator iter) :
      iter_ (iter),
      changed_ (false)
    {}
    virtual void visit (const PathComposite * path) {
      path -> getLeft () -> accept (this);
      path -> getRight () -> accept (this);
    }
    virtual void visit (const Arc * arc) {
      if (* iter_ != arc -> getIndex ()) {
	* iter_ = arc -> getIndex ();
	changed_ = true;
      }
      ++ iter_;
    }
    // Provides a convenience method for constructing and applying a
    // PathExtractLabels visitor to the given path.
    static bool extractLabels (Permutation & permutation, ConstPathRef path) {
      PathExtractLabels visitor (permutation.begin ());
      path -> accept (& visitor);
      return visitor.changed_;
    }
  };

  /**********************************************************************/
  
  Permutation::Permutation (Fsa::ConstAlphabetRef alphabet) :
    std::vector <size_t> (),
    labels_ (),
    alphabet_ (alphabet),
    changed_ (false)
  {}

  void Permutation::clear () {
    std::vector <size_t>::clear ();
    labels_.clear ();
  }

  Fsa::ConstAlphabetRef Permutation::alphabet () const {
    return alphabet_;
  }

  Fsa::LabelId Permutation::label (size_t index) const {
    return labels_ [index];
  }

  std::string Permutation::symbol (size_t index) const {
    return alphabet () -> symbol (label (index));
  }

  void Permutation::push_back (const std::string & symbol) {
    std::vector <size_t>::push_back (size ());
    labels_.push_back (alphabet_ -> index (symbol));
  }

  void Permutation::permute (const Permutation & other) {
    std::vector <size_t>::operator = (other);
  }

  void Permutation::reorder (const ConstPathRef & path) {
    changed_ = PathExtractLabels::extractLabels (* this, path);
  }

  bool Permutation::changed () const {
    return changed_;
  }

  void Permutation::changed (bool change) {
    changed_ = change;
  }

  std::ostream & operator << (std::ostream & os, const Permutation & p) {
    std::transform (p.begin (), p.end (),
		    std::ostream_iterator <std::string> (os, " "),
		    PermutationSymbol (p));
    return os;
  }

  void Permutation::writeXml (Core::XmlWriter & os) const {
    os << Core::XmlOpen ("permutation");
    os << (* this);
    os << Core::XmlClose ("permutation");
  }

  std::string Permutation::toString () const {
    std::ostringstream out;
    out << (* this);
    return out.str ();
  }

  // Resets to the identity permutation.  This could be O(n) instead of O(n log
  // n), but at present it is not time critical.
  void Permutation::identity () {
    std::sort (begin (), end ());
  }

  void Permutation::randomize () {
    std::random_shuffle (begin (), end ());
    changed_ = true;
  }

  /**********************************************************************/

  bool readPermutation (Permutation & permutation, std::istream & in) {
    permutation.clear ();
    
    std::string line;
    if (Core::wsgetline (in, line) != EOF) {
      Core::StringTokenizer tokenizer (line);
      for (Core::StringTokenizer::iterator token = tokenizer.begin ();
	   token != tokenizer.end (); ++token) {
	permutation.push_back (* token);
      }
      return true;
    }
    return false;
  }

  bool readPermutationWithAlphabet (Permutation & permutation, std::istream & in) {
    Fsa::StaticAlphabet * alphabet = new Fsa::StaticAlphabet;
    permutation.alphabet_ = Fsa::ConstAlphabetRef (alphabet);
    permutation.clear ();

    std::string line;
    if (Core::wsgetline (in, line) != EOF) {
      Core::StringTokenizer tokenizer (line);
      for (Core::StringTokenizer::iterator token = tokenizer.begin ();
	   token != tokenizer.end (); ++ token) {
	alphabet -> addSymbol (* token);
	permutation.push_back (* token);
      }
      return true;
    }
    return false;
  }

  void readAlignment (Permutation & permutation, std::istream & in) {
    for (Permutation::iterator i = permutation.begin (); i != permutation.end (); ++ i) {
      in >> (* i);
    }
  }

  void integerPermutation (Permutation & permutation, int n) {
    std::stringstream str;
    str << delimit (range (0), range (n), " ");
    readPermutationWithAlphabet (permutation, str);
  }

  /**********************************************************************/

  DependencyParents::DependencyParents (std::vector <int> & parents) :
    parents_ (parents)
  {}

  std::istream & operator >> (std::istream & in, DependencyParents & parents) {
    parents.parents_.clear ();
    std::transform (std::istream_iterator <int> (in),
		    std::istream_iterator <int> (),
		    std::back_inserter (parents.parents_),
		    std::bind2nd (std::minus <int> (), 1));
    return in;
  }

  std::ostream & operator << (std::ostream & out, const DependencyParents & parents) {
    std::transform (parents.parents_.begin (), parents.parents_.end (),
		    std::ostream_iterator <int> (out, " "),
		    std::bind2nd (std::plus <int> (), 1));
    return out;
  }

  bool readParents (std::vector <int> & parents, std::istream & in) {
    std::string line;
    if (Core::wsgetline (in, line) != EOF) {
      std::istringstream lineIn (line);
      DependencyParents dp (parents);
      lineIn >> dp;
      return true;
    }
    return false;
  }

  /**********************************************************************/

  Fsa::ConstAutomatonRef fsa (const Permutation & p, Fsa::ConstSemiringRef semiring) {
    Fsa::StaticAutomaton * f = new Fsa::StaticAutomaton;
    f -> setType (Fsa::TypeAcceptor);
    f -> setSemiring (semiring);
    f -> setProperties (Fsa::PropertySorted | Fsa::PropertyLinear | Fsa::PropertyAcyclic);
    f -> setInputAlphabet (p.alphabet ());

    Fsa::State * sp = f -> newState ();
    f -> setInitialStateId (sp -> id ());

    for (Permute::Permutation::const_iterator i = p.begin (); i != p.end (); ++ i) {
      Fsa::Arc * a = sp -> newArc ();
      a -> input_ = a -> output_ = p.label (* i);
      a -> weight_ = semiring -> one ();
      sp = f -> newState ();
      a -> target_ = sp -> id ();
    }
    sp -> setFinal (semiring -> one ());
    return Fsa::ConstAutomatonRef (f);
  }
}
