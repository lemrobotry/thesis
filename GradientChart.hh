#ifndef _PERMUTE_GRADIENT_CHART_HH
#define _PERMUTE_GRADIENT_CHART_HH

#include "Path.hh"
#include "Permutation.hh"
#include "PV.hh"

namespace Permute {

  class GradientScorer;
  
  // Uses normal form and the inside-outside algorithm to compute gradients of
  // the log likelihood of the current permutation with respect to the
  // parameters.
  class GradientChart {
  private:
    const Permutation & pi_;
    int n_;
    std::vector <double> inside_, outside_;
    
  public:
    GradientChart (const Permutation & pi);
    void parse (const ParseControllerRef &, GradientScorer &, PV &);

  private:
    int index (int i, int j, Path::Type type) const;
    const double & inside (int i, int j, Path::Type type) const;
    double & inside (int i, int j, Path::Type type);
    const double & outside (int i, int j, Path::Type type) const;
    double & outside (int i, int j, Path::Type type);
    double total (int i, int j) const;
  public:
    double Z () const;
  };

  class GradientScorer : public BeforeScorer {
  private:
    SumBeforeCostRef cost_;
    const Permutation & permutation_;
    std::vector <double> gradient_;
  public:
    GradientScorer (const SumBeforeCostRef & cost, const Permutation & pi);
    const double & gradient (int i, int j, int k) const;
    double & gradient (int i, int j, int k);
    void addGradient (int left, int right, double gradient);
    void finish (const ParseControllerRef & controller, bool numerator = true);
  };
}

#endif//_PERMUTE_GRADIENT_CHART_HH
