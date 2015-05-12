#include <algorithm>

#include <Core/CompressedStream.hh>
#include <Core/Statistics.hh>
#include <Core/XmlStream.hh>
#include <Fsa/Application.hh>
#include <Fsa/Basic.hh>
#include <Fsa/Best.hh>
#include <Fsa/Compose.hh>
#include <Fsa/Input.hh>
#include <Fsa/Static.hh>

#include "Chart.hh"
#include "FsaCellMap.hh"
#include "Limit.hh"
#include "ParseController.hh"
#include "Permutation.hh"
#include "PruneChart.hh"
#include "Scorer.hh"
#include "StatChart.hh"

APPLICATION

using namespace Permute;

class Test : public Fsa::Application {
private:
  static Core::ParameterString paramFsa;
  static Core::ParameterString paramInput;
  static Core::ParameterString paramOutput;
  static Core::ParameterBool paramSort;
  static Core::ParameterString paramParseController;
  static Core::ParameterInt paramQuadraticWidth;
  static Core::ParameterBool paramLeftAnchor;
  static Core::ParameterInt paramLeftAnchorWidth;
  static Core::ParameterBool paramRightAnchor;
  static Core::ParameterInt paramRightAnchorWidth;
  static Core::ParameterBool paramStats;
  static Core::ParameterBool paramPrune;
  static Core::ParameterBool paramOutside;
  Core::Timer timer;
  Core::XmlWriter * out;
public:
  Test () {
    setTitle ("test");
    setDefaultLoadConfigurationFile (false);
  }
  
  Fsa::ConstAutomatonRef read (const std::string &);
  double path_score (Fsa::ConstAutomatonRef);
  double score (Fsa::ConstAutomatonRef, const Permutation &);

  int main (const std::vector <std::string> & args) {
    Core::CompressedOutputStream co (paramOutput (config));
    Core::XmlWriter out (co);
    this -> out = & out;
    
    out << Core::XmlOpen ("neighborhood-search") << "\n";

    Fsa::ConstAutomatonRef fsa = read (paramFsa (config));

    Permutation p (fsa -> getInputAlphabet ());

    /**********************************************************************
     * ParseControllers
     **********************************************************************/
    
    ParseControllerRef pc;
    out << Core::XmlOpen ("parameters");
    if (paramParseController (config) == "quadratic") {
      out << Core::XmlFull ("power", 2)
	  << Core::XmlFull ("width", paramQuadraticWidth (config));
      pc = QuadraticParseController::create (paramQuadraticWidth (config));
      if (paramLeftAnchor (config)) {
	out << Core::XmlFull ("left-anchor", paramLeftAnchorWidth (config));
	pc = LeftAnchorParseController::decorate (pc, paramLeftAnchorWidth (config));
      }
      if (paramRightAnchor (config)) {
	out << Core::XmlFull ("right-anchor", paramRightAnchorWidth (config));
	pc = RightAnchorParseController::decorate (pc, p, paramRightAnchorWidth (config));
      }
    } else {
      out << Core::XmlFull ("power", 3);
      pc = CubicParseController::create ();
    }
    if (paramSort (config)) {
      out << Core::XmlEmpty ("sort");
    }
    if (paramOutside (config)) {
      out << Core::XmlEmpty ("prune") + Core::XmlAttribute ("outside", true);
    } else if (paramPrune (config)) {
      out << Core::XmlEmpty ("prune");
    }
    out << Core::XmlClose ("parameters") << "\n";

    /**********************************************************************/
      
    Core::TextInputStream input;
    if (paramInput (config) == "-") {
      std::cerr << "** INPUT FROM STANDARD IN **" << std::endl;
    } else {
      std::cerr << "** INPUT FROM: " << paramInput (config) << " **" << std::endl;
      input.open (paramInput (config));
    }

    for (int i = 1; readPermutation (p, input ? input : std::cin); ++ i) {
      out << Core::XmlOpen ("sentence")
	+ Core::XmlAttribute ("number", i)
	  << "\n";

      /**********************************************************************
       * Get the score of the true permutation.
       **********************************************************************/

      double start_score = score (fsa, p);
      out << Core::XmlFull ("gold-score", start_score) << "\n";

      /**********************************************************************/

      if (paramSort (config)) {
	std::sort (p.begin (), p.end ());
	p.writeXml (out);
	out << "\n";
	start_score = score (fsa, p);
	out << Core::XmlFull ("start-score", start_score) << "\n";
      }

      timer.start ();

      Fsa::ConstAutomatonRef limited = limit (fsa, p);
//       Fsa::info (limited, out);
//       out << "\n";
    
      std::pair <CellMapRef, CellRef> mapPair = FsaCellMap::mapFsa (limited);
      CellMapRef cellMap = CellMap::cache (mapPair.first);
      CellRef topCell = mapPair.second;

      /**********************************************************************/

      ChartRef chart (new ChartImpl (p, cellMap, topCell));
      ChartStats stats;
      if (paramStats (config)) {
	chart = stats.instrument (chart);
      }
      if (paramOutside (config)) {
	std::pair <CellMapRef, CellRef> unigramMapPair =
	  UnigramFsaCellMap::mapFsa (limited);
	ChartRef outsideChart (new ChartImpl (p, CellMap::cache (unigramMapPair.first), unigramMapPair.second));
	chart = prune (chart, outsideChart, start_score);
      } else if (paramPrune (config)) {
	chart = prune (chart, start_score);
      }

      /**********************************************************************/

      ScorerRef scorer (new Scorer);
      double best_score = scorer -> score (p);
      do {
	out << Core::XmlOpen ("iteration") << "\n";

	Core::Timer iterationTimer;
	iterationTimer.start ();
	
	Chart::permute (chart, pc, scorer);
	ConstPathRef bestPath = chart -> getBestPath ();
	out << Core::XmlFull ("score", bestPath -> getScore ()) << "\n";

	p.changed (false);
	if (bestPath -> getScore () > best_score) {
	  best_score = bestPath -> getScore ();
	  p.reorder (bestPath);
	}
	p.writeXml (out);
	out << "\n";
	
	iterationTimer.stop ();
	iterationTimer.write (out);
	out << "\n";

	/**********************************************************************
	 * Statistics
	 **********************************************************************/
	if (paramStats (config)) {
	  stats.write (out);
	  out << "\n";
	}
	/**********************************************************************/
	out << Core::XmlClose ("iteration") << "\n";
      } while (p.changed ());

      timer.stop ();
      timer.write (out);
      out << "\n";
      
      out << Core::XmlClose ("sentence") << "\n";
    }

    out << Core::XmlClose ("neighborhood-search") << "\n";
    return EXIT_SUCCESS;
  }
} app;

Core::ParameterString Test::paramFsa ("fsa", "the scoring automaton", "");
Core::ParameterString Test::paramInput ("input", "the input sentence source", "-");
Core::ParameterString Test::paramOutput ("output", "the output file", "-");
Core::ParameterBool Test::paramSort ("sort", "sort the arguments before permuting them", false);
Core::ParameterString Test::paramParseController ("controller", "the sub-permutation consideration scheme", "quadratic");
Core::ParameterInt Test::paramQuadraticWidth ("width", "the max width of one sub-permutation for use with the quadratic parse controller", 1, 1);
Core::ParameterBool Test::paramLeftAnchor ("left", "use a left anchored extension to the quadratic parse controller", false);
Core::ParameterInt Test::paramLeftAnchorWidth ("lwidth", "width of the left anchor extension", 1, 1);
Core::ParameterBool Test::paramRightAnchor ("right", "use a right anchored extension to the quadratic parse controller", false);
Core::ParameterInt Test::paramRightAnchorWidth ("rwidth", "width of the right anchor extension", 1, 1);
Core::ParameterBool Test::paramStats ("stats", "report some count statistics", false);
Core::ParameterBool Test::paramPrune ("prune", "use a pruned version of the chart", false);
Core::ParameterBool Test::paramOutside ("outside", "use an outside estimate for pruning", false);

Fsa::ConstAutomatonRef Test::read (const std::string & file) {
  (* out) << Core::XmlFull ("automaton", file) << "\n";
  Fsa::StaticAutomaton * fst = new Fsa::StaticAutomaton;
  Fsa::read (fst, file);
  return Fsa::ConstAutomatonRef (fst);
}

double Test::path_score (Fsa::ConstAutomatonRef path) {
  if (! path -> knowsProperty (Fsa::PropertyLinear)) {
    std::cerr << "path_score called on non-linear automaton." << std::endl;
    EXIT_FAILURE;
  } else {
    double score = 0.0;
    Fsa::ConstStateRef state;
    for (state = path -> getState (path -> initialStateId ());
	 ! state -> isFinal (); ) {
      Fsa::State::const_iterator arc = state -> begin ();
      score -= float (arc -> weight ());
      state = path -> getState (arc -> target ());
    }
    return score - float (state -> weight_);
  }
}

double Test::score (Fsa::ConstAutomatonRef fsa, const Permutation & p) {
  Fsa::StaticAutomaton * linear = new Fsa::StaticAutomaton (Fsa::TypeAcceptor);
  linear -> setSemiring (fsa -> semiring ());
  linear -> setInputAlphabet (fsa -> getInputAlphabet ());

  Fsa::StateRef state (linear -> newState ());
  linear -> setInitialStateId (state -> id ());

  for (Permutation::const_iterator index = p.begin (); index != p.end (); ++ index) {
    Fsa::StateRef target (linear -> newState ());
    state -> newArc (target -> id (), linear -> semiring () -> one (), p.label (* index));
    state = target;
  }

  state -> setFinal (linear -> semiring () -> one ());

  return path_score (Fsa::best (Fsa::composeMatching (Fsa::ConstAutomatonRef (linear), fsa, false)));
}
