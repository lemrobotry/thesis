#include "Application.hh"
#include "BleuScore.hh"

APPLICATION

using namespace Permute;

// Command-line arguments should include three file names: the reference
// ordering and two candidate orderings.  Reads each line from the three files
// in parallel and collects two vectors of per-sentence BLEU statistics.
//
// If --sign is set, performs a sign test by counting the number of sentences
// for which the precisions in the second candidate improves the total BLEU
// score over the precisions in the first candidate (corpus BLEU score measured
// sentence-by-sentence). 
//
// If --permutation is set, performs a paired permutation test.  Computes the
// difference between the BLEU scores, then performs several iterations of
// randomly permuting the pairs of scores in the vectors and computes the
// differences in BLEU score that result.  Measures and reports the percentile
// of the original difference.
class BleuSignificance : public Permute::Application {
private:
  static Core::ParameterBool paramSignTest;
  static Core::ParameterBool paramPermutationTest;
  bool SIGN_TEST;
  bool PERMUTATION_TEST;
  static Core::ParameterInt paramIterations;
  int ITERATIONS;
public:
  BleuSignificance () :
    Permute::Application ("bleu-significance")
  {}

  virtual void getParameters () {
    Application::getParameters ();
    SIGN_TEST = paramSignTest (config);
    PERMUTATION_TEST = paramPermutationTest (config);
    ITERATIONS = paramIterations (config);
  }

  virtual void printParameterDescription (std::ostream & out) const {
    paramSignTest.printShortHelp (out);
    paramPermutationTest.printShortHelp (out);
    paramIterations.printShortHelp (out);
  }

  int main (const std::vector <std::string> & args) {
    this -> getParameters ();

    Core::CompressedInputStream ref (args [0]);
    Core::CompressedInputStream test1 (args [1]);
    Core::CompressedInputStream test2 (args [2]);

    std::vector <BleuScore> bleu1;
    std::vector <BleuScore> bleu2;

    // Collects BLEU statistics.
    std::string ref_line, test1_line, test2_line;
    while (Core::wsgetline (ref, ref_line) != EOF) {
      Core::wsgetline (test1, test1_line);
      Core::wsgetline (test2, test2_line);

      std::map <std::vector <std::string>, int> ref_count;
      int ref_length = countNgrams (ref_line, ref_count);
      std::map <std::vector <std::string>, int> test1_count;
      countNgrams (test1_line, test1_count);
      std::map <std::vector <std::string>, int> test2_count;
      countNgrams (test2_line, test2_count);

      bleu1.push_back (computeBleuScore (ref_length, ref_count, test1_count));
      bleu2.push_back (computeBleuScore (ref_length, ref_count, test2_count));
    }

    // Computes the difference between the BLEU scores.
    BleuScore bleu_1 = totalBleuScore (bleu1);
    BleuScore bleu_2 = totalBleuScore (bleu2);
    double true_difference = bleu_2.score () - bleu_1.score ();
    std::cout << "First BLEU Score:  " << bleu_1 << std::endl
	      << "Second BLEU Score: " << bleu_2 << std::endl
	      << "Difference:        " << true_difference << std::endl;

    if (SIGN_TEST) {
      int less = 0;
      int equal = 0;
      int greater = 0;
      double score1 = bleu_1.score ();
      for (int i = 0; i < bleu1.size (); ++ i) {
	BleuScore score (bleu_1);
	score -= bleu1 [i];
	score += bleu2 [i];
	double score2 = score.score ();
	if (score2 > score1) {
	  ++ greater;
	} else if (score2 < score1) {
	  ++ less;
	} else {
	  ++ equal;
	}
      }

      std::cout << "Less:    " << less << std::endl
		<< "Equal:   " << equal << std::endl
		<< "Greater: " << greater << std::endl;
    }

    if (PERMUTATION_TEST) {
      // Performs several iterations of random permutation.
      int smaller_count = 0;
      for (int i = 0; i < ITERATIONS; ++ i) {
	for (int j = 0; j < bleu1.size (); ++ j) {
	  if (std::rand () & (1 << 25)) {
	    bleu1 [j].swap (bleu2 [j]);
	  }
	}
	double diff = totalBleuScore (bleu2).score () - totalBleuScore (bleu1).score ();
	if (std::abs (diff) < std::abs (true_difference)) {
	  ++ smaller_count;
	}
      }

      std::cout << "Percentile:        " << smaller_count << " / " << ITERATIONS
		<< " = " << static_cast <double> (smaller_count) / static_cast <double> (ITERATIONS)
		<< std::endl;
    }

    return EXIT_SUCCESS;
  }
} app;

Core::ParameterBool BleuSignificance::paramSignTest ("sign", "perform a sign test following Collins et al. 2005", true);
Core::ParameterBool BleuSignificance::paramPermutationTest ("permutation", "perform a paired permutation test", false);
Core::ParameterInt BleuSignificance::paramIterations ("iterations", "the number of permutations of the pairwise BLEU scores to perform", 1000, 1);
