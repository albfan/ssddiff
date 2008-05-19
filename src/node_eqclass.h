/* ===========================================================================
 *        Filename:  node_eqclass.h
 *     Description:  Node equality class
 * 
 *         Version:  $Rev$
 *         Changed:  $Date$
 *         Licence:  GPL (read COPYING file for details)
 * 
 *          Author:  Erich Schubert (eS), erich@debian.org
 *                   Institut für Informatik, LMU München
 * ======================================================================== */
#ifndef  SSD_NODE_EQCLASS_H
#define  SSD_NODE_EQCLASS_H

#include "config.h"

#include "ustring.h"
#include "node.h"

namespace SSD {

/** \brief Key for a node equality class, and hash function
 *
 * Calculates the node equality class for a given node, provides an ordering
 * for these classes and a primitive hash function suiteable for indexing.
 */
class NodeEqClass {
public:
	/** \brief node label */
	ustring label;
	/** \brief node content */
	ustring content;

	/** \brief get class for a given node
	 * \param n node to calculate the clayss for */
	NodeEqClass(Node* n);
	/** \brief get class for a given node
	 * \param n node to calculate the clayss for */
	NodeEqClass(Node& n);

	/** \brief ordering needed for sorted maps
	 * 
	 * Since this uses the ustring order, which is a memory order
	 * the sequence will vary from run to run. This randomization
	 * should give good performance on average, too.
	 * \param r2 node equality class to compare to
	 * \return comparison result
	 * */
	bool operator<(const NodeEqClass r2) const {
		if (label != r2.label) return label < r2.label;
		return content < r2.content;
	}
	/** \brief test equality of two classes
	 * \param r2 nodeeqclass to compare to
	 * \return comparison result
	 * */
	bool operator==(const NodeEqClass r2) const {
		return (label == r2.label) && (content == r2.content);
	}

	/** \brief hash function for equality classes
	 * this uses the string hash function, which supposedly is
	 * very efficient, being the pointer of the strings themselves
	 * \return hash value
	 * */
	size_t hashfun() const {
		size_t h2 = content.hashfun();
		/* avoid certain typical patterns that can occur when
		 * using a 1:1 XOR */
		return label.hashfun() ^ (h2 << 16) ^ (h2 >> 16); }

	/** \brief serialization to output streams
	 * \param out output stream
	 * \param cp node equality class to be printed
	 * \return updated stream
	 * */
	friend std::ostream &operator<<(std::ostream &out, const SSD::NodeEqClass& cp);
};

/** \brief C++ hash function object
 * uses the has function supplied in the object code */
struct hash_NEqC {
	/** \brief hash function call 
	 * \param neqc Node equality class
	 * \return hash value as obtained by NodeEqClass:hashfun()
	 * */
	size_t operator()(const NodeEqClass neqc) const { return neqc.hashfun(); };
};

/** \brief shorthand for template construct */
typedef
unordered_map< NodeEqClass, NodeVec, hash_NEqC > NodeEqClassVec;

}
#endif
