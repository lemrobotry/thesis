// Performs the MIRA update on the given vector of SparseParameterVectors,
// holding the differences between features of various permutations and the true
// permutation, and the corresponding losses.

#ifndef _PERMUTE_MIRA_HH
#define _PERMUTE_MIRA_HH

#include "PV.hh"

namespace Permute {

  bool operator < (const WRef &, const WRef &);
  
  class SparsePV : std::map <WRef, double> {
  private:
    double margin_;
  public:
    SparsePV (double = 1.0);
    void setMargin (double);
    double dot (const SparsePV &) const;
    double norm2 () const;
    double margin () const;
    void update (double);
    void build (const Permutation &, const SumBeforeCostRef &, double = 1.0);
    void build (const SumBeforeCostRef &, size_t, size_t, bool);
  private:
    void build_helper (const SumBeforeCostRef &, size_t, size_t, double);
  };
  
  int MIRA (std::vector <SparsePV> &);
}

#endif//_PERMUTE_MIRA_HH
