// NOTE: Not in use.  Attempts to make accessing the language model more
// efficient by storing and using it in a binary format.  This has probably been
// superseded by updates to SRILM that add such a format.

#ifndef _PERMUTE_BINARY_LM_HH
#define _PERMUTE_BINARY_LM_HH

#include <Fsa/Automaton.hh>

namespace Permute {

  Fsa::ConstAlphabetRef binarizeLM (std::istream &, const std::string &);
  
  Fsa::ConstAutomatonRef binaryLM (Fsa::ConstAlphabetRef, int, const std::string &, Fsa::ConstAlphabetRef);
  
}

#endif//_PERMUTE_BINARY_LM_HH
