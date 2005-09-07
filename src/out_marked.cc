/* ===========================================================================
 *        Filename:  out_marked.cc
 *     Description:  the "marked" output format
 * 
 *         Version:  $Rev$
 *         Changed:  $Date$
 *         Licence:  GPL (read COPYING file for details)
 * 
 *          Author:  Erich Schubert (eS), erich@debian.org
 *                   Institut für Informatik, LMU München
 * ======================================================================== */
#include "config.h"
#include "doc.h"
#include "diff.h"
#include "util.h"
#include <libxml/tree.h>
#include "string.h"
#include "out_marked.h"

/* FIXME: attributes are not being marked */

using namespace SSD;
namespace SSD {

static void markNode(xmlNode* node, xmlChar* text, xmlNsPtr ns) {
	if (xmlNodeIsText(node)) {
		if (node->prev) {
			xmlSetNsProp(node->prev,ns,(xmlChar*)"f",text);
		} else if (node->parent) {
			xmlSetNsProp(node->parent,ns,(xmlChar*)"c",text);
		} else {
			cerr << "Textnode has neither prev sib nor parent" << endl;
		};
	} else {
		xmlSetNsProp(node,ns,(xmlChar*)"n",text);
	}
}

void MarkedWriter::run(Doc& d1, Doc& d2, DiffDijkstra& diff) {
	doc1 = &d1; doc2 = &d2;
	xmlNsPtr n1 = xmlNewNs(xmlDocGetRootElement(doc1->getDOM()),(xmlChar*)"http://xmldesign.de/XML/diff/mark/0.1",(xmlChar*)"di");
	xmlNsPtr n2 = xmlNewNs(xmlDocGetRootElement(doc2->getDOM()),(xmlChar*)"http://xmldesign.de/XML/diff/mark/0.1",(xmlChar*)"di");
	int i=0;
	char buffer[10];
	for (NodeAssignments* iter = diff.result->ass; iter; iter = iter->next) {
		snprintf(buffer,10,"%d",i);
		if (iter->n1 && iter->n2) {
			markNode((xmlNode*)iter->n1->data,(xmlChar*)buffer,n1);
			markNode((xmlNode*)iter->n2->data,(xmlChar*)buffer,n2);
			i++;
		} else if (iter->n1) {
#ifdef MARK_DELETED
			markNode((xmlNode*)iter->n1->data,(xmlChar*)"del",n1);
#endif
		}
	}
}

void MarkedWriter::dump() {
	xmlDocDump(stdout,doc1->getDOM());
	xmlDocDump(stdout,doc2->getDOM());
}
} // namespace SSD
