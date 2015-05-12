#ifndef _PERMUTE_PRUNE_CHART_HH
#define _PERMUTE_PRUNE_CHART_HH

#include <Core/Types.hh>
#include "Chart.hh"
#include "Outside.hh"

namespace Permute {
  // Performs absolute pruning against the given threshold, which updates as
  // better permutations are discovered.
  ChartRef prune (ChartRef, double = Core::Type <double>::min);
  // Performs absolute pruning against the given threshold, with the addition of
  // outside estimates from the second chart parameter.
  ChartRef prune (ChartRef, ChartRef, double = Core::Type <double>::min);
  // Performs relative pruning using the given threshold, which represents the
  // degradation (negative) allowed from the best score.
  ChartRef pruneRelative (ChartRef, double = Core::Type <double>::min);
}

#endif//_PERMUTE_PRUNE_CHART_HH
