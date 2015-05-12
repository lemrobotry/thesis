#include <numeric>

#include "BlockSearch.hh"
#include "FullCell.hh"
#include "StartEndCell.hh"

namespace Permute {

  // The default case is to combine four spans: (0, i) (j, k) (i, j) (k, n).  If
  // i is 0, that reduces the count by one, likewise if k is n.
  class BlockInsertCell : public StartEndCell {
  public:
    BlockInsertCell (Fsa::ConstAutomatonRef fsa, ConstCellRef epsilonClosure) :
      StartEndCell (fsa, epsilonClosure)
    {}
    void build (const ConstCellRef & cell_0i,
		const ConstCellRef & cell_ij,
		const ConstCellRef & cell_jk,
		const ConstCellRef & cell_kn,
		double score);
    void build (const ConstCellRef & cell_0j,
		const ConstCellRef & cell_jk,
		const ConstCellRef & cell_kn,
		double score);
    void build (const ConstCellRef & cell_0j,
		const ConstCellRef & cell_jn,
		double score);
  };

  void BlockInsertCell::build (const ConstCellRef & cell_0i,
			       const ConstCellRef & cell_ij,
			       const ConstCellRef & cell_jk,
			       const ConstCellRef & cell_kn,
			       double score) {
    CellRef cell_0j (new FullCell);
    Cell::build (cell_0j, cell_0i, cell_ij, 0.0, false, Path::KEEP);
    ConstCellRef const_cell_0j (cell_0j);
    build (const_cell_0j, cell_jk, cell_kn, score);
  }

  void BlockInsertCell::build (const ConstCellRef & cell_0j,
			       const ConstCellRef & cell_jk,
			       const ConstCellRef & cell_kn,
			       double score) {
    CellRef cell_0k (new FullCell);
    Cell::build (cell_0k, cell_0j, cell_jk, 0.0, false, Path::KEEP);
    ConstCellRef const_cell_0k (cell_0k);
    build (const_cell_0k, cell_kn, score);
  }

  // @bug Creates a CellRef using this, which will lead to deletion unless
  // another reference is kept somewhere.
  void BlockInsertCell::build (const ConstCellRef & cell_0j,
			       const ConstCellRef & cell_jn,
			       double score) {
    CellRef cell_0n (this);
    Cell::build (cell_0n, cell_0j, cell_jn, score, false, Path::KEEP);
  }

  typedef std::vector <double> V1D;
  typedef std::vector <V1D> V2D;
  typedef std::vector <V2D> V3D;

  void build (BlockInsertCell & cell, ChartRef chart, int i, int j, int k, double score) {
    if (i == 0) {
      if (k == chart -> getLength ()) {
	cell.build (chart -> getConstCell (j, k),
		    chart -> getConstCell (i, j),
		    score);
      } else {
	cell.build (chart -> getConstCell (j, k),
		    chart -> getConstCell (i, j),
		    chart -> getConstCell (k, chart -> getLength ()),
		    score);
      }
    } else if (k == chart -> getLength ()) {
      cell.build (chart -> getConstCell (0, i),
		  chart -> getConstCell (j, k),
		  chart -> getConstCell (i, j),
		  score);
    } else {
      cell.build (chart -> getConstCell (0, i),
		  chart -> getConstCell (j, k),
		  chart -> getConstCell (i, j),
		  chart -> getConstCell (k, chart -> getLength ()),
		  score);
    }
  }

  double blockSearch (Permutation & pi,
		      const BeforeCostRef & bc,
		      int max_width,
		      Fsa::ConstAutomatonRef fsa,
		      CellMapRef cellMap,
		      CellRef topCell,
		      ConstCellRef epsilonClosure) {
    ChartRef chart (new ChartImpl (pi, cellMap, topCell, 0));
    // Builds the chart without swaps.
    for (int i = 0; i < pi.size (); ++ i) {
      for (int j = i + 1; j < pi.size (); ++ j) {
	Cell::build (chart -> getCell (i, j + 1),
		     chart -> getConstCell (i, j),
		     chart -> getConstCell (j, j + 1),
		     0.0, false, Path::KEEP);
      }
    }

    BlockInsertCell * cell = new BlockInsertCell (fsa, epsilonClosure);
    // Prevents deletion of the cell.
    CellRef cellRef (cell);
    // Initializes the BlockInsertCell with the unordered path.
    ConstPathRef bestPath = chart -> getBestPath ();
    cell -> add (bestPath);
    double bestScore = bestPath -> getScore ();

    max_width = std::min (max_width ? max_width : static_cast <int> (pi.size ()),
			  static_cast <int> (pi.size ()) / 2);
    V3D delta (max_width);
    for (int w = 0; w < max_width; ++ w) {
      V2D & dw = delta [w];
      dw.resize (pi.size () - w - 1);
      for (int i = 0; i < dw.size (); ++ i) {
	V1D & dwi = dw [i];
	dwi.resize (pi.size () + 1, 0.0);
	int j = i + w + 1;
	for (int k = 0; k < i; ++ k) { // (k, i, j)
	  dwi [k] =
	    bc -> cost (pi [j - 1], pi [k]) -
	    bc -> cost (pi [k], pi [j - 1]);
	}
	std::partial_sum (dwi.rend () - i, dwi.rend (),
			  dwi.rend () - i);
	if (w > 0) {
	  std::transform (dwi.begin (), dwi.begin () + i,
			  delta [w - 1] [i].begin (),
			  dwi.begin (),
			  std::plus <double> ());
	}
	for (int k = j + 1; k <= pi.size (); ++ k) { // (i, j, k)
	  dwi [k] =
	    bc -> cost (pi [k - 1], pi [i]) -
	    bc -> cost (pi [i], pi [k - 1]);
	}
	std::partial_sum (dwi.begin () + j + 1, dwi.end (),
			  dwi.begin () + j + 1);
	if (w > 0) {
	  std::transform (dwi.begin () + j + 1, dwi.end (),
			  delta [w - 1] [i + 1].begin () + j + 1,
			  dwi.begin () + j + 1,
			  std::plus <double> ());
	}
//  	V1D::const_iterator max_it = std::max_element (dwi.begin (), dwi.end ());
//  	if (* max_it > 0.0) {
// 	  insert (pi, i, j, max_it - dwi.begin ());
// 	  return * max_it;
//  	}
	// dwi [0, ..., i-1, j+1, ..., n] holds the LOP scores for each
	// destination point.  Combines those with the scores from the A model.
	for (int k = 0; k < i; ++ k) { // (k, i, j)
	  build (* cell, chart, k, i, j, dwi [k]);
	}
	for (int k = j + 1; k < pi.size (); ++ k) { // (i, j, k)
	  build (* cell, chart, i, j, k, dwi [k]);
	}
	
	ConstPathRef newBestPath = Cell::getBestPath (cellRef);
	if (newBestPath -> getScore () > bestScore) {
	  pi.reorder (newBestPath);
	  return newBestPath -> getScore () - bestScore;
	}
      }
    }

    return 0.0;
  }
}
