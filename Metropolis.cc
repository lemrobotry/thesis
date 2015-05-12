#include "Log.hh"
#include "Metropolis.hh"
#include "NormalForm.hh"

namespace Permute {
  // Computes the sum of the scores of all paths contained in the chart's
  // top-most cell.
  double total (ChartRef chart) {
    double t = Log::Zero;
    ConstCellRef top = chart -> getConstCell (0, chart -> getLength ());
    for (Cell::PathIterator i = top -> begin (); i != top -> end (); ++ i) {
      t = Log::add (t, (* i) -> getScore ());
    }
    return t;
  }
  
  Metropolis_Hastings::Metropolis_Hastings (const Permutation & pi,
					    ChartFactoryRef factory,
					    ParseControllerRef controller,
					    ScorerRef scorer) :
    pi_ (new Permutation (pi)),
    pi_star_ (new Permutation (pi)),
    chart_ (factory -> chart (* pi_)),
    chart_star_ (factory -> chart (* pi_star_)),
    controller_ (controller),
    scorer_ (scorer)
  {
    Chart::permute (chart_, controller_, scorer_);
    f_ = total (chart_);
  }

  Metropolis_Hastings::~Metropolis_Hastings () {
    delete pi_;
    delete pi_star_;
  }

  /**********************************************************************/

  // Samples a tree of normal-form paths to visit a sample tree, which implies a
  // sample permutation.  At each PathList node, recursively generates a
  // Bernoulli trial that accepts the head of the list with its conditional
  // probability, and visits the tail of the list otherwise.
  //
  // The static extract method creates a visitor an extracts a sample into the
  // given permutation.
  class SamplePath : public NormalFormVisitor {
  protected:
    Permutation::iterator iter_;
  public:
    SamplePath (Permutation::iterator iter) :
      iter_ (iter)
    {}
    virtual void visit (const PathList * paths) {
      if (Log::bernoulli (paths -> getHead () -> getScore (), paths -> getScore ())) {
	paths -> getHead () -> accept (this);
      } else {
	paths -> getTail () -> accept (this);
      }
    }
    virtual void visit (const PathComposite * path) {
      path -> getLeft () -> accept (this);
      path -> getRight () -> accept (this);
    }
    virtual void visit (const Arc * arc) {
      * iter_ ++ = arc -> getIndex ();
    }
    static void extract (Permutation & pi, ConstPathRef path) {
      SamplePath visitor (pi.begin ());
      path -> accept (& visitor);
    }
  };

  // Given a chart and the total score of all paths in the top cell thereof,
  // chooses among the paths contained therein in proportion to their scores,
  // and calls SamplePath::extract to sample a path into the given permutation.
  void extractSample (Permutation & pi, ChartRef chart, double total) {
    /* Choose a path from the top cell of the chart. */
    ConstPathRef path;
    ConstCellRef top = chart -> getConstCell (0, chart -> getLength ());
    for (Cell::PathIterator i = top -> begin (); i != top -> end (); ++ i) {
      if (Log::bernoulli ((* i) -> getScore (), total)) {
	path = (* i);
	break;
      } else {
	total = Log::subtract (total, (* i) -> getScore ());
      }
    }
    /* Extract a sample from the path. */
    SamplePath::extract (pi, path);
  }

  /**
   * @$ \alpha = \frac{f(\theta*) q(\theta_{t-1} | \theta*)}{f(\theta_{t-1}) q(\theta* | \theta_{t-1})} @$
   */
  // Performs a single Metropolis-Hastings sample.  Uses extractSample to get a
  // sample permutation in the current neighborhood, then computes the total
  // score of the sample's neighborhood using Chart::permute.  The ratio of the
  // current neighborhood's total score to the sample neighborhood's total score
  // is alpha.  With probability min(alpha, 1) accepts the sample permutation,
  // otherwise stays put.
  void Metropolis_Hastings::sample () {
    /* Extract a sample from the current chart. */
    extractSample (* pi_star_, chart_, f_);
    /* Get the total weight of the sample's neighborhood. */
    Chart::permute (chart_star_, controller_, scorer_);
    double f_star = total (chart_star_);
    /* Compute the probability of choosing the new point, and go there with it. */
    double alpha = Log::divide (f_, f_star);
    if (Log::bernoulli (alpha, Log::One)) {
      f_ = f_star;
      std::swap (pi_, pi_star_);
      std::swap (chart_, chart_star_);
    }
  }

  /**********************************************************************/

  // Counts the features that occur in the current permutation.  Should be
  // called after successive calls to sample.
  void Metropolis_Hastings::count (FeatureCounter & fc) {
    fc.count (* pi_, 1.0);
  }
}
