#include "Application.hh"
#include "PV.hh"

APPLICATION

using namespace Permute;

// Meant to profile construction of LOP matrices from features.
class ProfileSumBeforeScorer : public Application {
public:
  ProfileSumBeforeScorer () :
    Application ("profile-sum-before-scorer")
  {}

  int main (const std::vector <std::string> & args) {
    this -> getParameters ();

    PV pv;
    if (! this -> readPV (pv)) {
      return EXIT_FAILURE;
    }
  
    Permutation source, target, pos;

    std::istream & input = this -> input ();

    for (int sentence = 0;
	 sentence < SENTENCES && readPermutationWithAlphabet (source, input);
	 ++ sentence) {
      readPermutationWithAlphabet (pos, input);
      target = source;
      readAlignment (target, input);

      SumBeforeCostRef bc (new SumBeforeCost (source.size (), "ProfileSumBeforeScorer"));
      ScorerRef scorer = this -> sumBeforeScorer (bc, pv, source, pos);
    }

    return EXIT_SUCCESS;
  }
} app;
