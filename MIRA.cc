#include <tao.h>
#include "MIRA.hh"

namespace Permute {

  bool operator < (const WRef & left, const WRef & right) {
    return left.get () < right.get ();
  }

  SparsePV::SparsePV (double margin) :
    std::map <WRef, double> (),
    margin_ (margin)
  {}

  void SparsePV::setMargin (double margin) {
    margin_ = margin;
  }

  double SparsePV::dot (const SparsePV & other) const {
    double product = 0.0;
    const_iterator j = other.begin ();
    for (const_iterator i = begin (); i != end (); ++ i) {
      for (; j != other.end () && (* j) < (* i); ++ j);
      if (j -> first == i -> first) {
	product += (i -> second) * (j -> second);
      }
    }
    return product;
  }

  double SparsePV::norm2 () const {
    double product = 0.0;
    for (const_iterator it = begin (); it != end (); ++ it) {
      product += pow (it -> second, 2);
    }
    return product;
  }

  double SparsePV::margin () const {
    double m = - margin_;
    for (const_iterator it = begin (); it != end (); ++ it) {
      m += (it -> second) * double (it -> first);
    }
    return m;
  }

  void SparsePV::update (double lambda) {
    for (iterator it = begin (); it != end (); ++ it) {
      it -> first += lambda * it -> second;
    }
  }

  void SparsePV::build (const Permutation & pi,
			const SumBeforeCostRef & bc,
			double sign) {
    for (Permutation::const_iterator i = pi.begin (); i != -- pi.end (); ++ i) {
      for (Permutation::const_iterator j = i + 1; j != pi.end (); ++ j) {
	if ((* i) < (* j)) {
	  build_helper (bc, * i, * j, sign);
	}
      }
    }
  }

  void SparsePV::build (const SumBeforeCostRef & bc,
			size_t l,
			size_t r,
			bool ordered) {
    if (ordered) {
      build_helper (bc, l, r, 1.0);
    } else {
      build_helper (bc, r, l, -1.0);
    }
  }

  void SparsePV::build_helper (const SumBeforeCostRef & bc,
			       size_t l,
			       size_t r,
			       double sign) {
    const Sum & sum = bc -> operator () (l, r);
    for (std::vector <WRef>::const_iterator it = sum.begin ();
	 it != sum.end (); ++ it) {
      operator [] (* it) += sign;
    }
  }
  
  /**********************************************************************/

  typedef struct {
    Vec m;
    Mat H;
  } AppCtx;

  int objectiveAndGradient (TAO_SOLVER, Vec, double *, Vec, void *);
  int hessian (TAO_SOLVER, Vec, Mat *, Mat *, MatStructure *, void *) { return 0; }

  /**********************************************************************/
  
  // Accepts a vector of feature differences corresponding to different
  // permutations, and containing their appropriate margins under the loss
  // function.  Computes and performs the MIRA update on the parameters.
  //
  // First, initializes the user context, consisting of the vector of desired
  // margins and the Hessian, which consists of the dot products feature
  // differences.
  //
  // Second, initializes the Lagrange multipliers to the solution when the
  // feature differences are all orthogonal, which has a closed form.
  //
  // Third, specifies the bounds on the Lagrange multipliers, which must be
  // non-negative.
  //
  // Fourth, solves the quadratic program using Tao.
  //
  // Finally, updates each feature difference by multiplying each feature's
  // occurrence count by the corresponding Lagrangian.
  int MIRA (std::vector <SparsePV> & delta) {
    TAO_SOLVER tao;
    TAO_APPLICATION mira;
    TaoMethod method = "tao_gpcg";
    int info;

    int N = delta.size ();

    // Compute the margins and the Hessian.
    AppCtx user;
    info = VecCreateSeq (PETSC_COMM_SELF, N, & user.m); CHKERRQ (info);
    info = MatCreateSeqAIJ (PETSC_COMM_SELF, N, N, 7, PETSC_NULL, & user.H); CHKERRQ (info);
    info = MatSetOption (user.H, MAT_SYMMETRIC); CHKERRQ (info);
    for (int i = 0; i < N; ++ i) {
      info = VecSetValue (user.m, i, delta [i].margin (), INSERT_VALUES); CHKERRQ (info);
      for (int j = i; j < N; ++ j) {
	info = MatSetValue (user.H, i, j, delta [i].dot (delta [j]), INSERT_VALUES); CHKERRQ (info);
      }
    }
    info = MatAssemblyBegin (user.H, MAT_FINAL_ASSEMBLY); CHKERRQ (info);
    info = MatAssemblyEnd (user.H, MAT_FINAL_ASSEMBLY); CHKERRQ (info);    
    
    // Initialize lambda to the orthogonal solution.
    Vec lambda, gradient;
    info = VecDuplicate (user.m, & lambda); CHKERRQ (info);
    info = VecSet (lambda, 0.0); CHKERRQ (info);
    for (int i = 0; i < N; ++ i) {
      double m = delta [i].margin ();
      if (m < 0.0) {
	info = VecSetValue (lambda, i, - m / delta [i].norm2 (), INSERT_VALUES); CHKERRQ (info);
      }
    }
    info = VecDuplicate (lambda, & gradient); CHKERRQ (info);
    
    // Specify the bounds.
    Vec lower, upper;
    info = VecDuplicate (lambda, & lower); CHKERRQ (info);
    info = VecDuplicate (lower, & upper); CHKERRQ (info);
    info = VecSet (lower, 0.0); CHKERRQ (info);
    info = VecSet (upper, TAO_INFINITY); CHKERRQ (info);

    // Solve the quadratic program.
    info = TaoCreate (MPI_COMM_SELF, method, & tao); CHKERRQ (info);
    info = TaoPetscApplicationCreate (PETSC_COMM_SELF, & mira); CHKERRQ (info);
    info = TaoSetPetscFunctionGradient (mira, lambda, gradient, objectiveAndGradient, & user); CHKERRQ (info);
    info = TaoSetPetscHessian (mira, user.H, user.H, hessian, & user); CHKERRQ (info);
    info = TaoSetPetscVariableBounds (mira, lower, upper); CHKERRQ (info);
    info = TaoSetPetscInitialVector (mira, lambda); CHKERRQ (info);
    info = TaoSetApplication (tao, mira); CHKERRQ (info);
    info = TaoSolve (tao); CHKERRQ (info);
    info = TaoDestroy (tao); CHKERRQ (info);
    info = TaoApplicationDestroy (mira); CHKERRQ (info);

    // Extract the new parameters.  NOTE: VecGetArray returns a pointer to a
    // contiguous array that contains this processor's portion of the vector
    // data.  For the standard PETSc vectors, VecGetArray() returns a pointer to
    // the local data array and does not use any copies.
    double * lam;
    info = VecGetArray (lambda, & lam); CHKERRQ (info);
    for (int i = 0; i < N; ++ i) {
      delta [i].update (lam [i]);
    }

    // Clean up.
    info = VecDestroy (lambda); CHKERRQ (info);
    info = VecDestroy (gradient); CHKERRQ (info);
    info = VecDestroy (lower); CHKERRQ (info);
    info = VecDestroy (upper); CHKERRQ (info);
    info = VecDestroy (user.m); CHKERRQ (info);
    info = MatDestroy (user.H); CHKERRQ (info);

    return 0;
  }

  /**********************************************************************/

  // Computes the objective g and gradient dl using the vector of margins m and
  // the Hessian matrix H contained in the given user context.  As indicated
  // below, - g = m^T l + 1/2 l^T H l, and dl = m + H l.
  int objectiveAndGradient (TAO_SOLVER tao, Vec l, double * g, Vec dl, void * ctx) {
    int info;
    AppCtx * user = (AppCtx *) ctx;

    // dl = m + H l
    info = MatMultAdd (user -> H, l, user -> m, dl); CHKERRQ (info);
    // - g = m l + 1/2 l H l
    Vec Hl;
    info = VecDuplicate (l, & Hl); CHKERRQ (info);
    info = MatMult (user -> H, l, Hl); CHKERRQ (info);
    double lHl, lm;
    info = VecTDot (l, Hl, & lHl); CHKERRQ (info);
    info = VecDestroy (Hl); CHKERRQ (info);
    info = VecTDot (l, user -> m, & lm); CHKERRQ (info);
    (* g) = lm + 0.5 * lHl;
    
    return 0;
  }
}
