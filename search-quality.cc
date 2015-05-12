#include <numeric>

#include "Application.hh"
#include "Statistics.hh"

APPLICATION

using namespace Permute;

class SearchTrial {
  friend std::istream & operator >> (std::istream & in, SearchTrial & trial);
public:
  int index_;
  std::string matrix_;
  double objective_;
  int iterations_;
  double time_;

  SearchTrial () :
    index_ (0),
    matrix_ (""),
    objective_ (0),
    iterations_ (0),
    time_ (0)
  {}

  SearchTrial & operator += (const SearchTrial & other);
};

std::istream & operator >> (std::istream & in, SearchTrial & trial) {
  return in >> trial.index_
	    >> trial.matrix_
	    >> trial.objective_
	    >> trial.iterations_
	    >> trial.time_;
}

SearchTrial & SearchTrial::operator += (const SearchTrial & other) {
  index_ = std::max (index_, other.index_);
  matrix_ = other.matrix_;
  objective_ = std::max (objective_, other.objective_);
  iterations_ += other.iterations_;
  time_ += other.time_;
  return * this;
}

SearchTrial operator + (SearchTrial t1, const SearchTrial & t2) {
  return t1 += t2;
}  

// Reads in data consisting of search trials on LOLIB problems with objective
// function values and times required to achieve them.  Generates PERMUTATIONS
// permutations of these data for each problem, and computes the average and
// standard deviation of the objective function at time intervals of INTERVAL.
class SearchQuality : public Application {
private:
  static Core::ParameterInt paramPermutations;
  int PERMUTATIONS;
  static Core::ParameterFloat paramInterval;
  double INTERVAL;
public:
  SearchQuality () :
    Application ("search-quality")
  {}

  virtual void printParameterDescription (std::ostream & out) const {
    paramPermutations.printShortHelp (out);
    paramInterval.printShortHelp (out);
  }

  virtual void getParameters () {
    Application::getParameters ();
    PERMUTATIONS = paramPermutations (config);
    INTERVAL = paramInterval (config);
  }

  void process (std::vector <SearchTrial> & trials) {
    SearchTrial total = std::accumulate (trials.begin (), trials.end (),
					 SearchTrial ());
    std::vector <Statistics> stats (static_cast <int> (std::floor (total.time_ / INTERVAL)) + 1);
    for (int p = 0; p < PERMUTATIONS; ++ p) {
      std::random_shuffle (trials.begin (), trials.end ());
      std::vector <SearchTrial> partials (trials.size ());
      std::partial_sum (trials.begin (), trials.end (),
			partials.begin ());
      SearchTrial current;
      std::vector <SearchTrial>::const_iterator it = partials.begin ();
      for (int t = 0; t < stats.size (); ++ t) {
	while (t * INTERVAL > it -> time_ && it != partials.end ()) {
	  current = * it;
	  ++ it;
	}
	stats [t] += current.objective_;
      }
    }
    for (int t = 0; t < stats.size (); ++ t) {
      std::cout << trials [0].matrix_ << " "
		<< t * INTERVAL << " "
		<< stats [t].mean () << " "
		<< stats [t].stdev () << std::endl;
    }
  }

  int main (const std::vector <std::string> & args) {
    this -> getParameters ();

    SearchTrial trial;
    std::vector <SearchTrial> trials;

    // Reads the header line from the data file:
    std::string header;
    getline (std::cin, header);
    // Processes the remaining lines, containing trial entries.
    while (std::cin >> trial) {
      if (! trials.empty () && trials.back ().matrix_ != trial.matrix_) {
	process (trials);
	trials.clear ();
      }
      trials.push_back (trial);
    }
    process (trials);

    return EXIT_SUCCESS;
  }
} app;

Core::ParameterInt SearchQuality::paramPermutations
("permutations", "the number of permutations of the trials", 1, 1);
Core::ParameterFloat SearchQuality::paramInterval
("interval", "the time interval to use for reporting", 1, 0);
