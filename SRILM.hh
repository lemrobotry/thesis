#ifndef _PERMUTE_SRILM_HH
#define _PERMUTE_SRILM_HH

#include <Fsa/Alphabet.hh>
#include <Fsa/Semiring.hh>

namespace Permute {

  // Records state used for parsing SRILM files.  This class is abstract because
  // it doesn't implement process_ngram.
  //
  // @bug Should not read n-grams larger than the given N.
  class SRILMState {
  private:
    int n_, N_;
    bool known_max_;
    Fsa::ConstSemiringRef semiring_;
    static double log10toWeightFactor;
  public:
    SRILMState (int N = 0);
    virtual ~SRILMState () {};
    Fsa::ConstSemiringRef semiring () const { return semiring_; }
    void setSemiring (Fsa::ConstSemiringRef semiring) { semiring_ = semiring; }
    int n () const;
    bool full () const;
    void set (const std::string & line);
    void count (const std::string & line);
    bool valid () const;
    bool ngram () const;
    void read (std::istream &);
    void readline (const std::string & line);
    virtual void process_ngram (Fsa::Weight, const std::vector <std::string> &, Fsa::Weight) const = 0;
    virtual Fsa::Weight weight (std::istream &) const;
  };
  
  //  Reads the subset of ngrams from the input stream that consists entirely of
  //  symbols from the given alphabet, plus <s> and </s>.  Constructs an FSA
  //  that accepts alphabet*, incorporating backoff weights into new arcs (Fsa's
  //  support of failure arcs is incomplete).
  Fsa::ConstAutomatonRef srilm (std::istream &, int = 0, double = 1.0);
}

#endif//_PERMUTE_SRILM_HH

