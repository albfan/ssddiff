/* ===========================================================================
 *        Filename:  node.c
 *     Description:  A simple node class
 * 
 *         Version:  $Rev$
 *         Changed:  $Date$
 *         Licence:  GPL (read COPYING file for details)
 * 
 *          Author:  Erich Schubert (eS), erich@debian.org
 *                   Institut für Informatik, LMU München
 * ======================================================================== */
#include "node.h"

namespace SSD {

/* simple vector<Node*>* access wrapper */
void
Node::addChild(Node* node) {
	if (!node) throw "Node::addChild(NULL) called";
	if (!children) children = new NodeVec;
	children->push_back(node);
}

/* clean up data on destruction */
Node::~Node() {
	NodeVec::iterator iter;
	if (children) {
		for (iter=children->begin(); iter != children->end(); iter++)
			delete(*iter);
		children->clear();
		delete children;
	}
	if (relup)   delete(relup);
	if (reldown) delete(reldown);
}

/* nice output for debugging */
std::ostream &operator<<(std::ostream &out, const Node &node) {
	return out << "Node(" << node.label << "," << node.content << ")";
}

}
