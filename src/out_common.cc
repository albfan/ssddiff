/* ===========================================================================
 *        Filename:  out_common.cc
 *     Description:  common functions for output writers
 * 
 *         Version:  $Rev$
 *         Changed:  $Date$
 *         Licence:  GPL (read COPYING file for details)
 * 
 *          Author:  Erich Schubert (eS), erich@debian.org
 *                   Institut für Informatik, LMU München
 * ======================================================================== */
#include "doc.h"
#include "util.h"
#include <vector>
#include <map>
#include <libxml/tree.h>
#include <iostream>

using namespace SSD;
namespace SSD {

/* calculate the longest increasing subsequence
 * easier case of LCS, longest common subsequence */
void calcLIS(vector<pair<xmlNodePtr, xmlNodePtr> >& p1, vector<xmlNodePtr>& p2, vector<pair<xmlNodePtr, xmlNodePtr> >& lcs) {
	vector<pair<xmlNodePtr, xmlNodePtr> >::iterator pos1;
	vector<xmlNodePtr>::iterator pos2;
	/* map node ptrs to pos */
	int l = 0;
	hashmap<xmlNodePtr, int, hashfun<void*> > m; /* local numbers */
	hashmap<xmlNodePtr, int, hashfun<void*> >::iterator mi;
	/* assign increasing sequence of numbers to nodes that might appear
	 * in second chain as well */
	for (pos1 = p1.begin(); pos1 != p1.end(); pos1++) {
		m[pos1->second] = l;
		l++;
	}
	if (l==0) return;
	/* translate second chain to ints using the sequence made above */
	vector<int> list;
	for (pos2 = p2.begin(); pos2 != p2.end(); pos2++) {
		mi = m.find(*pos2);
		if (mi != m.end())
			list.push_back(mi->second);
	}
	/* make lookup table */
	l = list.size();
	if (l<=0) return;
	vector<int> len(l,1);
	vector<int> next(l,-1);
	for (int k=l-2; k>=0; k--)
		for (int n=l-1; n>k; n--)
			if ( (list[n] > list[k]) && (len[n]+1 > len[k]) ) {
				len[k] = len[n] + 1;
				next[k] = n;
			}
	/* read longest list */
	int end=0;
	int max=0;
	for (int k=0; k<l; k++)
		if (len[k] > max) { end=k; max = len[k]; }
	/* extract longest list */
	for (int k=end; k>=0; k = next[k]) {
		lcs.push_back(p1[list[k]]);
	}
#if 0
	/* debug output */
	cout << "LCS: " << max << ": ";
	for (int k=end; k>=0; k = next[k]) {
		cout << k << ":" << p1[list[k]].first->name << " ";
		if (p1[list[k]].first->name && p1[list[k]].first->children)
			cout << "(" << xmlNodeGetContent(p1[list[k]].first->children) << ") ";
	}
	cout << endl;
#endif
}

} /* namespace end */
