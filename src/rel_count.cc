/* ===========================================================================
 *        Filename:  rel_count.cc
 *     Description:  Relation count class
 * 
 *         Version:  $Rev$
 *         Changed:  $Date$
 *         Licence:  GPL (read COPYING file for details)
 * 
 *          Author:  Erich Schubert (eS), erich@debian.org
 *                   Institut für Informatik, LMU München
 * ======================================================================== */
#include "rel_count.h"
#include <utility> /* for make_pair */

namespace SSD {

/* for static lookup hashmap */
unsigned int RelCount::len = 0;
hashmap<RelEqClass, unsigned int, hash_releqc> RelCount::cmap;

RelCount::RelCount(hashmap<RelEqClass, int, hash_releqc>& map1, hashmap<RelEqClass, int, hash_releqc>& map2) : data(NULL) {
	hashmap<RelEqClass, int, hash_releqc>::iterator i1, i2;
	vector<short> tmpdata;
	
	/* initialize the first RelCount from the tables of two documents */
	if (len != 0) throw "RelCount has been initialized before";

	for (i1 = map1.begin(); i1 != map1.end(); i1++) {
		i2 = map2.find(i1->first);

		if (i2 == map2.end()) {
			cmap[i1->first] = RELCOUNT_UNIQUE;
		} else {
			if (i1->second == i2->second && i1->second == 1) {
				cmap[i1->first] = RELCOUNT_ONCE;
			} else {
				cmap[i1->first] = len;
				tmpdata.push_back(i1->second - i2->second);
				len++;
			}
		}
	}
	for (i2 = map2.begin(); i2 != map2.end(); i2++) {
		i1 = map1.find(i2->first);

		if (i1 == map1.end()) {
			cmap[i2->first] = RELCOUNT_UNIQUE;
		}
	}
	/* copy data to the minimal data structure */
	data = (short*) calloc(len, sizeof(short));
	if (tmpdata.size() != len) throw "Inconsitency detected!";
	for (unsigned int i=0; i<tmpdata.size(); i++)
		/* we multiply by 2, so we can subtract twice in diff.c */
		data[i] = tmpdata[i] * 2;
}

RelCount::RelCount(RelCount& rc) : data(NULL) {
	if (len > 0 && rc.data) {
		data = (short*) calloc(len, sizeof(short));
		memcpy(data, rc.data, sizeof(short)*len);
	}
}

RelCount::RelCount() : data(NULL) {
	if (len > 0) {
		data = (short*) calloc(len, sizeof(short));
	}
}

RelCount::~RelCount() {
	if (data) { free(data); data=NULL; }
}

int RelCount::modify(RelEqClass key, short val) {
	int cost=0;

	/* find the key in lookup table */
	hashmap<RelEqClass, unsigned int, hash_releqc>::iterator pos = cmap.find(key);
	if (pos != cmap.end()) {
		/* unique keys don't generate costs */
		if (pos->second == RELCOUNT_UNIQUE) return 0;
		/* key occuring once each have a cost of 1 if dropped from the first document */
		/* TODO: can it happen that we know first we'll drop it in the
		 * second document? likely? then we should assign costs early in
		 * that case somehow, too! Bitset? */
		if (pos->second == RELCOUNT_ONCE)   return (val > 0) ? 1 : 0;
#ifdef CAREFUL
		if (pos->second >= len)
			throw "pos->second out of range in RelCount::modify";
#endif

		int oldval = data[pos->second];
		int newval = oldval - val;
		data[pos->second] = newval;

		/* calculate costs */
		if (oldval > 0) {
			if (   val < 0) cost += -val;
			if (newval < 0) cost += -newval;
		} else
		if (oldval < 0) {
			if (   val > 0) cost += val;
			if (newval > 0) cost += newval;
		} else
		/* if (oldval == 0) */ {
			cost = abs(val); /* == abs(newval) */
		}
	} else
		throw "New key encountered during 'modify'.";
	return cost;
}

void RelCount::operator+=(const RelCount& other) {
	for (unsigned int i=0; i<len; i++)
		data[i] += other.data[i];
}

void RelCount::operator-=(const RelCount& other) {
	for (unsigned int i=0; i<len; i++)
		data[i] -= other.data[i];
}

std::ostream &operator<<(std::ostream &out, const RelCount& rc) {
	out << "[RC:";
	for (unsigned int i = 0; i < rc.len; i++) {
		out << rc.data[i] << ",";
	}
	out << "]";
	return out;
}

void RelCount::dumpIndex(std::ostream &out) {
	hashmap<RelEqClass, unsigned int, hash_releqc>::iterator i;
	for (i = cmap.begin(); i != cmap.end(); i++) {
		out << i->first << ": " << i->second << " ";
	}
}

int RelCount::calc_max_retained(hashmap<RelEqClass, int, hash_releqc>& map1, hashmap<RelEqClass, int, hash_releqc>& map2) {
	hashmap<RelEqClass, int, hash_releqc>::iterator i1, i2;
	int max_retained=0;
	
	for (i1 = map1.begin(); i1 != map1.end(); i1++) {
		i2 = map2.find(i1->first);

		if (i2 != map2.end()) {
			max_retained += min(i1->second, i2->second);
		}
	}
	return max_retained;
}
}
