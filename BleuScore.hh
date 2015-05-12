#ifndef _PERMUTE_BLEU_SCORE_HH
#define _PERMUTE_BLEU_SCORE_HH

#include <map>
#include <vector>

#include "eval.pb.h"
#include "Permutation.hh"

namespace Permute {

  typedef std::map <std::vector <std::string>, int> NgramCount;
  
  // Computes BLEU score including a single-reference brevity penalty.
  class BleuScore {
  private:
    BleuProto bleu_;
  public:
    BleuScore ();
    BleuScore (const BleuScore & bs);
    const BleuScore & operator = (const BleuScore & bs);
    const BleuScore & operator += (const BleuScore & bs);
    const BleuScore & operator -= (const BleuScore & bs);
    const BleuProto& GetProto() const;
    void addReferenceLength (int length);
    void addPrecision (int i, int correct, int total);
    double score () const { return score (4); }
    double score (int n) const;
    double precision (int i) const;
    double brevityPenalty () const;
    double smoothed () const;
    void swap (BleuScore & bs);
  };

  BleuScore operator + (BleuScore bs1, const BleuScore & bs2);
  BleuScore operator - (BleuScore bs1, const BleuScore & bs2);

  std::ostream & operator << (std::ostream & out, const BleuScore & bs);

  // Computes and returns the total BLEU score from a vector of scores.
  BleuScore totalBleuScore (const std::vector <BleuScore> & bleus);

  // Accumulates the number of occurrences of each n-gram in the given line into
  // the given map of counts.  Returns the number of tokens in the line.
  int countNgrams (const std::string & line, NgramCount & counts);

  // Computes the BLEU score of a sentence with respect to the reference using
  // maps of counts computed using countNgrams.  Iterates over the n-grams that
  // occur in test_count and accumulates the precision of each n-gram into the
  // BLEU score.  The precision of an n-gram is min(test, ref) / test, where test
  // is the count in the test sentence, and ref is the count in the reference
  // sentence.
  BleuScore computeBleuScore (int ref_length,
			      const NgramCount & ref_count,
			      const NgramCount & test_count);

  // Computes the BLEU score between reference counts and a candidate sentence.
  BleuScore computeBleuScore (int ref_length,
			      const NgramCount & ref_count,
			      const std::string & candidate);

  // Computes the BLEU score between a reference sentence and a candidate
  // sentence.
  BleuScore computeBleuScore (const std::string & reference,
			      const std::string & candidate);

  BleuScore computeBleuScore (const Permutation & reference,
			      const Permutation & candidate);

  /**********************************************************************/

  class BleuScoreAgainst {
  private:
    NgramCount ref_count_;
    int ref_length_;
  public:
    BleuScoreAgainst (const std::string & ref);
    BleuScoreAgainst (const Permutation & ref);
    BleuScore compute (const std::string & cand) const;
    BleuScore compute (const Permutation & cand) const;
  };
}

#endif//_PERMUTE_BLEU_SCORE_HH
