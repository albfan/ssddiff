/* ===========================================================================
 *        Filename:  out_merged.cc
 *     Description:  the "merged" output format
 * 
 *         Version:  $Rev$
 *         Changed:  $Date$
 *         Licence:  GPL (read COPYING file for details)
 * 
 *          Author:  Erich Schubert (eS), erich@debian.org
 *                   Institut für Informatik, LMU München
 * ======================================================================== */
#include <iostream>
#include <sstream>
#include <algorithm>
#include "config.h"
#include "doc.h"
#include "diff.h"
#include <libxml/tree.h>
#include "util.h"
#include "out_merged.h"
#include "out_common.h"

using namespace SSD;
namespace SSD {

#define NAMESPACE_MERGED (xmlChar*)"http://xmldesign.de/XML/diff/merged/0.1"

#define REM_ATTR   (xmlChar*)"attr-moved-away"
#define A_SEPARATOR (xmlChar*)";"

#define OUTPUT_FIRST  1
#define OUTPUT_SECOND 2
#define OUTPUT_BOTH   (OUTPUT_FIRST | OUTPUT_SECOND)

xmlChar* MergedWriter::stringsAction[] =
	{ NULL, (xmlChar*)"ins", (xmlChar*) "del",
	  (xmlChar*) "moved-away", (xmlChar*) "moved-here" };
xmlChar* MergedWriter::stringsRefer[] =
	{ (xmlChar*) "node", (xmlChar*) "content",
	  (xmlChar*) "following", (xmlChar*) "attr" };
xmlChar* MergedWriter::stringsSpecial[] = 
	{ (xmlChar*) "t-moved-here", (xmlChar*) "t-moved-away",
	  (xmlChar*) "t-inserted",   (xmlChar*) "t-deleted" };
xmlChar* MergedWriter::stringsAttributes[] = 
	{ (xmlChar*) "a-moved-here", (xmlChar*) "a-moved-away",
	  (xmlChar*) "a-inserted",   (xmlChar*) "a-deleted" };

enum { REFNODE, REFCONT, REFFOLLOW, REFATTR };
enum { TEXTMOVEDHERE, TEXTMOVEDAWAY, TEXTINSERTED, TEXTDELETED };
enum { ATTRMOVEDHERE, ATTRMOVEDAWAY, ATTRINSERTED, ATTRDELETED };

/* Marking text nodes is where it gets really messy.
 * First of all, two text nodes next to each other will collapse
 * into one, so we need to wrap one, when both are present.
 *
 * Secondly, we can't add attributes to text nodes, instead we need
 * to add the attributes to either the parent node, or the preceeding
 * sibling. */
void MergedWriter::markTextNode(xmlNode* pos, xmlNode* node, Action action, xmlNsPtr ns) {
/* TODO: strip wrappers when not needed */
	if (action == MOVEDAWAY) {
		/* text that is moved away should always be wrapped
		 * (unless of course it's moved along with it's parent)
		 * since that is more likely to keep the semantics somewhat
		 * intact. I mean, the text is no longer there, so it should
		 * be removed somehow. ;-) */
		xmlNodePtr wrap = xmlNewNode(ns, stringsSpecial[TEXTMOVEDAWAY]);
		xmlAddChild(pos,wrap);
		xmlAddChild(wrap, node);
	} else if (action == MOVEDHERE) {
		/* TODO: when to not add the wrapper? */
		xmlNodePtr wrap = xmlNewNode(ns, stringsSpecial[TEXTMOVEDHERE]);
		xmlAddChild(pos,wrap);
		xmlAddChild(wrap, node);
	} else if (action == DELETED) {
		xmlNodePtr wrap = xmlNewNode(ns, stringsSpecial[TEXTDELETED]);
		xmlAddChild(pos,wrap);
		xmlAddChild(wrap, node);
	} else if (action == INSERTED) {
		xmlNodePtr wrap = xmlNewNode(ns, stringsSpecial[TEXTINSERTED]);
		xmlAddChild(pos,wrap);
		xmlAddChild(wrap, node);
	} else {
		xmlAddChild(pos, node);
		if (node->prev) {
			xmlSetNsProp(node->prev,ns,stringsRefer[REFFOLLOW],stringsAction[action]);
		} else if (node->parent) {
			xmlSetNsProp(node->parent,ns,stringsRefer[REFCONT],stringsAction[action]);
		} else {
			throw "Textnode has neither prev sib nor parent!";
		};
	}
}

void MergedWriter::markNode(xmlNode* pos, xmlNode* node, Action action, xmlNsPtr ns) {
	if (xmlNodeIsText(node)) {
		markTextNode(pos, node, action, ns);
	} else {
		xmlAddChild(pos,node);
		xmlSetNsProp(node,ns,stringsRefer[REFNODE],stringsAction[action]);
	}
}

void MergedWriter::markAttribute(xmlNode* pos, xmlAttrPtr attr, Action action, xmlNsPtr ns) {
	xmlChar* astr;
	/* get string for this action */
	switch (action) {
		case MOVEDAWAY: astr = stringsAttributes[ATTRMOVEDAWAY]; break;
		case MOVEDHERE: astr = stringsAttributes[ATTRMOVEDHERE]; break;
		case DELETED:   astr = stringsAttributes[ATTRDELETED]; break;
		case INSERTED:  astr = stringsAttributes[ATTRINSERTED]; break;
		default:
			throw "Unknown action for attribute specified";
	}
	/* do we already have a value? */
	xmlChar* cur = xmlGetNsProp(pos, astr, ns->href);

	/* build new value for attribute */
	ostringstream buffer;
	if (cur)
		buffer << (char*)cur << A_SEPARATOR;
	if (attr->ns && attr->ns->prefix)
		buffer << (char*) attr->ns->prefix << ':';
	buffer << attr->name;

	xmlSetNsProp(pos, ns, astr, (xmlChar*) buffer.str().c_str());
}


void MergedWriter::diffAttributes(
	xmlNodePtr diff, xmlNsPtr ns,
	xmlNodePtr p1, xmlNodePtr p2,
	hashmap<xmlNodePtr, xmlNodePtr, hashfun<void*> >& map,
	set<xmlNodePtr>& known, int output_only)
{
	hashmap<xmlNodePtr, xmlNodePtr, hashfun<void*> >::iterator i;
	xmlAttrPtr a1, a2;
	xmlNodePtr remattr = NULL;
	if (p1)
	for (a1 = p1->properties; a1; a1 = a1->next) {
		i = map.find((xmlNodePtr)a1);
		if (i != map.end()) {
			a2 = (xmlAttrPtr) i->second;
			if (a2->parent == p2) {
				/* this is a kept attribute */
				if (a1->ns) {
					xmlNsPtr nsa = xmlSearchNsByHref(diff->doc, diff, a1->ns->href);
					if (!nsa) {
						nsa = xmlNewNs(diff, a1->ns->href, a1->ns->prefix);
					}
					xmlSetNsProp(diff, nsa, a1->name, xmlNodeListGetString(p1->doc,a1->children,1));
				} else {
					/* just copy the attribute */
					xmlSetProp(diff, a1->name, xmlNodeListGetString(p1->doc,a1->children,1));
				}
			} else {
				//cerr << "Attribute moved away: " << a1->name << endl;
				/* add the catcher node */
				if (!remattr) {
					remattr = xmlNewNode(ns, REM_ATTR);
					xmlAddChild(diff,remattr);
				}
				xmlNsPtr nsa = NULL;
				if (a1->ns) {
					nsa = xmlSearchNs(remattr->doc, remattr, a1->ns->prefix);
					if (!nsa) {
						nsa = xmlNewNs(remattr, a1->ns->href, a1->ns->prefix);
					}
				}
				xmlSetNsProp(remattr, nsa, a1->name, xmlGetNsProp(p1, a1->name, a1->ns ? a1->ns->prefix : NULL));
			}
		} else if (output_only & OUTPUT_FIRST) {
			//cerr << "Attribute deleted: " << a1->name << endl;
			if (p2) {
				markAttribute(diff, a1, DELETED, ns);
			}
			/* copy the deleted attribute */
			if (a1->ns) {
				xmlNsPtr nsa = xmlSearchNsByHref(diff->doc, diff, a1->ns->href);
				if (!nsa) {
					nsa = xmlNewNs(diff, a1->ns->href, a1->ns->prefix);
				}
				xmlSetNsProp(diff, nsa, a1->name, xmlNodeListGetString(p1->doc,a1->children,1));
			} else {
				/* just copy the attribute */
				xmlSetProp(diff, a1->name, xmlNodeListGetString(p1->doc,a1->children,1));
			}
		}
	}
	if (p2)
	for (a2 = p2->properties; a2; a2 = a2->next) {
		i = map.find((xmlNodePtr)a2);
		if (i != map.end()) {
			a1 = (xmlAttrPtr) i->second;
			if (a1->parent == p1) {
				/* this is a kept attribute */
			} else {
				//cerr << "Attribute moved here: " << a2->name << endl;
				/* copy the attribute here */
				if (a2->ns) {
					xmlNsPtr nsa = xmlSearchNsByHref(diff->doc, diff, a2->ns->href);
					if (!nsa) {
						nsa = xmlNewNs(diff, a2->ns->href, a2->ns->prefix);
					}
					xmlSetNsProp(diff, nsa, a2->name, xmlNodeListGetString(p2->doc,a2->children,1));
				} else {
					/* just copy the attribute */
					xmlSetProp(diff, a2->name, xmlNodeListGetString(p2->doc,a2->children,1));
				}
				markAttribute(diff, a2, MOVEDHERE, ns);
			}
		} else if (output_only & OUTPUT_SECOND) {
			//cerr << "Attribute inserted: " << a2->name << endl;
			if (p1) {
				markAttribute(diff, a2, INSERTED, ns);
			}
			/* copy the deleted attribute */
			if (a2->ns) {
				xmlNsPtr nsa = xmlSearchNsByHref(diff->doc, diff, a2->ns->href);
				if (!nsa) {
					nsa = xmlNewNs(diff, a2->ns->href, a2->ns->prefix);
				}
				xmlSetNsProp(diff, nsa, a2->name, xmlNodeListGetString(p2->doc,a2->children,1));
			} else {
				/* just copy the attribute */
				xmlSetProp(diff, a2->name, xmlNodeListGetString(p2->doc,a2->children,1));
			}
		}
	}
}

void MergedWriter::recCalcActions(xmlNodePtr diff, xmlNsPtr ns,
	xmlNodePtr p1, xmlNodePtr p2,
	hashmap<xmlNodePtr, xmlNodePtr, hashfun<void*> >& map,
	set<xmlNodePtr>& known, int output_only)
{
	hashmap<xmlNodePtr, xmlNodePtr, hashfun<void*> >::iterator i;
	xmlNodePtr pos1;
	xmlNodePtr pos2;

	/* for a good output we need the longest common subsequence at each level.
	 * this is easier here, since each node can match only once, this
	 * boils down to longest-increasing-subsequence */
	vector<pair<xmlNodePtr, xmlNodePtr> > lcs;
	{
		vector<pair<xmlNodePtr, xmlNodePtr> > l1;
		vector<xmlNodePtr> l2;
		for (xmlNodePtr i=p1; i; i=i->next) {
			hashmap<xmlNodePtr, xmlNodePtr, hashfun<void*> >::iterator f = map.find(i);
			if (f != map.end()) l1.push_back(make_pair(i,f->second));
		}
		for (xmlNodePtr i=p2; i; i=i->next)
			if (known.find(i) != known.end()) l2.push_back(i);
		calcLIS(l1, l2, lcs);
	}
	vector<pair<xmlNodePtr, xmlNodePtr> >::iterator lcsi = lcs.begin();
	pos1 = p1; pos2 = p2;

	while (pos1 || pos2) {
		/* Longest-Subsequence checkpoints */
		xmlNodePtr check1 = NULL;
		xmlNodePtr check2 = NULL;
		if (lcsi != lcs.end()) {
			check1 = lcsi->first;
			check2 = lcsi->second;
			lcsi++;
		}

		/* In order to keep a (hopefully) rather minimal diff, we try to keep the
		 * longest common subsequence (technically, a longest increasing subsequence)
		 * unmodified, instead moving things before and after these fixed elements.
		 *
		 * Therefore, we first remove all elements from the first document up to the LIS point,
		 * then add all nodes in the second up to the LIS point, then write the LIS point. */

		/* output all nodes from first document until LIS point */
		while (pos1 && (pos1 != check1)) {
			if (output_only & OUTPUT_FIRST) {
				/* ignored nodes are just dumped if applicable */
				if (known.find(pos1) == known.end()) {
					/* keep whitespace etc. from first document */
					xmlNodePtr copy = xmlCopyNode(pos1,2);
					xmlAddChild(diff,copy);
				} else {
					i = map.find(pos1);
					if (i != map.end()) {
						xmlNodePtr copy = xmlCopyNode(i->second,0);
						/* Nodes moved away */
						markNode(diff, copy, MOVEDAWAY, ns);
						diffAttributes(copy, ns, pos1, i->second, map, known, output_only & ~OUTPUT_SECOND);
						recCalcActions(copy, ns, pos1->children, i->second->children, map, known, output_only & ~OUTPUT_SECOND);
					} else {
						/* nodes removed altogether */
						xmlNodePtr copy = xmlCopyNode(pos1,0);
						/* TODO: why is this if needed? Won't this leak memory? */
						/*if (p2)*/ markNode(diff, copy, DELETED, ns);
						diffAttributes(copy, ns, pos1, NULL, map, known, output_only & ~OUTPUT_SECOND);
						recCalcActions(copy, ns, pos1->children, NULL, map, known, output_only & ~OUTPUT_SECOND);
					}
				}
			}
			pos1 = pos1->next;
		}
		
		while (pos2 && (pos2 != check2)) {
			if (output_only & OUTPUT_SECOND) {
				/* ignored nodes are just dumped if applicable */
				if (known.find(pos2) == known.end()) {
					/* ignore whitespace and similar nodes in second document */
					/* except when we are inserting subtrees or text would collapse */
					if (!xmlNodeIsText(xmlGetLastChild(diff))) {
						xmlNodePtr copy = xmlCopyNode(pos2,0);
						xmlAddChild(diff,copy);
					}
				} else {
					i = map.find(pos2);
					if (i != map.end()) {
						/* Nodes moved here */
						xmlNodePtr copy = xmlCopyNode(i->second,0);
						markNode(diff, copy, MOVEDHERE, ns);
						diffAttributes(copy, ns, i->second, pos2, map, known, output_only & ~OUTPUT_FIRST);
						recCalcActions(copy, ns, i->second->children, pos2->children, map, known, output_only & ~OUTPUT_FIRST);
					} else {
						/* nodes inserted */
						xmlNodePtr copy = xmlCopyNode(pos2,0);
						/* TODO: why is this if needed? Won't this leak memory? */
						/*if (p1)*/ markNode(diff, copy, INSERTED, ns);
						diffAttributes(copy, ns, NULL, pos2, map, known, output_only & ~OUTPUT_FIRST);
						recCalcActions(copy, ns, NULL, pos2->children, map, known, output_only & ~OUTPUT_FIRST);
					}
				}
			}
			pos2 = pos2->next;
		}

		/* we now arrived at the LIS point */
		if (pos1 && pos2) {
			if (output_only & (OUTPUT_FIRST | OUTPUT_SECOND)) {
				xmlNodePtr copy = xmlCopyNode(pos2,0);
				xmlAddChild(diff,copy);
				diffAttributes(copy, ns, pos1, pos2, map, known, output_only);
//**/					markNode(copy, (xmlChar*)"lcsi", ns);
				recCalcActions(copy, ns, pos1->children, pos2->children, map, known, output_only);
			}
			pos1 = pos1->next;
			pos2 = pos2->next;
		}
	}
}

void MergedWriter::run(Doc& doc1, Doc& doc2, DiffDijkstra& diff) {
	if (mergeddoc) xmlFreeDoc(mergeddoc);
	mergeddoc = xmlNewDoc((xmlChar*)"1.0");
	/* build a map for lookups node-in-second -> node-in-first */
	hashmap<xmlNodePtr, xmlNodePtr, hashfun<void*> > map_back;
	for (NodeAssignments* iter = diff.result->ass; iter; iter = iter->next) {
		if (iter->n1 && iter->n2) {
			map_back.insert(make_pair((xmlNode*)iter->n1->data,(xmlNode*)iter->n2->data));
			map_back.insert(make_pair((xmlNode*)iter->n2->data,(xmlNode*)iter->n1->data));
		}
	}

	set<xmlNodePtr>* k1 = (set<xmlNodePtr>*) &doc1.processed;
	set<xmlNodePtr>* k2 = (set<xmlNodePtr>*) &doc2.processed;

	/* merge maps; known is the lookup table of "XML nodes processed" (e.g. not containing whitespace) */
	set<xmlNodePtr> known;
	set_union(k1->begin(), k1->end(), k2->begin(), k2->end(),
			inserter(known, known.begin()));

	/* create new root node, with merged-doc namespace */
	xmlNodePtr root = xmlNewNode(NULL,(xmlChar*)"merged-doc");
	xmlDocSetRootElement(mergeddoc,root);

	xmlNsPtr ns = xmlNewNs(root, NAMESPACE_MERGED, (xmlChar*)"md");
	xmlSetNs(root,ns);

	/* calculate actions */
	recCalcActions(root,ns,xmlDocGetRootElement(doc1.getDOM()), xmlDocGetRootElement(doc2.getDOM()), map_back, known, OUTPUT_BOTH);

	/* strip extra root node if possible */
	if (root->children && !(root->children->next)) {
		xmlNodePtr newroot = root->children;

		/* re-root the tree */
		xmlUnlinkNode(root);
		xmlUnlinkNode(newroot);
		xmlDocSetRootElement(mergeddoc,newroot);
		/* fix namespaces */
		xmlReconciliateNs(mergeddoc, newroot);
		/* remove old root */
		xmlFreeNode(root);
		root=newroot;
	}
}

void MergedWriter::dump() {
	xmlDocDump(stdout,mergeddoc);
}

MergedWriter::~MergedWriter() {
	if (mergeddoc) xmlFreeDoc(mergeddoc); mergeddoc = NULL;
}

} // namespace SSD
