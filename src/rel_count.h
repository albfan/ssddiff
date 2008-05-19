/* ===========================================================================
 *        Filename:  rel_count.h
 *     Description:  Relation Count class
 * 
 *         Version:  $Rev$
 *         Changed:  $Date$
 *         Licence:  GPL (read COPYING file for details)
 * 
 *          Author:  Erich Schubert (eS), erich@debian.org
 *                   Institut für Informatik, LMU München
 * ======================================================================== */
#ifndef  SSD_REL_COUNT_H
#define  SSD_REL_COUNT_H
#include "config.h"
#include "rel_eqclass.h"
#include <vector>
#include <climits>

using namespace std;

namespace SSD {

#define RELCOUNT_UNIQUE (UINT_MAX - 1)
#define RELCOUNT_ONCE   (UINT_MAX - 2)

/** \brief relation count class */
/** This class plays an essential role in the improved cost functions.
 *  To find out more about its use, read the algorithm whitepapers.
 *
 *  Basically, this is a map<RelEqClass, int> - mapping node pairs to
 *  an estimation on how many relations we are going to lose.
 *
 *  Since each state will have such an object, this is a place worthy
 *  of optimization, especially WRT memory usage.
 *
 *  For this we use a static map from RelEqClasses to index numbers,
 *  and an short array to store the actual values. This reduces memory
 *  waste for duplicate hash maps etc.
 *
 *  Also we do not store entries for the case that there is only one
 *  relation of this class at most in one document. Then we can use this
 *  as indicator if we have already lost it or not directly.
 */
class RelCount {
private:
	/** \brief number of releqclasses we need to track */
	static unsigned int len;
	/** \brief static map from class to index number */
	static hashmap<RelEqClass, unsigned int, hash_releqc> cmap;
	/** \brief data storage. would be interesting to save the memory for this pointer, too... */
	short*	data;
public:
	/** \brief make an empty RelCount */
	RelCount();
	/** \brief make an initial RelCount by making the difference between two maps */
	/** this will initialize the static cmap used as hash. Never process two diffs at the same time!
	 *  Always use this once and make this the first RelCount object you create!
	 *  \param map1 relation counts in first document
	 *  \param map2 relation counts in second document */
	RelCount(hashmap<RelEqClass, int, hash_releqc>& map1, hashmap<RelEqClass, int, hash_releqc>& map2);
	/** \brief make a copy of an RelCount dataset */
	RelCount(RelCount& rc);
	/** \brief delete a RelCount dataset */
	~RelCount();
	/** \brief add a RelCount dataset to this set */
	/** \param other dataset to be added */
	void operator+=(const RelCount& other);
	/** \brief substract a RelCount dataset to this set */
	/** \param other dataset to be substracted */
	void operator-=(const RelCount& other);
	/** \brief modify a single value in the dataset */
	/** \param key relation class to be updated
	 *  \param val relative change to be done
	 *  \return costs caused */
	int  modify(RelEqClass key, short val);
	/* for debug output */
	/** \brief append to an output stream */
	/** \param out output stream to be appended to
	 *  \param rc RelCount dataset to be dumped */
	friend std::ostream &operator<<(std::ostream &out, const RelCount& rc);
	/** \brief dumping helper */
	/** \param out stream to be dumped to */
	static void dumpIndex(std::ostream &out);
	/** \brief calc maximum retaintable prediction
	 *  \param map1 relation counts in first document
	 *  \param map2 relation counts in second document */
	static int calc_max_retained(hashmap<RelEqClass, int, hash_releqc>& map1, hashmap<RelEqClass, int, hash_releqc>& map2);
};

}
#endif
