#include "ExpectationSemiring.hh"

namespace Permute {

  Expectation Expectation::Zero (Log::Zero, Log::Zero);
  Expectation Expectation::One (Log::One, Log::Zero);

  std::ostream & operator << (std::ostream & out, Expectation e) {
    return out << "<" << e.p_ << ", " << e.v_ << ">";
  }

  Expectation operator + (Expectation a, Expectation b) {
    return a += b;
  }
  Expectation operator - (Expectation a, Expectation b) {
    return a -= b;
  }
  Expectation operator * (Expectation a, Expectation b) {
    return a *= b;
  }

  /**********************************************************************/

  ExpectationSum::ExpectationSum () :
    sum_ (Expectation::Zero),
    subtract_ (Expectation::Zero)
  {}

  ExpectationSum & ExpectationSum::operator = (const Expectation & e) {
    sum_ = e;
    subtract_ = Expectation::Zero;
    return * this;
  }

  ExpectationSum & ExpectationSum::operator += (const Expectation & e) {
    sum_ += e;
    return * this;
  }

  ExpectationSum & ExpectationSum::operator -= (const Expectation & e) {
    subtract_ += e;
    return * this;
  }

  Expectation ExpectationSum::sum () const {
    return sum_ - subtract_;
  }
}
