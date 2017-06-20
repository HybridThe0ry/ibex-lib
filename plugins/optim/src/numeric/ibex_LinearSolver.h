//============================================================================
//                                  I B E X
// Interface with the linear solver
// File        : ibex_LinearSolver.h
// Author      : Jordan Ninin
// License     : See the LICENSE file
// Created     : May 15, 2013
// Last Update : May 15, 2013
//============================================================================

#ifndef IBEX_LINEARSOLVER_H_
#define IBEX_LINEARSOLVER_H_

#include "ibex_Setting.h"


#include <string.h>
#include <stdio.h>
#include "ibex_Vector.h"
#include "ibex_Matrix.h"
#include "ibex_IntervalVector.h"
#include "ibex_IntervalMatrix.h"
#include "ibex_Interval.h"
#include "ibex_CmpOp.h"
#include "ibex_Exception.h"
#include "ibex_LPException.h"

#ifdef _IBEX_WITH_SOPLEX_
	#ifdef DEBUG
		#undef DEBUG
		#ifndef DEBUGGING
			#define DEBUGGING
		#endif
		#include "soplex.h"
		#define DEBUG
	#else
		#include "soplex.h"
	#endif
#else
#ifdef _IBEX_WITH_CPLEX_
#include "ilcplex/cplex.h"

#else
#ifdef _IBEX_WITH_CLP_
// pourquoi enlever "coin/" dans le chemin?
#include "coin/ClpSimplex.hpp"
#else
#ifdef _IBEX_WITH_ILOCPLEX_
#include <ilcplex/ilocplex.h>
// TODO not finish yet
#else
#ifdef _IBEX_WITH_NOLP_
// nothing
#endif
#endif
#endif
#endif
#endif

namespace ibex {

class LinearSolver {


private:

	int nb_vars;
	int nb_ctrs;

	Interval obj_value;

	double epsilon;

	double * primal_solution;
	double * dual_solution;
	int size_dual_solution;
	int status_prim; //= 1 if OK
	int status_dual; //= 1 if OK

	IntervalVector boundvar;


#ifdef _IBEX_WITH_SOPLEX_
	soplex::SoPlex *mysoplex;
#endif

#ifdef _IBEX_WITH_CPLEX_
	CPXENVptr  envcplex;
	CPXLPptr lpcplex;
	int * indice;
	double * 	tmp;
	int *  		r_matbeg;
	double *  	r_matval;
	int * 		r_matind;
#endif

#ifdef _IBEX_WITH_ILOCPLEX_
	IloEnv		*myenv;
    IloModel 	*mymodel;
	IloCplex	*mycplex;
#endif


#ifdef _IBEX_WITH_CLP_
	ClpSimplex 	*myclp;
	int * _which;
	int * _col1Index;
#endif


public:

	static const double default_eps;

	/** the maximal bound of the variable, set to 1e20	 */
	static const double default_max_bound;

    /** Default max_time_out, set to 100s  */
    static const int default_max_time_out;

    /** Default max_iter, set to 100 iterations */
    static const int default_max_iter;


	/** Default max_diam_deriv value, set to 1e6  **/
	static const Interval default_limit_diam_box;

	typedef enum  {OPTIMAL=1, INFEASIBLE=2, OPTIMAL_PROVED=3, INFEASIBLE_PROVED=4, UNKNOWN=0, TIME_OUT=-1, MAX_ITER=-2} Status_Sol;

	typedef enum  {MINIMIZE, MAXIMIZE} Sense;

	LinearSolver(int nb_vars, int nb_ctr, int max_iter= default_max_iter,
			int max_time_out= default_max_time_out, double eps=default_eps);

	~LinearSolver();

	Status_Sol solve();
	Status_Sol solve_robust();



	void writeFile(const char* name="save_LP.lp");



// GET
	int getNbRows() const;

	Interval getObjValue() const;

	void getCoefObj(Vector& obj) const;

	void getCoefConstraint(Matrix& A) const;

	void getCoefConstraint_trans(Matrix& A_trans) const;

	void getB(IntervalVector& lhs_rhs) const;

	void getPrimalSol(Vector & prim) const;

	void getDualSol(Vector & dual) const;

	void getInfeasibleDir(Vector & sol) const;

	double getEpsilon() const;


// SET

	void cleanConst();

	void cleanAll();

	void setMaxIter(int max);

	void setMaxTimeOut(int time);

	void setSense(Sense s);


	void setObj(const Vector& coef);

	void setObjVar(int var, double coef);

	void setBounds(const IntervalVector& bounds);

	void setBoundsVar(int var, const Interval& bound);

	void setEpsilon(double eps);

	void addConstraint(const Vector & row, CmpOp sign, double rhs );

	void addConstraint(const Matrix & A, CmpOp sign, const Vector& rhs );

private:
	friend class CtcPolytopeHull;

	/**  Call to linear solver to optimize one variable */
	Status_Sol solve_var(Sense sense, int var, Interval & obj);

	/**
	 * Neumaier Shcherbina postprocessing in case of optimal solution found : the result obj is made reliable
	 */
	Interval  NeumaierShcherbina_postprocessing_var(int var, Sense sense);
	Interval  NeumaierShcherbina_postprocessing();

	/**
	 *  Neumaier Shcherbina postprocessing in case of infeasibilty found by LP  returns true if the infeasibility is proved
	 */
	bool NeumaierShcherbina_infeasibilitytest();

};

/** \brief Stream out \a x. */
std::ostream& operator<<(std::ostream& os, const LinearSolver::Status_Sol x);


} // end namespace ibex

#endif /* IBEX_LINEARSOLVER_H_ */
