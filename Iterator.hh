#ifndef _PERMUTE_ITERATOR_HH
#define _PERMUTE_ITERATOR_HH

#include <algorithm>
#include <iostream>

#include "Permutation.hh"

// Utilities related to iterators.

namespace Permute {

  // Casts a range of integers as iterators that can be used with standard
  // library algorithms.  From range (i) to range (k) means the set {i, i + 1,
  // . . . , k - 1}.
  class range : public std::iterator <std::input_iterator_tag, int> {
    int it_;
  public:
    range (int it) : it_ (it) {}
    int operator * () const { return it_; }
    range operator ++ () { return ++ it_; }
    range operator ++ (int) { return it_ ++; }
    bool operator == (const range & r) const { return it_ == r.it_; }
    bool operator != (const range & r) const { return ! operator == (r); }
  };

  /**********************************************************************/

  template <class InputIterator> class Delimit;

  // This forward declaration enables the compiler to recognize the friend
  // declaration.
  template <class InputIterator>
  std::ostream & operator << (std::ostream &, const Delimit <InputIterator> &);

  // Prints a vector with a given delimiter.
  template <class InputIterator>
  class Delimit {
    friend std::ostream & operator << <> (std::ostream &, const Delimit &);
  private:
    const InputIterator & begin_;
    const InputIterator & end_;
    const char * delimiter_;
  public:
    Delimit (const InputIterator & begin,
	     const InputIterator & end,
	     const char * delimiter) :
      begin_ (begin),
      end_ (end),
      delimiter_ (delimiter)
    {}
  };

  template <class InputIterator>
  std::ostream & operator << (std::ostream & out,
			      const Delimit <InputIterator> & d) {
    std::copy (d.begin_, d.end_,
	       std::ostream_iterator <typename InputIterator::value_type> (out, d.delimiter_));
    return out;
  }

  // Convenience function for constructing instances of Delimit.
  template <class InputIterator>
  Delimit <InputIterator> delimit (const InputIterator & b,
				   const InputIterator & e,
				   const char * d) {
    return Delimit <InputIterator> (b, e, d);
  }

  template <class Container>
  Delimit <typename Container::const_iterator> delimit(const Container& c,
						       const char* d) {
    return delimit(c.begin(), c.end(), d);
  }

  /**********************************************************************/

  // Converts Permutation indices into strings using Permutation::symbol.
  class PermutationSymbol :
    public std::unary_function <size_t, std::string>
  {
  private:
    const Permutation & pi_;
  public:
    PermutationSymbol (const Permutation & pi) : pi_ (pi) {}
    std::string operator () (size_t i) const {
      return pi_.symbol (i);
    }
  };

  /**********************************************************************/

  class PrintPath :
    public std::unary_function <ConstPathRef, void>
  {
  private:
    Permutation & pi_;
  public:
    PrintPath (Permutation & pi) : pi_ (pi) {}
    void operator () (const ConstPathRef & path) {
      pi_.reorder (path);
      std::cout << pi_ << std::endl;
    }
  };

  /**********************************************************************/

  class Boolean {
    friend std::ostream & operator << (std::ostream &, const Boolean &);
  private:
    bool b_;
  public:
    Boolean (bool b) : b_ (b) {}
  };

  /**********************************************************************/

  template <class BinaryFunction, class UnaryFunction>
  class ComposeBinaryUnary :
    public std::binary_function <typename UnaryFunction::argument_type,
				 typename UnaryFunction::argument_type,
				 typename BinaryFunction::result_type>
  {
  private:
    const BinaryFunction & b_;
    const UnaryFunction & u_;
  public:
    ComposeBinaryUnary (const BinaryFunction & b, const UnaryFunction & u) :
      b_ (b),
      u_ (u)
    {}
    typename BinaryFunction::result_type
    operator () (const typename UnaryFunction::argument_type & a,
		 const typename UnaryFunction::argument_type & b) const {
      return b_ (u_ (a), u_ (b));
    }
  };

  template <class BinaryFunction, class UnaryFunction>
  ComposeBinaryUnary <BinaryFunction, UnaryFunction>
  compose_bu (const BinaryFunction & b, const UnaryFunction & u) {
    return ComposeBinaryUnary <BinaryFunction, UnaryFunction> (b, u);
  }

}

#endif//_PERMUTE_ITERATOR_HH
