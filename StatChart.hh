#ifndef _PERMUTE_STAT_CHART_HH
#define _PERMUTE_STAT_CHART_HH

#include <Core/XmlStream.hh>
#include "Chart.hh"

namespace Permute {
  // Counts getCell and add call occurrences.
  class ChartStats {
  public:
    int getCellCount;
    int addCount;
    ChartStats ();
    ChartRef instrument (ChartRef);
    void reset ();
    void write (Core::XmlWriter &) const;
  };

  // Decorates a chart with an instance of ChartStats.
  //
  // @bug "The mutable keyword overrides any enclosing const statement. A
  // mutable member of a const object can be modified."  Perhaps the getCell
  // method can be const after all.  Just declare stats_ mutable.
  class StatChart : public ChartDecorator {
  private:
    ChartStats * stats_;
  public:
    StatChart (ChartRef, ChartStats *);
    virtual CellRef getCell (int, int);
  };
}

#endif//_PERMUTE_STAT_CHART_HH
