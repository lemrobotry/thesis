// A PathVisitor knows how to visit path trees consisting of PathComposite
// internal nodes and Arc leaf nodes.

#ifndef _PERMUTE_PATH_VISITOR_HH
#define _PERMUTE_PATH_VISITOR_HH

namespace Permute {
  class PathComposite;
  class Arc;
  
  class PathVisitor {
  public:
    virtual ~PathVisitor () {};
    virtual void visit (const PathComposite *) = 0;
    virtual void visit (const Arc *) = 0;
  };
}

#endif//_PERMUTE_PATH_VISITOR_HH


