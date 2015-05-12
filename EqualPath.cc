#include "EqualPath.hh"
#include "PathVisitor.hh"

namespace Permute {

  // Extracts the two children of the top-most PathComposite in the visited
  // path.
  class ExtractChildren : public PathVisitor {
  private:
    ConstPathRef left_;
    ConstPathRef right_;
  public:
    virtual void visit (const PathComposite *);
    virtual void visit (const Arc *) {}
    ConstPathRef getLeft () const;
    ConstPathRef getRight () const;
  };

  ConstPathRef ExtractChildren::getLeft () const {
    return left_;
  }

  ConstPathRef ExtractChildren::getRight () const {
    return right_;
  }

  ////////////////////////////////////////////////////////////////////////////////

  // Extracts the index of the top-most Arc in the visited path.
  class ExtractIndex : public PathVisitor {
  private:
    size_t index_;
  public:
    virtual void visit (const PathComposite *) {}
    virtual void visit (const Arc *);
    size_t getIndex () const;
  };

  void ExtractIndex::visit (const Arc * arc) {
    index_ = arc -> getIndex ();
  }

  size_t ExtractIndex::getIndex () const {
    return index_;
  }
  
  ////////////////////////////////////////////////////////////////////////////////

  // Checks if the visited path is the same as a reference path.
 class EqualPath : public PathVisitor {
  private:
    ConstPathRef path_;
    std::vector <ConstPathRef> stack_;
    bool equal_;
    ExtractChildren extract_children_;
    ExtractIndex extract_index_;
  public:
    EqualPath (const ConstPathRef & path);
    virtual void visit (const PathComposite * pc);
    virtual void visit (const Arc * arc);
    bool equal () const;
  };
  void ExtractChildren::visit (const PathComposite * pc) {
    left_ = pc -> getLeft ();
    right_ = pc -> getRight ();
  }

  EqualPath::EqualPath (const ConstPathRef & path) :
    path_ (path),
    stack_ (1, path_),
    equal_ (true),
    extract_children_ (),
    extract_index_ ()
  {}

  // Invariant: the stack contains the sequence of nodes leading to this
  // position in the reference path.
  void EqualPath::visit (const PathComposite * pc) {
    // If the paths have already been determined unequal, it is dangerous to
    // proceed.
    if (equal_) {
      ConstPathRef my_pc = stack_.back ();
      if (my_pc -> type () == pc -> type () &&
	  my_pc -> getStart () == pc -> getStart () &&
	  my_pc -> getEnd () == pc -> getEnd () &&
	  my_pc -> getScore () == pc -> getScore ()) {
	my_pc -> accept (& extract_children_);
	stack_.push_back (extract_children_.getLeft ());
	pc -> getLeft () -> accept (this);
	stack_.push_back (extract_children_.getRight ());
	pc -> getRight () -> accept (this);
	stack_.pop_back ();
      } else {
	equal_ = false;
      }
    }
  }

  void EqualPath::visit (const Arc * arc) {
    if (equal_) {
      ConstPathRef my_arc = stack_.back ();
      if (my_arc -> type () == arc -> type () &&
	  my_arc -> getStart () == arc -> getStart () &&
	  my_arc -> getEnd () == arc -> getEnd () &&
	  my_arc -> getScore () == arc -> getScore ()) {
	my_arc -> accept (& extract_index_);
	if (extract_index_.getIndex () != arc -> getIndex ()) {
	  equal_ = false;
	}
	stack_.pop_back ();
      } else {
	equal_ = false;
      }
    }
  }

  bool EqualPath::equal () const {
    return equal_;
  }

  ////////////////////////////////////////////////////////////////////////////////

  class ExtractPermutation : public PathVisitor {
  private:
    std::vector <size_t> permutation_;
  public:
    virtual void visit (const PathComposite * pc) {
      pc -> getLeft () -> accept (this);
      pc -> getRight () -> accept (this);
    }
    virtual void visit (const Arc * arc) {
      permutation_.push_back (arc -> getIndex ());
    }
    const std::vector <size_t> & getPermutation () const {
      return permutation_;
    }
  };

  bool equalPaths (const ConstPathRef & p1, const ConstPathRef & p2) {
//     EqualPath visitor (p1);
//     p2 -> accept (& visitor);
//     return visitor.equal ();

    ExtractPermutation v1, v2;
    p1 -> accept (& v1);
    p2 -> accept (& v2);
    return (v1.getPermutation () == v2.getPermutation ());
  }

  ////////////////////////////////////////////////////////////////////////////////

  // Prints a parenthesized representation of the tree contained in a visited
  // path.  At each composite node prints (TYPE LEFT RIGHT) where TYPE is KEEP or
  // SWAP, and LEFT and RIGHT are subtrees contained in the left and right
  // children.  At each arc node prints the arc's index.
  class PrintPath : public PathVisitor {
  private:
    std::ostream & out_;
  public:
    PrintPath (std::ostream & out) :
      out_ (out)
    {}
    virtual void visit (const Permute::PathComposite * pc) {
    
      out_ << "("
	   << (pc -> type () == Permute::Path::KEEP ? "KEEP " : "SWAP ");
      pc -> getLeft () -> accept (this);
      out_ << " ";
      pc -> getRight () -> accept (this);
      out_ << ")";
    }
    virtual void visit (const Permute::Arc * arc) {
      out_ << arc -> getIndex ();
    }
  };

  std::ostream & operator << (std::ostream & out, const ConstPathRef & path) {
    PrintPath visitor (out);
    path -> accept (& visitor);
    return out;
  }

}

