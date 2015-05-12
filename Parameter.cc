#include <Core/Application.hh>
#include <Core/StringUtilities.hh>
#include <Fsa/AlphabetXml.hh>
#include "Parameter.hh"

namespace Permute {

  Feature::Feature () :
    i_ (Fsa::Any),
    j_ (Fsa::Any),
    im1_ (Fsa::Any),
    ip1_ (Fsa::Any),
    jm1_ (Fsa::Any),
    jp1_ (Fsa::Any),
    b_ (Fsa::Any),
    distance_ (0),
    op_ (CompGreaterThan)
  {}

  bool Feature::fires (const Permutation & pos, size_t i, size_t j) const {
    if (i_ != Fsa::Any && pos.label (i) != i_) return false;
    if (j_ != Fsa::Any && pos.label (j) != j_) return false;
    if (im1_ != Fsa::Any) {
      if (i > 0 && pos.label (i - 1) != im1_) return false;
      else if (i == 0 && im1_ != Fsa::InvalidLabelId) return false;
    }
    if (ip1_ != Fsa::Any && pos.label (i + 1) != ip1_) return false;
    if (jm1_ != Fsa::Any && pos.label (j - 1) != jm1_) return false;
    if (jp1_ != Fsa::Any) {
      if (j + 1 < pos.size () && pos.label (j + 1) != jp1_) return false;
      else if (j + 1 == pos.size () && jp1_ != Fsa::InvalidLabelId) return false;
    }
    if (b_ != Fsa::Any) {
      bool found = false;
      for (size_t b = i + 1; b < j; ++ b) {
	if (pos.label (b) == b_) {
	  found = true;
	  break;
	}
      }
      if (! found) return false;
    }
    switch (op_) {
    case CompEquals:
      return (j - i) == distance_;
    case CompLessThan:
      return (j - i) < distance_;
    case CompGreaterThan:
      return (j - i) > distance_;
    }
  }

  void Feature::setFirstPOS (POS i) {
    i_ = i;
  }
  void Feature::setFirstPOSm1 (POS im1) {
    im1_ = im1;
  }
  void Feature::setFirstPOSp1 (POS ip1) {
    ip1_ = ip1;
  }
  void Feature::setSecondPOS (POS j) {
    j_ = j;
  }
  void Feature::setSecondPOSm1 (POS jm1) {
    jm1_ = jm1;
  }
  void Feature::setSecondPOSp1 (POS jp1) {
    jp1_ = jp1;
  }
  void Feature::setBetweenPOS (POS b) {
    b_ = b;
  }
  void Feature::setPOS (const Feature & phi) {
    POS * me = (POS *) this,
      * phi_pos = (POS *) (& phi);
    for (int i = 0; i < 7; ++ i) {
      me [i] = phi_pos [i];
    }
  }
  void Feature::setDistance (int d) {
    distance_ = d;
  }
  void Feature::setComparison (Comparison op) {
    op_ = op;
  }

  void Feature::writeXml (Fsa::ConstAlphabetRef pos, Core::XmlWriter & xo) const {
    std::string op = "UNKNOWN";
    switch (op_) {
    case CompEquals:
      op = "EQUALS"; break;
    case CompLessThan:
      op = "LESS"; break;
    case CompGreaterThan:
      op = "GREATER"; break;
    }
    if (i_ != Fsa::Any) xo << Core::XmlFull ("first", pos -> symbol (i_));
    if (j_ != Fsa::Any) xo << Core::XmlFull ("second", pos -> symbol (j_));
    if (im1_ != Fsa::Any) xo << Core::XmlFull ("firstm1", pos -> symbol (im1_));
    if (ip1_ != Fsa::Any) xo << Core::XmlFull ("firstp1", pos -> symbol (ip1_));
    if (jm1_ != Fsa::Any) xo << Core::XmlFull ("secondm1", pos -> symbol (jm1_));
    if (jp1_ != Fsa::Any) xo << Core::XmlFull ("secondp1", pos -> symbol (jp1_));
    if (b_ != Fsa::Any) xo << Core::XmlFull ("between", pos -> symbol (b_));
    xo << Core::XmlFull ("distance", distance_)
       << Core::XmlFull ("comparison", op)
       << "\n";
  }

  Feature Feature::lower_bound (POS i, POS j) {
    Feature f;
    f.setFirstPOS (i);
    f.setSecondPOS (j);
    f.setFirstPOSm1 (Core::Type <POS>::min);
    f.setFirstPOSp1 (Core::Type <POS>::min);
    f.setSecondPOSm1 (Core::Type <POS>::min);
    f.setSecondPOSp1 (Core::Type <POS>::min);
    f.setBetweenPOS (Core::Type <POS>::min);
    f.setDistance (Core::Type <int>::min);
    f.setComparison (CompEquals);
    return f;
  }

  Feature Feature::upper_bound (POS i, POS j) {
    Feature f;
    f.setFirstPOS (i);
    f.setSecondPOS (j);
    f.setFirstPOSm1 (Core::Type <POS>::max);
    f.setFirstPOSp1 (Core::Type <POS>::max);
    f.setSecondPOSm1 (Core::Type <POS>::max);
    f.setSecondPOSp1 (Core::Type <POS>::max);
    f.setBetweenPOS (Core::Type <POS>::max);
    f.setDistance (Core::Type <int>::max);
    f.setComparison (CompGreaterThan);
    return f;
  }

  /**********************************************************************/
  
  Fsa::ConstAlphabetRef ParameterVector::getPOS () const {
    return POS_;
  }

  void ParameterVector::setPOS (Fsa::ConstAlphabetRef pos) {
    POS_ = pos;
  }

  ParameterVector::iterator ParameterVector::begin () {
    return parameter_map::begin ();
  }
  ParameterVector::iterator ParameterVector::end () {
    return parameter_map::end ();
  }

  ParameterVector::const_iterator ParameterVector::begin () const {
    return parameter_map::begin ();
  }
  ParameterVector::const_iterator ParameterVector::end () const {
    return parameter_map::end ();
  }

  ParameterVector::parameter_iterator ParameterVector::begin_p () {
    return parameter_map::begin ();
  }
  ParameterVector::parameter_iterator ParameterVector::end_p () {
    return parameter_map::end ();
  }

  ParameterVector::parameter_iterator ParameterVector::begin_p (POS i, POS j) {
    Feature phi = Feature::lower_bound (i, j);
    return lower_bound (phi);
  }
  ParameterVector::parameter_iterator ParameterVector::end_p (POS i, POS j) {
    Feature phi = Feature::upper_bound (i, j);
    return upper_bound (phi);
  }

  ParameterVector::const_parameter_iterator ParameterVector::begin_p () const {
    return parameter_map::begin ();
  }
  ParameterVector::const_parameter_iterator ParameterVector::end_p () const {
    return parameter_map::end ();
  }

  ParameterVector::const_parameter_iterator ParameterVector::begin_p (POS i, POS j) const {
    Feature phi = Feature::lower_bound (i, j);
    return lower_bound (phi);
  }
  ParameterVector::const_parameter_iterator ParameterVector::end_p (POS i, POS j) const {
    Feature phi = Feature::upper_bound (i, j);
    return upper_bound (phi);
  }
  
  /**********************************************************************/

  int count (const Parameter & theta, const Permutation & pos, size_t i, size_t j) {
    return theta.first.fires (pos, i, j);
  }

  double weight (const Parameter & theta, const Permutation & pos, size_t i, size_t j) {
    return theta.first.fires (pos, i, j) ? theta.second : 0.0;
  }

  /**********************************************************************/
  
  double sum (const ParameterVector & theta, const Permutation & pos, size_t i, size_t j) {
    double w = 0.0;
    POS pi = pos.label (i), pj = pos.label (j);
    ParameterVector::const_parameter_iterator th;
    for (th = theta.begin_p (pi, pj); th != theta.end_p (pi, pj); ++ th) {
      w += weight (* th, pos, i, j);
    }
    for (th = theta.begin_p (Fsa::Any, pj); th != theta.end_p (Fsa::Any, pj); ++ th) {
      w += weight (* th, pos, i, j);
    }
    for (th = theta.begin_p (pi, Fsa::Any); th != theta.end_p (pi, Fsa::Any); ++ th) {
      w += weight (* th, pos, i, j);
    }
    for (th = theta.begin_p (Fsa::Any, Fsa::Any); th != theta.end_p (Fsa::Any, Fsa::Any); ++ th) {
      w += weight (* th, pos, i, j);
    }
    return w;
  }

  double count (ParameterVector & theta, const Permutation & pos, size_t i, size_t j, float c) {
    POS pi = pos.label (i), pj = pos.label (j);
    ParameterVector::parameter_iterator th;
    for (th = theta.begin_p (pi, pj); th != theta.end_p (pi, pj); ++ th) {
      th -> second += c * count (* th, pos, i, j);
    }
    for (th = theta.begin_p (Fsa::Any, pj); th != theta.end_p (Fsa::Any, pj); ++ th) {
      th -> second += c * count (* th, pos, i, j);
    }
    for (th = theta.begin_p (pi, Fsa::Any); th != theta.end_p (pi, Fsa::Any); ++ th) {
      th -> second += c * count (* th, pos, i, j);
    }
    for (th = theta.begin_p (Fsa::Any, Fsa::Any); th != theta.end_p (Fsa::Any, Fsa::Any); ++ th) {
      th -> second += c * count (* th, pos, i, j);
    }
  }

  /**********************************************************************/

  FeatureCounter::FeatureCounter (ParameterVector & pv, const Permutation & pos) :
    pv_ (pv),
    pos_ (pos)
  {}

  void FeatureCounter::count (const Permutation & p, double c) {
    for (Permutation::const_iterator p_i = p.begin (); p_i != -- p.end (); ++ p_i) {
      size_t i = * p_i;
      for (Permutation::const_iterator p_j = p_i + 1; p_j != p.end (); ++ p_j) {
	size_t j = * p_j;
	if (i < j) {
	  Permute::count (pv_, pos_, i, j, c);
	}
      }
    }
  }

  /**********************************************************************/

  bool readXml (ParameterVector & pv, std::istream & in) {
    if (! in) {
      return false;
    } else {
      pv.clear ();
      ParameterVectorXmlParser parser (Core::Application::us () -> getConfiguration (), pv);
      return parser.parseStream (in);
    }
  }

  bool writeXml (const ParameterVector & pv, std::ostream & out) {
    if (! out) {
      return false;
    } else {
      Core::XmlWriter xout (out);
      xout.putDeclaration ("UTF8");
      xout << "\n";
      xout << Core::XmlOpen ("parameters") << "\n";

      Fsa::ConstAlphabetRef pos = pv.getPOS ();
      xout << Core::XmlOpen ("pos-alphabet") << "\n";
      pos -> writeXml (xout);
      xout << Core::XmlClose ("pos-alphabet") << "\n";

      for (ParameterVector::const_parameter_iterator p = pv.begin_p (); p != pv.end_p (); ++ p) {
	xout << Core::XmlOpen ("parameter") << "\n";
	xout << Core::XmlOpen ("feature") << "\n";
	p -> first.writeXml (pos, xout);
	xout << Core::XmlClose ("feature") << "\n";
	xout << Core::XmlFull ("weight", p -> second) << "\n";
	xout << Core::XmlClose ("parameter") << "\n";
      }
      xout << Core::XmlClose ("parameters") << "\n";
    }
  }

  /**********************************************************************/

  void ParameterVectorXmlParser::startParameter (const Core::XmlAttributes atts) {
    weight_ = 0.0;
  }

  void ParameterVectorXmlParser::endParameter () {
    pv_.insert (std::make_pair (phi_, weight_));
  }

  void ParameterVectorXmlParser::startFeature (const Core::XmlAttributes atts) {
    new (& phi_) Feature;
  }

  void ParameterVectorXmlParser::startContent (const Core::XmlAttributes atts) {
    content_.resize (0);
  }

  void ParameterVectorXmlParser::content (const char * ch, int len) {
    content_.append (ch, len);
  }

  void ParameterVectorXmlParser::endFirstPOS () {
    Core::stripWhitespace (content_);
    if (! content_.empty ()) {
      phi_.setFirstPOS (pv_.getPOS () -> index (content_));
    } else {
      parser () -> error ("first POS contains no content");
    }
  }

  void ParameterVectorXmlParser::endFirstPOSm1 () {
    Core::stripWhitespace (content_);
    if (! content_.empty ()) {
      phi_.setFirstPOSm1 (pv_.getPOS () -> index (content_));
    } else {
      phi_.setFirstPOSm1 (Fsa::InvalidLabelId);
    }
  }

  void ParameterVectorXmlParser::endFirstPOSp1 () {
    Core::stripWhitespace (content_);
    if (! content_.empty ()) {
      phi_.setFirstPOSp1 (pv_.getPOS () -> index (content_));
    } else {
      parser () -> error ("first POS + 1 contains no content");
    }
  }

  void ParameterVectorXmlParser::endSecondPOS () {
    Core::stripWhitespace (content_);
    if (! content_.empty ()) {
      phi_.setSecondPOS (pv_.getPOS () -> index (content_));
    } else {
      parser () -> error ("second POS contains no content");
    }
  }

  void ParameterVectorXmlParser::endSecondPOSm1 () {
    Core::stripWhitespace (content_);
    if (! content_.empty ()) {
      phi_.setSecondPOSm1 (pv_.getPOS () -> index (content_));
    } else {
      parser () -> error ("second POS - 1 contains no content");
    }
  }

  void ParameterVectorXmlParser::endSecondPOSp1 () {
    Core::stripWhitespace (content_);
    if (! content_.empty ()) {
      phi_.setSecondPOSp1 (pv_.getPOS () -> index (content_));
    } else {
      phi_.setSecondPOSp1 (Fsa::InvalidLabelId);
    }
  }

  void ParameterVectorXmlParser::endBetweenPOS () {
    Core::stripWhitespace (content_);
    if (! content_.empty ()) {
      phi_.setBetweenPOS (pv_.getPOS () -> index (content_));
    } else {
      parser () -> error ("between POS contains no content");
    }
  }

  void ParameterVectorXmlParser::endDistance () {
    Core::stripWhitespace (content_);
    if (! content_.empty ()) {
      char * end;
      int d = strtoll (content_.c_str (), & end, 0);
      if (* end != '\0') {
	parser () -> error ("invalid distance \"%s\"", content_.c_str ());
      } else {
	phi_.setDistance (d);
      }
    } else {
      parser () -> error ("distance contains no content");
    }
  }

  void ParameterVectorXmlParser::endComparison () {
    Core::stripWhitespace (content_);
    if (content_ == "EQUALS") {
      phi_.setComparison (CompEquals);
    } else if (content_ == "LESS") {
      phi_.setComparison (CompLessThan);
    } else if (content_ == "GREATER") {
      phi_.setComparison (CompGreaterThan);
    } else {
      parser () -> error ("invalid comparison");
    }
  }

  void ParameterVectorXmlParser::endWeight () {
    Core::stripWhitespace (content_);
    if (! content_.empty ()) {
      char * end;
      double w = strtod (content_.c_str (), & end);
      if (* end != '\0') {
	parser () -> error ("invalid weight \"%s\"", content_.c_str ());
      } else {
	weight_ = w;
      }
    } else {
      parser () -> error ("weight contains no content");
    }
  }

  ParameterVectorXmlParser::ParameterVectorXmlParser (const Core::Configuration & c, ParameterVector & pv) :
    Core::XmlSchemaParser (c),
    pv_ (pv)
  {
    Core::Ref <Fsa::StaticAlphabet> pos (new Fsa::StaticAlphabet);
    pv_.setPOS (pos);
    setRoot (new Core::XmlMixedElementRelay
	     ("parameters", this, 0, 0, 0,
	      XML_CHILD(new Fsa::AlphabetXmlParser ("pos-alphabet", this, pos)),
	      XML_CHILD(new Core::XmlMixedElementRelay
			("parameter", this, startHandler (& Self::startParameter), endHandler (& Self::endParameter), 0,
			 XML_CHILD(new Core::XmlMixedElementRelay
				   ("feature", this, startHandler (& Self::startFeature), 0, 0,
				    XML_CHILD(new Core::XmlMixedElementRelay
					      ("first", this, startHandler (& Self::startContent),
					       endHandler (& Self::endFirstPOS), charactersHandler (& Self::content),
					       XML_NO_MORE_CHILDREN)),
				    XML_CHILD(new Core::XmlMixedElementRelay
					      ("second", this, startHandler (& Self::startContent),
					       endHandler (& Self::endSecondPOS), charactersHandler (& Self::content),
					       XML_NO_MORE_CHILDREN)),
				    XML_CHILD(new Core::XmlMixedElementRelay
					      ("firstm1", this, startHandler (& Self::startContent),
					       endHandler (& Self::endFirstPOSm1), charactersHandler (& Self::content),
					       XML_NO_MORE_CHILDREN)),
				    XML_CHILD(new Core::XmlMixedElementRelay
					      ("firstp1", this, startHandler (& Self::startContent),
					       endHandler (& Self::endFirstPOSp1), charactersHandler (& Self::content),
					       XML_NO_MORE_CHILDREN)),
				    XML_CHILD(new Core::XmlMixedElementRelay
					      ("secondm1", this, startHandler (& Self::startContent),
					       endHandler (& Self::endSecondPOSm1), charactersHandler (& Self::content),
					       XML_NO_MORE_CHILDREN)),
				    XML_CHILD(new Core::XmlMixedElementRelay
					      ("secondp1", this, startHandler (& Self::startContent),
					       endHandler (& Self::endSecondPOSp1), charactersHandler (& Self::content),
					       XML_NO_MORE_CHILDREN)),
				    XML_CHILD(new Core::XmlMixedElementRelay
					      ("between", this, startHandler (& Self::startContent),
					       endHandler (& Self::endBetweenPOS), charactersHandler (& Self::content),
					       XML_NO_MORE_CHILDREN)),
				    XML_CHILD(new Core::XmlMixedElementRelay
					      ("distance", this, startHandler (& Self::startContent),
					       endHandler (& Self::endDistance), charactersHandler (& Self::content),
					       XML_NO_MORE_CHILDREN)),
				    XML_CHILD(new Core::XmlMixedElementRelay
					      ("comparison", this, startHandler (& Self::startContent),
					       endHandler (& Self::endComparison), charactersHandler (& Self::content),
					       XML_NO_MORE_CHILDREN)),
				    XML_NO_MORE_CHILDREN)),
			 XML_CHILD(new Core::XmlMixedElementRelay
				   ("weight", this, startHandler (& Self::startContent),
				    endHandler (& Self::endWeight), charactersHandler (& Self::content),
				    XML_NO_MORE_CHILDREN)),
			 XML_NO_MORE_CHILDREN)),
	      XML_NO_MORE_CHILDREN));
  }

  ParameterVectorXmlParser::~ParameterVectorXmlParser () {
    delete root ();
  }

  bool ParameterVectorXmlParser::parseString (const std::string & str) {
    return Core::XmlSchemaParser::parseString (str.c_str ()) == 0;
  }

  bool ParameterVectorXmlParser::parseStream (std::istream & in) {
    return Core::XmlSchemaParser::parseStream (in) == 0;
  }

  bool ParameterVectorXmlParser::parseFile (const std::string & filename) {
    return Core::XmlSchemaParser::parseFile (filename.c_str ()) == 0;
  }

  /**********************************************************************/

  SparseParameter::SparseParameter (parameter_map::iterator parameter, double count) :
    parameter_ (parameter),
    count_ (count)
  {}
  double SparseParameter::weight () const {
    return parameter_ -> second;
  }
  void SparseParameter::increment (double sign) const {
    count_ += sign;
  }
  void SparseParameter::update (double lambda) const {
    parameter_ -> second += count_ * lambda;
  }
  bool SparseParameter::operator < (const SparseParameter & other) const {
    return ltf (parameter_ -> first, other.parameter_ -> first);
  }
  bool SparseParameter::operator == (const SparseParameter & other) const {
    return parameter_ == other.parameter_;
  }

  LessThanFeature SparseParameter::ltf;

  /**********************************************************************/

  SparseParameterVector::SparseParameterVector (double margin) :
    std::set <SparseParameter> (),
    margin_ (margin)
  {}
  void SparseParameterVector::setMargin (double margin) {
    margin_ = margin;
  }
  double SparseParameterVector::dot (const SparseParameterVector & other) const {
    double product = 0.0;
    const_iterator j = other.begin ();
    for (const_iterator i = begin (); i != end (); ++ i) {
      for (; j != other.end () && (* j) < (* i); ++ j);
      if ((* j) == (* i)) {
	product += (i -> count_) * (j -> count_);
      }
    }
    return product;
  }
  double SparseParameterVector::norm2 () const {
    double product = 0.0;
    for (const_iterator i = begin (); i != end (); ++ i) {
      product += pow (i -> count_, 2);
    }
    return product;
  }
  double SparseParameterVector::margin () const {
    double m = - margin_;
    for (const_iterator i = begin (); i != end (); ++ i) {
      m += (i -> count_) * (i -> weight ());
    }
    return m;
  }
  void SparseParameterVector::update (double lambda) {
    for (iterator i = begin (); i != end (); ++ i) {
      i -> update (lambda);
    }
  }
  // Calls build_helper for each pair of positions in the given permutation.
  void SparseParameterVector::build (ParameterVector & pv, const Permutation & p, double sign) {
    for (Permutation::const_iterator i = p.begin (); i != -- p.end (); ++ i) {
      for (Permutation::const_iterator j = i + 1; j != p.end (); ++ j) {
	if ((* i) < (* j)) {
	  build_helper (pv, p, * i, * j, sign);
	}
      }
    }
  }
  // Calls build_helper for a particular pair of indices, with the sign of the
  // update dependent on the boolean ordered parameter. 
  void SparseParameterVector::build (ParameterVector & pv, const Permutation & p, size_t l, size_t r, bool ordered) {
    if (ordered) {
      build_helper (pv, p, l, r, 1.0);
    } else {
      build_helper (pv, p, r, l, -1.0);
    }
  }
  // Iterates over all parameters in the given ParameterVector that match one of
  // (POS(l), POS(r)), (Any, POS(r)), (POS(l), Any), or (Any, Any), and for each
  // that fires on the given pair in the given permutation, either inserts or
  // increments the corresponding SparseParameter.
  void SparseParameterVector::build_helper (ParameterVector & pv, const Permutation & pos, size_t l, size_t r, double sign) {
    POS pl = pos.label (l), pr = pos.label (r);
    ParameterVector::parameter_iterator p;
    for (p = pv.begin_p (pl, pr); p != pv.end_p (pl, pr); ++ p) {
      if (p -> first.fires (pos, l, r)) {
	std::pair <iterator, bool> pair = insert (SparseParameter (p, sign));
	if (! pair.second) {
	  pair.first -> increment (sign);
	}
      }
    }
    for (p = pv.begin_p (Fsa::Any, pr); p != pv.end_p (Fsa::Any, pr); ++ p) {
      if (p -> first.fires (pos, l, r)) {
	std::pair <iterator, bool> pair = insert (SparseParameter (p, sign));
	if (! pair.second) {
	  pair.first -> increment (sign);
	}
      }
    }
    for (p = pv.begin_p (pl, Fsa::Any); p != pv.end_p (pl, Fsa::Any); ++ p) {
      if (p -> first.fires (pos, l, r)) {
	std::pair <iterator, bool> pair = insert (SparseParameter (p, sign));
	if (! pair.second) {
	  pair.first -> increment (sign);
	}
      }
    }
    for (p = pv.begin_p (Fsa::Any, Fsa::Any); p != pv.end_p (Fsa::Any, Fsa::Any); ++ p) {
      if (p -> first.fires (pos, l, r)) {
	std::pair <iterator, bool> pair = insert (SparseParameter (p, sign));
	if (! pair.second) {
	  pair.first -> increment (sign);
	}
      }
    }
  }
}
