#include "StatChart.hh"

namespace Permute {
  ChartStats::ChartStats () :
    getCellCount (0),
    addCount (0)
  {}

  ChartRef ChartStats::instrument (ChartRef chart) {
    return ChartRef (new StatChart (chart, this));
  }

  void ChartStats::reset () {
    getCellCount = 0;
    addCount = 0;
  }

  void ChartStats::write (Core::XmlWriter & os) const {
    os << Core::XmlOpen ("stats")
       << Core::XmlFull ("get-cell-count", getCellCount)
       << Core::XmlFull ("add-count", addCount)
       << Core::XmlClose ("stats");
  }

  /**********************************************************************/
  
  // Decorates a cell with an instance of ChartStats.  Increments addCount every
  // time the cell's add method is called.
  class StatCell : public CellDecorator {
  private:
    ChartStats * stats_;
    StatCell (CellRef cell, ChartStats * stats) :
      CellDecorator (cell),
      stats_ (stats)
    {}
  public:
    virtual bool add (ConstPathRef path) {
      stats_ -> addCount ++;
      return CellDecorator::add (path);
    }
    static CellRef decorate (CellRef cell, ChartStats * stats) {
      return CellRef (new StatCell (cell, stats));
    }
  };
  
  /**********************************************************************/
  
  StatChart::StatChart (ChartRef chart, ChartStats * stats) :
    ChartDecorator (chart),
    stats_ (stats)
  {}

  CellRef StatChart::getCell (int begin, int end) {
    stats_ -> getCellCount ++;
    return StatCell::decorate (decorated_ -> getCell (begin, end), stats_);
  }
}
