/* ===========================================================================
 *        Filename:  node.h
 *     Description:  Simple Node class
 * 
 *         Version:  $Rev$
 *         Changed:  $Date$
 *         Licence:  GPL (read COPYING file for details)
 * 
 *          Author:  Erich Schubert (eS), erich@debian.org
 *                   Institut für Informatik, LMU München
 * ======================================================================== */
#ifndef  SSDTREE_H
#define  SSDTREE_H
#include <vector>
#include <iostream>
// for std::pair
#include <utility>
#include <map>

#include "config.h"
#include "rel_eqclass.h"
#include "rel_count.h"

using namespace std;

namespace SSD {

class Node;

/** \brief Explicit notation is easier to read and easier to change */
typedef vector<Node*>	NodeVec;

/** \brief Class encapsulating a single node
 *  this is an abstraction layer away from the libxml data structure
 *  while allowing at the same time storage of additional information */
class Node {
public:
	/** \brief node label (for elements and attributes) */
	ustring		label;
	/** \brief node content (for text nodes and attributes) */
	ustring		content;
	/** \brief nodes related to this in "upward" direction */
	NodeVec*	relup;
	/** \brief nodes related to this in "down" direction */
	NodeVec*	reldown;
	/** \brief document tree children of this node */
	NodeVec*	children;
	/** \brief document tree parent of this node */
	Node*		parent;

	/** \brief additional data (for example underlying libxml node) */
	void*		data;
	/** \brief make a new node from given data
	 *  \param l Label of the node
	 *  \param c Text content of the node
	 *  \param par Parent node
	 *  \param d Additional data (for example underlying libxml node) */
	Node(ustring l, ustring c, Node* par, void* d) :
		label(l), content(c),
		relup(NULL), reldown(NULL),
		children(NULL), parent(par), data(d)
		{};
	/** \brief add a child to the nodes child list */
	void addChild(Node* node);
	/** \brief free node structure */
	~Node();

	/** \brief print node information to stream
	 *  \param st stream to append to
	 *  \param n node to append to stream
	 *  \return stream with output appended */
	friend ostream &operator<<(ostream &st, const Node &n);
};
}

#endif   /* ----- #ifndef SSDTREE_H  ----- */
