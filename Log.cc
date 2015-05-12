#include <Core/Types.hh>
#include "Log.hh"

namespace Permute {

  double Log::Zero (Core::Type <double>::min);
  double Log::One (0.0);

  std::ostream & operator << (std::ostream & out, Log l) {
    return out << l.log_ << "=log(" << l.toP () << ")";
  }

  Log operator + (Log a, Log b) {
    return a += b;
  }
  Log operator - (Log a, Log b) {
    return a -= b;
  }
  Log operator * (Log a, Log b) {
    return a *= b;
  }
  Log operator / (Log a, Log b) {
    return a /= b;
  }
}
