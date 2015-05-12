#ifndef _PERMUTE_INPUT_DATA_HH
#define _PERMUTE_INPUT_DATA_HH

#include "Permutation.hh"

namespace Permute {

  class InputData {
    friend std::istream & operator >> (std::istream &, InputData &);
  private:
    bool dependency_;
    Permutation source_, pos_, labels_, target_;
    std::vector <int> parents_;
  public:
    InputData (bool dependency = false);
    const Permutation & source () const { return source_; }
    Permutation & source () { return const_cast <Permutation &> (static_cast <const InputData *> (this) -> source ()); }
    const Permutation & pos () const { return pos_; }
    Permutation & pos () { return const_cast <Permutation &> (static_cast <const InputData *> (this) -> pos ()); }
    const Permutation & labels () const { return labels_; }
    Permutation & labels () { return const_cast <Permutation &> (static_cast <const InputData *> (this) -> labels ()); }
    const Permutation & target () const { return target_; }
    Permutation & target () { return const_cast <Permutation &> (static_cast <const InputData *> (this) -> target ()); }
    const std::vector <int> & parents () const { return parents_; }
    std::vector <int> & parents () { return const_cast <std::vector <int> &> (static_cast <const InputData *> (this) -> parents ()); }
  };

}

#endif//_PERMUTE_INPUT_DATA_HH
