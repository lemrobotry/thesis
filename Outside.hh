#ifndef _PERMUTE_OUTSIDE_HH
#define _PERMUTE_OUTSIDE_HH

#include <Core/ReferenceCounting.hh>
#include "Chart.hh"

namespace Permute {
  // Wraps a chart with methods to compute outside estimates.
  class Outside : public Core::ReferenceCounted {
  private:
    ChartRef chart_;
    int n_;
    std::vector <double> outside_;
    double & operator () (int, int);
  public:
    Outside (ChartRef);
    void estimate (ParseControllerRef, ScorerRef);
    double outside (int, int) const;
  };

  typedef Core::Ref <Outside> OutsideRef;

  // Returns the given chart wrapped by Outside.
  OutsideRef outside (ChartRef);
}

#endif//_PERMUTE_OUTSIDE_HH
