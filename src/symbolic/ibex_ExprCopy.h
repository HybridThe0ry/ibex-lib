//============================================================================
//                                  I B E X                                   
// File        : ibex_ExprCopy.h
// Author      : Gilles Chabert
// Copyright   : IMT Atlantique (France)
// License     : See the LICENSE file
// Created     : Jun 19, 2012
// Last Update : Apr 25, 2018
//============================================================================

#ifndef __IBEX_EXPR_COPY_H__
#define __IBEX_EXPR_COPY_H__

#include "ibex_ExprVisitor.h"
#include "ibex_Array.h"
#include "ibex_Domain.h"
#include "ibex_NodeMap.h"

namespace ibex {


/**
 * \brief Deep copy of an array of variables.
 *
 * As for ExprCopy, the size of \a dest may be greater than \a src (but not smaller).
 *
 * Useful to duplicate functions, via #Copy.
 */
void varcopy(const Array<const ExprSymbol>& src, Array<const ExprSymbol>& dest);

/**
 * \brief Check that two array of variables are the same.
 *
 */
bool varequals(const Array<const ExprSymbol>& args1, Array<const ExprSymbol>& args2);


/**
 * \brief Get the ith component of a symbol list.
 *
 * Example: if args=(x[3],y,z[2]) then var_component(args,4) return the expression z[0].
 */
const ExprNode& var_component(const Array<const ExprSymbol>& args, int i);

/**
 * \brief Duplicate an expression
 */
class ExprCopy : public virtual ExprVisitor {

public:
	/*
	 * \brief Duplicate an expression (with new symbols).
	 *
	 * \pre Each symbol in y must belong to "old_x".
	 *
	 * Symbols in \a old_x are matched to symbols in \a new_x with respect to their order.
	 *
	 * \param shared - if true, the node map structure is maintained through all calls to "copy" (this
	 *                 allows to benefit from the DAG structure of several expressions copied in turn).
	 *                 The resulting expressions are therefore not independent in this case.
	 *
	 * \pre The size of \a new_x must be greater or equal to the size of \a old_x. It is not
	 *      required to be the same size to allow the use of extra variables (that do not occur in the expression).
	 *      This is used, e.g., in ibex_Optimizer to transform a function x->g(x) into (x,y)->g(x).
	 */
	const ExprNode& copy(const Array<const ExprSymbol>& old_x, const Array<const ExprNode>& new_x, const ExprNode& y, bool shared=false);

	/*
	 * \brief Duplicate an expression (with new symbols).
	 *
	 * \see copy(const Array<const ExprSymbol>& old_x, const Array<const ExprNode>& new_x, const ExprNode& y, bool shared=false).
	 */
	const ExprNode& copy(const Array<const ExprSymbol>& old_x, const Array<const ExprSymbol>& new_x, const ExprNode& y, bool shared=false);

protected:
	void visit(const ExprNode& e);
	void visit(const ExprIndex& i);
	void visit(const ExprSymbol& x);
	void visit(const ExprConstant& c);
	void visit(const ExprVector& e);
	void visit(const ExprApply& e);
	void visit(const ExprChi& e);
	void visit(const ExprAdd& e);
	void visit(const ExprMul& e);
	void visit(const ExprSub& e);
	void visit(const ExprDiv& e);
	void visit(const ExprMax& e);
	void visit(const ExprMin& e);
	void visit(const ExprAtan2& e);
	void visit(const ExprMinus& e);
	void visit(const ExprTrans& e);
	void visit(const ExprSign& e);
	void visit(const ExprAbs& e);
	void visit(const ExprPower& e);
	void visit(const ExprSqr& e);
	void visit(const ExprSqrt& e);
	void visit(const ExprExp& e);
	void visit(const ExprLog& e);
	void visit(const ExprCos& e);
	void visit(const ExprSin& e);
	void visit(const ExprTan& e);
	void visit(const ExprCosh& e);
	void visit(const ExprSinh& e);
	void visit(const ExprTanh& e);
	void visit(const ExprAcos& e);
	void visit(const ExprAsin& e);
	void visit(const ExprAtan& e);
	void visit(const ExprAcosh& e);
	void visit(const ExprAsinh& e);
	void visit(const ExprAtanh& e);

	NodeMap<const ExprNode*> clone;
};


/* ============================================================================
 	 	 	 	 	 	 	 inline implementation
  ============================================================================*/

inline const ExprNode& ExprCopy::copy(const Array<const ExprSymbol>& old_x, const Array<const ExprSymbol>& new_x, const ExprNode& y, bool shared) {
	return this->copy(old_x, (const Array<const ExprNode>&) new_x, y, shared);
}


} // end namespace ibex

#endif // __IBEX_EXPR_COPY_H__
