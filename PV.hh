#ifndef _PERMUTE_PV_HH
#define _PERMUTE_PV_HH

#include <numeric>

#include <Core/Choice.hh>
#include <Core/Hash.hh>

#include "BeforeScorer.hh"
#include "Permutation.hh"

namespace Permute {

  // The types of distance comparisons, including =, <, <=, >, >=.
  typedef enum {
    ComparisonEquals = 0,
    ComparisonLessThan,
    ComparisonLessOrEquals,
    ComparisonGreaterThan,
    ComparisonGreaterOrEquals
  } ComparisonOperator;

  // The basic feature templates of which each of the feature templates
  // comprising a PV is composed.
  typedef enum {
    lPOSm1 = 0,
    lPOS,
    lWord,
    lPOSp1,
    bPOS,
    rPOSm1,
    rPOS,
    rWord,
    rPOSp1,
    Dist,
    // Dependency parse features
    lParent,
    rParent,
    lSibling,
    rSibling,
    // Partial word features
    lPrefix,
    rPrefix,
    lSuffix,
    rSuffix,
    NoFeatureName
  } FeatureName;

  /**********************************************************************/

  // W is a reference-counted weight.  It provides a setter (operator =), an
  // accumulator (operator +=) and a getter (operator double).
  class W : public Core::ReferenceCounted {
  private:
    double w_;
  public:
    explicit W (double = 0.0);
    ~W ();
    W & operator = (double);
    W & operator += (double);
    W & operator /= (double);
    operator double () const;
  };

  /**********************************************************************/

  // WRef is a reference to a W.  It specializes the reference counting
  // interface to include the setter, accumulator, and getter methods of W.
  class WRef : public Core::Ref <W> {
  private:
    typedef Core::Ref <W> Parent;
  public:
    explicit WRef (double = 0.0);
    const WRef & operator = (double) const;
    const WRef & operator += (double) const;
    const WRef & operator /= (double) const;
    operator double () const;
  };

  /**********************************************************************/

  // A FeatureType contains a map from value strings to strings containing
  // integers compressed into the minimum necessary number of bytes.
  class FeatureType {
  private:
    typedef Core::StringHashMap <std::string>::const_iterator const_iterator;
    std::string name_;
    Core::StringHashMap <std::string> map_;
    Core::StringHashMap <std::string> inverse_;
    int count_;
    int bytes_;
    std::vector <unsigned char> next_;
  public:
    FeatureType (const std::string & name, int count);

    const std::string & name () const { return name_; }
    int count () const { return count_; }
    int bytes () const { return bytes_; }
    const std::string & intern (const std::string &);
    const std::string & unintern (const std::string &);
  };

  /**********************************************************************/

  // A feature template is a set of FeatureNames.
  class FeatureTemplate : public std::vector <char> {
  public:
    FeatureTemplate () :
      std::vector <char> (NoFeatureName, false)
    {}
    size_t active () const {
      return std::accumulate (begin (), end (), size_t (0));
    }
  };

  std::ostream & operator << (std::ostream &, const FeatureTemplate &);

  /**********************************************************************/

  // Helper class used for printing integer representations of compressed
  // feature strings.
  class CompressedFeature {
  public:
    std::string f_;
    CompressedFeature (const std::string f) :
      f_ (f)
    {}
  };

  std::ostream & operator << (std::ostream &, const CompressedFeature &);

  /**********************************************************************/

  // Maps FeatureNames to string values.
  class FeatureMap {
  private:
    std::vector <std::string> map_;
    FeatureTemplate template_;
  public:
    FeatureMap ();
    void setValue (FeatureName feature, const std::string & value);
    void setValue (const std::string & feature, const std::string & value);
    void setValue (const std::string & pair);
    void setAll (const std::string & all);
    const std::string & getValue (FeatureName feature) const;
    const std::string & getValue (const std::string & feature) const;
    size_t size () const { return map_.size (); }
    const FeatureTemplate & mask () const { return template_; }
    std::string compress (const std::string & id,
			  const std::vector <FeatureType *> & types) const;
    bool has (const FeatureTemplate & ft) const;
    void fill (std::ostream & out, const FeatureTemplate & ft) const;
    std::ostream & print (std::ostream & out) const;
  };

  std::ostream & operator << (std::ostream &, const FeatureMap &);

  /**********************************************************************/

  // A list of templates encoded as integers and mapped to single-character
  // strings.  Currently breaks if there are more than 256 feature templates.
  class TemplateList {
  public:
    typedef std::map <std::string, FeatureType *> TypeMap;
    typedef std::vector <FeatureType *> TypeVector;
    typedef std::map <FeatureTemplate, std::string> TemplateMap;
    typedef TemplateMap::const_iterator const_iterator;
  private:
    TypeMap types_;
    TypeVector feature_types_;
    TemplateMap map_;
    int count_;
  public:
    TemplateList ();
    // Does not copy the template map, just types_ and feature_types_.
    TemplateList (const TemplateList &);
    ~TemplateList ();
    const_iterator begin () const { return map_.begin (); }
    const_iterator end () const { return map_.end (); }

    void addType (const std::string & name, int count);
    void addFeatureType (const std::string & feature, const std::string & type);
    void addTemplate (const std::string & templ);
    void addTemplate (const FeatureTemplate & templ);

    std::string compress (const std::string &) const;
    std::string uncompress (const std::string &) const;

    void intern (FeatureMap & fmap) const;
    void intern (FeatureMap & fmap, FeatureName templ) const;
    void featuresWithTemplate (std::vector <std::string> & f,
			       const FeatureMap & fmap,
			       FeatureName templ,
			       bool bit) const;
    std::string featureFromTemplate (const FeatureMap & fmap,
				     const FeatureTemplate & ft) const;
    std::string featureFromIterator (const FeatureMap & fmap,
				     const_iterator it) const;

    std::vector <FeatureTemplate> generalize (const FeatureTemplate & ft) const;
  private:
    FeatureTemplate getTemplate (const std::string &) const;

    friend bool writeXml (const TemplateList &, Core::XmlWriter &);
  };

  /**********************************************************************/

  // The Boost hash function shows much better performance than the STL string
  // hasher.  437251 / 440160 unique hashes, versus 154279 / 440160 for STL on
  // compressed features.
  class StringHash : public std::unary_function <const std::string &, size_t> {
  public:
    size_t operator () (const std::string & s) const {
      size_t seed = 0;
      for (std::string::const_iterator it = s.begin (), end = s.end ();
	   it != end; ++ it) {
	seed ^= static_cast <size_t> (* it) + 0x9e3779b9 + (seed << 6) + (seed >> 2);
      }
      return seed;
    }
  };

  // A PV is simply a hash table mapping strings to weights.  The strings are
  // encoded features and the weights are stored as instances of WRef.  PV also
  // holds vectors of feature templates and distance comparisons so that it can
  // compute feature strings given indices into a permutation.
  //
  // The features method populates a vector of strings with the list of features
  // that fire for a given pair (i,j) of positions in a given permutation.
  class PV : public __gnu_cxx::hash_map <std::string, WRef, StringHash, Core::StringEquality> {
  private:
    typedef __gnu_cxx::hash_map <std::string, WRef, StringHash, Core::StringEquality> Parent;
    typedef std::vector <std::pair <ComparisonOperator, int> > DistanceVector;

    TemplateList templates_;
    DistanceVector distances_;

  public:
    PV ();
    // Does not copy feature values, only templates_ and distances_.
    PV (const PV &);
    void addType (const std::string &, int);
    void addFeatureType (const std::string &, const std::string &);
    void addDistance (ComparisonOperator, int);
    void addTemplate (const std::string &);
    void addTemplate (const FeatureTemplate &);
    WRef & getParameter (const std::string &);

    const TemplateList & templates () const { return templates_; }
    std::string uncompress (const std::string &) const;

    void features (std::vector <std::string> &, const Permutation & words, const Permutation & pos, size_t, size_t) const;
    void features (std::vector <std::string> &,
		   const Permutation & words,
		   const Permutation & pos,
		   const std::vector <int> & parents,
		   const Permutation & labels,
		   size_t i, size_t j) const;
    
    std::string distance (int) const;

    static const int PREFIX_SIZE;
    static std::string prefix (const std::string &);
    static const int SUFFIX_SIZE;
    static std::string suffix (const std::string &);

    friend bool writeXml (const PV &, std::ostream &);
  };

  bool readXml (PV &, std::istream &);
  bool readFile (PV &, const std::string &);
  bool aggregateFile (PV &, const std::string &);

  /**********************************************************************/

  // Serves as the sum of a list of weights.  The accumulator (operator +=) adds
  // an additional weight to the sum.  The getter method (operator double)
  // computes the value of the sum.  The add method accumulates the given value
  // onto each of the weights in the sum.
  //
  // Invariant: sum_ always contains the sum of the weights in v_.  Thus, sum_
  // is initialized to zero, operator += accumulates into sum, and add
  // accumulates into sum once for each weight in v_.
  class Sum {
  private:
    std::vector <WRef> v_;
    double sum_;
  public:
    Sum ();
    Sum (const Sum &);
    Sum & operator += (const WRef &);
    operator double () const;
    void add (double);
    std::vector <WRef>::const_iterator begin () const { return v_.begin (); }
    std::vector <WRef>::const_iterator end () const { return v_.end (); }
  private:
    Sum & operator = (const Sum &);
  };

  /**********************************************************************/

  // Implements the BeforeCostInterface by storing a Sum at each matrix position
  // rather than a single value.  Allows learning algorithms to propagate
  // updates to matrix positions back to the features from which the matrix was
  // computed.
  class SumBeforeCost : public BeforeCostInterface {
  private:
    Fsa::Vector <Sum> matrix_;
    std::string name_;
  public:
    SumBeforeCost (size_t, const std::string & name);
    ~ SumBeforeCost ();
    virtual double cost (int, int) const;
    virtual const std::string & name () const;
    Sum & operator () (int, int);
  };

  typedef Core::Ref <SumBeforeCost> SumBeforeCostRef;
  
}

#endif//_PERMUTE_PV_HH
