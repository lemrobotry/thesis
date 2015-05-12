// Constructs an n-gram model from the counts on each line of the input.  Each
// line must consist of a count and an n-gram, all white space delimited.

#ifndef _PERMUTE_NGRAM_HH
#define _PERMUTE_NGRAM_HH

#include <Fsa/Automaton.hh>

namespace Permute {
  Fsa::ConstAutomatonRef ngram (std::istream &);
}

#endif//_PERMUTE_NGRAM_HH
