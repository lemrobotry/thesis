#ifndef _PERMUTE_LOP_CHART_HH
#define _PERMUTE_LOP_CHART_HH

#include "Path.hh"
#include "Permutation.hh"
#include "LOPkBest.hh"

namespace Permute {

  class LOPCell {
  public:
    int midpoint;
    bool swap;
    double score;
    LOPCell () :
      midpoint (0),
      swap (false),
      score (0.0)
    {}
    LOPCell (int midpoint, bool swap, double score) :
      midpoint (midpoint),
      swap (swap),
      score (score)
    {}
    bool max_equals (int midpoint, bool swap, double score) {
      if (score > this -> score) {
	this -> midpoint = midpoint;
	this -> swap = swap;
	this -> score = score;
	return true;
      } else {
	return false;
      }
    }
    bool max_equals (const LOPCell & cell) {
      return max_equals (cell.midpoint, cell.swap, cell.score);
    }
  };

  ////////////////////////////////////////////////////////////////////////////////

  class LOPChart {
  private:
    Permutation & pi_;
    int n_;
    int window_;
    std::vector <LOPCell> cells_;
  public:
    LOPChart (Permutation &, int = 0);
    int index (int i, int j) const;
    void permute (const ParseControllerRef &, ScorerRef &);
    ConstPathRef getBestPath () const;
    int getWindow () const;
    int getLength () const;
  private:
    LOPCell & cell (int i, int j);
    const LOPCell & cell (int i, int j) const;
    ConstPathRef path (int i, int j) const;
  };

  ////////////////////////////////////////////////////////////////////////////////

  class NormalLOPChart {
  private:
    Permutation & pi_;
    int n_;
    int window_;
    std::vector <ConstPathRef> cells_;
  public:
    NormalLOPChart (Permutation &, int = 0);
    int index (int i, int j, Path::Type type) const;
    void permute (const ParseControllerRef &, ScorerRef &);
    const ConstPathRef & getBestPath () const;
    int getWindow () const;
    int getLength () const;
  private:
    const ConstPathRef & cell (int i, int j, Path::Type type) const;
    ConstPathRef & cell (int i, int j, Path::Type type);
    const ConstPathRef & betterPath (int i, int j) const;
  };

  ////////////////////////////////////////////////////////////////////////////////

  class QuadraticNormalLOPCell {
  private:
    std::vector <LOPCell> cells_;
  public:
    QuadraticNormalLOPCell () :
      cells_ (3)
    {}
    LOPCell & white () { return cells_ [0]; }
    LOPCell & black () { return cells_ [1]; }
    LOPCell & red () { return cells_ [2]; }
    // Alters the cells to hold non-white, non-black, and any.
    void finish () {
      cells_ [0].max_equals (cells_ [2]); // non-black
      cells_ [1].max_equals (cells_ [2]); // non-white
      cells_ [2].max_equals (cells_ [0]);
      cells_ [2].max_equals (cells_ [1]);
    }
    const LOPCell & non_white () const { return cells_ [1]; }
    const LOPCell & non_black () const { return cells_ [0]; }
    const LOPCell & any () const { return cells_ [2]; }
  };

  class QuadraticNormalLOPChart {
  private:
    Permutation & pi_;
    int n_;
    int w_;
    bool left_;
    std::vector <QuadraticNormalLOPCell> cells_;
  public:
    QuadraticNormalLOPChart (Permutation &, int = 1, bool = true);
    int index (int i, int j) const;
    void permute (const ParseControllerRef &, ScorerRef &);
    int getLength () const;
    ConstPathRef getBestPath () const;
  private:
    QuadraticNormalLOPCell & cell (int i, int j);
    const QuadraticNormalLOPCell & cell (int i, int j) const;
    ConstPathRef path (int i, int j, int color) const;
    ConstPathRef path (int i, int j, const LOPCell & cell) const;
  };

  ////////////////////////////////////////////////////////////////////////////////

  class NormVertices {
  private:
    std::vector <LOP::Vertex> vertices_;
  public:
    NormVertices () : vertices_ (3) {}
    LOP::Vertex & non_black () { return vertices_ [0]; }
    LOP::Vertex & non_white () { return vertices_ [1]; }
    LOP::Vertex & any () { return vertices_ [2]; }
  };

  ////////////////////////////////////////////////////////////////////////////////

  class LOPkBestChart {
  private:
    Permutation & pi_;
    int n_;
    std::vector <NormVertices> cells_;
  public:
    LOPkBestChart (Permutation &);
    int index (int i, int j) const;
    void permute (const ParseControllerRef &, ScorerRef &);
    void getBestPaths (std::vector <ConstPathRef> & v, int k);
    int getLength () const { return n_; }
  private:
    NormVertices & cell (int i, int j);
  };

  ////////////////////////////////////////////////////////////////////////////////

  class QNormLOPkBestChart {
  private:
    Permutation & pi_;
    int n_;
    int w_;
    std::vector <NormVertices> cells_;
  public:
    QNormLOPkBestChart (Permutation &, int = 1);
    int index (int i, int j) const;
    void permute (const ParseControllerRef &, ScorerRef &);
    int getLength () const { return n_; }
    void getBestPaths (std::vector <ConstPathRef> & v, int k);
    const ConstPathRef & samplePath (double threshold, double alpha);
  private:
    NormVertices & cell (int i, int j);
  };
}

#endif//_PERMUTE_LOP_CHART_HH
