#ifndef _PERMUTE_EXPECTATION_SEMIRING_HH
#define _PERMUTE_EXPECTATION_SEMIRING_HH

#include <iostream>
#include "Log.hh"

namespace Permute {

  class Expectation {
    friend std::ostream & operator << (std::ostream & out, Expectation e);
  private:
    Log p_;
    Log v_;
  public:
    Expectation (Log p = Log::Zero, Log v = Log::Zero) :
      p_ (p),
      v_ (v)
    {}
    Log p () const { return p_; }
    Log v () const { return v_; }
    double expectation () const {
      if (v_.isZero ()) {
	return 0.0;
      } else {
	return (v_ / p_).toP ();
      }
    }
    bool operator == (Expectation e) const {
      return p_ == e.p_ && v_ == e.v_;
    }
    bool operator != (Expectation e) const {
      return p_ != e.p_ || v_ != e.v_;
    }
    Expectation & operator += (Expectation e) {
      p_ += e.p_;
      v_ += e.v_;
      return * this;
    }
    Expectation & operator -= (Expectation e) {
      p_ -= e.p_;
      v_ -= e.v_;
      return * this;
    }
    // Updates v_ first for correctness.
    Expectation & operator *= (Expectation e) {
      v_ *= e.p_;
      v_ += p_ * e.v_;
      p_ *= e.p_;
      return * this;
    }
    static Expectation p_pv (Log p, Log v) {
      return Expectation (p, p * v);
    }
    static Expectation One;
    static Expectation Zero;
  };

  Expectation operator + (Expectation a, Expectation b);
  Expectation operator - (Expectation a, Expectation b);
  Expectation operator * (Expectation a, Expectation b);

  // Aggregates Expectations through addition and subtraction.  Avoids the
  // subtraction operator until the sum is requested.
  class ExpectationSum {
  private:
    Expectation sum_;
    Expectation subtract_;
  public:
    ExpectationSum ();
    ExpectationSum & operator = (const Expectation & e);
    ExpectationSum & operator += (const Expectation & e);
    ExpectationSum & operator -= (const Expectation & e);
    Expectation sum () const;
  };

};

#endif//_PERMUTE_EXPECTATION_SEMIRING_HH
