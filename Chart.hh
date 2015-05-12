#ifndef _PERMUTE_CHART_HH
#define _PERMUTE_CHART_HH

#include <Core/ReferenceCounting.hh>
#include "Cell.hh"
#include "CellMap.hh"
#include "ParseController.hh"
#include "Permutation.hh"
#include "Scorer.hh"

namespace Permute {
  class Chart;
  typedef Core::Ref <Chart> ChartRef;
  
  // Provides an interface for storing cells containing partial permutations
  // (getCell, getConstCell, getLength, getBestPath), as well as methods for
  // filling in these cells (permute, beforePermute, afterPermute).
  class Chart : public Core::ReferenceCounted {
  public:
    virtual ~Chart () {};
    virtual CellRef getCell (int, int) = 0;
    virtual ConstCellRef getConstCell (int, int) = 0;
    virtual int getLength () const = 0;
    virtual ConstPathRef getBestPath () = 0;
    virtual void beforePermute (ParseControllerRef, ScorerRef) = 0;
    virtual void afterPermute (ParseControllerRef, ScorerRef) = 0;
    virtual int getWindow () const = 0;
    
    static void permute (ChartRef, ParseControllerRef, ScorerRef);
    static int index (int, int, int);
    static int triangle (int);
  };

  // Implements the Chart interface storing cells in a vector and using Chart's
  // static index method to map spans (i,k) to one-dimensional indices.
  class ChartImpl : public Chart {
    friend class Chart;
  protected:
    Permutation & permutation_;
    CellMapRef map_;
    std::vector <CellRef> cells_;
    int window_;
  public:
    ChartImpl (Permutation &, CellMapRef, CellRef, int);
    virtual CellRef getCell (int, int);
    virtual ConstCellRef getConstCell (int, int);
    virtual int getLength () const;
    virtual ConstPathRef getBestPath ();
    virtual void beforePermute (ParseControllerRef, ScorerRef) {}
    virtual void afterPermute (ParseControllerRef, ScorerRef) {}
    virtual int getWindow () const;
  private:
    int index (int, int) const;
  };

  // Decorates an existing Chart and implements the Chart interface by
  // forwarding each call to the decorated Chart.
  class ChartDecorator : public Chart {
  protected:
    ChartRef decorated_;
  public:
    ChartDecorator (ChartRef);
    virtual CellRef getCell (int, int);
    virtual ConstCellRef getConstCell (int, int);
    virtual int getLength () const;
    virtual ConstPathRef getBestPath ();
    virtual void beforePermute (ParseControllerRef, ScorerRef);
    virtual void afterPermute (ParseControllerRef, ScorerRef);
    virtual int getWindow () const;
  };
}

#endif//_PERMUTE_CHART_HH
