// Roy W. Tromble
// 31 October 2007

#include "Application.hh"
#include "BeforeScorer.hh"
#include "LinearOrdering.hh"

APPLICATION

using namespace Permute;

class SearchFunctor : public std::binary_function<Permutation &, const BeforeCostRef &, double> {
public:
  virtual ~SearchFunctor () {}
  virtual result_type operator () (first_argument_type a1, second_argument_type a2) = 0;
};

typedef double (* SearchFunction) (Permutation &, const BeforeCostRef &);
class SearchFunctionAdapter : public SearchFunctor {
private:
  SearchFunction fun_;
public:
  SearchFunctionAdapter (SearchFunction fun) :
    fun_ (fun)
  {}
  virtual result_type operator () (first_argument_type a1, second_argument_type a2) {
    return fun_ (a1, a2);
  }
};

class BlockLSf : public SearchFunctor {
private:
  int block_width_;
public:
  BlockLSf (int block_width) :
    block_width_ (block_width)
  {}
  virtual result_type operator () (first_argument_type a1, second_argument_type a2) {
    return block_lsf (a1, a2, block_width_);
  }
};

// Runs Schiavinotto & Stützle's (2004) local search (LS_f).  Reports the score
// and the time for each random restart.
class LocalSearch : public Application {
private:
  static Core::ParameterBool paramRandom;
  static Core::ParameterBool paramHybrid;
  bool RANDOM, HYBRID;
  static Core::ParameterInt paramRestart, paramBlockWidth;
  int RESTART, BLOCK_WIDTH;

  enum SearchMethod {
    search_method_lsf,
    search_method_insert,
    search_method_adjacent,
    search_method_block_lsf
  };
  static Core::Choice searchMethodChoice;
  static Core::ParameterChoice paramSearchMethod;
  SearchMethod SEARCH_METHOD;
  
  Core::Timer timer;
public:
  LocalSearch () :
    Application ("local-search")
  {}

  SearchFunctor * getSearchFunction () const {
    switch (SEARCH_METHOD) {
    case search_method_lsf: return new SearchFunctionAdapter (visit);
    case search_method_insert: return new SearchFunctionAdapter (search_insert);
    case search_method_adjacent: return new SearchFunctionAdapter (search_adjacent);
    case search_method_block_lsf: return new BlockLSf (BLOCK_WIDTH);
    default: return 0;
    }
  }

  virtual void getParameters () {
    Application::getParameters ();
    RANDOM = paramRandom (config);
    HYBRID = paramHybrid (config);
    RESTART = paramRestart (config);
    BLOCK_WIDTH = paramBlockWidth (config);
    SEARCH_METHOD = SearchMethod (paramSearchMethod (config));
  }

  virtual void printParameterDescription (std::ostream & out) const {
    paramRandom.printShortHelp (out);
    paramHybrid.printShortHelp (out);
    paramRestart.printShortHelp (out);
    paramSearchMethod.printShortHelp (out);
    paramBlockWidth.printShortHelp (out);
  }

  int main (const std::vector <std::string> & args) {
    this -> getParameters ();

    SearchFunctor & search_function = * getSearchFunction ();

    std::cout << "Name Score Iterations Time" << std::endl;
    int index = 0;
    for (std::vector <std::string>::const_iterator it = args.begin (); it != args.end (); ++ it) {
      this -> LOLIB_FILE = * it;
      // Reads the LOLIB cost matrix.
      BeforeCostRef bc (this -> lolib ());

      // Constructs the permutation.
      std::stringstream str;
      for (int i = 0; i < bc -> size (); ++ i) {
	str << i << ' ';
      }
      Permutation pi;
      readPermutationWithAlphabet (pi, str);

      for (int i = 0; i < RESTART; ++ i) {
	timer.start ();
	if (RANDOM) {
	  pi.identity ();
	  pi.randomize ();
	}

#ifndef NDEBUG
	std::cerr << pi << std::endl;
#endif
	int iterations = 1;
	if (HYBRID) {
	  for (; visit (pi, bc) > 0; ++ iterations);
	}
	for (; search_function (pi, bc) > 0; ++ iterations) {
#ifndef NDEBUG
	  std::cerr << pi << std::endl;
#endif
	}
	double score = bc -> score (pi);

	timer.stop ();

	std::cout << index ++ << ' '
		  << '"' << bc -> name () << '"' << ' '
		  << score << ' '
		  << iterations << ' '
		  << timer.user () << std::endl;
      }
    }

    return EXIT_SUCCESS;
  }
} app;

Core::ParameterBool LocalSearch::paramRandom ("random", "start at a random permutation?", true);
Core::ParameterBool LocalSearch::paramHybrid ("hybrid", "use LS_f before switching to a different method", false);
Core::ParameterInt LocalSearch::paramRestart ("restart", "the number of restarts", 100, 1);
Core::ParameterInt LocalSearch::paramBlockWidth ("block-width", "the maximum width to consider during block_lsf", Core::Type <int>::max, 1);
Core::Choice LocalSearch::searchMethodChoice ("lsf", search_method_lsf,
					      "insert", search_method_insert,
					      "adjacent", search_method_adjacent,
					      "block_lsf", search_method_block_lsf,
					      CHOICE_END);
Core::ParameterChoice LocalSearch::paramSearchMethod ("search-method",
						      & LocalSearch::searchMethodChoice,
						      "the search method",
						      LocalSearch::search_method_lsf);
