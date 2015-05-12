#include <Core/Application.hh>
#include <Core/Choice.hh>
#include <Core/StringUtilities.hh>
#include <Core/Unicode.hh>
#include <Core/XmlParser.hh>

#include "Each.hh"
#include "PV.hh"

namespace Permute {

  Core::Choice ComparisonChoice ("=", ComparisonEquals,
				 "<", ComparisonLessThan,
				 "<=", ComparisonLessOrEquals,
				 ">", ComparisonGreaterThan,
				 ">=", ComparisonGreaterOrEquals,
				 CHOICE_END),

    FeatureChoice ("l-pos-1", lPOSm1,
		   "l-pos", lPOS,
		   "l-word", lWord,
		   "l-pos+1", lPOSp1,
		   "b-pos", bPOS,
		   "r-pos-1", rPOSm1,
		   "r-pos", rPOS,
		   "r-word", rWord,
		   "r-pos+1", rPOSp1,
		   "dist", Dist,
		   "l-par", lParent,
		   "r-par", rParent,
		   "l-sib", lSibling,
		   "r-sib", rSibling,
		   "l-prefix", lPrefix,
		   "r-prefix", rPrefix,
		   "l-suffix", lSuffix,
		   "r-suffix", rSuffix,
		   CHOICE_END);

  /**********************************************************************
   * FeatureType methods
   **********************************************************************/

  FeatureType::FeatureType (const std::string & name, int count) :
    name_ (name),
    map_ (),
    inverse_ (),
    count_ (count),
    bytes_ (count == 0 ? 0 : int (ceil (log2 (count) / 8.0))),
    next_ (bytes_, 0)
  {}

  const std::string & FeatureType::intern (const std::string & value) {
    const_iterator it = map_.find (value);
    if (it != map_.end ()) {
      return it -> second;
    } else {
      std::string compressed (next_.begin (), next_.end ());
      for (std::vector <unsigned char>::iterator it = next_.begin ();
	   it != next_.end () && ++ (* it) == 0;
	   ++ it);
      inverse_ [compressed] = value;
      return (map_ [value] = compressed);
    }
  }

  const std::string & FeatureType::unintern (const std::string & vz) {
    return inverse_ [vz];
  }

  /**********************************************************************
   * FeatureTemplate methods
   **********************************************************************/

  std::ostream & operator << (std::ostream & out, const FeatureTemplate & ft) {
    for (FeatureTemplate::const_iterator it = ft.begin (); it != ft.end (); ++ it) {
      if (* it) {
	out << FeatureChoice [static_cast <FeatureName> (it - ft.begin ())] << " ";
      }
    }
    return out;
  }

  /**********************************************************************
   * CompressedFeature methods
   **********************************************************************/

  std::ostream & operator << (std::ostream & out, const CompressedFeature & c) {
    for (std::string::const_iterator it = c.f_.begin (); it != c.f_.end (); ++ it) {
      out << static_cast <int> (* it) << " ";
    }
    return out;
  }

  /**********************************************************************
   * FeatureMap methods
   **********************************************************************/

  FeatureMap::FeatureMap () :
    map_ (FeatureChoice.nChoices (), ""),
    template_ ()
  {}

  void FeatureMap::setValue (FeatureName feature,
			     const std::string & value) {
    map_ [feature] = value;
    template_ [feature] = true;
  }
  void FeatureMap::setValue (const std::string & feature,
			     const std::string & value) {
    setValue (FeatureName (FeatureChoice [feature]), value);
  }
  void FeatureMap::setValue (const std::string & pair) {
    std::vector <std::string> fv = Core::split (pair, "=");
    if (fv.size () == 2) {
      setValue (fv [0], fv [1]);
    }
  }

  void FeatureMap::setAll (const std::string & all) {
    std::vector <std::string> pairs = Core::split (all, " ");
    std::for_each (pairs.begin (), pairs.end (),
		   each_fun (this, & FeatureMap::setValue));
  }

  const std::string & FeatureMap::getValue (FeatureName feature) const {
    return map_ [feature];
  }
  const std::string & FeatureMap::getValue (const std::string & feature) const {
    return getValue (FeatureName (FeatureChoice [feature]));
  }

  // Iterates over feature values present in the mask and replaces their values
  // with compressed versions from the FeatureType maps.
  // @bug Probably inefficient.
  std::string FeatureMap::compress (const std::string & id,
				    const std::vector <FeatureType *> & types) const {
    std::ostringstream buffer;
    buffer << id;
    for (FeatureTemplate::const_iterator it = template_.begin (); it != template_.end (); ++ it) {
      if (* it) {
	int i = it - template_.begin ();
	buffer << types [i] -> intern (map_ [i]);
      }
    }
    return buffer.str ();
  }

  bool FeatureMap::has (const FeatureTemplate & ft) const {
    for (FeatureTemplate::const_iterator it = template_.begin (), ft_it = ft.begin ();
	 it != template_.end ();
	 ++ it, ++ ft_it) {
      if (* ft_it && ! * it) {
	return false;
      }
    }
    return true;
  }

  void FeatureMap::fill (std::ostream & out, const FeatureTemplate & ft) const {
    for (FeatureTemplate::const_iterator it = ft.begin (); it != ft.end (); ++ it) {
      if (* it) {
	out << getValue (static_cast <FeatureName> (it - ft.begin ()));
      }
    }
  }

  std::ostream & FeatureMap::print (std::ostream & out) const {
    for (FeatureTemplate::const_iterator it = template_.begin ();
	 it != template_.end (); ++ it) {
      if (* it) {
	FeatureName index = static_cast <FeatureName> (it - template_.begin ());
	out << FeatureChoice [index] << "=" << getValue (index) << " ";
      }
    }
    return out;
  }

  std::ostream & operator << (std::ostream & out, const FeatureMap & map) {
    return map.print (out);
  }

  /**********************************************************************
   * TemplateList methods
   **********************************************************************/

  TemplateList::TemplateList () :
    types_ (),
    feature_types_ (FeatureChoice.nChoices ()),
    map_ (),
    count_ (0)
  {}

  TemplateList::TemplateList (const TemplateList & tl) :
    types_ (),
    feature_types_ (FeatureChoice.nChoices ()),
    map_ (),
    count_ (0)
  {
    for (TypeMap::const_iterator it = tl.types_.begin ();
	 it != tl.types_.end (); ++ it) {
      addType (it -> first, it -> second -> count ());
    }
    for (int i = 0; i < FeatureChoice.nChoices (); ++ i) {
      addFeatureType (FeatureChoice [FeatureName (i)],
		      tl.feature_types_ [i] -> name ());
    }
  }
  
  TemplateList::~TemplateList () {
    for (TypeMap::const_iterator it = types_.begin (); it != types_.end (); ++ it) {
      delete it -> second;
    }
  }    

  // @bug Does nothing if the type already exists.  Should make sure that the
  // capacity of the type is enough for both old and new.
  void TemplateList::addType (const std::string & name,
			      int count) {
    if (! types_ [name]) {
      types_ [name] = new FeatureType (name, count);
    }
  }

  void TemplateList::addFeatureType (const std::string & feature,
				     const std::string & type) {
    feature_types_ [FeatureChoice [feature]] = types_ [type];
  }

  // Adds a feature template, in the form of a space-delimited string, to the
  // set of templates, stored as bit vectors.
  void TemplateList::addTemplate (const std::string & templ) {
    addTemplate (getTemplate (templ));
  }

  void TemplateList::addTemplate (const FeatureTemplate & templ) {
    map_ [templ] = std::string (1, char (count_ ++));
  }

  // Converts a string containing space-delimited feature=value pairs into a
  // compressed string whose first position contains the template number, and
  // whose remaining positions contain interned representations of the feature
  // values.
  std::string TemplateList::compress (const std::string & feature) const {
    FeatureMap fmap;
    fmap.setAll (feature);
    const_iterator it = map_.find (fmap.mask ());
    if (it != map_.end ()) {
      return fmap.compress (it -> second, feature_types_);
    } else {
      return "";
    }
  }

  // Finds the mask corresponding to the given compressed feature, then fills in
  // the values according to the mask.
  // @bug Mask finding is inefficient.
  std::string TemplateList::uncompress (const std::string & fz) const {
    std::istringstream input (fz);
    std::ostringstream output;
    char buffer [4];
    FeatureTemplate mask;
    input.read (buffer, 1);
    std::string mz (buffer, buffer + 1);
    for (TemplateMap::const_iterator it = map_.begin (); it != map_.end (); ++ it) {
      if (it -> second == mz) {
	mask = it -> first;
	break;
      }
    }
    bool first = true;
    for (int i = 0; i < FeatureChoice.nChoices (); ++ i) {
      if (mask [i]) {
	if (first) { first = false; } else { output << " "; }
	output << FeatureChoice [FeatureName (i)]
	       << "=";
	input.read (buffer, feature_types_ [i] -> bytes ());
	std::string vz (buffer, buffer + feature_types_ [i] -> bytes ());
	output << feature_types_ [i] -> unintern (vz);
      }
    }
    return output.str ();
  }

  // Returns a mask with the bits of all the given features set.
  FeatureTemplate TemplateList::getTemplate (const std::string & temp) const {
    FeatureTemplate template_mask;
    std::vector <std::string> features = Core::split (temp, " ");
    for (std::vector <std::string>::const_iterator it = features.begin ();
	 it != features.end (); ++ it) {
      template_mask [FeatureChoice [* it]] = true;
    }
    return template_mask;
  }

  // Interns all of the features in the given map.  Call before
  // featuresWithTemplate.
  void TemplateList::intern (FeatureMap & fmap) const {
    const FeatureTemplate & mask = fmap.mask ();
    for (int it = 0; it < FeatureChoice.nChoices (); ++ it) {
      if (mask [it]) {
	intern (fmap, static_cast <FeatureName> (it));
      }
    }
  }

  // Interns a given feature if there is a map to use.  If not, leaves it
  // unchanged (it won't be used in any feature strings.
  void TemplateList::intern (FeatureMap & fmap, FeatureName templ) const {
    if (feature_types_ [templ]) {
      fmap.setValue (templ, feature_types_ [templ] -> intern (fmap.getValue (templ)));
    }
  }
  
  // Adds an instantiation of each template with the given FeatureName/bit pair
  // to the given vector with template values taken from the given map.
  // @precondition The map must already have been interned.
  void TemplateList::featuresWithTemplate (std::vector <std::string> & f,
					   const FeatureMap & fmap,
					   FeatureName templ,
					   bool bit) const {
    for (const_iterator it = map_.begin (), end = map_.end ();
	 it != end; ++ it) {
      if (fmap.has (it -> first) && it -> first [templ] == bit) {
	f.push_back (featureFromIterator (fmap, it));
      }
    }
  }

  // NOTE: Caller's responsibility to check that fmap.has (ft).
  std::string TemplateList::featureFromTemplate (const FeatureMap & fmap,
						 const FeatureTemplate & ft) const {
    const_iterator it = map_.find (ft);
    if (it != map_.end ()) {
      return featureFromIterator (fmap, it);
    } else {
      return "";
    }
  }

  std::string TemplateList::featureFromIterator (const FeatureMap & fmap,
						 const_iterator it) const {
    std::ostringstream buffer;
    buffer << it -> second;
    fmap.fill (buffer, it -> first);
    return buffer.str ();
  }

  // Computes the number of active features the first template contains that the
  // second does not.  If the second contains active features not in the first,
  // the difference is zero.
  //
  // NOTE: Handles bPOS specially.  If the first template has bPOS but the
  // second doesn't, generalization of the first to the second will not work, so
  // returns zero.
  int featureTemplateDifference (const FeatureTemplate & ft1,
				 const FeatureTemplate & ft2) {
    int diff = 0;
    if (ft1 [bPOS] > ft2 [bPOS]) {
      return 0;
    }
    for (int i = 0; i < FeatureChoice.nChoices (); ++ i) {
      if (ft1 [i] < ft2 [i]) {
	return 0;
      } else if (ft1 [i] > ft2 [i]) {
	++ diff;
      }
    }
    return diff;
  }

  // Iterates over the templates in the map, and computes the difference between
  // them and the given template in terms of active features.  If a template
  // contains active features not in the given template, the difference is set
  // to zero, otherwise it is the difference in the number of features.  Returns
  // a vector containing those templates with the minimum positive difference,
  // or an empty vector if none exist.
  std::vector <FeatureTemplate>
  TemplateList::generalize (const FeatureTemplate & ft) const {
    std::vector <FeatureTemplate> gv;
    std::map <FeatureTemplate, int> diff;
    int min_diff = std::numeric_limits <int>::max ();
    for (const_iterator it = map_.begin (); it != map_.end (); ++ it) {
      int d = featureTemplateDifference (ft, it -> first);
      if (d) {
	diff [it -> first] = d;
	min_diff = std::min (min_diff, d);
      }
    }
    for (std::map <FeatureTemplate, int>::const_iterator it = diff.begin ();
	 it != diff.end (); ++ it) {
      if (it -> second == min_diff) {
	gv.push_back (it -> first);
      }
    }
    return gv;
  }
  
  /**********************************************************************
   * PV public methods
   **********************************************************************/

  PV::PV () :
    Parent (),
    templates_ (),
    distances_ ()
  {}

  PV::PV (const PV & pv) :
    Parent (),
    templates_ (pv.templates_),
    distances_ (pv.distances_)
  {}

  // Adds the given type, with the given count, to the inventory.  Creates a new
  // map to associate the values of the type with strings of the appropriate
  // number of bytes.
  void PV::addType (const std::string & type, int count) {
    templates_.addType (type, count);
  }

  // Adds the given feature type.
  void PV::addFeatureType (const std::string & feature, const std::string & type) {
    templates_.addFeatureType (feature, type);
  }

  // Adds the given (operator, distance) pair to the list of distance templates
  // if it is not already present.
  void PV::addDistance (ComparisonOperator comp, int dist) {
    // Don't add duplicates.
    for (DistanceVector::const_iterator d = distances_.begin (); d != distances_.end (); ++ d) {
      if (d -> first == comp && d -> second == dist)
	return;
    }
    distances_.push_back (std::make_pair (comp, dist));
  }

  // Splits the given string into a sequence of simple feature names, then
  // adds the resulting sequence of feature names to the templates list.
  void PV::addTemplate (const std::string & templ) {
    templates_.addTemplate (templ);
  }

  void PV::addTemplate (const FeatureTemplate & templ) {
    templates_.addTemplate (templ);
  }

  // Converts a long feature=value feature name into a compressed feature name
  // and returns the associated weight.
  WRef & PV::getParameter (const std::string & feature) {
    return operator [] (templates_.compress (feature));
  }

  std::string PV::uncompress (const std::string & fz) const {
    return templates_.uncompress (fz);
  }

  void PV::features (std::vector <std::string> & f,
		     const Permutation & words,
		     const Permutation & pos,
		     size_t i, size_t j) const {
    Fsa::ConstAlphabetRef WORDS = words.alphabet (), POS = pos.alphabet ();
    FeatureMap fmap;
    fmap.setValue (lPOSm1, i == 0 ? "<s>" : POS -> symbol (pos.label (pos [i - 1])));
    fmap.setValue (lPOS, POS -> symbol (pos.label (pos [i])));
    fmap.setValue (lWord, WORDS -> symbol (words.label (words [i])));
    fmap.setValue (lPOSp1, POS -> symbol (pos.label (pos [i + 1])));
    fmap.setValue (rPOSm1, POS -> symbol (pos.label (pos [j - 1])));
    fmap.setValue (rPOS, POS -> symbol (pos.label (pos [j])));
    fmap.setValue (rWord, WORDS -> symbol (words.label (words [j])));
    fmap.setValue (rPOSp1, j + 1 == pos.size () ? "</s>" : POS -> symbol (pos.label (pos [j + 1])));
    fmap.setValue (Dist, distance (j - i));
    fmap.setValue (lPrefix, prefix (words.symbol (i)));
    fmap.setValue (rPrefix, prefix (words.symbol (j)));
    fmap.setValue (lSuffix, suffix (words.symbol (i)));
    fmap.setValue (rSuffix, suffix (words.symbol (j)));
    templates_.intern (fmap);
    templates_.featuresWithTemplate (f, fmap, bPOS, 0); 
    for (size_t b = i + 1; b < j; ++ b) {
      fmap.setValue (bPOS, POS -> symbol (pos.label (pos [b])));
      templates_.intern (fmap, bPOS);
      templates_.featuresWithTemplate (f, fmap, bPOS, 1);
    }
  }

  void PV::features (std::vector <std::string> & f,
		     const Permutation & words,
		     const Permutation & pos,
		     const std::vector <int> & parents,
		     const Permutation & labels,
		     size_t i, size_t j) const {
    FeatureMap fmap;
    fmap.setValue (lPOSm1, i == 0 ? "<s>" : pos.symbol (i - 1));
    fmap.setValue (lPOS, pos.symbol (i));
    fmap.setValue (lWord, words.symbol (i));
    fmap.setValue (lPOSp1, pos.symbol (i + 1));
    fmap.setValue (rPOSm1, pos.symbol (j - 1));
    fmap.setValue (rPOS, pos.symbol (j));
    fmap.setValue (rWord, words.symbol (j));
    fmap.setValue (rPOSp1, j + 1 == pos.size () ? "</s>" : pos.symbol (j + 1));
    fmap.setValue (Dist, distance (j - i));
    if (parents [i] == j) {
      fmap.setValue (lParent, labels.symbol (i));
    }
    if (parents [j] == i) {
      fmap.setValue (rParent, labels.symbol (j));
    }
    if (parents [i] == parents [j]) {
      fmap.setValue (lSibling, labels.symbol (i));
      fmap.setValue (rSibling, labels.symbol (j));
    }
    fmap.setValue (lPrefix, prefix (words.symbol (i)));
    fmap.setValue (rPrefix, prefix (words.symbol (j)));
    fmap.setValue (lSuffix, suffix (words.symbol (i)));
    fmap.setValue (rSuffix, suffix (words.symbol (j)));
    templates_.intern (fmap);
    templates_.featuresWithTemplate (f, fmap, bPOS, 0);
    for (size_t b = i + 1; b < j; ++ b) {
      fmap.setValue (bPOS, pos.symbol (b));
      templates_.intern (fmap, bPOS);
      templates_.featuresWithTemplate (f, fmap, bPOS, 1);
    }
  }

  /**********************************************************************
   * PV private methods
   **********************************************************************/

  // Performs the given comparison on the given pair of values and returns the
  // result of the comparison.
  bool compare (int i, ComparisonOperator op, int j) {
    switch (op) {
    case ComparisonEquals:
      return (i == j);
    case ComparisonLessThan:
      return (i < j);
    case ComparisonLessOrEquals:
      return (i <= j);
    case ComparisonGreaterThan:
      return (i > j);
    case ComparisonGreaterOrEquals:
      return (i >= j);
    default:
      return false;
    }
  }

  // Iterates over the distance templates and returns a string corresponding to
  // the first one for which compare succeeds.  If none succeeds, returns the
  // distance itself.
  std::string PV::distance (int dist) const {
    std::ostringstream stream;
    for (DistanceVector::const_iterator d = distances_.begin (); d != distances_.end (); ++ d) {
      if (compare (dist, d -> first, d -> second)) {
	stream << ComparisonChoice [d -> first] << d -> second;
	return stream.str ();
      }
    }
    stream << dist;
    return stream.str ();
  }

  const int PV::PREFIX_SIZE (5);

  std::string PV::prefix (const std::string & s) {
    std::wstring wide = widen(s);
    if (wide.size () < PREFIX_SIZE) {
      return s;
    } else {
      return narrow(wide.substr (0, PREFIX_SIZE));
    }
  }

  const int PV::SUFFIX_SIZE (5);
  
  std::string PV::suffix (const std::string & s) {
    std::wstring wide = widen(s);
    if (wide.size () < SUFFIX_SIZE) {
      return s;
    } else {
      return narrow(wide.substr (wide.size () - SUFFIX_SIZE, SUFFIX_SIZE));
    }
  }

  /**********************************************************************
   * PVXmlParser
   **********************************************************************/

  class PVXmlParser : public Core::XmlSchemaParser {
  protected:
    typedef PVXmlParser Self;

//     Core::XmlMixedElementRelay xml_parameters_,
//       xml_type_count_,
//       xml_feature_type_,
//       xml_distance_,
//       xml_comparison_,
//       xml_template_,
//       xml_parameter_,
//       xml_feature_,
//       xml_weight_;

    PV & pv_;
    std::string content_;

    std::string type_;
    int count_;
    
    ComparisonOperator comp_;
    int dist_;

    std::string feature_;
    double weight_;
    
    void startContent (const Core::XmlAttributes);
    void content (const char *, int);

    void endTypeCount ();
    void endFeatureType ();
    void startDistance (const Core::XmlAttributes);
    void distance (const char *, int);
    virtual void endDistance ();
    virtual void endComparison ();
    virtual void endTemplate ();
    void startParameter (const Core::XmlAttributes);
    virtual void endParameter ();
    virtual void endFeature ();
    virtual void endWeight ();

  public:
    PVXmlParser (const Core::Configuration &, PV &);
    ~ PVXmlParser ();
    bool parseString (const std::string &);
    bool parseStream (std::istream &);
    bool parseFile (const std::string &);
  };

  /**********************************************************************
   * XML functions
   **********************************************************************/

  bool readXml (PV & pv, std::istream & in) {
    if (! in) {
      return false;
    } else {
      PVXmlParser parser (Core::Application::us () -> getConfiguration (), pv);
      return parser.parseStream (in);
    }
  }

  bool readFile (PV & pv, const std::string & file) {
    PVXmlParser parser (Core::Application::us () -> getConfiguration (), pv);
    return parser.parseFile (file);
  }

  bool writeXml (const PV & pv, std::ostream & out) {
    if (! out) {
      return false;
    } else {
      Core::XmlWriter xout (out);
      xout.putDeclaration ("UTF8");
      xout << "\n"
	   << Core::XmlOpen ("parameters")
	   << "\n";
      writeXml (pv.templates_, xout);
      for (PV::DistanceVector::const_iterator d = pv.distances_.begin (); d != pv.distances_.end (); ++ d) {
	xout << Core::XmlOpen ("distance")
	     << Core::XmlFull ("comparison", ComparisonChoice [d -> first])
	     << d -> second
	     << Core::XmlClose ("distance")
	     << "\n";
      }
      for (PV::const_iterator p = pv.begin (); p != pv.end (); ++ p) {
	xout << Core::XmlOpen ("parameter")
	     << Core::XmlFull ("feature", pv.templates_.uncompress (p -> first))
	     << Core::XmlFull ("weight", double (p -> second))
	     << Core::XmlClose ("parameter")
	     << "\n";
      }
      xout << Core::XmlClose ("parameters")
	   << "\n";
    }
  }

  bool writeXml (const TemplateList & tl, Core::XmlWriter & xout) {
    for (TemplateList::TypeMap::const_iterator it = tl.types_.begin ();
	 it != tl.types_.end (); ++ it) {
      xout << Core::XmlOpen ("type-count")
	   << it -> first
	   << " "
	   << it -> second -> count ()
	   << Core::XmlClose ("type-count")
	   << "\n";
    }
    for (int i = 0; i < FeatureChoice.nChoices (); ++ i) {
      if (tl.feature_types_ [i]) {
	xout << Core::XmlOpen ("feature-type")
	     << FeatureChoice [FeatureName (i)]
	     << " "
	     << tl.feature_types_ [i] -> name ()
	     << Core::XmlClose ("feature-type")
	     << "\n";
      }
    }
    for (TemplateList::const_iterator it = tl.map_.begin ();
	 it != tl.map_.end (); ++ it) {
      xout << Core::XmlOpen ("template");
      bool first = true;
      for (int i = 0; i < FeatureChoice.nChoices (); ++ i) {
	if (it -> first [i]) {
	  if (first) { first = false; } else { xout << " "; }
	  xout << FeatureChoice [FeatureName (i)];
	}
      }
      xout << Core::XmlClose ("template")
	   << "\n";
    }
    return true;
  }

  /**********************************************************************
   * PVXmlParser public methods
   **********************************************************************/

  PVXmlParser::PVXmlParser (const Core::Configuration & c, PV & pv) :
    Core::XmlSchemaParser (c),
//     xml_parameters_ ("parameters", this, 0, 0, 0),
//     xml_type_count_ ("type-count", this,
// 		     startHandler (& Self::startContent),
// 		     endHandler (& Self::endTypeCount),
// 		     charactersHandler (& Self::content)),
//     xml_feature_type_ ("feature-type", this,
// 		       startHandler (& Self::startContent),
// 		       endHandler (& Self::endFeatureType),
// 		       charactersHandler (& Self::content)),
//     xml_distance_ ("distance", this,
// 		   startHandler (& Self::startDistance),
// 		   endHandler (& Self::endDistance),
// 		   charactersHandler (& Self::distance)),
//     xml_comparison_ ("comparison", this,
// 		     startHandler (& Self::startContent),
// 		     endHandler (& Self::endComparison),
// 		     charactersHandler (& Self::content)),
//     xml_template_ ("template", this,
// 		   startHandler (& Self::startContent),
// 		   endHandler (& Self::endTemplate),
// 		   charactersHandler (& Self::content)),
//     xml_parameter_ ("parameter", this,
// 		    startHandler (& Self::startParameter),
// 		    endHandler (& Self::endParameter),
// 		    0),
//     xml_feature_ ("feature", this,
// 		  startHandler (& Self::startContent),
// 		  endHandler (& Self::endFeature),
// 		  charactersHandler (& Self::content)),
//     xml_weight_ ("weight", this,
// 		 startHandler (& Self::startContent),
// 		 endHandler (& Self::endWeight),
// 		 charactersHandler (& Self::content)),
    pv_ (pv),
    content_ (""),
    type_ (""),
    count_ (0),
    comp_ (ComparisonEquals),
    dist_ (0),
    feature_ (""),
    weight_ (0.0)
  {
//     xml_parameter_.addChild (& xml_feature_);
//     xml_parameter_.addChild (& xml_weight_);

//     xml_distance_.addChild (& xml_comparison_);

//     xml_parameters_.addChild (& xml_type_count_);
//     xml_parameters_.addChild (& xml_feature_type_);
//     xml_parameters_.addChild (& xml_distance_);
//     xml_parameters_.addChild (& xml_template_);
//     xml_parameters_.addChild (& xml_parameter_);

//     setRoot (& xml_parameters_);
    
    setRoot (new Core::XmlMixedElementRelay
 	     ("parameters", this, 0, 0, 0,
 	      XML_CHILD(new Core::XmlMixedElementRelay
 			("type-count", this,
 			 startHandler (& Self::startContent),
 			 endHandler (& Self::endTypeCount),
 			 charactersHandler (& Self::content))),
 	      XML_CHILD(new Core::XmlMixedElementRelay
 			("feature-type", this,
 			 startHandler (& Self::startContent),
 			 endHandler (& Self::endFeatureType),
 			 charactersHandler (& Self::content))),
 	      XML_CHILD(new Core::XmlMixedElementRelay
 			("distance", this,
 			 startHandler (& Self::startDistance),
 			 endHandler (& Self::endDistance),
 			 charactersHandler (& Self::distance),
 			 XML_CHILD(new Core::XmlMixedElementRelay
 				   ("comparison", this,
 				    startHandler (& Self::startContent),
 				    endHandler (& Self::endComparison),
 				    charactersHandler (& Self::content))),
 			 XML_NO_MORE_CHILDREN)),
 	      XML_CHILD(new Core::XmlMixedElementRelay
 			("template", this,
 			 startHandler (& Self::startContent),
 			 endHandler (& Self::endTemplate),
 			 charactersHandler (& Self::content))),
 	      XML_CHILD(new Core::XmlMixedElementRelay
 			("parameter", this,
 			 startHandler (& Self::startParameter),
 			 endHandler (& Self::endParameter),
 			 0,
 			 XML_CHILD(new Core::XmlMixedElementRelay
 				   ("feature", this,
 				    startHandler (& Self::startContent),
 				    endHandler (& Self::endFeature),
 				    charactersHandler (& Self::content))),
 			 XML_CHILD(new Core::XmlMixedElementRelay
 				   ("weight", this,
 				    startHandler (& Self::startContent),
 				    endHandler (& Self::endWeight),
 				    charactersHandler (& Self::content))),
 			 XML_NO_MORE_CHILDREN)),
 	      XML_NO_MORE_CHILDREN));
  }

  PVXmlParser::~PVXmlParser () {
    delete root ();
  }

  bool PVXmlParser::parseString (const std::string & str) {
    return Core::XmlSchemaParser::parseString (str.c_str ()) == 0;
  }

  bool PVXmlParser::parseStream (std::istream & in) {
    return Core::XmlSchemaParser::parseStream (in) == 0;
  }

  bool PVXmlParser::parseFile (const std::string & filename) {
    return Core::XmlSchemaParser::parseFile (filename.c_str ()) == 0;
  }

  /**********************************************************************
   * PVXmlParser private methods
   **********************************************************************/

  void PVXmlParser::startContent (const Core::XmlAttributes atts) {
    content_.resize (0);
  }
  void PVXmlParser::content (const char * ch, int len) {
    content_.append (ch, len);
  }

  void PVXmlParser::endTypeCount () {
    std::istringstream in (content_);
    in >> type_ >> count_;
    pv_.addType (type_, count_);
  }
  void PVXmlParser::endFeatureType () {
    std::istringstream in (content_);
    in >> feature_ >> type_;
    pv_.addFeatureType (feature_, type_);
  }

  void PVXmlParser::startDistance (const Core::XmlAttributes atts) {
    comp_ = ComparisonEquals;
    dist_ = 0;
  }
  void PVXmlParser::distance (const char * ch, int len) {
    char * end;
    dist_ = strtoll (ch, & end, 0);
  }
  void PVXmlParser::endDistance () {
    pv_.addDistance (comp_, dist_);
  }

  void PVXmlParser::endComparison () {
    int comp = ComparisonChoice [content_];
    content_.resize (0);
    if (comp_ == Core::Choice::Illegal) {
      parser () -> error ("invalid comparison \"%s\"", content_.c_str ());
    } else {
      comp_ = ComparisonOperator (comp);
    }
  }

  void PVXmlParser::endTemplate () {
    Core::stripWhitespace (content_);
    pv_.addTemplate (content_);
  }

  void PVXmlParser::startParameter (const Core::XmlAttributes atts) {
    feature_.resize (0);
    weight_ = 0.0;
  }
  void PVXmlParser::endParameter () {
    pv_.getParameter (feature_) = weight_;
  }

  void PVXmlParser::endFeature () {
    Core::stripWhitespace (content_);
    feature_ = content_;
    if (feature_.empty ()) {
      parser () -> error ("feature contains nothing");
    }
  }

  void PVXmlParser::endWeight () {
    Core::stripWhitespace (content_);
    if (! content_.empty ()) {
      char * end;
      weight_ = strtod (content_.c_str (), & end);
      if (* end != '\0') {
	parser () -> error ("invalid weight \"%s\"", content_.c_str ());
      }
    } else {
      parser () -> error ("weight contains nothing");
    }
  }

  /**********************************************************************
   * AggregatePVXmlParser
   **********************************************************************/

  class AggregatePVXmlParser : public PVXmlParser {
  protected:
    virtual void endDistance ();
    virtual void endTemplate ();
    virtual void endParameter ();
  public:
    AggregatePVXmlParser (const Core::Configuration &, PV &);
  };

  AggregatePVXmlParser::AggregatePVXmlParser (const Core::Configuration & c, PV & pv) :
    PVXmlParser (c, pv)
  {}

  void AggregatePVXmlParser::endDistance () {}
  void AggregatePVXmlParser::endTemplate () {}
  void AggregatePVXmlParser::endParameter () {
    pv_.getParameter (feature_) += weight_;
  }

  bool aggregateFile (PV & pv, const std::string & file) {
    AggregatePVXmlParser parser (Core::Application::us () -> getConfiguration (), pv);
    return parser.parseFile (file);
  }
  
  /**********************************************************************
   * W public methods
   **********************************************************************/

  W::W (double w) :
    Core::ReferenceCounted (),
    w_ (w)
  {}

  W::~W () {}

  W & W::operator = (double w) {
    w_ = w;
    return * this;
  }

  W & W::operator += (double w) {
    w_ += w;
    return * this;
  }

  W & W::operator /= (double w) {
    w_ /= w;
    return * this;
  }

  W::operator double () const {
    return w_;
  }

  /**********************************************************************
   * WRef public methods
   **********************************************************************/

  WRef::WRef (double w) :
    Parent (new W (w))
  {}

  const WRef & WRef::operator = (double w) const {
    Parent::operator * () = w;
    return * this;
  }

  const WRef & WRef::operator += (double w) const {
    Parent::operator * () += w;
    return * this;
  }

  const WRef & WRef::operator /= (double w) const {
    Parent::operator * () /= w;
    return * this;
  }

  WRef::operator double () const {
    return Parent::operator * ();
  }

  /**********************************************************************
   * Sum public methods
   **********************************************************************/

  Sum::Sum () :
    v_ (),
    sum_ (0.0)
  {}

  Sum::Sum (const Sum & s) :
    v_ (s.v_),
    sum_ (s.sum_)
  {}

  Sum & Sum::operator += (const WRef & w) {
    v_.push_back (w);
    sum_ += (* w);
  }

  Sum::operator double () const {
    return sum_;
  }

  // @bug Why use the W operator += instead of the WRef operator +=?
  void Sum::add (double update) {
    for (std::vector <WRef>::iterator w = v_.begin (); w != v_.end (); ++ w) {
      (* * w) += update;
      sum_ += update;
    }
  }

  /**********************************************************************
   * Sum private methods
   **********************************************************************/

  Sum & Sum::operator = (const Sum & s) {
    v_ = s.v_;
    sum_ = s.sum_;
    return * this;
  }

  /**********************************************************************
   * SumBeforeCost public methods
   **********************************************************************/

  SumBeforeCost::SumBeforeCost (size_t n, const std::string & name) :
    BeforeCostInterface (n),
    matrix_ (n * n),
    name_ (name)
  {}

  SumBeforeCost::~ SumBeforeCost () {
    
  }
  
  double SumBeforeCost::cost (int i, int j) const {
    return double (matrix_ [index (i, j)]);
  }

  const std::string & SumBeforeCost::name () const {
    return name_;
  }
  
  Sum & SumBeforeCost::operator () (int i, int j) {
    return matrix_ [index (i, j)];
  }
  
}
