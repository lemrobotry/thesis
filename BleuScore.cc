#include <algorithm>
#include <cmath>
#include <functional>
#include <iomanip>

#include <Core/StringUtilities.hh>

#include "BleuScore.hh"
#include "Each.hh"

namespace Permute {

  BleuScore::BleuScore () :
    bleu_()
  {
    bleu_.set_ref_length(0);
    for (int i = 0; i < 4; ++i) {
      bleu_.add_correct(0);
      bleu_.add_total(0);
    }
  }

  BleuScore::BleuScore (const BleuScore & bs) :
    bleu_(bs.bleu_)
  {}

  const BleuScore & BleuScore::operator = (const BleuScore & bs) {
    bleu_ = bs.bleu_;
    return * this;
  }

  const BleuScore & BleuScore::operator += (const BleuScore & bs) {
    addReferenceLength(bs.bleu_.ref_length());
    for (int i = 0; i < bleu_.total_size(); ++i) {
      addPrecision(i + 1, bs.bleu_.correct(i), bs.bleu_.total(i));
    }
    return * this;
  }

  const BleuScore & BleuScore::operator -= (const BleuScore & bs) {
    addReferenceLength(-bs.bleu_.ref_length());
    for (int i = 0; i < bleu_.total_size(); ++i) {
      addPrecision(i + 1, -bs.bleu_.correct(i), -bs.bleu_.total(i));
    }
    return * this;
  }

  const BleuProto& BleuScore::GetProto() const {
    return bleu_;
  }

  void BleuScore::addReferenceLength (int length) {
    bleu_.set_ref_length(bleu_.ref_length() + length);
  }

  void BleuScore::addPrecision (int i, int correct, int total) {
    bleu_.set_correct(i - 1, bleu_.correct(i - 1) + correct);
    bleu_.set_total(i - 1, bleu_.total(i - 1) + total);
  }

  double BleuScore::score (int n) const {
    double s = 0.0;
    for (int i = 1; i <= n; ++ i) {
      s += std::log (precision (i));
    }
    s /= n;
    s += brevityPenalty ();
    return std::exp (s);
  }

  double BleuScore::precision (int i) const {
    return static_cast <double> (bleu_.correct(i - 1)) / static_cast <double> (bleu_.total(i - 1));
  }

  double BleuScore::brevityPenalty () const {
    if (bleu_.total(0) < bleu_.ref_length()) {
      return 1.0 - static_cast <double> (bleu_.ref_length()) / static_cast <double> (bleu_.total(0));
    } else {
      return 0.0;
    }
  }

  double BleuScore::smoothed () const {
    double s = 0.0;
    for (int i = 1; i <= 4; ++ i) {
      s += score (i) / std::pow (2.0, 4 - i + 1);
    }
    return s;
  }

  void BleuScore::swap (BleuScore & bs) {
    std::swap(bleu_, bs.bleu_);
  }

  BleuScore operator + (BleuScore bs1, const BleuScore & bs2) {
    return bs1 += bs2;
  }

  BleuScore operator - (BleuScore bs1, const BleuScore & bs2) {
    return bs1 -= bs2;
  }

  std::ostream & operator << (std::ostream & out, const BleuScore & bs) {
    int p = out.precision ();
    out << std::setprecision (4) << 100.0 * bs.score () << ",";
    for (int i = 1; i <= 4; ++ i) {
      out << " " << std::setprecision (3)
	  << 100.0 * bs.precision (i);
    }
    out << " (BP=" << std::setprecision (3)
	<< std::exp (bs.brevityPenalty ())
	<< ")";
    out.precision (p);
    return out;
  }

  BleuScore totalBleuScore (const std::vector <BleuScore> & bleus) {
    BleuScore total;
    std::for_each (bleus.begin (), bleus.end (),
		   Permute::each_fun (& total, & BleuScore::operator +=));
    return total;
  }

  int countNgrams (const std::string & line, NgramCount & counts) {
    std::string my_line (line);
    Core::stripWhitespace (my_line);
    std::vector <std::string> words = Core::split (my_line, " ");
    for (std::vector <std::string>::const_iterator it = words.begin ();
	 it != words.end (); ++ it) {
      // Collects n-grams from 1 to 4, but stops if the end is reached.
      for (int length = 1; length <= 4 && (it + (length - 1)) != words.end (); ++ length) {
	std::vector <std::string> ngram (it, it + length);
	++ counts [ngram];
      }
    }
    return words.size ();
  }

  BleuScore computeBleuScore (int ref_length,
			      const NgramCount & ref_count,
			      const NgramCount & test_count) {
    BleuScore bleu;
    bleu.addReferenceLength (ref_length);
    for (NgramCount::const_iterator it = test_count.begin ();
	 it != test_count.end (); ++ it) {
      int ref = 0;
      NgramCount::const_iterator ref_it = ref_count.find (it -> first);
      if (ref_it != ref_count.end ()) {
	ref = ref_it -> second;
      }
      // length of n-gram, matches in test, total in test
      bleu.addPrecision (it -> first.size (),
			 std::min (it -> second, ref),
			 it -> second);
    }
    return bleu;
  }

  BleuScore computeBleuScore (int ref_length,
			      const NgramCount & ref_count,
			      const std::string & candidate) {
    NgramCount cand_count;
    countNgrams (candidate, cand_count);
    return computeBleuScore (ref_length,
			     ref_count,
			     cand_count);
  }
  

  BleuScore computeBleuScore (const std::string & reference,
			      const std::string & candidate) {
    NgramCount ref_count, cand_count;
    countNgrams (candidate, cand_count);
    return computeBleuScore (countNgrams (reference, ref_count),
			     ref_count,
			     cand_count);
  }

  BleuScore computeBleuScore (const Permutation & reference,
			      const Permutation & candidate) {
    return computeBleuScore (reference.toString (),
			     candidate.toString ());
  }

  /**********************************************************************/

  BleuScoreAgainst::BleuScoreAgainst (const std::string & ref) :
    ref_count_ (),
    ref_length_ (countNgrams (ref, ref_count_))
  {}

  BleuScoreAgainst::BleuScoreAgainst (const Permutation & ref) :
    ref_count_ (),
    ref_length_ (countNgrams (ref.toString (), ref_count_))
  {}

  BleuScore BleuScoreAgainst::compute (const std::string & cand) const {
    NgramCount cand_count;
    countNgrams (cand, cand_count);
    return computeBleuScore (ref_length_, ref_count_, cand_count);
  }

  BleuScore BleuScoreAgainst::compute (const Permutation & cand) const {
    return this -> compute (cand.toString ());
  }
  
}
