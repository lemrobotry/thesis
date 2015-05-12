#include "PrintTimer.hh"

namespace Permute {

  PrintTimer::PrintTimer (std::ostream & out) :
    out_ (out),
    times_ ()
  {}

  void PrintTimer::start (const std::string & s) {
    out_ << s << std::endl;
    Core::Timer & timer = times_ [s];
    timer.start ();
  }

  Core::Timer PrintTimer::stop (const std::string & s) {
    Core::Timer & timer = times_ [s];
    timer.stop ();
    out_ << "Done " << s << " [" << timer.user () << "s]" << std::endl;
    return timer;
  }
}
    
