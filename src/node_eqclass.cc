/* ===========================================================================
 *        Filename:  node_eqclass.cc
 *     Description:  Node equality class
 * 
 *         Version:  $Rev$
 *         Changed:  $Date$
 *         Licence:  GPL (read COPYING file for details)
 * 
 *          Author:  Erich Schubert (eS), erich@debian.org
 *                   Institut für Informatik, LMU München
 * ======================================================================== */
#include "node_eqclass.h"

namespace SSD {

NodeEqClass::NodeEqClass(Node* n) :
	label(n->label), content(n->content) { }

NodeEqClass::NodeEqClass(Node& n) :
	label(n.label), content(n.content) { }

std::ostream &operator<<(std::ostream &out, const NodeEqClass ec) {
    return out << "NEqC(" << ec.label << "," << ec.content << ")";
}

}
