#include "Iterator.hh"

namespace Permute {
  std::ostream & operator << (std::ostream & out, const Boolean & b) {
    return out << (b.b_ ? "true" : "false");
  }
}
