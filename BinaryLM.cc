#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

#include "BinaryLM.hh"
#include "SRILM.hh"

namespace Permute {

  /**********************************************************************
   * The format of the binary language model file is as follows:
   *   A sequence of (LMStateHeader, LMArc list) pairs, beginning with
   *   the state corresponding to the start symbol '<s>'.
   **********************************************************************/

  struct LMStateHeader {
    int order;
    Fsa::StateId backoff_state;
    double backoff_weight;
    double final_weight;
    unsigned arc_list_size;
  };
  struct LMArc {
    Fsa::LabelId symbol;
    Fsa::StateId target_state;
    double weight;
  };

  LMArc * binary_search (LMArc * begin, LMArc * end, Fsa::LabelId index) {
    if (begin >= end) {
      return 0;
    } else {
      LMArc * middle = begin + ((end - begin) / 2);
      if (middle -> symbol == index) {
	return middle;
      } else if (middle -> symbol < index) {
	return binary_search (middle + 1, end, index);
      } else {
	return binary_search (begin, middle, index);
      }
    }
  }
  
  /**********************************************************************/

  class BinaryLM : public Fsa::Automaton {
  private:
    Fsa::ConstAlphabetRef alphabet_;
    Fsa::ConstAlphabetRef lm_alphabet_;
    int order_;
    int fd_;
  public:
    BinaryLM (Fsa::ConstAlphabetRef alphabet, int order) :
      Fsa::Automaton (),
      alphabet_ (alphabet),
      order_ (order),
      fd_ (-1)
    {}
    virtual ~BinaryLM () {
      ::close (fd_);
    }
    bool valid () const {
      return (fd_ >= 0);
    }
    bool open (const std::string & file, Fsa::ConstAlphabetRef lm_alphabet) {
      if (valid ()) { ::close (fd_); }
      fd_ = ::open (file.c_str (), O_RDONLY);
      lm_alphabet_ = lm_alphabet;
      return valid ();
    }
    /**********************************************************************
     * Fsa::Automaton methods
     **********************************************************************/
    virtual Fsa::Type type () const {
      return Fsa::TypeAcceptor;
    }
    virtual Fsa::ConstSemiringRef semiring () const {
      return Fsa::LogSemiring;
    }
    virtual Fsa::StateId initialStateId () const {
      return 0;
    }
    virtual Fsa::ConstAlphabetRef getInputAlphabet () const {
      return alphabet_;
    }
    virtual Fsa::ConstStateRef getState (Fsa::StateId s) const {
      Fsa::State * state (new Fsa::State (s));
      // Seek the file position.
      LMStateHeader info;
      getStateHeader (s, & info);
      if (info.final_weight != Core::Type <double>::max) {
	state -> setFinal (Fsa::Weight (info.final_weight));
      }
      // Find all alphabet's symbols specified at this state.
      std::vector <Fsa::Arc *> unseen;
      LMArc arc_list [info.arc_list_size];
      ::read (fd_, arc_list, sizeof (arc_list));
      for (Fsa::Alphabet::const_iterator sym = alphabet_ -> begin (); sym != alphabet_ -> end (); ++ sym) {
	Fsa::Arc * arc = state -> newArc ();
	arc -> input_ = arc -> output_ = sym;
	LMArc * seen = find (arc_list, info.arc_list_size, * sym);
	if (seen == 0) {
	  unseen.push_back (arc);
	} else {
	  arc -> target_ = getTargetState (seen -> target_state);
	  arc -> weight_ = Fsa::Weight (seen -> weight);
	}
      }
      // Back off.
      Fsa::ConstStateRef backoff = getState (info.backoff_state);
      Fsa::Weight bow (info.backoff_weight);
      std::vector <Fsa::Arc *>::const_iterator ua = unseen.begin ();
      for (Fsa::State::const_iterator boa = backoff -> begin ();
	   boa != backoff -> end () && ua != unseen.end (); ++ boa) {
	if (boa -> input () == (* ua) -> input_) {
	  (* ua) -> target_ = boa -> target ();
	  (* ua) -> weight_ = semiring () -> extend (boa -> weight (), bow);
	  ++ ua;
	}
      }
      // Cache the state?

      return Fsa::ConstStateRef (state);
    }
    virtual std::string describe () const {
      return "BinaryLM";
    }
    /**********************************************************************/
  private:
    void getStateHeader (Fsa::StateId s, LMStateHeader * header) const {
      ::lseek (fd_, s, SEEK_SET);
      ::read (fd_, header, sizeof (LMStateHeader));
    }
    // The default target state may carry too much information for the
    // user, in which case its backoff state should always be
    // appropriate.  Therefore, this method probably need not be
    // recursive.  But, we'll start safe.
    Fsa::StateId getTargetState (Fsa::StateId target) const {
      LMStateHeader info;
      getStateHeader (target, & info);
      if (info.order > order_) {
	return getTargetState (info.backoff_state);
      } else {
	return target;
      }
    }
    // Binary search for the symbol's equivalent in the LM's
    // alphabet.  Return 0 if not found.
    LMArc * find (LMArc * arc_list, unsigned size, std::string symbol) const {
      return binary_search (arc_list, arc_list + size, lm_alphabet_ -> index (symbol));
    }
  };

  Fsa::ConstAutomatonRef binaryLM (Fsa::ConstAlphabetRef alphabet, int order, const std::string & file, Fsa::ConstAlphabetRef lm_alphabet) {
    BinaryLM * blm (new BinaryLM (alphabet, order));
    if (blm -> open (file, lm_alphabet)) {
      return Fsa::ConstAutomatonRef (blm);
    } else {
      delete blm;
      return Fsa::ConstAutomatonRef ();
    }
  }

  /**********************************************************************/

  class BinarizeSRILMState : public SRILMState {
  public:
    BinarizeSRILMState () :
      SRILMState ()
    {}
    virtual void process_ngram (Fsa::Weight ngw, const std::vector <std::string> & ngram, Fsa::Weight bow) const {
      
    }
  };

  // Read an LM in SRI format and construct a binary version in the
  // provided file.
  //
  // For now, assume the LM will fit in memory and build a vector of
  // LMStateHeaders accompanied by their own vectors of LMArcs.
  //
  // Position 0 is reserved for the start state '<s>'.
  Fsa::ConstAlphabetRef binarizeLM (std::istream & in, const std::string & file) {
    typedef std::pair <LMStateHeader, std::vector <LMArc> > LMState;
    std::vector <LMState> states;
  }
  
}
