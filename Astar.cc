#include "Astar.hh"
#include <Core/PriorityQueue.hh>

namespace Permute {

  // PathItem associates a path with a span in the current permutation, and
  // provides a Hash class.
  class PathItem {
  public:
    ConstPathRef path;
    int begin;
    int end;

    PathItem () : path (Path::nullPath ()), begin (0), end (0) {}
    PathItem (ConstPathRef p, int b, int e) : path (p), begin (b), end (e) {}

    bool operator == (const PathItem & other) const {
      return (begin == other.begin)
	&& (end == other.end)
	&& (path -> getStart () == other.path -> getStart ())
	&& (path -> getEnd () == other.path -> getEnd ());
    }

    class Hash {
    public:
      size_t operator () (const PathItem & p) const {
	size_t result = p.path -> getStart ();
	result = 5 * result + p.path -> getEnd ();
	result = 5 * result + p.begin;
	result = 5 * result + p.end;
	return result;
      }
    };
  };

  typedef std::pair <PathItem, double> QItem;
  typedef Core::select1st <QItem> QItem1st;
  class QItemGreater {
  public:
    bool operator () (const QItem & i1, const QItem & i2) {
      return i1.second > i2.second;
    }
  };
  typedef Core::TracedPriorityQueue <QItem, PathItem, QItem1st, QItemGreater, PathItem::Hash> PriorityQ;

  QItem item (ConstPathRef path, int begin, int end, double outside) {
    return std::make_pair (PathItem (path, begin, end), outside + path -> getScore ());
  }

  // Provides a stand-in for Chart::permute that evaluates paths in A* priority
  // order rather than in CKY order.  Uses outside estimates for A* priority.
  //
  // @bug Make sure to correctly incorporate initial epsilon paths and state
  // final weights.
  void Astar::permute (ChartRef chart, OutsideRef outside, ParseControllerRef controller, ScorerRef scorer) {
    scorer -> clear ();
    chart -> beforePermute (controller, scorer);
    outside -> estimate (controller, scorer);

    PriorityQ q;
    
    // Puts each of the width-one spans into the priority queue.
    for (int b = 0; b < chart -> getLength (); ++ b) {
      ConstCellRef cell = chart -> getConstCell (b, b + 1);
      for (Cell::PathIterator path = cell -> begin (); path != cell -> end (); ++ path) {
	q.insertOrRelax (item (* path, b, b + 1, outside -> outside (b, b + 1)));
      }
    }

    // For each path on the priority queue, combines it with spans to the left
    // or right, preserving or swapping order, matching start or end states as
    // appropriate.  Adds new paths to the queue using insertOrRelax (Item).
    while (! q.empty ()) {
      PathItem i = q.top ().first; q.pop ();
      ConstPathRef path = i.path;
      int begin = i.begin,
	end = i.end;
      // Adds the path to the chart.
      chart -> getCell (begin, end) -> add (path);
      // If the path is complete, stops early.
      if (begin == 0 && end == chart -> getLength ()) {
	break;
      }
      // Iterates over cells to the left.
      for (int left_begin = begin - 1; left_begin >= 0; -- left_begin) {
	if (controller -> allows (left_begin, begin, end)) {
	  ConstCellRef left = chart -> getConstCell (left_begin, begin);
	  if (left -> empty ()) break;
	  double outside_score = outside -> outside (left_begin, end);
	  double score = scorer -> score (left_begin, begin, end);
	  // Iterates over paths ending at our start point.
	  for (Cell::PathIterator leftPath = left -> rbegin (path -> getStart ());
	       leftPath != left -> rend (path -> getStart ());
	       ++ leftPath) {
	    q.insertOrRelax (item (Path::connect (* leftPath, path, score, false), left_begin, end, outside_score));
	  }
	  score = scorer -> score (end, begin, left_begin);
	  // Iterates over paths starting at our end point.
	  for (Cell::PathIterator leftPath = left -> begin (path -> getEnd ());
	       leftPath != left -> end (path -> getEnd ());
	       ++ leftPath) {
	    q.insertOrRelax (item (Path::connect (* leftPath, path, score, true), left_begin, end, outside_score));
	  }
	}
      }
      // Iterates over cells to the right.
      for (int right_end = end + 1; right_end <= chart -> getLength (); ++ right_end) {
	if (controller -> allows (begin, end, right_end)) {
	  ConstCellRef right = chart -> getConstCell (end, right_end);
	  if (right -> empty ()) break;
	  double outside_score = outside -> outside (begin, right_end);
	  double score = scorer -> score (begin, end, right_end);
	  // Iterates over paths starting at our end point.
	  for (Cell::PathIterator rightPath = right -> begin (path -> getEnd ());
	       rightPath != right -> end (path -> getEnd ());
	       ++ rightPath) {
	    q.insertOrRelax (item (Path::connect (path, * rightPath, score, false), begin, right_end, outside_score));
	  }
	  score = scorer -> score (right_end, end, begin);
	  // Iterates over paths ending at our start point.
	  for (Cell::PathIterator rightPath = right -> rbegin (path -> getStart ());
	       rightPath != right -> rend (path -> getStart ());
	       ++ rightPath) {
	    q.insertOrRelax (item (Path::connect (path, * rightPath, score, true), begin, right_end, outside_score));
	  }
	}
      }
    }
    
    chart -> afterPermute (controller, scorer);
  }

}
