#ifndef _PERMUTE_EACH_HH
#define _PERMUTE_EACH_HH

#include <functional>

namespace Permute {
  // A class that acts as a functor for a for_each call.  It holds a pointer to
  // a base object, on which a function is called, and is templated on the type
  // of that object, the type of the argument to the member function, and the
  // return type of the member function.
  template <class Base, class Arg, class Result>
  class EachFunctor : public std::unary_function <Arg, Result> {
  private:
    typedef Result (Base::* MemFun) (const Arg &);
    Base * base_;
    MemFun fun_;
  public:
    EachFunctor (Base * base, MemFun fun) :
      base_ (base),
      fun_ (fun)
    {}
    Result operator () (const Arg & arg) const {
      return (base_ ->* fun_) (arg);
    }
  };

  // A helper function for creating an instance of EachFunctor.
  template <class Base, class Arg, class Result>
  EachFunctor <Base, Arg, Result>
  each_fun (Base * base, Result (Base::* fun) (const Arg &)) {
    return EachFunctor <Base, Arg, Result> (base, fun);
  }

  template <class Base, class Arg, class Result>
  class ConstEachFunctor : public std::unary_function <Arg, Result> {
  private:
    typedef Result (Base::* MemFun) (const Arg &) const;
    const Base * base_;
    MemFun fun_;
  public:
    ConstEachFunctor (const Base * base, MemFun fun) :
      base_ (base),
      fun_ (fun)
    {}
    Result operator () (const Arg & arg) const {
      return (base_ ->* fun_) (arg);
    }
  };

  template <class Base, class Arg, class Result>
  ConstEachFunctor <Base, Arg, Result>
  each_fun (Base * base, Result (Base::* fun) (const Arg &) const) {
    return ConstEachFunctor <Base, Arg, Result> (base, fun);
  }
}

#endif//_PERMUTE_EACH_HH
