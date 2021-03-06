//============================================================================
//                                  I B E X                                   
// File        : ibex_String.h
// Author      : Gilles Chabert
// Copyright   : IMT Atlantique (France)
// License     : See the LICENSE file
// Created     : Jul 18, 2012
// Last Update : Nov 21, 2017
//============================================================================

#ifndef __IBEX_STRING_H__
#define __IBEX_STRING_H__

namespace ibex {

/**
 * \brief Write an index at the end of a string, surrounded with two
 * symbols (like '['...']' or '{'..'}', etc.), including the
 * '\0' terminal character.
 *
 * For example, append_index("foo",12) ---> "foo[12]"
 *
 * \warning Up to 6 digits are allowed.
 *
 * \return the new string. It has to be freed by the caller.
 */
char* append_index(const char* base, char lbracket, char rbracket, int index);

/**
 * \brief Generate a variable name (_x_i).
 *
 * Note: needs to be disallocated.
 */
char* next_generated_var_name();

/**
 * \brief Generate a function name (_f_i).
 *
 * Note: needs to be disallocated.
 *
 */
char* next_generated_func_name();

} // end namespace ibex

#endif // __IBEX_STRING_H__
