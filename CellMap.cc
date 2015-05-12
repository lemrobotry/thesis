#include "BestCell.hh"
#include "CellMap.hh"

namespace Permute {
  // Caches Cells returned from the wrapped CellMap based on the index given to
  // the () operator, and returns the cached version for that index when it is
  // available, rather than querying the wrapped CellMap again.
  class CacheCellMap : public CellMap {
  private:
    CellMapRef map_;
    std::map <size_t, ConstCellRef> cache_;
    typedef std::map <size_t, ConstCellRef>::const_iterator iterator;
  public:
    CacheCellMap (CellMapRef map) :
      map_ (map),
      cache_ ()
    {}
    virtual ConstCellRef operator () (size_t index, Fsa::LabelId id) {
      iterator cached = cache_.find (index);
      if (cached != cache_.end ()) {
	return cached -> second;
      } else {
	ConstCellRef cell = map_ -> operator () (index, id);
	cache_ [index] = cell;
	return cell;
      }
    }
  };

  /**********************************************************************/

  CellMapRef CellMap::cache (CellMapRef map) {
    return CellMapRef (new CacheCellMap (map));
  }

  /**********************************************************************/

  // Always returns a BestCell containing a single arc from state 0 to state 0
  // with weight 0.0.
  class TrivialCellMap : public CellMap {
  public:
    virtual ConstCellRef operator () (size_t index, Fsa::LabelId) {
      CellRef cell (new BestCell);
      cell -> add (Path::arc (index, 0, 0, 0.0));
      return cell;
    }
  };

  CellMapRef CellMap::trivial () {
    return cache (CellMapRef (new TrivialCellMap));
  }
}
