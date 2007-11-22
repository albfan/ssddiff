/* ===========================================================================
 *        Filename:  doc.cc
 *     Description:  Doc with XML loading capabilities
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
#include <iostream>

#include <libxml/encoding.h>
#include <libxml/xmlreader.h>
#include <libxml/xmlIO.h>
#include <libxml/tree.h>

#include <libxml/xpath.h>

namespace SSD {

bool Doc::useWhitespace = false;

/* clean the loaded document */
void
Doc::flushDoc() {
	// clean the document
	if (root) {
		delete root; root=NULL;
	}
	if (dom) {
		xmlFreeDoc(dom); dom=NULL;
	}
	nodes.clear();
}

void
Doc::walkTreeXPath(xmlXPathContextPtr xpathctx, xmlXPathCompExprPtr xpath, Node* node) {
#ifdef CAREFUL
	if (!node) throw "Doc::walkTreeXPath called without a node";
#endif

	xpathctx->node = (xmlNodePtr) node->data;

	xmlXPathObjectPtr xpathobj = xmlXPathCompiledEval( xpath, xpathctx );
	if (!xpathobj) throw "Doc::walkTreeXPath: xmlXPathCompiledEval failed";
	//if (!xpathobj->nodesetval) throw "Doc::walkTreeXPath: xmlXPathCompiledEval didn't return result";
	if (xpathobj->nodesetval) {
		if (xpathobj->nodesetval->nodeNr > 0)
			if (!node->reldown) node->reldown = new NodeVec;
		for (int i = 0; i < xpathobj->nodesetval->nodeNr; i++) {
			xmlNodePtr cur = xpathobj->nodesetval->nodeTab[i];
			/* find the corresponding Node in our data structure */
			if (xml_to_node.find(cur) != xml_to_node.end()) {
				Node* reln = xml_to_node[cur];
				/* register node as related */
				node->reldown->push_back(reln);
				if (!reln->relup)
					reln->relup = new NodeVec;
				reln->relup->push_back(node);
				/* do document relation count */
				RelEqClass key(node, reln);
				
				hash_map<RelEqClass, int, hash_releqc>::iterator pos = relcount.find(key);
				if (pos != relcount.end()) {
					relcount[key]++;
				} else {
					relcount.insert(make_pair(key,1));
				}
			}
		}
	}
	xmlXPathFreeObject(xpathobj);

	/* recurse into children */
	if (node->children)
		for (NodeVec::iterator i = node->children->begin(); i != node->children->end(); i++)
			walkTreeXPath(xpathctx, xpath, *i);
}

bool
Doc::loadXML(const char* filename) {
	xmlNodePtr rootn = NULL;
	if (dom) { flushDoc(); }

	dom = xmlParseFile(filename);

	if (!dom)
		throw "Couldn't load document";
	if (!dom->doc)
		throw "Couldn't load document - no doc";
	if (! (rootn = xmlDocGetRootElement(dom->doc)) )
		throw "Couldn't load document - no root";

	walkTree(NULL,rootn);

	return true;
}

void
Doc::processXPath(const char* xp) {
	/* generate xpath setup */
	xmlXPathContextPtr xpathctx = xmlXPathNewContext(dom);
	if (!xpathctx) throw "Doc::processXPath - xmlXPathNewContext failed.";
	xmlXPathCompExprPtr xpath = xmlXPathCtxtCompile(xpathctx, BAD_CAST xp);
	if (!xpath) throw "Doc::processXPath - xmlXPathCtxtCompile failed. Invalid xpath expression.";

	/* collect relations */
	walkTreeXPath(xpathctx, xpath, root);

	xmlXPathFreeCompExpr(xpath);
	xmlXPathFreeContext(xpathctx);
}

#ifdef NEED_INDEX
void
Doc::add_to_index(Node* node) {
	index_by_label[NodeEqClass(node)].push_back(node);
}
#endif

#ifdef NEED_PROCESSED_SET
void Doc::add_to_processed(xmlNodePtr node) {
	processed.insert((void*) node);
}
#endif

void
Doc::walkTree(Node* pos, xmlNodePtr node) {
	Node* newnode=NULL;
	xmlAttrPtr attr=NULL;
	while (node) {
		switch(node->type) {
		case XML_ELEMENT_NODE:
			newnode = appendNodeElement(pos,node);
#ifdef NEED_PROCESSED_SET
			add_to_processed(node);
#endif
			xml_to_node.insert(make_pair(node,newnode));
			/* put into nodes vector */
			nodes.push_back(newnode);
			if (!root) { root = newnode; }
#ifdef NEED_INDEX
			add_to_index(newnode);
#endif

			// parse attributes
			attr = node->properties;
			while(attr) {
				Node* newattr = appendNodeAttribute(newnode,node,attr);
#ifdef NEED_PROCESSED_SET
				add_to_processed((xmlNodePtr)attr);
#endif
				xml_to_node.insert(make_pair((xmlNodePtr)attr,newattr));
				/* put into nodes vector */
				nodes.push_back(newattr);
#ifdef NEED_INDEX
				add_to_index(newattr);
#endif
				attr = attr->next;
			}

			walkTree(newnode,node->children);
			break;
		case XML_TEXT_NODE:
			newnode = appendNodeText(pos,node);
			if (newnode) {
#ifdef NEED_PROCESSED_SET
				add_to_processed(node);
#endif
				xml_to_node.insert(make_pair(node,newnode));
				/* put into nodes vector */
				nodes.push_back(newnode);
#ifdef NEED_INDEX
				add_to_index(newnode);
#endif
			}
			break;
		case XML_COMMENT_NODE:
			/* not really supported either, but we assume that we may
			 * just ignore comments */
			break;
		default:
			std::cerr << "Unsupported node type: " << node->type << std::endl;
		}
		node = node->next;
	}
}

Node*
Doc::appendNodeElement(Node* parent, xmlNodePtr node) {
	ustring name(node->name);
#ifdef CAREFUL
	if (name.empty()) {
		std::cerr << "Element node without text!"<< std::endl;
		return NULL;
	}
#endif
	/* create the new node */
	Node* newnode = new Node(name,empty_ustring,parent,node);
	if (parent) parent->addChild(newnode);
//	std::cout << "Added labeled node '" << name << "'" << std::endl;
	return newnode;
}

Node*
Doc::appendNodeText(Node* parent, xmlNodePtr node) {
	xmlChar* content = xmlNodeGetContent(node);
	ustring value(content);
	if (content) xmlFree(content);
	if (!useWhitespace && value.empty()) return NULL;

	/* create the new node */
	Node* newnode = new Node(empty_ustring,value,parent,node);
#ifdef CAREFUL
	if (!parent) throw "No parent given for text node.";
#endif
	parent->addChild(newnode);
//	std::cout << "Added text node" << std::endl;
	return newnode;
}

Node*
Doc::appendNodeAttribute(Node* parent, xmlNodePtr node, xmlAttrPtr attr) {
	ustring name(attr->name);
	ustring value(xmlNodeListGetString(node->doc,attr->children,1));
#ifdef CAREFUL
	if (name.empty()) {
		std::cerr << "Attribute node without name... " << std::endl;
		return NULL;
	}
#endif

	/* create the new node */
	Node* newnode = new Node(name,value,parent,attr);
#ifdef CAREFUL
	if (!parent) throw "No parent given von Attribute node.";
#endif
	parent->addChild(newnode);
//	std::cout << "Added attribute node " << name << "='" << value << "'" << std::endl;
	return newnode;
}

/* destructor, flushing contents */
Doc::~Doc() {
	flushDoc();
}

/* constructor */
Doc::Doc() : root(NULL), dom(NULL)
{
	// nothing to do.
}

} /* Namespace SSD */
