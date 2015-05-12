// Defines an abstract base class and several concrete subclasses that Chart
// uses to determine the set of trees explored during permutation parsing.

#ifndef _PERMUTE_PARSE_CONTROLLER_HH
#define _PERMUTE_PARSE_CONTROLLER_HH

#include <Core/ReferenceCounting.hh>
#include "Permutation.hh"

namespace Permute {

  // Forward declares BeforeScorer.
  class BeforeScorer;
  
  // Returns an iterator over midpoints for a given (start, end) constituent
  // pair (begin and end methods), or allows queries of particular midpoint
  // values (allows method).
  class ParseController : public Core::ReferenceCounted {
  protected:
    // Implements underlying iteration over allowed midpoints for constituent
    // combination in permutation parsing.
    class iteratorImpl : public Core::ReferenceCounted {
    protected:
      int current_;
    public:
      explicit iteratorImpl (int current) : current_ (current) {}
      virtual iteratorImpl & operator ++ () {
	++ current_;
	return * this;
      }
      virtual int operator * () const {
	return current_;
      }
      virtual bool operator == (const iteratorImpl & i) const {
	return current_ == i.current_;
      }
      virtual bool operator != (const iteratorImpl & i) const {
	return ! (* this == i);
      }
    };
    typedef Core::Ref <iteratorImpl> IterRef;
  public:
    // Wraps an implementation so that it can be polymorphic.
    // @bug Don't use reference counting.
    class iterator {
    private:
      IterRef iter_;
    public:
      iterator (iteratorImpl * impl) : iter_ (impl) {}
      iterator & operator ++ () {
	iter_ -> operator ++ ();
	return * this;
      }
      operator int () const {
	return iter_ -> operator * ();
      }
      bool operator == (const iterator & other) const {
	return int (* this) == int (other);
      }
      bool operator != (const iterator & other) const {
	return ! (* this == other);
      }
    };
    virtual iterator begin (int, int) const = 0;
    virtual iterator end (int, int) const;
    virtual bool allows (int, int, int) const = 0;
    virtual double grammar (const BeforeScorer &, int, int, int) const;
  };

  typedef Core::Ref <const ParseController> ParseControllerRef;

  /**********************************************************************/

  // Limits the set of midpoints for the span (start, end) to those within a
  // given width of start or end.
  class QuadraticParseController : public ParseController {
  protected:
    int size_;
    class gapIterator : public iteratorImpl {
    private:
      int left_, right_;
    public:
      gapIterator (int current, int left, int right);
      virtual gapIterator & operator ++ ();
    };
  public:
    QuadraticParseController (int = 1);
    virtual iterator begin (int, int) const;
    virtual bool allows (int, int, int) const;
    static ParseControllerRef create (int = 1);
  };

  /**********************************************************************/

  // Allows all midpoints in the span (start, end).
  class CubicParseController : public ParseController {
  public:
    virtual iterator begin (int, int) const;
    virtual bool allows (int, int, int) const;
    static ParseControllerRef create ();
  };

  /**********************************************************************/

  // Decorates a given ParseController, overriding some decisions.
  class ParseControllerDecorator : public ParseController {
  protected:
    ParseControllerRef decorated_;
  public:
    ParseControllerDecorator (ParseControllerRef);
    virtual iterator begin (int, int) const;
    virtual iterator end (int, int) const;
    virtual bool allows (int, int, int) const;
    virtual double grammar (const BeforeScorer &, int, int, int) const;
  };

  /**********************************************************************/

  // Allows any midpoint of (start, end) when start is within width of 0.
  class LeftAnchorParseController : public ParseControllerDecorator {
  private:
    int width_;
  public:
    LeftAnchorParseController (ParseControllerRef, int);
    virtual iterator begin (int, int) const;
    virtual bool allows (int, int, int) const;
    virtual double grammar (const BeforeScorer &, int, int, int) const;
    static ParseControllerRef decorate (ParseControllerRef, int);
  };

  /**********************************************************************/

  // Allows any midpoint of (start, end) when end is within width of the length
  // of the given permutation.
  class RightAnchorParseController : public ParseControllerDecorator {
  private:
    const Permutation & permutation_;
    int width_;
  public:
    RightAnchorParseController (ParseControllerRef, const Permutation &, int);
    virtual iterator begin (int, int) const;
    virtual bool allows (int, int, int) const;
    virtual double grammar (const BeforeScorer &, int, int, int) const;
    static ParseControllerRef decorate (ParseControllerRef, const Permutation &, int);
  };
}

#endif//_PERMUTE_PARSE_CONTROLLER_HH
