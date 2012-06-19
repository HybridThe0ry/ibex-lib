//============================================================================
//                                  I B E X                                   
// File        : ibex_ParserResult.h
// Author      : Gilles Chabert
// Copyright   : Ecole des Mines de Nantes (France)
// License     : See the LICENSE file
// Created     : Jun 12, 2012
// Last Update : Jun 12, 2012
//============================================================================

#ifndef __IBEX_PARSER_RESULT_H__
#define __IBEX_PARSER_RESULT_H__

#include <vector>
#include "ibex_ParserExpr.h"
#include "ibex_ParserNumConstraint.h"
#include "ibex_Function.h"

namespace ibex {

namespace parser {
class P_Result {
public:
	P_Result();

	/** Auxiliary functions */
	std::vector<Function*> func;

	/** Main function */
	Function* f;

	/** Indices of sybs */
	std::vector<int> sybs;

	/** Indices of eprs */
	std::vector<int> eprs;

	/** Domains */
	std::vector<Interval> domains;

	/** Constraints */
	std::vector<NumConstraint*> ctrs;
};


} // end namespace parser
} // end namespace ibex
#endif // IBEX_PARSERRESULT_H_
