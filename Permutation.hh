#ifndef _PERMUTE_PERMUTATION_HH
#define _PERMUTE_PERMUTATION_HH

#include <vector>
#include <Core/XmlStream.hh>
#include <Fsa/Types.hh>
#include "Path.hh"

namespace Permute {
  // A permutation is a vector of indices from 0 to N - 1, where N is its
  // length.  It also holds a vector of labels from an alphabet indicating the
  // labels corresponding to the indices.  It provides several helper methods,
  // including appending a symbol, permuting according to an existing
  // permutation, reordering according to the symbols along a path, and
  // randomizing.
  class Permutation : public std::vector <size_t> {
    // Prints the symbol sequence implied by the given permutation.
    friend std::ostream & operator << (std::ostream &, const Permutation &);
    // Reads a sequence of symbols into a permutation and populates it alphabet
    // with the set of symbols in the sequence.
    friend bool readPermutationWithAlphabet (Permutation &, std::istream &);
  private:
    std::vector <Fsa::LabelId> labels_;
    Fsa::ConstAlphabetRef alphabet_;
    bool changed_;
  public:
    Permutation (Fsa::ConstAlphabetRef = Fsa::ConstAlphabetRef ());
    // Creates a subpermutation
    template <class InputIterator>
    Permutation (const Permutation & pi, InputIterator begin, InputIterator end) :
      std::vector <size_t> (begin, end),
      labels_ (pi.labels_),
      alphabet_ (pi.alphabet_),
      changed_ (false)
    {}
    
    void clear ();

    Fsa::ConstAlphabetRef alphabet () const;
    Fsa::LabelId label (size_t) const;
    // Returns the symbol corresponding to the given permuted index (passes
    // index through the label method).
    std::string symbol (size_t) const;
    void push_back (const std::string &);
    void permute (const Permutation &);
    void reorder (const ConstPathRef &);
    bool changed () const;
    void changed (bool);
    void writeXml (Core::XmlWriter &) const;
    std::string toString () const;

    void identity ();
    void randomize ();
  };

  // Reads a sequence of symbols into a permutation from an input stream.
  bool readPermutation (Permutation &, std::istream &);
  // Reads a sequence of indices into a permutation from an input stream.  The
  // permutation must receive its alphabet and labels separately.
  void readAlignment (Permutation &, std::istream &);
  // Creates a permutation of the numbers from 0 to n - 1.
  void integerPermutation (Permutation &, int n);

  /**********************************************************************/

  // Modifies the indices on a list of dependency parents so that root is -1
  // instead of 0, etc.
  class DependencyParents {
    friend std::istream & operator >> (std::istream &, DependencyParents &);
    friend std::ostream & operator << (std::ostream &, const DependencyParents &);
  private:
    std::vector <int> & parents_;
  public:
    DependencyParents (std::vector <int> & parents);
  };

  // Reads a sequence of indices into a vector from an input stream.  Subtracts
  // one from each index so the root node is -1.
  bool readParents (std::vector <int> &, std::istream &);

  /**********************************************************************/

  // Constructs an automaton accepting a given permutation, using the given
  // semiring.
  Fsa::ConstAutomatonRef fsa (const Permutation &, Fsa::ConstSemiringRef);
}

#endif//_PERMUTE_PERMUTATION_HH
