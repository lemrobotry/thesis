#include "FeatureMapTest.hh"

CPPUNIT_TEST_SUITE_REGISTRATION( FeatureMapTest );

void FeatureMapTest::setUp () {
  map_ = new Permute::FeatureMap ();
  pos_ = new Permute::FeatureType ("pos", 55);
  dist_ = new Permute::FeatureType ("dist", 7);
  word_ = new Permute::FeatureType ("word", 100000);
  types_.clear ();
  types_.push_back (pos_);
  types_.push_back (pos_);
  types_.push_back (word_);
  types_.push_back (pos_);
  types_.push_back (pos_);
  types_.push_back (pos_);
  types_.push_back (pos_);
  types_.push_back (word_);
  types_.push_back (pos_);
  types_.push_back (dist_);
}

void FeatureMapTest::tearDown () {
  delete map_;
  delete pos_;
  delete dist_;
  delete word_;
}

void FeatureMapTest::testSetAll () {
  map_ -> setAll ("l-pos=ADJ r-pos=NN l-pos-1=DT l-pos+1=NN "
		  "r-pos-1=ADJ r-pos+1=VBZ dist=1");
  
  // Binary: 001101011011
//   CPPUNIT_ASSERT_EQUAL( 0x35B, map_ -> mask () );
  
  CPPUNIT_ASSERT_EQUAL( std::string ("ADJ"), map_ -> getValue ("l-pos") );
  CPPUNIT_ASSERT_EQUAL( std::string ("NN"), map_ -> getValue ("r-pos") );
  CPPUNIT_ASSERT_EQUAL( std::string ("DT"), map_ -> getValue ("l-pos-1") );
  CPPUNIT_ASSERT_EQUAL( std::string ("NN"), map_ -> getValue ("l-pos+1") );
  CPPUNIT_ASSERT_EQUAL( std::string ("ADJ"), map_ -> getValue ("r-pos-1") );
  CPPUNIT_ASSERT_EQUAL( std::string ("VBZ"), map_ -> getValue ("r-pos+1") );
  CPPUNIT_ASSERT_EQUAL( std::string ("1"), map_ -> getValue ("dist") );
  CPPUNIT_ASSERT_EQUAL( std::string (""), map_ -> getValue ("l-word") );
  CPPUNIT_ASSERT_EQUAL( std::string (""), map_ -> getValue ("r-word") );
  CPPUNIT_ASSERT_EQUAL( std::string (""), map_ -> getValue ("b-pos") );
  
  CPPUNIT_ASSERT_EQUAL( std::string ("ADJ"), map_ -> getValue (Permute::lPOS) );
  CPPUNIT_ASSERT_EQUAL( std::string ("NN"), map_ -> getValue (Permute::rPOS) );
  CPPUNIT_ASSERT_EQUAL( std::string ("DT"), map_ -> getValue (Permute::lPOSm1) );
  CPPUNIT_ASSERT_EQUAL( std::string ("NN"), map_ -> getValue (Permute::lPOSp1) );
  CPPUNIT_ASSERT_EQUAL( std::string ("ADJ"), map_ -> getValue (Permute::rPOSm1) );
  CPPUNIT_ASSERT_EQUAL( std::string ("VBZ"), map_ -> getValue (Permute::rPOSp1) );
  CPPUNIT_ASSERT_EQUAL( std::string ("1"), map_ -> getValue (Permute::Dist) );
  CPPUNIT_ASSERT_EQUAL( std::string (""), map_ -> getValue (Permute::lWord) );
  CPPUNIT_ASSERT_EQUAL( std::string (""), map_ -> getValue (Permute::rWord) );
  CPPUNIT_ASSERT_EQUAL( std::string (""), map_ -> getValue (Permute::bPOS) );
  
  std::string id (1, char (1));
  std::string c = map_ -> compress (id, types_);
  CPPUNIT_ASSERT_EQUAL( size_t (8), c.length () );
  CPPUNIT_ASSERT_EQUAL( id, c.substr(0, 1) );
  CPPUNIT_ASSERT_EQUAL( pos_ -> intern ("DT"), c.substr (1, 1) );
  CPPUNIT_ASSERT_EQUAL( pos_ -> intern ("ADJ"), c.substr (2, 1) );
  CPPUNIT_ASSERT_EQUAL( pos_ -> intern ("NN"), c.substr (3, 1) );
  CPPUNIT_ASSERT_EQUAL( pos_ -> intern ("ADJ"), c.substr (4, 1) );
  CPPUNIT_ASSERT_EQUAL( pos_ -> intern ("NN"), c.substr (5, 1) );
  CPPUNIT_ASSERT_EQUAL( pos_ -> intern ("VBZ"), c.substr (6, 1) );
  CPPUNIT_ASSERT_EQUAL( dist_ -> intern ("1"), c.substr (7, 1) );
}
