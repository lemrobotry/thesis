#include <cmath>
#include <algorithm>
#include "Statistics.hh"

namespace Permute {

  Statistics::Statistics () :
    count_ (0),
    sum_ (0),
    ssum_ (0)
  {}

  Statistics & Statistics::operator += (double x) {
    ++ count_;
    sum_ += x;
    ssum_ += std::pow (x, 2);
    return * this;
  }

  void Statistics::reset () {
    count_ = 0;
    sum_ = 0;
    ssum_ = 0;
  }

  int Statistics::n () const {
    return count_;
  }

  double Statistics::mean () const {
    return sum_ / n ();
  }

  double Statistics::variance () const {
    if (n () <= 1 ) {
      return 0;
    } else {
      return std::max (0.0, (ssum_ - n () * std::pow (mean (), 2))) / (n () - 1);
    }
  }

  double Statistics::stdev () const {
    return std::sqrt (variance ());
  }
  
}
