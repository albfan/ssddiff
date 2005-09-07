/* ===========================================================================
 *        Filename:  RelEqClass.cc
 *     Description:  SSD Relation equality class
 * 
 *         Version:  $Rev$
 *         Changed:  $Date$
 *         Licence:  GPL (read COPYING file for details)
 * 
 *          Author:  Erich Schubert (eS), erich@debian.org
 *                   Institut für Informatik, LMU München
 * ======================================================================== */
#include "rel_eqclass.h"
#include "node.h"
#include "node_eqclass.h"

namespace SSD {

RelEqClass::RelEqClass(const Node& n1, const Node& n2)
: fl(n1.label), fc(n1.content), sl(n2.label), sc(n2.content) { }

RelEqClass::RelEqClass(const Node* n1, const Node* n2)
: fl(n1->label), fc(n1->content), sl(n2->label), sc(n2->content) { }

RelEqClass::RelEqClass(const Node& n1, const NodeEqClass& n2)
: fl(n1.label), fc(n1.content), sl(n2.label), sc(n2.content) { }

RelEqClass::RelEqClass(const NodeEqClass& n1, const Node& n2)
: fl(n1.label), fc(n1.content), sl(n2.label), sc(n2.content) { }

std::ostream &operator<<(std::ostream &out, const RelEqClass& ec) {
    return out << "(" << ec.fl << "," << ec.fc << "," << ec.sl << "," << ec.sc << ")";
}

}
