#include "InputData.hh"

namespace Permute {

  InputData::InputData (bool dependency) :
    dependency_ (dependency),
    source_ (),
    pos_ (),
    labels_ (),
    target_ (),
    parents_ ()
  {}

  std::istream & operator >> (std::istream & in, InputData & data) {
    readPermutationWithAlphabet (data.source_, in);
    readPermutationWithAlphabet (data.pos_, in);
    if (data.dependency_) {
      readParents (data.parents_, in);
      readPermutationWithAlphabet (data.labels_, in);
    }
    data.target_ = data.source_;
    readAlignment (data.target_, in);
    return in;
  }
  
}
