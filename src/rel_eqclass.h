/* ===========================================================================
 *        Filename:  rel_eqclass.h
 *     Description:  Relation equality class
 * 
 *         Version:  $Rev$
 *         Changed:  $Date$
 *         Licence:  GPL (read COPYING file for details)
 * 
 *          Author:  Erich Schubert (eS), erich@debian.org
 *                   Institut für Informatik, LMU München
 * ======================================================================== */
#ifndef  SSD_REL_EQCLASS_H
#define  SSD_REL_EQCLASS_H
#include "config.h"
#include "ustring.h"

namespace SSD {

class Node;
class NodeEqClass;

/** \brief Relation Equality Class - get the equality class for two given nodes */
/** this requires unified strings to work */
class RelEqClass {
	/** \brief first label   */
	ustring fl;
	/** \brief first content */
	ustring fc;
	/** \brief first label   */
	ustring sl;
	/** \brief first content */
	ustring sc;
public:
	/** \brief get class for two nodes by reference */
	/** \param n1 first node
	 *  \param n2 second node */
	RelEqClass(const Node& n1, const Node& n2);
	/** \brief get class for two nodes by pointer */
	/** \param n1 first node
	 *  \param n2 second node */
	RelEqClass(const Node* n1, const Node* n2);
	/** \brief get class for a node and a given equality class */
	/** \param n1 node
	 *  \param n2 equality class */
	RelEqClass(const Node& n1, const NodeEqClass& n2);
	/** \brief get class for a given equality class and a node */
	/** \param n1 equality class
	 *  \param n2 node */
	RelEqClass(const NodeEqClass& n1, const Node& n2);

	/** \brief strict weak ordering for sorted maps */
	/** \param r2 class to be compared with */
	bool operator<(const RelEqClass r2) const {
		if (fl != r2.fl) return fl < r2.fl;
		if (fc != r2.fc) return fc < r2.fc;
		if (sl != r2.sl) return sl < r2.sl;
		                 return sc < r2.sc;
	}
	/** \brief test for equality */
	/** \param r2 class to be compared with */
	bool operator==(const RelEqClass r2) const {
		return (fl == r2.fl) && (fc == r2.fc) && (sl == r2.sl) && (sc == r2.sc);
	}

	/** \brief hash function for maps */
	size_t hash() const {
		size_t h2 = fc.hash();
		size_t h3 = sl.hash();
		size_t h4 = sc.hash();
		return fl.hash() ^ (h2 << 16) ^ (h2 >> 16) ^ (h3 << 8) ^ (h3 >> 24) ^ (h4 << 24) ^ (h4 >> 8); }

	/** \brief helper function to allow dumping onto output streams */
	/** \param out output stream to be written to
	 *  \param cp Class to be written */
	friend std::ostream &operator<<(std::ostream &out, const SSD::RelEqClass& cp);
};

/** \brief hash functor for relation equality classes */
struct hash_releqc {
	/** \brief hash function for relation equality classes */
	size_t operator()(const RelEqClass req) const { return req.hash(); };
};
}
#endif
