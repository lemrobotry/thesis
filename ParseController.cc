#include "ParseController.hh"
#include "BeforeScorer.hh"

namespace Permute {

  ParseController::iterator ParseController::end (int begin, int end) const {
    return new ParseController::iteratorImpl (end);
  }

  // Implements the default rule for computing the grammar cost, which is
  // compatible with both the cubic neighborhood and the quadratic
  // neighborhood.
  double ParseController::grammar (const BeforeScorer & b, int i, int j, int k) const {
    if (i < k) {
      return b.score (i, j, k - 1)
	+ b.score (i + 1, j, k)
	- b.score (i + 1, j, k - 1)
	+ b.cost (i, k - 1);
    } else {
      return b.score (i - 1, j, k)
	+ b.score (i, j, k + 1)
	- b.score (i - 1, j, k + 1)
	+ b.cost (i - 1, k);
    }
  }

  /**********************************************************************/

  QuadraticParseController::gapIterator::gapIterator (int current, int left, int right) :
    ParseController::iteratorImpl (current),
    left_ (left),
    right_ (right)
  {}

  QuadraticParseController::gapIterator & QuadraticParseController::gapIterator::operator ++ () {
    ++current_;
    if (current_ > left_ && current_ < right_) {
      current_ = right_;
    }
    return * this;
  }

  /**********************************************************************/

  QuadraticParseController::QuadraticParseController (int size) :
    size_ (size)
  {}

  ParseController::iterator QuadraticParseController::begin (int begin, int end) const {
    return new QuadraticParseController::gapIterator (begin + 1, begin + size_, end - size_);
  }

  bool QuadraticParseController::allows (int begin, int middle, int end) const {
    return ((middle - begin) <= size_) || ((end - middle) <= size_);
  }

  ParseControllerRef QuadraticParseController::create (int size) {
    return ParseControllerRef (new QuadraticParseController (size));
  }

  /**********************************************************************/

  ParseController::iterator CubicParseController::begin (int begin, int end) const {
    return new ParseController::iteratorImpl (begin + 1);
  }

  ParseControllerRef CubicParseController::create () {
    return ParseControllerRef (new CubicParseController);
  }

  bool CubicParseController::allows (int, int, int) const {
    return true;
  }

  /**********************************************************************/

  ParseControllerDecorator::ParseControllerDecorator (ParseControllerRef controller) :
    decorated_ (controller)
  {}

  ParseController::iterator ParseControllerDecorator::begin (int begin, int end) const {
    return decorated_ -> begin (begin, end);
  }

  ParseController::iterator ParseControllerDecorator::end (int begin, int end) const {
    return decorated_ -> end (begin, end);
  }

  bool ParseControllerDecorator::allows (int begin, int middle, int end) const {
    return decorated_ -> allows (begin, middle, end);
  }

  double ParseControllerDecorator::grammar (const BeforeScorer & b, int i, int j, int k) const {
    return decorated_ -> grammar (b, i, j, k);
  }

  /**********************************************************************/

  LeftAnchorParseController::LeftAnchorParseController (ParseControllerRef controller, int width) :
    ParseControllerDecorator (controller),
    width_ (width)
  {}

  ParseController::iterator LeftAnchorParseController::begin (int begin, int end) const {
    if (begin < width_) {
      return new ParseController::iteratorImpl (begin + 1);
    } else {
      return ParseControllerDecorator::begin (begin, end);
    }
  }

  bool LeftAnchorParseController::allows (int begin, int middle, int end) const {
    if (begin < width_) {
      return true;
    } else {
      return ParseControllerDecorator::allows (begin, middle, end);
    }
  }

  double LeftAnchorParseController::grammar (const BeforeScorer & b, int i, int j, int k) const {
    if (i < k && i < width_) {
      return b.score (i, j, k - 1)
	+ b.score (i, j - 1, k)
	- b.score (i, j - 1, k - 1)
	+ b.cost (j - 1, k - 1);
    } else if (k < width_) {
      return b.score (i - 1, j, k)
	+ b.score (i, j - 1, k)
	- b.score (i - 1, j - 1, k)
	+ b.cost (i - 1, j - 1);
    } else {
      return ParseControllerDecorator::grammar (b, i, j, k);
    }
  }

  ParseControllerRef LeftAnchorParseController::decorate (ParseControllerRef controller, int width) {
    return ParseControllerRef (new LeftAnchorParseController (controller, width));
  }

  /**********************************************************************/

  RightAnchorParseController::RightAnchorParseController (ParseControllerRef controller, const Permutation & permutation, int width) :
    ParseControllerDecorator (controller),
    permutation_ (permutation),
    width_ (width)
  {}

  ParseController::iterator RightAnchorParseController::begin (int begin, int end) const {
    if (end > permutation_.size () - width_) {
      return new ParseController::iteratorImpl (begin + 1);
    } else {
      return ParseControllerDecorator::begin (begin, end);
    }
  }

  bool RightAnchorParseController::allows (int begin, int middle, int end) const {
    if (end > permutation_.size () - width_) {
      return true;
    } else {
      return ParseControllerDecorator::allows (begin, middle, end);
    }
  }

  // @bug Test.
  double RightAnchorParseController::grammar (const BeforeScorer & b, int i, int j, int k) const {
    if (i < k && k > permutation_.size () - width_) {
      return b.score (i + 1, j, k)
	+ b.score (i, j + 1, k)
	- b.score (i + 1, j + 1, k)
	+ b.cost (i, j);
    } else if (i > permutation_.size () - width_) {
      return b.score (i, j, k + 1)
	+ b.score (i, j + 1, k)
	- b.score (i, j + 1, k + 1)
	+ b.cost (j, k);
    } else {
      return ParseControllerDecorator::grammar (b, i, j, k);
    }
  }

  ParseControllerRef RightAnchorParseController::decorate (ParseControllerRef controller, const Permutation & permutation, int width) {
    return ParseControllerRef (new RightAnchorParseController (controller, permutation, width));
  }
}
