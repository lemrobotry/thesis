#ifndef _PERMUTE_PRINT_TIMER_HH
#define _PERMUTE_PRINT_TIMER_HH

#include <map>
#include <Core/Statistics.hh>

namespace Permute {

  class PrintTimer {
  private:
    std::ostream & out_;
    std::map <std::string, Core::Timer> times_;
  public:
    PrintTimer (std::ostream &);
    void start (const std::string &);
    Core::Timer stop (const std::string &);
    template <class T> void start (const std::string &, const T &);
    template <class T> Core::Timer stop (const std::string &, const T &);
  };

  template <class T>
  void PrintTimer::start (const std::string & s, const T & t) {
    std::ostringstream o;
    o << s << t;
    start (o.str ());
  }

  template <class T>
  Core::Timer PrintTimer::stop (const std::string & s, const T & t) {
    std::ostringstream o;
    o << s << t;
    return stop (o.str ());
  }

}

#endif//_PERMUTE_PRINT_TIMER_HH
