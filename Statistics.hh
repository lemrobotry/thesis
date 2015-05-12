#ifndef _PERMUTE_STATISTICS_HH
#define _PERMUTE_STATISTICS_HH

namespace Permute {

  // Keeps a running totals for computing mean and variance.
  class Statistics {
  private:
    int count_;
    double sum_;
    double ssum_;
  public:
    Statistics ();
    Statistics & operator += (double);
    Statistics & operator = (double x) { return operator += (x); }
    void reset ();

    int n () const;
    double mean () const;
    double variance () const;
    double stdev () const;
  };

}

#endif//_PERMUTE_STATISTICS_HH
