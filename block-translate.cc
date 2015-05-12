#include <Fsa/Cache.hh>
#include <Fsa/Compose.hh>
#include <Fsa/Output.hh>

#include "Application.hh"
#include "BlockSearch.hh"
#include "Fsa.hh"
#include "FsaCellMap.hh"
#include "StartEndCell.hh"

APPLICATION

using namespace Permute;

class BlockTranslate : public Application {
private:
  BestOutput bestOutput_;
  Core::Timer timer_;
public:
  BlockTranslate () :
    Application ("block-translate"),
    bestOutput_ (),
    timer_ ()
  {}

  int main (const std::vector <std::string> & args) {
    this -> getParameters ();

    std::cerr << "Reading PV: [" << LOP_FILE << "]" << std::endl;
    timer_.start ();
    PV pv;
    if (! this -> readPV (pv)) {
      return EXIT_FAILURE;
    }
    timer_.stop ();
    std::cerr << "Done: [" << timer_.user () << "s]" << std::endl;

    Permutation source, target, pos, labels;
    std::vector <int> parents;

    std::istream & in = this -> input ();
    std::ostream & out = this -> output ();

    std::cerr << "Reading LM: [" << LMODEL_FILE << "]" << std::endl;
    timer_.start ();
    Fsa::ConstAutomatonRef language = this -> lmodel ();
    timer_.stop ();
    std::cerr << "Done: [" << timer_.user () << "s]" << std::endl;

    while (readPermutationWithAlphabet (source, in)) {
      readPermutationWithAlphabet (pos, in);
      if (DEPENDENCY) {
	readParents (parents, in);
	readPermutationWithAlphabet (labels, in);
      }
      target = source;
      readAlignment (target, in);

      std::cerr << "Translating: ["
		<< source
		<< "]" << std::endl;
      timer_.start ();

      SumBeforeCostRef bc (new SumBeforeCost (source.size (), "block-translate"));
      this -> sumBeforeCost (bc, pv, source, pos, parents, labels);

      Fsa::ConstAutomatonRef
	//	distortion = this -> distortion (source),
	channel = this -> ttable (source, language -> getInputAlphabet ()),
	a = Fsa::cache (Fsa::composeMatching (channel, language, false));
// 	a = Fsa::cache (Fsa::composeMatching (distortion,
// 					      Fsa::composeMatching (channel,
// 								    language,
// 								    false),
// 					      false));

      FsaCellMap * cellMapP = new FsaCellMap (a);
      CellMapRef cellMap (cellMapP);
      ConstCellRef closure (cellMapP -> epsilonClosure ());
      CellRef topCell (new StartEndCell (a, closure));
      
      while (blockSearch (source, bc, WINDOW, a, cellMap, topCell, closure) > 0.0);

      Fsa::ConstAutomatonRef
	sentence = fsa (source, Fsa::TropicalSemiring),
	best = bestOutput_ (sentence, a);
      Fsa::writeLinear (best, out);
      out << std::endl;

      timer_.stop ();
      std::cerr << "Done: [" << timer_.user () << "s]" << std::endl;
    }
    
    return EXIT_SUCCESS;  
  }
  
} app;
