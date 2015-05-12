#include <Fsa/AlphabetXml.hh>

#include "Application.hh"
#include "BeforeScorer.hh"
#include "BleuScore.hh"
#include "ChartFactory.hh"

namespace Permute {
  Core::ParameterString Application::paramOutput ("output", "the output file", "-"),
    Application::paramInput ("input", "the input file", "-"),
    Application::paramDevInput ("dev-input", "the development set input file", "-"),
    Application::paramDevCommand ("dev-command", "the shell command to evaluate dev decoding", ""),
    Application::paramAlphabetFile ("alphabet-file", "the alphabet file"),
    Application::paramTTableFile ("ttable-file", "the phrase translation table file"),
    Application::paramLModelFile ("lmodel-file", "the language model file"),
    Application::paramLOPFile ("lop-file", "the LOP parameter file"),
    Application::paramLOPOutputFile ("lop-output-file", "the LOP parameter output file"),
    Application::paramLOLIBFile ("lolib-file", "the score matrix file");

  Core::ParameterBool Application::paramDebug ("debug", "application dependent behavior", false),
    Application::paramIterateSearch ("iterate-search", "perform iterated local search", true),
    Application::paramDependency ("dependency", "use dependency features and streams", false);

  Core::ParameterInt Application::paramLearningIterations ("learning-iterations", "the number of iterations of learning to perform", 0, 0),
    Application::paramSentences ("sentences", "the number of sentences to train on", Core::Type<int>::max, 1),
    Application::paramTTableWeightCount ("ttable-scores", "number of scores for each phrase translation", 5, 1),
    Application::paramTTableLimit ("ttable-limit", "number of phrase translations used for each foreign phrase", 20, 0),
    Application::paramLModelOrder ("lmodel-order", "the order of the language model", 0, 0),
    Application::paramQuadraticWidth ("quadratic-width", "the width of the quadratic parse controller", 1, 1),
    Application::paramQuadraticLeft ("quadratic-left", "the left anchor width", 0, 0),
    Application::paramQuadraticRight ("quadratic-right", "the right anchor width", 0, 0),
    Application::paramWindow ("window", "the maximum allowed swap width", 0, 0);

  Core::ParameterFloat Application::paramDistortionWeight ("weight-d", "the weight of the geometric distortion model", 0.6, 0.0),
    Application::paramLModelWeight ("weight-l", "the weight of the language model", 0.5),
    Application::paramWordWeight ("weight-w", "the weight of the word penalty", -1.0),
    Application::paramTolerance ("tolerance", "the stopping criterion for weight convergence", 1e-6, 0.0);

  Core::ParameterFloatVector Application::paramTTableWeights ("weight-t", "the translation model weights");

  Core::Choice Application::ParseControllerChoice ("cubic", pc_cubic, "quadratic", pc_quadratic, CHOICE_END);
  Core::ParameterChoice Application::paramParseControllerType ("parse-controller", & Application::ParseControllerChoice, "the parse controller type", Application::pc_cubic);

  std::vector <int> Application::defaultParents_;
  Permutation Application::defaultLabels_;

  /**********************************************************************/

  Application::Application (const std::string & title) :
    Fsa::Application (),
    input_ (0),
    output_ (0),
    ttable_ (0),
    ttableWeightModel_ (0),
    learning_iteration_ (1)
  {
    setTitle (title);
    setDefaultLoadConfigurationFile (false);
    setDefaultOutputXmlHeader (false);
  }

  Application::~Application () {
    delete ttableWeightModel_;
    delete ttable_;
    delete input_;
    delete output_;
  }

  std::string Application::getParameterDescription () const {
    std::ostringstream out;
    paramOutput.printShortHelp (out);
    paramInput.printShortHelp (out);
    paramDevInput.printShortHelp (out);
    paramDevCommand.printShortHelp (out);
    paramAlphabetFile.printShortHelp (out);
    paramTTableFile.printShortHelp (out);
    paramLModelFile.printShortHelp (out);
    paramLOPFile.printShortHelp (out);
    paramLOPOutputFile.printShortHelp (out);
    paramLOLIBFile.printShortHelp (out);

    paramDebug.printShortHelp (out);
    paramIterateSearch.printShortHelp (out);
    paramDependency.printShortHelp (out);

    paramLearningIterations.printShortHelp (out);
    paramSentences.printShortHelp (out);
    paramTTableWeightCount.printShortHelp (out);
    paramTTableLimit.printShortHelp (out);
    paramLModelOrder.printShortHelp (out);
    paramQuadraticWidth.printShortHelp (out);
    paramQuadraticLeft.printShortHelp (out);
    paramQuadraticRight.printShortHelp (out);
    paramWindow.printShortHelp (out);

    paramDistortionWeight.printShortHelp (out);
    paramLModelWeight.printShortHelp (out);
    paramWordWeight.printShortHelp (out);
    paramTolerance.printShortHelp (out);

    paramTTableWeights.printShortHelp (out);

    paramParseControllerType.printShortHelp (out);

    out << "specific options" << std::endl;
    this -> printParameterDescription (out);

    return out.str ();
  }

  void Application::printParameterDescription (std::ostream & out) const {
    return;
  }

  // Accesses each command-line parameter a single time and stores it in a
  // member variable to avoid leaking memory.
  void Application::getParameters () {
    INPUT = paramInput (config);
    OUTPUT = paramOutput (config);
    DEV_INPUT = paramDevInput (config);
    DEV_COMMAND = paramDevCommand (config);
    ALPHABET_FILE = paramAlphabetFile (config);
    TTABLE_FILE = paramTTableFile (config);
    LMODEL_FILE = paramLModelFile (config);
    LOP_FILE = paramLOPFile (config);
    LOP_OUTPUT_FILE = paramLOPOutputFile (config);
    LOLIB_FILE = paramLOLIBFile (config);
    DEBUG = paramDebug (config);
    ITERATE_SEARCH = paramIterateSearch (config);
    DEPENDENCY = paramDependency (config);
    SENTENCES = paramSentences (config);
    LEARNING_ITERATIONS = paramLearningIterations (config);
    TTABLE_WEIGHT_COUNT = paramTTableWeightCount (config);
    TTABLE_LIMIT = paramTTableLimit (config);
    LMODEL_ORDER = paramLModelOrder (config);
    QUADRATIC_WIDTH = paramQuadraticWidth (config);
    QUADRATIC_LEFT = paramQuadraticLeft (config);
    QUADRATIC_RIGHT = paramQuadraticRight (config);
    WINDOW = paramWindow (config);
    DISTORTION_WEIGHT = paramDistortionWeight (config);
    LMODEL_WEIGHT = paramLModelWeight (config);
    WORD_WEIGHT = paramWordWeight (config);
    TOLERANCE = paramTolerance (config);
    PARSE_CONTROLLER_TYPE = ParseControllerType (paramParseControllerType (config));
  }

  // Returns a new copy of the INPUT file, or std::cin if INPUT is "-".
  std::istream & Application::input () {
    if (INPUT == "-") {
      return std::cin;
    } else {
      delete input_;
      input_ = new Core::CompressedInputStream (INPUT);
      return * input_;
    }
  }

  // Returns a new copy of the DEV_INPUT file, or std::cin if DEV_INPUT is "-".
  std::istream & Application::devInput () {
    if (DEV_INPUT == "-") {
      return std::cin;
    } else {
      delete devInput_;
      devInput_ = new Core::CompressedInputStream (DEV_INPUT);
      return * devInput_;
    }
  }

  // Returns a new copy of the OUTPUT file, or std::cout if OUTPUT is "-".
  std::ostream & Application::output () {
    if (OUTPUT == "-") {
      return std::cout;
    } else {
      delete output_;
      output_ = new Core::CompressedOutputStream (OUTPUT);
      return * output_;
    }
  }

  /**********************************************************************/

  // Returns the alphabet read from ALPHABET_FILE.
  Fsa::ConstAlphabetRef Application::alphabet () const {
    return Fsa::readAlphabet (ALPHABET_FILE);
  }

  /**********************************************************************/

  // Returns the ParseController indicated by the command-line parameters.  This
  // is either a cubic controller or a quadratic controller with a particular
  // width, optionally decorated with left and right anchor widths.
  ParseControllerRef Application::parseController (const Permutation & p) const {
    ParseControllerRef controller;
    if (PARSE_CONTROLLER_TYPE == pc_cubic) {
      controller = CubicParseController::create ();
    } else if (PARSE_CONTROLLER_TYPE == pc_quadratic) {
      controller = QuadraticParseController::create (QUADRATIC_WIDTH);
      if (QUADRATIC_LEFT > 0) {
	controller = LeftAnchorParseController::decorate (controller, QUADRATIC_LEFT);
      }
      if (QUADRATIC_RIGHT > 0) {
	controller = RightAnchorParseController::decorate (controller, p, QUADRATIC_RIGHT);
      }
    }
    return controller;
  }

  // Returns an empty scorer.
  ScorerRef Application::scorer () const {
    return ScorerRef (new Scorer);
  }

  /**********************************************************************/

  // Returns a distortion automaton appropriate for the given Permutation, using
  // the DISTORTION_WEIGHT parameter.
  Fsa::ConstAutomatonRef Application::distortion (const Permutation & p) const {
    return Permute::distortion (p, - ::log (DISTORTION_WEIGHT));
  }

  // Returns a TTable automaton appropriate for the given Permutation, using the
  // TTABLE_FILE parameter and the command-line TTable weights.
  Fsa::ConstAutomatonRef Application::ttable (const Permutation & p,
					      Fsa::ConstAlphabetRef output) {
    if (ttable_ == 0) {
      ttable_ = new PhraseDictionaryTree (TTABLE_WEIGHT_COUNT);
      ttable_ -> Read (TTABLE_FILE);
    }
    if (ttableWeightModel_ == 0) {
      Core::ParameterFloatVector::Vector weightt = paramTTableWeights (config);
      if (weightt.empty ()) {
	for (int i = 0; i < TTABLE_WEIGHT_COUNT; ++ i) {
	  weightt.push_back (1.0 / TTABLE_WEIGHT_COUNT);
	}
      }
      ttableWeightModel_ = new TTableWeights (weightt);
    }

    return Permute::ttable (p, output, * ttableWeightModel_, * ttable_, WORD_WEIGHT);
  }

  // Returns a language model automaton representing the subset of the SRILM in
  // LMODEL_FILE that includes symbols from the given alphabet.  Paramters
  // LMODEL_ORDER and LMODEL_WEIGHT determine the size and weight of the model.
  Fsa::ConstAutomatonRef Application::lmodel () const {
    Core::CompressedInputStream lm (LMODEL_FILE);
    return Permute::srilm (lm, LMODEL_ORDER, LMODEL_WEIGHT);
  }

  /**********************************************************************/

  // Returns a ParameterVector read from LOP_FILE.
  bool Application::parameters (ParameterVector & pv) const {
    ParameterVectorXmlParser parser (config, pv);
    bool rv = parser.parseFile (LOP_FILE);
    if (! rv) {
      std::cerr << "Could not read LOP parameter file: " << LOP_FILE << std::endl;
    }
    return rv;
  }

  // Writes a ParameterVector to LOP_OUTPUT_FILE.
  void Application::outputParameters (const ParameterVector & pv) const {
    Core::CompressedOutputStream output;
    output.open (LOP_OUTPUT_FILE);
    writeXml (pv, output);
  }

  /**********************************************************************/

  // Returns a Scorer for the given permutation derived from a ParameterVector.
  ScorerRef Application::beforeScorer (const Permutation & words, const ParameterVector & pv, const Permutation & pos) const {
    BeforeCost * bc (new BeforeCost (pos.size (), "Application::beforeScorer"));
    for (Permutation::const_iterator i = pos.begin (); i != -- pos.end (); ++ i) {
      for (Permutation::const_iterator j = i + 1; j != pos.end (); ++ j) {
	bc -> setCost (* i, * j, sum (pv, pos, * i, * j));
      }
    }
    return ScorerRef (new BeforeScorer (BeforeCostRef (bc), words));
  }

  // Returns a Scorer for the given permutation that measures loss relative to
  // the given target permutation.
  ScorerRef Application::lossScorer (const Permutation & words, const Permutation & target) const {
    return tauScorer (target, words);
  }

  // Returns the unnormalized LOP score matrix contained in the --lolib-file
  // parameter.
  BeforeCost * Application::lolib () const {
    Core::CompressedInputStream input (LOLIB_FILE);
    if (! input) {
      std::cerr << "Could not read LOLIB matrix file: " << LOLIB_FILE << std::endl;
      return 0;
    }
    return readLOLIB (input);
  }

  /**********************************************************************/

  // Reads a given PV from LOP_FILE and returns success.
  bool Application::readPV (PV & pv) const {
    bool rv = readFile (pv, LOP_FILE);
    if (! rv) {
      std::cerr << "Could not read LOP parameter file: " << LOP_FILE << std::endl;
    }
    return rv;
  }

  // Writes the given PV to LOP_OUTPUT_FILE.
  void Application::writePV (const PV & pv, int iter) const {
    Core::CompressedOutputStream output;
    if (iter) {
      std::ostringstream file;
      file << LOP_OUTPUT_FILE.substr (0, LOP_OUTPUT_FILE.rfind (".xml.gz"))
	   << ".iter-" << iter << ".xml.gz";
      output.open (file.str ());
    } else {
      output.open (LOP_OUTPUT_FILE);
    }
    writeXml (pv, output);
  }

  void Application::sumBeforeCost (SumBeforeCostRef bc, const PV & pv,
				   const Permutation & words,
				   const Permutation & pos,
				   const std::vector <int> & parents,
				   const Permutation & labels) const {
    for (Permutation::const_iterator i = words.begin (); i != -- words.end (); ++ i) {
      for (Permutation::const_iterator j = i + 1; j != words.end (); ++ j) {
	if ((* i) >= (* j)) {
	  std::cerr << "sumBeforeCost received non-identity permutation!" << std::endl;
	  return;
	}
	std::vector <std::string> features;
	if (DEPENDENCY) {
	  pv.features (features, words, pos, parents, labels, (* i), (* j));
	} else {
	  pv.features (features, words, pos, (* i), (* j));
	}
	for (std::vector <std::string>::const_iterator f = features.begin (); f != features.end (); ++ f) {
	  PV::const_iterator phi = pv.find (* f);
	  if (phi != pv.end ()) {
	    (* bc) (* i, * j) += phi -> second;
	  }
	}
      }
    }
  }
  
  // Returns a Scorer for the given permutation derived from the given PV.
  //
  // Assumes that we're going to be dealing with the identity permutation, so
  // that i will always be less than j.  That's essential because the PV won't
  // correctly compute features when its arguments are out of order.
  //
  // NOTE: PV is const so that weights for missing features can't be added to
  // it.
  ScorerRef Application::sumBeforeScorer (SumBeforeCostRef bc, const PV & pv,
					  const Permutation & words,
					  const Permutation & pos,
					  const std::vector <int> & parents,
					  const Permutation & labels) const {
    sumBeforeCost (bc, pv, words, pos, parents, labels);
    return ScorerRef (new BeforeScorer (BeforeCostRef (bc), words));
  }

  ScorerRef Application::sumBeforeScorer (SumBeforeCostRef bc, const PV & pv,
					  const InputData & data) const {
    sumBeforeCost (bc, pv, data.source (), data.pos (), data.parents (), data.labels ());
    return ScorerRef (new BeforeScorer (BeforeCostRef (bc), data.source ()));
  }

  /**********************************************************************/

  // Decodes the dev set using the given ParameterVector with the given
  // weights.  Sends the output of each (source, POS, alignment) to a pipe
  // running DEV_COMMAND.
  void Application::decodeDev (const ParameterVector & copy, const std::vector <double> & weights) {
    ParameterVector pv (copy);
    Permute::set (pv, weights);

    ChartFactoryRef factory = ChartFactory::create ();
    Permutation source, target,
      pos (pv.getPOS ());
    ParseControllerRef controller = this -> parseController (source);

    // open the output pipe
    FILE * pipe = popen (DEV_COMMAND.c_str (), "w");

    std::istream & in = this -> devInput ();
    while (readPermutationWithAlphabet (source, in)) {
      readPermutation (pos, in);
      target = source;
      readAlignment (target, in);

      ScorerRef scorer = this -> beforeScorer (source, pv, pos);
      ChartRef chart = factory -> chart (source, WINDOW);

      double best_score = scorer -> score (source);
      do {
	Chart::permute (chart, controller, scorer);
	ConstPathRef bestPath = chart -> getBestPath ();
	source.changed (false);
	if (bestPath -> getScore () > best_score) {
	  best_score = bestPath -> getScore ();
	  source.reorder (bestPath);
	}
      } while (ITERATE_SEARCH && source.changed ());

      fprintf (pipe, "%s", source.toString ().c_str ());
    }

    // close the output pipe
    pclose (pipe);

    return;
  }

  /**********************************************************************/

  // Decodes the dev set using the given PV.  Sends the output of each (source,
  // POS, alignment) to a pipe running DEV_COMMAND.
  void Application::decodeDev (const PV & pv) {
    ChartFactoryRef factory = ChartFactory::create ();
    Permutation source, target, pos;
    ParseControllerRef controller = this -> parseController (source);

    BleuScore bleu;

    std::istream & in = this -> devInput ();
    while (readPermutationWithAlphabet (source, in)) {
      readPermutationWithAlphabet (pos, in);
      target = source;
      readAlignment (target, in);

      SumBeforeCostRef bc (new SumBeforeCost (source.size (), "Application::decodeDev"));
      ScorerRef scorer = this -> sumBeforeScorer (bc, pv, source, pos);
      ChartRef chart = factory -> chart (source, WINDOW);

      double best_score = scorer -> score (source);
      do {
	Chart::permute (chart, controller, scorer);
	ConstPathRef bestPath = chart -> getBestPath ();
	source.changed (false);
	if (bestPath -> getScore () > best_score) {
	  best_score = bestPath -> getScore ();
	  source.reorder (bestPath);
	}
      } while (ITERATE_SEARCH && source.changed ());

      bleu += computeBleuScore (target, source);
    }

    std::cerr << "BLEU = " << bleu << std::endl;

    return;
  }

  void Application::perceptronUpdate (SumBeforeCostRef bc,
				      const Permutation & pi,
				      double amount) const {
    for (Permutation::const_iterator i = pi.begin (); i != -- pi.end (); ++ i) {
      for (Permutation::const_iterator j = i + 1; j != pi.end (); ++ j) {
	if (* i < * j) {
	  (* bc) (* i, * j).add (amount);
	}
      }
    }
  }

  bool Application::converged () {
    return ++ learning_iteration_ > LEARNING_ITERATIONS;
  }
}
