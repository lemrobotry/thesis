#ifndef _PERMUTE_BEFORE_SCORER_TEST_HH
#define _PERMUTE_BEFORE_SCORER_TEST_HH

#include <cppunit/extensions/HelperMacros.h>

#include <BeforeScorer.hh>

class BeforeScorerTest : public CppUnit::TestFixture {
  CPPUNIT_TEST_SUITE( BeforeScorerTest );
  CPPUNIT_TEST( testBinomial );
  CPPUNIT_TEST( testIndex );
  CPPUNIT_TEST( testLeftAnchorRecurrence );
  CPPUNIT_TEST_SUITE_END();
private:
  Permute::Permutation pi;
  Permute::BeforeScorer * scorer;
public:
  void setUp ();
  void tearDown ();

  void testBinomial ();
  void testIndex ();
  void testLeftAnchorRecurrence ();
};

#endif//_PERMUTE_BEFORE_SCORER_TEST_HH
