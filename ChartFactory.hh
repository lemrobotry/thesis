#ifndef _PERMUTE_CHART_FACTORY_HH
#define _PERMUTE_CHART_FACTORY_HH

#include "Chart.hh"

namespace Permute {

  class ChartFactory;
  typedef Core::Ref <const ChartFactory> ChartFactoryRef;

  // Creates an instance of Chart given an instance of Permutation.  Provides
  // several static methods that return appropriate factories.
  class ChartFactory : public Core::ReferenceCounted {
  public:
    virtual ChartRef chart (Permutation &, int = 0) const;

    static ChartFactoryRef create ();
    static ChartFactoryRef kbest ();
    static ChartFactoryRef create (Fsa::ConstAutomatonRef);
    static ChartFactoryRef pruned ();
    static ChartFactoryRef pruned (Fsa::ConstAutomatonRef);
    static ChartFactoryRef outside (Fsa::ConstAutomatonRef);
  };
}

#endif//_PERMUTE_CHART_FACTORY_HH
