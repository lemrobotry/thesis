#include <algorithm>
#include "PruneChart.hh"

namespace Permute {
  // Decorates a cell, requiring that paths have score above a given threshold
  // in order to be added to the cell.  Note that this threshold is absolute,
  // not relative to the best score in the cell.
  class PrunedCell : public CellDecorator {
  private:
    double threshold_;
  public:
    PrunedCell (CellRef cell, double threshold) :
      CellDecorator (cell),
      threshold_ (threshold)
    {}
    virtual bool add (ConstPathRef path) {
      if (path -> getScore () < threshold_) {
	return false;
      } else {
	return CellDecorator::add (path);
      }
    }
  };

  /**********************************************************************/
  
  // Decorates a chart by decorating every cell in the chart with a PrunedCell,
  // initially using a given threshold.  Every time the chart's getBestPath is
  // called, max(threshold, best score) replaces the threshold, so that during
  // the next parsing pass, paths worse than the identity permutation are
  // pruned.
  class PrunedChart : public ChartDecorator {
  protected:
    double threshold_;
  public:
    PrunedChart (ChartRef chart, double threshold) :
      ChartDecorator (chart),
      threshold_ (threshold)
    {}
    virtual CellRef getCell (int begin, int end) {
      return CellRef (new PrunedCell (ChartDecorator::getCell (begin, end), threshold_));
    }
    virtual ConstPathRef getBestPath () {
      ConstPathRef best = ChartDecorator::getBestPath ();
      threshold_ = std::max (threshold_, best -> getScore ());
      return best;
    }
  };

  // Wraps the given chart with a PrunedChart initialized with the given
  // threshold.
  ChartRef prune (ChartRef chart, double threshold) {
    return ChartRef (new PrunedChart (chart, threshold));
  }

  /**********************************************************************/

  // Strengthens the pruning of PrunedChart with outside estimates.  Now, a path
  // is pruned if its score plus its outside estimate is lower than the
  // threshold.
  class OutsidePrunedChart : public PrunedChart {
  private:
    OutsideRef outside_;
  public:
    OutsidePrunedChart (ChartRef chart, OutsideRef outside, double threshold) :
      PrunedChart (chart, threshold),
      outside_ (outside)
    {}
    virtual CellRef getCell (int begin, int end) {
      return CellRef (new PrunedCell (ChartDecorator::getCell (begin, end),
				      threshold_ - outside_ -> outside (begin, end)));
    }
    // Computes outside estimates for the current permutation before parsing
    // proceeds.
    virtual void beforePermute (ParseControllerRef controller, ScorerRef scorer) {
      outside_ -> estimate (controller, scorer);
      PrunedChart::beforePermute (controller, scorer);
    }
  };

  // Wraps the first chart with an OutsidePrunedChart using outside estimates
  // from the second chart.
  ChartRef prune (ChartRef chart, ChartRef outsideChart, double threshold) {
    return ChartRef (new OutsidePrunedChart (chart,
					     outside (outsideChart),
					     threshold));
  }

  /**********************************************************************/

  // Decorates a cell with relative pruning.  Records the best score contained
  // so far, in addition to the threshold.  If a new path has a better score,
  // the best score is adjusted.  Otherwise the path is added only if its score
  // is better than the best score plus the threshold.  In other words, the
  // threshold should be negative.
  class RelativePrunedCell : public CellDecorator {
  private:
    double best_;
    double threshold_;
  public:
    RelativePrunedCell (CellRef cell, double threshold) :
      CellDecorator (cell),
      best_ (Core::Type <double>::min),
      threshold_ (threshold)
    {}
    virtual bool add (ConstPathRef path) {
      if (path -> getScore () > best_) {
	if (CellDecorator::add (path)) {
	  best_ = path -> getScore ();
	  return true;
	} else {
	  return false;
	}
      } else if (path -> getScore () > (best_ + threshold_)) {
	return CellDecorator::add (path);
      }
      return false;
    }
  };

  /**********************************************************************/

  // Decorates a chart by decorating every cell in the chart with a
  // RelativePrunedCell, using a given threshold.
  class RelativePrunedChart : public ChartDecorator {
  protected:
    double threshold_;
  public:
    RelativePrunedChart (ChartRef chart, double threshold) :
      ChartDecorator (chart),
      threshold_ (threshold)
    {}
    virtual CellRef getCell (int begin, int end) {
      return CellRef (new RelativePrunedCell (ChartDecorator::getCell (begin, end), threshold_));
    }
  };

  // Wraps the given chart with a RelativePrunedChart using the given
  // threshold.
  ChartRef pruneRelative (ChartRef chart, double threshold) {
    return ChartRef (new RelativePrunedChart (chart, threshold));
  }
  
}
