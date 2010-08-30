// Copyright (C) 2009 International Business Machines and others.
// All Rights Reserved.
// This code is published under the Common Public License.
//
// $Id$
//
// Authors:  Andreas Waechter, Sanjeeb Dash     IBM    2009-06-18
//                  (based on IpTSymLinearSolver.hpp rev 959)

#ifndef __IPPARTSYMLINEARSOLVER_HPP__
#define __IPPARTSYMLINEATSOLVER_HPP__

#include "IpSymLinearSolver.hpp"
#include "IpSparseSymLinearSolverInterface.hpp"
#include "IpTSymScalingMethod.hpp"
#include "IpSymMatrix.hpp"
#include "IpTripletToCSRConverter.hpp"
#include <vector>
#include <list>

namespace Ipopt
{

  /** General driver for linear solvers for sparse indefinite
   *  symmetric matrices for a parallel Ipopt version.  The values of
   *  the matrix are collected on the processor with rank 0.  The
   *  linear solver is called only on processor zero (for now).
   *
   *  This interface includes a call to a method for scaling of the
   *  matrix (if given).  This class takes in the contructor a pointer
   *  to the interface to an actual linear solver, and possibly a
   *  pointer to a method for computing scaling factors.  It
   *  translates the SymMatrix into the format required by the linear
   *  solver and calls the solver via the
   *  ParTSymLinearSolverInterface.  If a scaling method has been
   *  given, the matrix, the right hand side, and the solution are
   *  scaled.
   */
  class ParTSymLinearSolver: public SymLinearSolver
  {
  public:
    /** @name Constructor/Destructor */
    //@{
    /** Constructor.  The solver_interface is a pointer to a linear
     *  solver for symmetric matrices in triplet format.  If
     *  scaling_method not NULL, it must be a pointer to a class for
     *  computing scaling factors for the matrix.
     *
     *  If call_solverinterface_on_all_procs is set to true, then all
     *  processes will call the same methods of the solver_interface.
     *  Otherwise, those methods are called only on the root node. */
    ParTSymLinearSolver(SmartPtr<SparseSymLinearSolverInterface> solver_interface,
                        SmartPtr<TSymScalingMethod> scaling_method,
                        bool call_solverinterface_on_all_procs = false);

    /** Destructor */
    virtual ~ParTSymLinearSolver();
    //@}

    /** overloaded from AlgorithmStrategyObject */
    bool InitializeImpl(const OptionsList& options,
                        const std::string& prefix);

    /** @name Methods for requesting solution of the linear system. */
    //@{
    /** Solve operation for multiple right hand sides.  For details
     * see the description in the base class SymLinearSolver.
     */
    virtual ESymSolverStatus MultiSolve(const SymMatrix &A,
                                        std::vector<SmartPtr<const Vector> >& rhsV,
                                        std::vector<SmartPtr<Vector> >& solV,
                                        bool check_NegEVals,
                                        Index numberOfNegEVals);

    /** Number of negative eigenvalues detected during last
     * factorization.  Returns the number of negative eigenvalues of
     * the most recent factorized matrix.
     */
    virtual Index NumberOfNegEVals() const;
    //@}

    //* @name Options of Linear solver */
    //@{
    /** Request to increase quality of solution for next solve.
     * Ask linear solver to increase quality of solution for the next
     * solve (e.g. increase pivot tolerance).  Returns false, if this
     * is not possible (e.g. maximal pivot tolerance already used.)
     */
    virtual bool IncreaseQuality();

    /** Query whether inertia is computed by linear solver.
     * Returns true, if linear solver provides inertia.
     */
    virtual bool ProvidesInertia() const;
    //@}

    /** Methods for OptionsList */
    //@{
    static void RegisterOptions(SmartPtr<RegisteredOptions> roptions);
    //@}

  private:
    /**@name Default Compiler Generated Methods
     * (Hidden to avoid implicit creation/calling).
     * These methods are not implemented and 
     * we do not want the compiler to implement
     * them for us, so we declare them private
     * and do not define them. This ensures that
     * they will not be implicitly created/called. */
    //@{
    /** Default Constructor */
    ParTSymLinearSolver();

    /** Copy Constructor */
    ParTSymLinearSolver(const ParTSymLinearSolver&);

    /** Overloaded Equals Operator */
    void operator=(const ParTSymLinearSolver&);
    //@}

    /** @name Information about the matrix */
    //@{
    /** Tag for the incoming matrix */
    TaggedObject::Tag atag_;

    /** Number of rows and columns of the matrix */
    Index dim_;

    /** Number of nonzeros of the matrix in triplet format. Note that
     *  some elements might appear multiple times in which case the
     *  values are added. */
    Index nonzeros_triplet_;
    /** Number of nonzeros in compressed format.  This is only
     *  computed if the sparse linear solver works with the CSR
     *  format. */
    Index nonzeros_compressed_;
    /** Number of nonzeros in local part */
    Index local_nonzeros_triplet_;
    //@}

    /** @name Initialization flags */
    //@{
    /** Flag indicating if the internal structures are initialized.
     *  For initialization, this object needs to have seen a matrix */
    bool have_structure_;
    /** Flag indicating whether the scaling objected is to be switched
     *  on when increased quality is requested */
    bool linear_scaling_on_demand_;
    /** Flag indicating if the InitializeStructure method has been
     *  called for the linear solver. */
    bool initialized_;
    //@}

    /** Strategy Object for an interface to a linear solver. */
    SmartPtr<SparseSymLinearSolverInterface> solver_interface_;
    /** @name Stuff for scaling of the linear system. */
    //@{
    /** Strategy Object for a method that computes scaling factors for
     *  the matrices.  If NULL, no scaling is performed. */
    SmartPtr<TSymScalingMethod> scaling_method_;
    /** Array storing the scaling factors */
    double* scaling_factors_;
    /** Flag indicating whether scaling should be performed */
    bool use_scaling_;
    /** Flag indicating whether we just switched on the scaling */
    bool just_switched_on_scaling_;
    //@}

    /** @name information about the matrix. */
    //@{
    /** row indices of matrix in triplet (MA27) format.
     */
    Index* airn_;
    /** column indices of matrix in triplet (MA27) format.
     */
    Index* ajcn_;
    /** Pointer to object for conversion from triplet to compressed
     *  format.  This is only required if the linear solver works with
     *  the compressed representation. */
    SmartPtr<TripletToCSRConverter> triplet_to_csr_converter_;
    /** Flag indicating what matrix data format the solver requires. */
    SparseSymLinearSolverInterface::EMatrixFormat matrix_format_;
    //@}

    /** @name Algorithmic parameters */
    //@{
    /** Flag indicating whether the TNLP with identical structure has
     *  already been solved before. */
    bool warm_start_same_structure_;
    //@}

    /** @name Internal functions */
    //@{
    /** Initialize nonzero structure.
     *  Set dim_ and nonzeros_, and copy the nonzero structure of symT_A
     *  into airn_ and ajcn_
     */
    ESymSolverStatus InitializeStructure(const SymMatrix& symT_A);

    /** Copy the elements of the matrix in the required format into
     *  the array that is provided by the solver interface. */
    void GiveMatrixToSolver(bool new_matrix, const SymMatrix& sym_A);
    //@}

    /** MPI rank */
    int my_rank_;
    /** Number of MPI processors */
    int num_proc_;
    /** Only for root process. Information for Gatherv */
    int* recvcounts_;
    /** Only for root process. Information for Gatherv */
    int* displs_;
    /** Flag indicating if solver_interface methods should be called by all
     *  all processes or only by root process. */
    bool call_solverinterface_on_all_procs_;
  };

} // namespace Ipopt
#endif