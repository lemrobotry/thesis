#ifndef _PERMUTE_APPLICATION_HH
#define _PERMUTE_APPLICATION_HH

#include <Core/CompressedStream.hh>

#include <Fsa/Application.hh>

#include "Distortion.hh"
#include "InputData.hh"
#include "Parameter.hh"
#include "ParseController.hh"
#include "Permutation.hh"
#include "PV.hh"
#include "Scorer.hh"
#include "SRILM.hh"
#include "TTable.hh"

namespace Permute {

  // Provides command-line parameters and methods useful to multiple
  // applications involving permutation search.
  class Application : public Fsa::Application {
  private:
    static std::vector <int> defaultParents_;
    static Permutation defaultLabels_;
  protected:
    static Core::ParameterString
    paramInput,
      paramOutput,
      paramDevInput,
      paramDevCommand,
      paramAlphabetFile,
      paramTTableFile,
      paramLModelFile,
      paramLOPFile,
      paramLOPOutputFile,
      paramLOLIBFile;
    std::string INPUT, OUTPUT, DEV_INPUT, DEV_COMMAND,
      ALPHABET_FILE, TTABLE_FILE, LMODEL_FILE, LOP_FILE, LOP_OUTPUT_FILE,
      LOLIB_FILE;
    static Core::ParameterBool
    paramDebug,
      paramIterateSearch,
      paramDependency;
    bool DEBUG,
      ITERATE_SEARCH,
      DEPENDENCY;
    static Core::ParameterInt
    paramSentences,
      paramLearningIterations,
      paramTTableWeightCount,
      paramTTableLimit,
      paramLModelOrder,
      paramQuadraticWidth,
      paramQuadraticLeft,
      paramQuadraticRight,
      paramWindow;
    int SENTENCES, LEARNING_ITERATIONS, TTABLE_WEIGHT_COUNT, TTABLE_LIMIT,
      LMODEL_ORDER, QUADRATIC_WIDTH, QUADRATIC_LEFT, QUADRATIC_RIGHT, WINDOW;
    static Core::ParameterFloat
    paramDistortionWeight,
      paramLModelWeight,
      paramWordWeight,
      paramTolerance;
    double DISTORTION_WEIGHT, LMODEL_WEIGHT, WORD_WEIGHT, TOLERANCE;
    static Core::ParameterFloatVector paramTTableWeights;
    enum ParseControllerType {
      pc_quadratic,
      pc_cubic
    };
    static Core::Choice ParseControllerChoice;
    static Core::ParameterChoice paramParseControllerType;
    ParseControllerType PARSE_CONTROLLER_TYPE;

    virtual void getParameters ();
    
  private:
    Core::CompressedInputStream * input_;
    Core::CompressedInputStream * devInput_;
    Core::CompressedOutputStream * output_;
    PhraseDictionaryTree * ttable_;
    WeightModel * ttableWeightModel_;
    int learning_iteration_;
    
  public:
    Application (const std::string &);
    virtual ~Application ();

    virtual std::string getParameterDescription () const;
    virtual void printParameterDescription (std::ostream &) const;

    std::istream & input ();
    std::istream & devInput ();
    std::ostream & output ();

    Fsa::ConstAlphabetRef alphabet () const;

    ParseControllerRef parseController (const Permutation &) const;
    ScorerRef scorer () const;

    Fsa::ConstAutomatonRef distortion (const Permutation &) const;
    Fsa::ConstAutomatonRef ttable (const Permutation &,
				   Fsa::ConstAlphabetRef);
    Fsa::ConstAutomatonRef lmodel () const;

    bool parameters (ParameterVector &) const;
    void outputParameters (const ParameterVector &) const;

    ScorerRef beforeScorer (const Permutation &, const ParameterVector &, const Permutation &) const;
    ScorerRef lossScorer (const Permutation & words, const Permutation & target) const;

    BeforeCost * lolib () const;

    void decodeDev (const ParameterVector &, const std::vector <double> &);

    bool readPV (PV &) const;
    void writePV (const PV &, int iter = 0) const;
    void sumBeforeCost (SumBeforeCostRef bc, const PV & pv,
			const Permutation & words, const Permutation & pos,
			const std::vector <int> & = defaultParents_,
			const Permutation & = defaultLabels_) const;
    ScorerRef sumBeforeScorer (SumBeforeCostRef bc, const PV & pv,
			       const Permutation & words, const Permutation & pos,
			       const std::vector <int> & parents = defaultParents_,
			       const Permutation & labels = defaultLabels_) const;
    ScorerRef sumBeforeScorer (SumBeforeCostRef bc, const PV & pv,
			       const InputData & data) const;
    void decodeDev (const PV &);

    void perceptronUpdate (SumBeforeCostRef bc, const Permutation & pi, double amount) const;

    template <class A, class B>
    bool converged (const A &, const B &);

    bool converged ();
  };

  // Prints to std::cerr the Euclidean distance from A to B, the Euclidean
  // norm of B, the ratio between the distance and the norm, and the cosine
  // distance between A and B.  Returns true if --learning-iterations have been
  // surpassed or if the distance/norm ratio is less than --tolerance.
  template <class A, class B>
  bool Application::converged (const A & from, const B & to) {
    double d = Permute::distance (from, to);
    double n = Permute::norm (to);
    std::cerr << d << '\t'
	      << n << '\t'
	      << d / n << '\t'
	      << Permute::cos (from, to) << std::endl;
    if (LEARNING_ITERATIONS > 0) {
      return ++ learning_iteration_ > LEARNING_ITERATIONS;
    } else {
      return ((d / n) < TOLERANCE);
    }
  }
}

#endif//_PERMUTE_APPLICATION_HH
