/* ============================================================================
 * I B E X - Function basic evaluation
 * ============================================================================
 * Copyright   : Ecole des Mines de Nantes (FRANCE)
 * License     : This program can be distributed under the terms of the GNU LGPL.
 *               See the file COPYING.LESSER.
 *
 * Author(s)   : Gilles Chabert
 * Created     : Jan 14, 2012
 * ---------------------------------------------------------------------------- */

#ifndef _IBEX_EVAL_H_
#define _IBEX_EVAL_H_

#include "ibex_CompiledFunction.h"
#include "ibex_IntervalMatrixArray.h"
#include "ibex_Dim.h"
#include "ibex_Decorator.h"

#include <iostream>

namespace ibex {

class Eval;

/**
 * \ingroup level1
 * \brief Label for the basic evaluation of a function.
 */
class Domain : public ExprLabel {
public:

	/**
	 * The dimension of the domain
	 */
	const Dim dim;

	/**
	 * \brief Creates a new domain of dimension \a dim.
	 */
	Domain(const Dim& dim) : dim(dim), is_reference(false) {
		switch(dim.type()) {
		case Dim::SCALAR:       domain = new Interval(); break;
		case Dim::ROW_VECTOR:   domain = new IntervalVector(dim.dim3); break;
		case Dim::COL_VECTOR:   domain = new IntervalVector(dim.dim2); break;
		case Dim::MATRIX:       domain = new IntervalMatrix(dim.dim2,dim.dim3); break;
		case Dim::MATRIX_ARRAY: domain = new IntervalMatrixArray(dim.dim1,dim.dim2,dim.dim3); break;
		}
	}

	/**
	 * \brief Creates a reference to an interval.
	 */
	Domain(Interval& itv) : dim(0,0,0), is_reference(true) {
		domain = &itv;
	}

	/**
	 * \brief Creates a reference to an interval vector.
	 */
	Domain(IntervalVector& v, bool in_row) : dim(0,in_row?0:v.size(),in_row?v.size():0), is_reference(true) {
		domain = &v;
	}

	/**
	 * \brief Creates a reference to an interval matrix.
	 */
	Domain(IntervalMatrix& m) : dim(0,m.nb_rows(),m.nb_cols()), is_reference(true) {
		domain = &m;
	}

	/**
	 * \brief Creates a reference to an array of interval matrices.
	 */
	Domain(IntervalMatrixArray& ma) : dim(ma.size(),ma.nb_rows(),ma.nb_cols()), is_reference(true) {
		domain = &ma;
	}

	virtual ~Domain() {
		if (!is_reference)
			switch(dim.type()) {
			case Dim::SCALAR:       delete &i();  break;
			case Dim::ROW_VECTOR:
			case Dim::COL_VECTOR:   delete &v();  break;
			case Dim::MATRIX:       delete &m();  break;
			case Dim::MATRIX_ARRAY: delete &ma(); break;
			}
	}

	/**
	 * Return the domain as an interval.
	 */
	inline Interval& i()        {
		assert(dim.is_scalar());
		return *(Interval*) domain;
	}

	inline IntervalVector& v()  {
		assert(dim.is_vector());
		return *(IntervalVector*) domain;
	}

	inline IntervalMatrix& m()  {
		assert(dim.type()==Dim::MATRIX);
		return *(IntervalMatrix*) domain;
	}

	inline IntervalMatrixArray& ma() {
		assert(dim.type()==Dim::MATRIX_ARRAY);
		return *(IntervalMatrixArray*) domain;
	}

	inline const Interval& i() const  {
		assert(dim.is_scalar());
		return *(Interval*) domain;
	}

	inline const IntervalVector& v() const  {
		assert(dim.is_vector());
		return *(IntervalVector*) domain;
	}

	inline const IntervalMatrix& m() const  {
		assert(dim.type()==Dim::MATRIX);
		return *(IntervalMatrix*) domain;
	}

	inline const IntervalMatrixArray& ma() const {
		assert(dim.type()==Dim::MATRIX_ARRAY);
		return *(IntervalMatrixArray*) domain;
	}

private:
	void* domain;
	bool is_reference;
};

std::ostream& operator<<(std::ostream& os,const Domain&);

/**
 * \ingroup level1
 * \brief Decorates/undecorates a function for basic evaluation.
 */
class EvalDecorator : public Decorator<Domain> {
public:

	virtual void decorate(const Function& f) const;

protected:
	/* Visit an expression. */
	virtual void visit(const ExprNode& n);
	/* Visit an indexed expression. */
	virtual void visit(const ExprIndex& idx);
	/* Visit a symbol. */
	virtual void visit(const ExprSymbol& s);
	/* Visit a constant. */
	virtual void visit(const ExprConstant& c);
	/* Visit an n-ary operator. */
	virtual void visit(const ExprNAryOp&);
	/* Visit an unary operator. */
	virtual void visit(const ExprUnaryOp&);
	/* Visit a binary operator. */
	virtual void visit(const ExprBinaryOp&);
	/* Visit a vector of expressions. */
	virtual void visit(const ExprVector& v);
	/* Visit a function application. */
	virtual void visit(const ExprApply&);

};

/**
 * \brief Function evaluator.
 */
class Eval : public FwdAlgorithm<Domain> {

public:
	/**
	 * \brief Build an evaluator for a function f.
	 *
	 * The function f in argument is not "compiled".
	 * The evaluator is in charge of decorating/compiling the function.
	 * A reference to the compiled function is stored in the
	 * field \a f and can therefore be used from outside (like
	 * in HC4Revise).
	 */
	Eval(const Function& f);

	/**
	 * \brief Build an evaluator for a function f.
	 *
	 * The function is already compiled with a label typed Domain.
	 */
	Eval(const CompiledFunction<Domain>& f);

	/**
	 * \brief Delete the evaluator.
	 */
	~Eval();

	/**
	 * Run the forward algorithm.
	 */
	Domain& forward(const IntervalVector& box) const;

	/**
	 * Run the forward algorithm and return the
	 * domain of the root node, that must be scalar.
	 */
	Interval eval(const IntervalVector& box) const;

	/**
	 * Run the forward algorithm and return the
	 * domain of the root node, that must be a vector.
	 */
	IntervalVector eval_vector(const IntervalVector& box) const;

	/**
	 * Run the forward algorithm and return the
	 * domain of the root node, that must be a matrix.
	 */
	IntervalMatrix eval_matrix(const IntervalVector& box) const;

	/**
	 * \brief The compiled function.
	 */
	const CompiledFunction<Domain>& f;

	/**
	 * \brief Domain of the ith input symbol.
	 */
	Domain& domain(int i);

	/**
	 * \brief Domain of the ith input symbol.
	 */
	const Domain& domain(int i) const;

protected:

	friend class CompiledFunction<Domain>;
	friend class EvalDecorator;

	inline void index_fwd(const ExprIndex& e, const Domain& x, Domain& y);
	       void vector_fwd(const ExprVector& v, const Domain** compL, Domain& y);
	inline void cst_fwd(const ExprConstant& c, Domain& y);
	inline void symbol_fwd(const ExprSymbol& s, Domain& y);
	       void apply_fwd(const ExprApply& a, const Domain** argL, Domain& y);
	inline void add_fwd(const ExprAdd&, const Domain& x1, const Domain& x2, Domain& y);
	inline void mul_fwd(const ExprMul&, const Domain& x1, const Domain& x2, Domain& y);
	inline void sub_fwd(const ExprSub&, const Domain& x1, const Domain& x2, Domain& y);
	inline void div_fwd(const ExprDiv&, const Domain& x1, const Domain& x2, Domain& y);
	inline void max_fwd(const ExprMax&, const Domain& x1, const Domain& x2, Domain& y);
	inline void min_fwd(const ExprMin&, const Domain& x1, const Domain& x2, Domain& y);
	inline void atan2_fwd(const ExprAtan2&, const Domain& x1, const Domain& x2, Domain& y);
	inline void minus_fwd(const ExprMinus&, const Domain& x, Domain& y);
	inline void sign_fwd(const ExprSign&, const Domain& x, Domain& y);
	inline void abs_fwd(const ExprAbs&, const Domain& x, Domain& y);
	inline void power_fwd(const ExprPower& p, const Domain& x, Domain& y);
	inline void sqr_fwd(const ExprSqr&, const Domain& x, Domain& y);
	inline void sqrt_fwd(const ExprSqrt&, const Domain& x, Domain& y);
	inline void exp_fwd(const ExprExp&, const Domain& x, Domain& y);
	inline void log_fwd(const ExprLog&, const Domain& x, Domain& y);
	inline void cos_fwd(const ExprCos&, const Domain& x, Domain& y);
	inline void sin_fwd(const ExprSin&, const Domain& x, Domain& y);
	inline void tan_fwd(const ExprTan&, const Domain& x, Domain& y);
	inline void cosh_fwd(const ExprCosh&, const Domain& x, Domain& y);
	inline void sinh_fwd(const ExprSinh&, const Domain& x, Domain& y);
	inline void tanh_fwd(const ExprTanh&, const Domain& x, Domain& y);
	inline void acos_fwd(const ExprAcos&, const Domain& x, Domain& y);
	inline void asin_fwd(const ExprAsin&, const Domain& x, Domain& y);
	inline void atan_fwd(const ExprAtan&, const Domain& x, Domain& y);
	inline void acosh_fwd(const ExprAcosh&, const Domain& x, Domain& y);
	inline void asinh_fwd(const ExprAsinh&, const Domain& x, Domain& y);
	inline void atanh_fwd(const ExprAtanh&, const Domain& x, Domain& y);

	inline void add_V_fwd(const ExprAdd&, const Domain& x1, const Domain& x2, Domain& y);
	inline void add_M_fwd(const ExprAdd&, const Domain& x1, const Domain& x2, Domain& y);
	inline void mul_SV_fwd(const ExprMul&, const Domain& x1, const Domain& x2, Domain& y);
	inline void mul_SM_fwd(const ExprMul&, const Domain& x1, const Domain& x2, Domain& y);
	inline void mul_VV_fwd(const ExprMul&, const Domain& x1, const Domain& x2, Domain& y);
	inline void mul_MV_fwd(const ExprMul&, const Domain& x1, const Domain& x2, Domain& y);
	inline void mul_MM_fwd(const ExprMul&, const Domain& x1, const Domain& x2, Domain& y);
	inline void sub_V_fwd(const ExprSub&, const Domain& x1, const Domain& x2, Domain& y);
	inline void sub_M_fwd(const ExprSub&, const Domain& x1, const Domain& x2, Domain& y);

	mutable Domain** symbolLabels; // domains of the symbols (created by the decorator)

private:
	void read(const IntervalVector&) const; // load a box into symbolLabels

	bool proper_compiled_func;
};

/**
 * \ingroup level1
 * \brief Evaluation label for function application nodes.
 *
 * A function application node has a specific label that, in addition
 * to the domain, contains an evaluator of the function called.
 */
class EvalApplyLabel : public Domain {
public:
	EvalApplyLabel(const Dim& dim, const Function& f) : Domain(dim), fevl(f) { }

	Eval fevl;    //  for each function, there is an associated evaluator
};

/* ============================================================================
 	 	 	 	 	 	 	 implementation
  ============================================================================*/

inline Domain& Eval::domain(int i) {
	assert(i>=0 && i<f.f.nb_symbols());
	return *symbolLabels[i];
}

inline const Domain& Eval::domain(int i) const {
	assert(i>=0 && i<f.f.nb_symbols());
	return *symbolLabels[i];
}

inline Domain& Eval::forward(const IntervalVector& box) const {
	read(box); // load the domains of all the symbols
	return f.forward(*this);
}

inline Interval Eval::eval(const IntervalVector& box) const {
	return forward(box).i();
}

inline IntervalVector Eval::eval_vector(const IntervalVector& box) const {
	return forward(box).v();
}

inline IntervalMatrix Eval::eval_matrix(const IntervalVector& box) const {
	return forward(box).m();
}

inline void Eval::Eval::index_fwd(const ExprIndex& e, const Domain& x, Domain& y) { }

inline void Eval::symbol_fwd(const ExprSymbol& s, Domain& y) { }

inline void Eval::cst_fwd(const ExprConstant& c, Domain& y) {
	switch (c.type()) {
	case Dim::SCALAR:       y.i() = c.get_value();         break;
	case Dim::ROW_VECTOR:
	case Dim::COL_VECTOR:   y.v() = c.get_vector_value();  break;
	case Dim::MATRIX:       y.m() = c.get_matrix_value();  break;
	case Dim::MATRIX_ARRAY: assert(false); /* impossible */ break;
	}
}
inline void Eval::add_fwd(const ExprAdd&, const Domain& x1, const Domain& x2, Domain& y)     { y.i()=x1.i()+x2.i(); }
inline void Eval::mul_fwd(const ExprMul&, const Domain& x1, const Domain& x2, Domain& y)     { y.i()=x1.i()*x2.i(); }
inline void Eval::sub_fwd(const ExprSub&, const Domain& x1, const Domain& x2, Domain& y)     { y.i()=x1.i()-x2.i(); }
inline void Eval::div_fwd(const ExprDiv&, const Domain& x1, const Domain& x2, Domain& y)     { y.i()=x1.i()/x2.i(); }
inline void Eval::max_fwd(const ExprMax&, const Domain& x1, const Domain& x2, Domain& y)     { y.i()=max(x1.i(),x2.i()); }
inline void Eval::min_fwd(const ExprMin&, const Domain& x1, const Domain& x2, Domain& y)     { y.i()=min(x1.i(),x2.i()); }
inline void Eval::atan2_fwd(const ExprAtan2&, const Domain& x1, const Domain& x2, Domain& y) { y.i()=atan2(x1.i(),x2.i()); }

inline void Eval::minus_fwd(const ExprMinus&, const Domain& x, Domain& y)                       { y.i()=-x.i(); }
inline void Eval::sign_fwd(const ExprSign&, const Domain& x, Domain& y)                         { y.i()=sign(x.i()); }
inline void Eval::abs_fwd(const ExprAbs&, const Domain& x, Domain& y)                           { y.i()=abs(x.i()); }
inline void Eval::power_fwd(const ExprPower& p, const Domain& x, Domain& y)                     { y.i()=pow(x.i(),p.expon); }
inline void Eval::sqr_fwd(const ExprSqr&, const Domain& x, Domain& y)                           { y.i()=sqr(x.i()); }
inline void Eval::sqrt_fwd(const ExprSqrt&, const Domain& x, Domain& y)                         { y.i()=sqrt(x.i()); }
inline void Eval::exp_fwd(const ExprExp&, const Domain& x, Domain& y)                           { y.i()=exp(x.i()); }
inline void Eval::log_fwd(const ExprLog&, const Domain& x, Domain& y)                           { y.i()=log(x.i()); }
inline void Eval::cos_fwd(const ExprCos&, const Domain& x, Domain& y)                           { y.i()=cos(x.i()); }
inline void Eval::sin_fwd(const ExprSin&, const Domain& x, Domain& y)                           { y.i()=sin(x.i()); }
inline void Eval::tan_fwd(const ExprTan&, const Domain& x, Domain& y)                           { y.i()=tan(x.i()); }
inline void Eval::cosh_fwd(const ExprCosh&, const Domain& x, Domain& y)                         { y.i()=cosh(x.i()); }
inline void Eval::sinh_fwd(const ExprSinh&, const Domain& x, Domain& y)                         { y.i()=sinh(x.i()); }
inline void Eval::tanh_fwd(const ExprTanh&, const Domain& x, Domain& y)                         { y.i()=tanh(x.i()); }
inline void Eval::acos_fwd(const ExprAcos&, const Domain& x, Domain& y)                         { y.i()=acos(x.i()); }
inline void Eval::asin_fwd(const ExprAsin&, const Domain& x, Domain& y)                         { y.i()=asin(x.i()); }
inline void Eval::atan_fwd(const ExprAtan&, const Domain& x, Domain& y)                         { y.i()=atan(x.i()); }
inline void Eval::acosh_fwd(const ExprAcosh&, const Domain& x, Domain& y)                       { y.i()=acosh(x.i()); }
inline void Eval::asinh_fwd(const ExprAsinh&, const Domain& x, Domain& y)                       { y.i()=asinh(x.i()); }
inline void Eval::atanh_fwd(const ExprAtanh&, const Domain& x, Domain& y)                       { y.i()=atanh(x.i()); }

inline void Eval::add_V_fwd(const ExprAdd&, const Domain& x1, const Domain& x2, Domain& y)         { y.v()=x1.v()+x2.v(); }
inline void Eval::add_M_fwd(const ExprAdd&, const Domain& x1, const Domain& x2, Domain& y)         { y.m()=x1.m()+x2.m(); }
inline void Eval::mul_SV_fwd(const ExprMul&, const Domain& x1, const Domain& x2, Domain& y)        { y.v()=x1.i()*x2.v(); }
inline void Eval::mul_SM_fwd(const ExprMul&, const Domain& x1, const Domain& x2, Domain& y)        { y.m()=x1.i()*x2.m(); }
inline void Eval::mul_VV_fwd(const ExprMul&, const Domain& x1, const Domain& x2, Domain& y)        { y.i()=x1.v()*x2.v(); }
inline void Eval::mul_MV_fwd(const ExprMul&, const Domain& x1, const Domain& x2, Domain& y)        { y.v()=x1.m()*x2.v(); }
inline void Eval::mul_MM_fwd(const ExprMul&, const Domain& x1, const Domain& x2, Domain& y)        { y.m()=x1.m()*x2.m(); }
inline void Eval::sub_V_fwd(const ExprSub&, const Domain& x1, const Domain& x2, Domain& y)         { y.v()=x1.v()-x2.v(); }
inline void Eval::sub_M_fwd(const ExprSub&, const Domain& x1, const Domain& x2, Domain& y)         { y.m()=x1.m()-x2.m(); }

} // namespace ibex
#endif // IBEX_EVAL_H_
