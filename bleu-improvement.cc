#include "Application.hh"
#include "BleuScore.hh"

APPLICATION

using namespace Permute;

class GreaterDifference :
  std::binary_function <const std::pair <int, double> &, const std::pair <int, double> &, bool>
{
public:
  bool operator () (const std::pair <int, double> & a,
		    const std::pair <int, double> & b) const {
    return a.second > b.second;
  }
};

class PrintDifference :
  std::unary_function <const std::pair <int, double> &, void>
{
public:
  void operator () (const std::pair <int, double> & diff) const {
    std::cout << diff.first << '\t' << diff.second << std::endl;
  }
};

class BleuImprovement : public Permute::Application {
private:
  static Core::ParameterInt paramBestN,
    paramWorstN,
    paramMiddleN;
  int BEST_N, WORST_N, MIDDLE_N;
public:
  BleuImprovement () :
    Permute::Application ("bleu-improvement")
  {}

  virtual void getParameters () {
    Application::getParameters ();
    BEST_N = paramBestN (config);
    WORST_N = paramWorstN (config);
    MIDDLE_N = paramMiddleN (config);
  }

  virtual void printParameterDescription (std::ostream & out) const {
    paramBestN.printShortHelp (out);
    paramWorstN.printShortHelp (out);
    paramMiddleN.printShortHelp (out);
  }

  int main (const std::vector <std::string> & args) {
    this -> getParameters ();

    Core::CompressedInputStream ref (args [0]);
    Core::CompressedInputStream test1 (args [1]);
    Core::CompressedInputStream test2 (args [2]);

    std::vector <BleuScore> bleu1;
    std::vector <BleuScore> bleu2;

    std::string ref_line, test1_line, test2_line;
    while (Core::wsgetline (ref, ref_line) != EOF) {
      Core::wsgetline (test1, test1_line);
      Core::wsgetline (test2, test2_line);

      NgramCount ref_count;
      int ref_length = countNgrams (ref_line, ref_count);

      bleu1.push_back (computeBleuScore (ref_length, ref_count, test1_line));
      bleu2.push_back (computeBleuScore (ref_length, ref_count, test2_line));
    }

    BleuScore total_bleu_1 = totalBleuScore (bleu1);
    std::vector <std::pair <int, double> > diffs;
    for (int i = 0; i < bleu1.size (); ++ i) {
      BleuScore score (total_bleu_1);
      score -= bleu1 [i];
      score += bleu2 [i];
      diffs.push_back (std::make_pair (i + 1, score.score () - total_bleu_1.score ()));
    }

    GreaterDifference greater_diff;
    std::sort (diffs.begin (), diffs.end (), greater_diff);

    PrintDifference print_diff;
    if (BEST_N) {
      std::cout << "Best:" << std::endl;
      std::for_each (diffs.begin (), diffs.begin () + BEST_N, print_diff);
    }
    if (WORST_N) {
      std::cout << std::endl << "Worst:" << std::endl;
      std::for_each (diffs.rbegin (), diffs.rbegin () + WORST_N, print_diff);
    }
    if (MIDDLE_N) {
      std::cout << std::endl << "Middle:" << std::endl;
      std::vector <std::pair <int, double> >::const_iterator it =
	diffs.begin () + diffs.size () / 2 - MIDDLE_N / 2;
      std::for_each (it, it + MIDDLE_N, print_diff);
    }

    return EXIT_SUCCESS;
  }
} app;

Core::ParameterInt BleuImprovement::paramBestN ("best", "the number of best sentences to output", 50, 0);
Core::ParameterInt BleuImprovement::paramWorstN ("worst", "the number of worst sentences to output", 50, 0);
Core::ParameterInt BleuImprovement::paramMiddleN ("middle", "the number of middle sentences to output", 50, 0);
