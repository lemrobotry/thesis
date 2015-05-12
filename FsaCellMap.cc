#include <Fsa/Dfs.hh>
#include "BestCell.hh"
#include "EpsilonClosure.hh"
#include "FsaCellMap.hh"
#include "FullCell.hh"
#include "Path.hh"
#include "StartEndCell.hh"

namespace Permute {
  // Finds all arcs that match a given label in the given automaton, and adds
  // them to the given cell.
  class CellDfs : public Fsa::DfsState {
  protected:
    size_t index_;
    Fsa::LabelId label_;
    CellRef cell_;
  public:
    CellDfs (Fsa::ConstAutomatonRef fsa, size_t index, Fsa::LabelId label, Cell * cell) :
      Fsa::DfsState (fsa),
      index_ (index),
      label_ (label),
      cell_ (cell)
    {}
    virtual void exploreArc (Fsa::ConstStateRef from, const Fsa::Arc & a) {
      if (a.input () == label_) {
	if (label_ == Fsa::Epsilon) {
	  cell_ -> add (Path::epsilon (from -> id (), a.target (), - float (a.weight ())));
	} else {
	  cell_ -> add (Path::arc (index_, from -> id (), a.target (), - float (a.weight ())));
	}
      }
    }
    virtual void exploreTreeArc (Fsa::ConstStateRef from, const Fsa::Arc & a) {
      exploreArc (from, a);
    }
    virtual void exploreNonTreeArc (Fsa::ConstStateRef from, const Fsa::Arc & a) {
      exploreArc (from, a);
    }
    virtual CellRef getCell () {
      return cell_;
    }
  };

  /**********************************************************************/
  
  FsaCellMap::FsaCellMap (Fsa::ConstAutomatonRef fsa) :
    fsa_ (fsa)
  {
    CellDfs dfs (fsa, 0, Fsa::Epsilon, new FullCell);
    dfs.dfs ();
    closure_ = Permute::epsilonClosure (dfs.getCell ());
  }

  // First, uses CellDfs to find all arcs matching the given label.  Then,
  // computes the epsilon-closure of those arcs by combining them with the
  // epsilon paths stored in closure_ using Cell::build.  Adds all the closure
  // paths to the cell and returns it.
  ConstCellRef FsaCellMap::operator () (size_t index, Fsa::LabelId label) {
    CellDfs dfs (fsa_, index, label, new FullCell);
    dfs.dfs ();
    CellRef cell = dfs.getCell ();
    CellRef closure (new FullCell);
    Cell::build (closure, cell, closure_, 0.0, false, Path::NEITHER);
    Cell::build (closure, cell, closure_, 0.0, false, Path::KEEP);
    for (Cell::PathIterator path = closure -> begin (); path != closure -> end (); ++ path) {
      cell -> add (* path);
    }
    return cell;
  }

  ConstCellRef FsaCellMap::epsilonClosure () const {
    return closure_;
  }

  // Returns an FsaCellMap and a StartEndCell constructed from the given automaton.
  std::pair <CellMapRef, CellRef> FsaCellMap::mapFsa (Fsa::ConstAutomatonRef fsa) {
    FsaCellMap * cellMap = new FsaCellMap (fsa);
    return std::make_pair (CellMapRef (cellMap),
			   CellRef (new StartEndCell (fsa, cellMap -> closure_)));
  }

  /**********************************************************************/

  // Finds all arcs that match a given label in a given automaton and adss them
  // to a BestCell with start and end states replaced with 0.
  class UnigramCellDfs : public CellDfs {
  public:
    UnigramCellDfs (Fsa::ConstAutomatonRef fsa, size_t index, Fsa::LabelId label) :
      CellDfs (fsa, index, label, new BestCell)
    {}
    virtual void exploreArc (Fsa::ConstStateRef from, const Fsa::Arc & a) {
      if (a.input () == label_) {
	cell_ -> add (Path::arc (index_, 0, 0, - float (a.weight ())));
      }
    }
  };

  /**********************************************************************/

  UnigramFsaCellMap::UnigramFsaCellMap (Fsa::ConstAutomatonRef fsa) :
    FsaCellMap (fsa)
  {}

  // Uses UnigramCellDfs to find all arcs matching the given label, and returns
  // a cell containing them, with start and end state replaced by 0.
  ConstCellRef UnigramFsaCellMap::operator () (size_t index, Fsa::LabelId label) {
    UnigramCellDfs dfs (fsa_, index, label);
    dfs.dfs ();
    return dfs.getCell ();
  }

  // Returns a UnigramFsaCellMap and a BestCell constructed from the given
  // automaton.
  //
  // @bug Shouldn't this use a StartEndCell that takes into account the best
  // final weight?
  std::pair <CellMapRef, CellRef> UnigramFsaCellMap::mapFsa (Fsa::ConstAutomatonRef fsa) {
    UnigramFsaCellMap * cellMap = new UnigramFsaCellMap (fsa);
    return std::make_pair (CellMapRef (cellMap),
			   CellRef (new BestCell));
  }
}
