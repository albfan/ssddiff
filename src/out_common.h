/* ===========================================================================
 *        Filename:  out_common.h
 *     Description:  Header for common output functions
 * 
 *         Version:  $Rev$
 *         Changed:  $Date$
 *         Licence:  GPL (read COPYING file for details)
 * 
 *          Author:  Erich Schubert (eS), erich@debian.org
 *                   Institut für Informatik, LMU München
 * ======================================================================== */
#ifndef OUT_OUTPUT_COMMON_H
#define OUT_OUTPUT_COMMON_H
#include <vector>
#include "libxml/tree.h"
namespace SSD {
	/** \brief calculate the longest increasing subsequence efficiently */
	/** common function used by many output writers. This corresponds to the use of LCS
	 *  in other diff algorithms, we try to minimize the number of move operations this way.
	 *  \param p1 a list of node pairs (in doc1, in doc2),
	 *  \param p2 a list of nodes in doc2 only
	 *  \param lis return vector, sublist of p1, and second parameters sublist of p2, too */
	void calcLIS(vector<pair<xmlNodePtr, xmlNodePtr> >& p1, vector<xmlNodePtr>& p2, vector<pair<xmlNodePtr, xmlNodePtr> >& lis);
}
#endif
