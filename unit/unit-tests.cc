#include <cppunit/CompilerOutputter.h>
#include <cppunit/extensions/TestFactoryRegistry.h>
#include <cppunit/ui/text/TestRunner.h>

int main (int argc, char ** argv) {
  CppUnit::TextTestRunner runner;
  runner.addTest (CppUnit::TestFactoryRegistry::getRegistry ().makeTest ());
  runner.setOutputter (new CppUnit::CompilerOutputter (& runner.result (), std::cerr));
  return ! runner.run ();
}
