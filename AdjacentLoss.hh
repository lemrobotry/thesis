#ifndef _PERMUTE_ADJACENT_LOSS_HH
#define _PERMUTE_ADJACENT_LOSS_HH

#include "ExpectationSemiring.hh"
#include "GradientChart.hh"
#include "ParseController.hh"
#include "Permutation.hh"

namespace Permute {

  class AdjacentLoss {
  private:
    const Permutation & target_;
    std::vector <size_t> succ_;
  public:
    AdjacentLoss (const Permutation & target);
    double score (const Permutation & pi) const;
  };

  /**********************************************************************/

  template <class Value>
  class AdjacentItem {
  private:
    typedef enum {
      LEAF = 0,
      KEEP_SAME_R = 0,
      KEEP_NEW_R,
      SWAP,
      SIZE
    } Index;
    bool leaf_;
    std::vector <Value> values_;
  public:
    AdjacentItem (bool leaf = false) :
      leaf_ (leaf),
      values_ (leaf ? 1 : SIZE)
    {}
    void fill (const Value & v) { std::fill (values_.begin (), values_.end (), v); }

    bool isLeaf () const { return leaf_; }
    const Value & leaf () const { return values_ [LEAF]; }
    Value & leaf () {
      leaf_ = true;
      values_.resize (1);
      return const_cast <Value &>
	(static_cast <const AdjacentItem <Value> *>
	 (this) -> leaf ());
    }
    const Value & keepSameRight () const { return values_ [KEEP_SAME_R]; }
    Value & keepSameRight () {
      return const_cast <Value &>
	(static_cast <const AdjacentItem <Value> *>
	 (this) -> keepSameRight ());
    }
    const Value & keepNewRight () const { return values_ [KEEP_NEW_R]; }
    Value & keepNewRight () {
      return const_cast <Value &>
	(static_cast <const AdjacentItem <Value> *>
	 (this) -> keepNewRight ());
    }
    const Value & swap () const { return values_ [SWAP]; }
    Value & swap () {
      return const_cast <Value &>
	(static_cast <const AdjacentItem <Value> *>
	 (this) -> swap ());
    }

    Value keep () const;
    Value newRight () const;
    Value any () const;
  };

  template <class Value>
  std::ostream & operator << (std::ostream & out, const AdjacentItem <Value> & item);

  /**********************************************************************/

  class ExpectationGradientScorer;

  class AdjacentGradientChart {
  public:
    typedef AdjacentItem <Expectation> Item;
  private:
    const Permutation & pi_;
    int n_;
    std::vector <Item> inside_, outside_;
  public:
    AdjacentGradientChart (const Permutation & pi);
    void parse (const ParseControllerRef &, ExpectationGradientScorer &, PV &);

    int index (int begin, int end) const;
    
    const Item & inside (int begin, int end) const;
    Item & inside (int begin, int end);

    const Item & outside (int begin, int end) const;
    Item & outside (int begin, int end);

    Expectation Z () const { return inside (0, n_).any (); }
  };

  class ExpectationGradientScorer : public BeforeScorer {
  private:
    SumBeforeCostRef cost_;
    const Permutation & permutation_;
    std::vector <ExpectationSum> gradient_;
    std::vector <Expectation> matrix_;
  public:
    ExpectationGradientScorer (const SumBeforeCostRef & cost, const Permutation & pi);
    const ExpectationSum & gradient (int i, int j, int k) const;
    ExpectationSum & gradient (int i, int j, int k);
    const Expectation & expectation (int i, int j) const;
    Expectation & expectation (int i, int j);
    void addExpectation (int left, int right, Expectation gradient);
    void finish (const ParseControllerRef & controller, Expectation Z);
  };

}

#endif//_PERMUTE_ADJACENT_LOSS_HH
