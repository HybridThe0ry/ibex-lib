//============================================================================
//                                  I B E X
// File        : ibex_Optimizer.h
// Author      : Matias Campusano, Damir Aliquintui, Ignacio Araya
// Copyright   : IMT Atlantique (France)
// License     : See the LICENSE file
// Created     : Sep 24, 2017
// Last Update : Sep 24, 2017
//============================================================================

#ifndef __IBEX_OPTIMIZERMOP_H__
#define __IBEX_OPTIMIZERMOP_H__

#include "ibex_Ctc.h"
#include "ibex_Bsc.h"
#include "ibex_LoupFinderMOP.h"
#include "ibex_CellMOP.h"
#include "ibex_CtcKhunTucker.h"
#include "ibex_DistanceSortedCellBufferMOP.h"
#include "ibex_pyPlotter.h"

#include <set>
#include <map>
#include <list>
//#include "ibex_DistanceSorted.h"

using namespace std;
namespace ibex {

/**
 * comparation function for sorting NDS by decreasing y
 */
struct sorty{
	bool operator()(const pair<double,double> p1, const pair<double,double> p2){
		return p1.second>p2.second;
	}
};



/**
 * \brief Global biObjetive Optimizer (ibexMOP).
 *
 * This class is an implementation of a global optimization algorithm for biObjective problems
 * described in https://github.com/INFPUCV/ibex-lib/tree/master/plugins/optim-mop by Araya et al.
 *
 * \remark In all the comments of this class, "NDS" means "Non Dominated Set"
 * related to the objectives f1 and f2
 */
class OptimizerMOP {

public:

	/**
	 * \brief Return status of the optimizer
	 *
	 * See comments for optimize(...) below.
	 */
	typedef enum {SUCCESS, INFEASIBLE, NO_FEASIBLE_FOUND, UNBOUNDED_OBJ, TIME_OUT, UNREACHED_PREC} Status;

	/**
	 *  \brief Create an optimizer.
	 *
	 * Inputs:
	 *   \param n        - number of variables of the <b>original system</b>
	 *   \param f1	     - the objective function f1
     *	 \param f2       - the objective function f2
	 *   \param ctc      - contractor for <b>extended<b> boxes (of size n+2)
	 *   \param bsc      - bisector for <b>extended<b> boxes (of size n+2)
	 *   \param buffer   - buffer for <b>extended<b> boxes (of size n+2)
	 *   \param finder   - the finder of ub solutions
	 *   \param eps	     - the required precision
	 *

	 *
	 * \warning The optimizer relies on the contractor \a ctc to contract the domain of the goal variable
	 *          and increase the uplo. If this contractor never contracts this goal variable,
	 *          the optimizer will only rely on the evaluation of f and will be very slow.
	 *
	 * We are assuming that the objective variables are n and n+1
	 *
	 */
	OptimizerMOP(int n, const Function &f1,  const Function &f2,
			Ctc& ctc, Bsc& bsc, CellBufferOptim& buffer, LoupFinderMOP& finder,  double eps=default_eps);

	/**
	 * \brief Delete *this.
	 */
	virtual ~OptimizerMOP();

	/**
	 * \brief Run the optimization.
	 *
	 * \param init_box             The initial box
	 *
	 * \return SUCCESS             if the global minimum (with respect to the precision required) has been found.
	 *                             In particular, at least one feasible point has been found, less than obj_init_bound, and in the
	 *                             time limit.
	 **
	 *         TIMEOUT             if time is out.
	 *
	 */
	Status optimize(const IntervalVector& init_box);

	/* =========================== Output ============================= */

	/**
	 * \brief Displays on standard output a report of the last call to optimize(...).
	 *
	 * Information provided:
	 * <ul><li> total running time
	 *     <li> totl number of cells (boxes)
	 *     <li> total number of best non-dominated solutions found
	 *     <li> the set of non-dominated solutions found
	 * </ul>
	 */
	void report(bool verbose=true);


	/**
	 * \brief Get the status.
	 *
	 * \return the status of last call to optimize(...).
	 */
	Status get_status() const;

	/**
	 * \brief Get the "UB" set of the pareto front.
	 *
	 * \return the UB of the last call to optimize(...).
	 */
	map< pair <double, double>, IntervalVector >& get_UB()  { return NDS; }

	//std::set< point2 >& get_LB()  { return LB; }

	/**
	 * \brief Get the time spent.
	 *
	 * \return the total CPU time of last call to optimize(...)
	 */
	double get_time() const;

	/**
	 * \brief Get the number of cells.
	 *
	 * \return the number of cells generated by the last call to optimize(...).
	 */
	double get_nb_cells() const;

	/**
	 * \brief returns the distance from the box to the current NDS
	 */
	static double distance2(const Cell* c);

	/* =========================== Settings ============================= */

	/**
	 * \brief Number of variables.
	 */
	const int n;

	/**
	 * \brief Objective functions
	 * Functions have the form: f1 - z1  and f2 - z2. Thus, in order to
	 * evaluate them we have to set z1 and z2 to [0,0].
	 */
	const Function& goal1;
	const Function& goal2;

	/**
	 * \brief Contractor for the extended system.
	 *
	 * The extended system:
	 * (y=f(x), g_1(x)<=0,...,g_m(x)<=0).
	 */
	Ctc& ctc;

	/**
	 * \brief Bisector.
	 *
	 * Must work on extended boxes.
	 */
	Bsc& bsc;

	/**
	 * Cell buffer.
	 */
	CellBuffer& buffer;

	/**
	 * \brief LoupFinder
	 */
	LoupFinderMOP& finder;

	/** Required precision for the envelope */
	double eps;


	/** Default precision: 0.01 */
	static const double default_eps;

	/**
	 * \brief Trace activation flag.
	 */
	int trace;

	/**
	 * \brief Time limit.
	 *
	 * Maximum CPU time used by the strategy.
	 * This parameter allows to bound time consumption.
	 * The value can be fixed by the user.
	 */
	double timeout;


	/* ======== Some other parameters of the solver =========== */

	//if true: Save a file to be plotted by plot.py (default value= false).
	bool static _plot;

	//Min distance between two non dominated points to be considered, expressed as a fraction of eps (default value= 0.1)
	double static _min_ub_dist;

	//True: the solver uses the upper envelope of the cy contract for contraction
	static bool _cy_upper;

	//True: the solver uses the lower envelope of the cy contract in contraction
	static bool cy_contract_var;

	//True: the solver reduces the search spaces by reducing the NDS vectors in (eps, eps)
	static bool _eps_contract;

	static int _print_convergence;


protected:

  double max_dist(map<Cell*, double> cell_dist);

	/**
	 * The contraction using y+cy
	 */
	void cy_contract(Cell& c);

	/**
	 * \brief Contract and bound procedure for processing a box.
	 *
	 * <ul>
	 * <li> contract the cell using #dominance_peeler() and #discard_generalized_monotonicty_test(),
	 * <li> contract with the contractor ctc,
	 * </ul>
	 *
	 */
	void contract_and_bound(Cell& c, const IntervalVector& init_box);

	/**
	 * \brief The box is reduced using the NDS
	 *
	 * Details are given in [Martin, B. et al.
	 * Constraint propagation using dominance in interval
	 * Branch & Bound for nonlinear biobjective optimization (2017)]
	 */
	void dominance_peeler(IntervalVector& box);

    /**
     *  \brief returns true if the facet orthogonal to the i direction of the box is feasible.
     *
     *  See #discard_generalized_monotonicty_test()
     */
    bool is_inner_facet(IntervalVector box, int i, Interval bound){
    	box.resize(n);
    	box[i]=bound;
    	return finder.norm_sys.is_inner(box);
    }

    /**
     * \brief return true is the pair is dominated by some NDS point, false otherwise
     */
    bool is_dominated(pair< double, double>& eval);

    /**
     * \brief Implements the method for discarding boxes proposed in [J. Fernandez and B. Toth,
     * "Obtaining the efficient set of nonlinear biobjective optimiza-tion  problems  via  interval
     * branch-and-bound  methods" (2009)]
     */
	void discard_generalized_monotonicty_test(IntervalVector& box, const IntervalVector& initbox);


	/**
	 * \brief Main procedure for updating the NDS.
	 * <ul>
	 * <li> finds two points in a polytope using the #LoupFinderMOP,
	 * <li> generate n equi-distant points between the two points,
	 * <li> correct the points using a Hansen feasibility test (See #PdcHansenFeasibility)
	 * <li> add the non-dominated vectors to NDS
	 * </ul>
	 */
	bool update_NDS(const IntervalVector& box);


private:

	/**
	 * \brief Evaluate the goal in the point x
	 */
	Interval eval_goal(const Function& goal, IntervalVector& x);

	/**
	 * min feasible value found for each objective
	 */
    pair <double, double> y1_ub, y2_ub;

	/* Remember return status of the last optimization. */
	Status status;



	//TODO: this should not be static

	/** The current non-dominated set sorted by increasing x */
	static map< pair <double, double>, IntervalVector > NDS;

	/** The current non-dominated set sorted by decreasing y */
	map< pair <double, double>, IntervalVector, sorty > NDSy;


	/* CPU running time of the current optimization. */
	double time;

	/** Number of cells pushed into the heap (which passed through the contractors) */
	int nb_cells;

};

inline OptimizerMOP::Status OptimizerMOP::get_status() const { return status; }

inline double OptimizerMOP::get_time() const { return time; }

inline double OptimizerMOP::get_nb_cells() const { return nb_cells; }




} // end namespace ibex

#endif // __IBEX_OPTIMIZER_H__
