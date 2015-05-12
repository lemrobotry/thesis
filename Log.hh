// Provides a class that converts between probabilities and log probabilities
// (not negated).  Provides basic arithmetic operations with semantics in the
// probability domain but operations in the log domain.  Generates random log
// probabilities uniform in the probability domain, and computes Bernoulli
// trials using this randomness.

#ifndef _PERMUTE_LOG_HH
#define _PERMUTE_LOG_HH

#include <cmath>
#include <cstdlib>
#include <iostream>

namespace Permute {
  class Log {
    friend std::ostream & operator << (std::ostream & out, Log l);
  private:
    double log_;
  public:
    Log (double log = One) :
      log_ (log)
    {}
    bool operator == (Log l) const { return log_ == l.log_; }
    bool operator != (Log l) const { return log_ != l.log_; }
    bool operator < (Log l) const { return log_ < l.log_; }
    bool operator > (Log l) const { return log_ > l.log_; }
    Log & operator += (Log l) {
      increase (log_, l.log_);
      return * this;
    }
    Log & operator -= (Log l) {
      log_ = subtract (log_, l.log_);
      return * this;
    }
    Log & operator *= (Log l) {
      log_ += l.log_;
      return * this;
    }
    Log & operator /= (Log l) {
      log_ -= l.log_;
      return * this;
    }
    double toP () const { return l2p (log_); }
    bool isZero () const { return log_ == Zero; }
    bool isOne () const { return log_ == One; }
      
    static double Zero;
    static double One;
    static void increase (double & a, double b) {
      a = add (a, b);
    }
    static double add (double a, double b) {
      if (a < b) { return add (b, a); }
      else if (std::isinf (a)) { return b; }
      else { return a + log1p (std::exp (b - a)); }
    }
    static double subtract (double a, double b) {
      if (a < b) { std::cerr << "Log::subtract: a < b" << std::endl; return Zero; }
      else if (std::isinf (b)) { return a; }
      else { return a + log1p (- std::exp (b - a)); }
    }
    static double multiply (double a, double b) { return a + b; }
    static double divide (double a, double b) { return a - b; }
    static double random (double l) {
      return multiply (l, divide (p2l (std::rand ()), p2l (RAND_MAX)));
    }
    static bool bernoulli (double p, double norm) {
      return random (norm) < p;
    }
    static double p2l (double p) { return std::log (p); }
    static double l2p (double l) { return std::exp (l); }
  };

  Log operator + (Log a, Log b);
  Log operator - (Log a, Log b);
  Log operator * (Log a, Log b);
  Log operator / (Log a, Log b);
}

#endif//_PERMUTE_LOG_HH
