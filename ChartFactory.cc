#include "BestCell.hh"
#include "CellMap.hh"
#include "ChartFactory.hh"
#include "FsaCellMap.hh"
#include "FullCell.hh"
#include "Limit.hh"
#include "PruneChart.hh"

namespace Permute {

  // Returns a Chart with a trivial CellMap (no automaton) and a BestCell at the
  // (0,n) position.
  ChartRef ChartFactory::chart (Permutation & pi, int window) const {
    CellMapRef cellMap = CellMap::trivial ();
    CellRef topCell (new BestCell);
    return ChartRef (new ChartImpl (pi, cellMap, topCell, window));
  }

  ChartFactoryRef ChartFactory::create () {
    return ChartFactoryRef (new ChartFactory);
  }

  /**********************************************************************/

  // Crates Charts appropriate for k-best parsing.
  class kBestChartFactory : public ChartFactory {
  public:
    virtual ChartRef chart (Permutation &, int = 0) const;
  };

  // Returns a Chart with a trivial CellMap (no automaton) and a FullCell at the
  // (0,n) position for use with k-best parsing.
  ChartRef kBestChartFactory::chart (Permutation & pi, int window) const {
    CellMapRef cellMap = CellMap::trivial ();
    CellRef topCell (new FullCell);
    return ChartRef (new ChartImpl (pi, cellMap, topCell, window));
  }

  ChartFactoryRef ChartFactory::kbest () {
    return ChartFactoryRef (new kBestChartFactory);
  }

  /**********************************************************************/

  // Creates Charts that use an automaton to score permutations.
  template <class FsaCM>
  class FsaChartFactory : public ChartFactory {
  protected:
    Fsa::ConstAutomatonRef fsa_;
  public:
    FsaChartFactory (Fsa::ConstAutomatonRef fsa) :
      fsa_ (fsa)
    {}
    virtual ChartRef chart (Permutation &, int = 0) const;
  };

  // Returns a Chart with a CellMap and (0,n) cell determined by the template
  // argument's mapFsa method.
  template <class FsaCM>
  ChartRef FsaChartFactory <FsaCM>::chart (Permutation & pi, int window) const {
    Fsa::ConstAutomatonRef limited = limit (fsa_, pi);
    std::pair <CellMapRef, CellRef> mapPair = FsaCM::mapFsa (limited);
    return ChartRef (new ChartImpl (pi, CellMap::cache (mapPair.first), mapPair.second, window));
  }

  ChartFactoryRef ChartFactory::create (Fsa::ConstAutomatonRef fsa) {
    return ChartFactoryRef (new FsaChartFactory <FsaCellMap> (fsa));
  }

  /**********************************************************************/

  // Decorates another ChartFactory with threshold pruning.
  class PrunedChartFactory : public ChartFactory {
  protected:
    ChartFactoryRef factory_;
  public:
    PrunedChartFactory (ChartFactoryRef factory) :
      factory_ (factory)
    {}
    virtual ChartRef chart (Permutation &, int = 0) const;
  };

  // Returns a pruned version of the decorated factory's Chart.
  ChartRef PrunedChartFactory::chart (Permutation & pi, int window) const {
    return prune (factory_ -> chart (pi, window));
  }

  ChartFactoryRef ChartFactory::pruned () {
    return ChartFactoryRef (new PrunedChartFactory (ChartFactory::create ()));
  }
  ChartFactoryRef ChartFactory::pruned (Fsa::ConstAutomatonRef fsa) {
    return ChartFactoryRef (new PrunedChartFactory (ChartFactory::create (fsa)));
  }

  /**********************************************************************/

  // Decorates another ChartFactory with outside pruning.  Extends
  // PrunedChartFactory so there are actually three factories involved: this
  // factory, its outside_ factory, and PrunedChartFactory's factory_.  Uses the
  // chart that outside_ generates to prune the chart that factory_ generates.
  class OutsideChartFactory : public PrunedChartFactory {
  protected:
    ChartFactoryRef outside_;
  public:
    OutsideChartFactory (ChartFactoryRef factory, Fsa::ConstAutomatonRef fsa) :
      PrunedChartFactory (factory),
      outside_ (new FsaChartFactory <UnigramFsaCellMap> (fsa))
    {}
    virtual ChartRef chart (Permutation &, int = 0) const;
  };

  // Returns the Chart that the decorated factory creates pruned using outside
  // estimates from the outside_ factory's Chart.
  // 
  // @bug Calls limit twice with the same (Automaton, Permutation) pair.
  ChartRef OutsideChartFactory::chart (Permutation & pi, int window) const {
    return prune (factory_ -> chart (pi, window), outside_ -> chart (pi, window));
  }
  
  ChartFactoryRef ChartFactory::outside (Fsa::ConstAutomatonRef fsa) {
    return ChartFactoryRef (new OutsideChartFactory (ChartFactory::create (fsa), fsa));
  }
}
