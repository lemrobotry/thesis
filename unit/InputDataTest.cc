#include "InputDataTest.hh"
#include <sstream>
#include <Join.hh>
#include <Core/CompressedStream.hh>

CPPUNIT_TEST_SUITE_REGISTRATION( InputDataTest );

using namespace Permute;

namespace {
  std::string ToString(const std::vector<int>& v) {
    std::ostringstream out;
    out << util::join(v, " ");
    return out.str();
  }
}

void InputDataTest::testOneInput() {
  std::string expected_source("a simple sentence . ");
  std::string expected_pos("DET ADJ NN $. ");
  std::string expected_target("sentence a . simple ");

  Core::CompressedInputStream in("data/one-input.txt");
  InputData data(false);
  CPPUNIT_ASSERT(in >> data);
  CPPUNIT_ASSERT_EQUAL(expected_source, data.source().toString());
  CPPUNIT_ASSERT_EQUAL(expected_pos, data.pos().toString());
  CPPUNIT_ASSERT_EQUAL(expected_target, data.target().toString());

  CPPUNIT_ASSERT(!(in >> data));
}

void InputDataTest::testTwoInputs() {
  Core::CompressedInputStream in("data/two-inputs.txt");
  InputData data(false);
  CPPUNIT_ASSERT(in >> data);
  CPPUNIT_ASSERT(in >> data);
  CPPUNIT_ASSERT(!(in >> data));
}

void InputDataTest::testDependency() {
  std::string expected_source("a simple sentence . ");
  std::string expected_pos("DET ADJ NN $. ");
  std::string expected_target("sentence a . simple ");
  std::string expected_parents("1 2 3 -1 ");
  std::string expected_labels("FAKE DEP LABELS ROOT ");
  
  Core::CompressedInputStream in ("data/dependency.txt");
  InputData data(true);
  CPPUNIT_ASSERT(in >> data);
  CPPUNIT_ASSERT_EQUAL(expected_parents, ToString(data.parents()));
  CPPUNIT_ASSERT_EQUAL(expected_labels, data.labels().toString());
  CPPUNIT_ASSERT_EQUAL(expected_source, data.source().toString());
  CPPUNIT_ASSERT_EQUAL(expected_pos, data.pos().toString());
  CPPUNIT_ASSERT_EQUAL(expected_target, data.target().toString());

  CPPUNIT_ASSERT(!(in >> data));
}
