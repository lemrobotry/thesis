#ifndef _PERMUTE_PARAMETER_HH
#define _PERMUTE_PARAMETER_HH

#include <numeric>

#include <Core/XmlParser.hh>
#include <Fsa/Types.hh>
#include <Fsa/Alphabet.hh>

#include "Permutation.hh"

namespace Permute {
  typedef Fsa::LabelId POS;
  
  typedef enum {
    CompEquals = 0,
    CompLessThan,
    CompGreaterThan
  } Comparison;

  /**********************************************************************/
  
  class Feature {
    friend class LessThanFeature;
  private:
    POS i_, j_, im1_, ip1_, jm1_, jp1_, b_;
    int distance_;
    Comparison op_;
  public:
    Feature ();
    bool fires (const Permutation &, size_t, size_t) const;
    void setFirstPOS (POS);
    void setFirstPOSm1 (POS);
    void setFirstPOSp1 (POS);
    void setSecondPOS (POS);
    void setSecondPOSm1 (POS);
    void setSecondPOSp1 (POS);
    void setBetweenPOS (POS);
    void setPOS (const Feature &);
    void setDistance (int);
    void setComparison (Comparison);
    void writeXml (Fsa::ConstAlphabetRef, Core::XmlWriter &) const;

    static Feature lower_bound (POS, POS);
    static Feature upper_bound (POS, POS);
  };

  /**********************************************************************/

  class LessThanFeature {
  public:
    bool operator () (const Feature & a, const Feature & b) const {
      POS * aa = (POS *) (& a),
	* ba = (POS *) (& b);
      for (int i = 0; i < 7; ++ i) {
	if (aa [i] < ba [i]) return true;
	else if (ba [i] < aa [i]) return false;
      }
      if (a.distance_ < b.distance_) {
	return true;
      } else if (a.distance_ == b.distance_) {
	return (a.op_ < b.op_);
      }
      return false;
    }
  };

  /**********************************************************************/

  typedef std::map <Feature, double, LessThanFeature> parameter_map;

  class WeightIterator {
  private:
    parameter_map::iterator i_;
  public:
    WeightIterator (parameter_map::iterator i) : i_ (i) {}
    double & operator * () { return i_ -> second; }
    WeightIterator & operator ++ () { ++ i_; return * this; }
    bool operator != (const WeightIterator & other) const { return i_ != other.i_; }
  };

  class ConstWeightIterator {
  private:
    parameter_map::const_iterator i_;
  public:
    ConstWeightIterator (parameter_map::const_iterator i) : i_ (i) {}
    double operator * () const { return i_ -> second; }
    ConstWeightIterator & operator ++ () { ++ i_; return * this; }
    bool operator != (const ConstWeightIterator & other) const { return i_ != other.i_; }
  };

  /**********************************************************************/

  class ParameterVector : public parameter_map {
  public:
    typedef WeightIterator iterator;
    typedef ConstWeightIterator const_iterator;
    typedef parameter_map::iterator parameter_iterator;
    typedef parameter_map::const_iterator const_parameter_iterator;
  private:
    Fsa::ConstAlphabetRef POS_;
  public:
    ParameterVector () {}

    Fsa::ConstAlphabetRef getPOS () const;
    void setPOS (Fsa::ConstAlphabetRef);

    iterator begin ();
    iterator end ();
    const_iterator begin () const;
    const_iterator end () const;

    parameter_iterator begin_p ();
    parameter_iterator end_p ();
    parameter_iterator begin_p (POS, POS);
    parameter_iterator end_p (POS, POS);
    
    const_parameter_iterator begin_p () const;
    const_parameter_iterator end_p () const;
    const_parameter_iterator begin_p (POS, POS) const;
    const_parameter_iterator end_p (POS, POS) const;
  };

  typedef ParameterVector::value_type Parameter;

  int count (const Parameter &, const Permutation &, size_t, size_t);
  double weight (const Parameter &, const Permutation &, size_t, size_t);

  /**********************************************************************/

  double sum (const ParameterVector &, const Permutation &, size_t, size_t);
  void count (std::vector <double> &, const ParameterVector &, const Permutation &, size_t, size_t, float);

  template <class TO, class FROM>
  void update (TO & to, const FROM & from, double multiplier = 1.0) {
    typename TO::iterator t = to.begin ();
    typename FROM::const_iterator f = from.begin ();
    for (; t != to.end () && f != from.end (); ++ t, ++ f) {
      (* t) += multiplier * double (* f);
    }
  }

  template <class TO, class FROM>
  void set (TO & to, const FROM & from, double multiplier = 1.0) {
    typename TO::iterator t = to.begin ();
    typename FROM::const_iterator f = from.begin ();
    for (; t != to.end () && f != from.end (); ++ t, ++ f) {
      (* t) = multiplier * double (* f);
    }
  }

  template <class TO, class FROM>
  double distance (const TO & a, const FROM & b) {
    double ss = 0.0;
    typename TO::const_iterator x = a.begin ();
    typename FROM::const_iterator y = b.begin ();
    for (; x != a.end () && y != b.end (); ++ x, ++ y) {
      ss += pow (double (* x) - double (* y), 2);
    }
    return sqrt (ss);
  }

  template <class Sequence>
  double norm (const Sequence & s) {
    double ss = 0.0;
    for (typename Sequence::const_iterator i = s.begin (); i != s.end (); ++ i) {
      ss += pow (* i, 2);
    }
    return sqrt (ss);
  }

  template <class A, class B>
  double dot (const A & a, const B & b) {
    return std::inner_product (a.begin (), a.end (), b.begin (), 0.0);
  }

  template <class A, class B>
  double cos (const A & a, const B & b) {
    return dot (a, b) / (norm (a) * norm (b));
  }

  /**********************************************************************/

  class FeatureCounter {
  private:
    ParameterVector & pv_;
    const Permutation & pos_;
  public:
    FeatureCounter (ParameterVector &, const Permutation &);
    void count (const Permutation &, double);
  };

  /**********************************************************************/

  bool readXml (ParameterVector &, std::istream &);
  bool writeXml (const ParameterVector &, std::ostream &);

  class ParameterVectorXmlParser : public Core::XmlSchemaParser {
  private:
    typedef ParameterVectorXmlParser Self;
    
    ParameterVector & pv_;
    Feature phi_;
    double weight_;
    std::string content_;

    void startParameter (const Core::XmlAttributes);
    void endParameter ();
    void startFeature (const Core::XmlAttributes);
    void startContent (const Core::XmlAttributes);
    void content (const char *, int);
    void endFirstPOS ();
    void endFirstPOSm1 ();
    void endFirstPOSp1 ();
    void endSecondPOS ();
    void endSecondPOSm1 ();
    void endSecondPOSp1 ();
    void endBetweenPOS ();
    void endDistance ();
    void endComparison ();
    void endWeight ();

  public:
    ParameterVectorXmlParser (const Core::Configuration &, ParameterVector &);
    ~ ParameterVectorXmlParser ();
    bool parseString (const std::string &);
    bool parseStream (std::istream &);
    bool parseFile (const std::string &);
  };

  /**********************************************************************/

  // A SparseParameter holds a pointer, in the form of an iterator, to a
  // parameter in a parameter_map, and an occurrence count, in the form of a
  // double.  It provides methods for extracting the weight of the parameter,
  // incrementing the occurrence count, updating the weight according to the
  // count, and comparing against other SparseParameters.
  class SparseParameter {
  private:
    static LessThanFeature ltf;
  public:
    mutable parameter_map::iterator parameter_;
    mutable double count_;
    SparseParameter (parameter_map::iterator, double);
    double weight () const;
    void increment (double) const;
    void update (double) const;
    bool operator < (const SparseParameter &) const;
    bool operator == (const SparseParameter &) const;
  };

  /**********************************************************************/

  // A SparseParameterVector consists of a set of SparseParameters.  It holds a
  // margin for MIRA learning.  It provides methods for computing the dot
  // product with another vector and computing its squared norm, as well as for
  // updating all its contained parameters.
  class SparseParameterVector : std::set <SparseParameter> {
  private:
    double margin_;
  public:
    SparseParameterVector (double = 1.0);
    void setMargin (double);
    double dot (const SparseParameterVector &) const;
    double norm2 () const;
    double margin () const;
    void update (double);
    void build (ParameterVector &, const Permutation &, double = 1.0);
    void build (ParameterVector &, const Permutation &, size_t, size_t, bool);
  private:
    void build_helper (ParameterVector &, const Permutation &, size_t, size_t, double);
  };
}

#endif//_PERMUTE_PARAMETER_HH
