/* ===========================================================================
 *        Filename:  doc.h
 *     Description:  Document class
 * 
 *         Version:  $Rev$
 *         Changed:  $Date$
 *         Licence:  GPL (read COPYING file for details)
 * 
 *          Author:  Erich Schubert (eS), erich@debian.org
 *                   Institut für Informatik, LMU München
 * ======================================================================== */
#ifndef  SSD_DOC_H
#define  SSD_DOC_H

#include "config.h"
#include "node.h"
#include "ustring.h"
#include "node_eqclass.h"
#include "util.h"

#include <vector>
#include <iostream>
#include <map>
#include <set>

#include <libxml/tree.h>
#include <libxml/xpath.h>

using namespace std;

namespace SSD {

/** \brief Document wrapper class
 *  this class wraps around a document, in the current version only XML
 *  documents are supported, but other similar formats might be added
 *  later on (by making Doc an abstract class and deriving from it */
class Doc {
protected:
	/** \brief document root node */
	Node*		root;
	/** \brief XML dom document tree for output reconstruction */
	xmlDocPtr	dom;

	/** \brief list containing all nodes in the document for iteration */
	NodeVec		nodes;

	/* process a node in the reader */
	/** \brief Walk the document tree recursively
	 *  This will transform the libXML dom tree into a tree
	 *  of SSD::Node objects
	 *  \param pos  current position in SSD::Node tree when walking
	 *  \param node current position in libxml tree */
	void walkTree(Node* pos, xmlNodePtr node);
	/** \brief Make a new element Node
	 *  \param parent parent node for new element
	 *  \param node libxml node information
	 *  \return new Node object for this node */
	Node* appendNodeElement(Node* parent, xmlNodePtr node);
	/** \brief Make a new "text" Node
	 *  \param parent parent node for new element
	 *  \param node libxml node information
	 *  \return new Node object for this node */
	Node* appendNodeText(Node* parent, xmlNodePtr node);
	/** \brief Make a new "attribute" Node
	 *  \param parent parent node for new element
	 *  \param node parent libxml node
	 *  \param attr libxml attribute
	 *  \return new Node object for this node */
	Node* appendNodeAttribute(Node* parent, xmlNodePtr node, xmlAttrPtr attr);

#ifdef NEED_INDEX
	/** \brief insert a node into the index structure
	 *  \param node Node to be added to index */
	void add_to_index(Node* node);
#endif
#ifdef NEED_PROCESSED_SET
	/** \brief add an libxml node into the processed set
	 *
	 *  this is used for reconstruction of the XML document; unprocessed
	 *  libxml nodes (such as ignored whitespace) are handeled differently
	 *  \param node libxml node to be added */
	void add_to_processed(xmlNodePtr node);
#endif
	
	/** \brief walk document by using an xpath expression
	 *
	 *  this is used to build the list of related nodes for a given XPath expression
	 *  \param xpathctx XPath context (allocated externally for efficiency)
	 *  \param xpath compiled XPath expression (compiled exernally for efficiency)
	 *  \param node reference node */
	void walkTreeXPath(xmlXPathContextPtr xpathctx, xmlXPathCompExprPtr xpath, Node* node);
public:
	/** \brief flag wheter to ignore whitespace or not */
	static bool useWhitespace;
#ifdef NEED_INDEX
	/** \brief index of nodes by label */
	NodeEqClassVec	index_by_label;
#endif
#ifdef NEED_PROCESSED_SET
	/** \brief set of libxml nodes processed (i.e. not ignored whitespace etc.) */
	std::set<void* >	processed;
#endif
	/** \brief map to find the Node object for a given libxml node */
	hashmap<xmlNodePtr, Node*, hashfun<void*> >	xml_to_node;
	/** \brief count of relations in file to calculate credits */
	hashmap<RelEqClass, int, hash_releqc>		relcount;

	/** \brief load an XML document using libxml
	 *  \param filename filename to be loaded
	 *  \return true on successful load */
	bool loadXML(const char* filename);
	/** \brief build related-to data for a given xpath
	 *  \param xpath XPath expression to be used */
	void processXPath(const char* xpath);
	/** \brief create an empty doc object */
	Doc();
	/** \brief drop document information */
	void flushDoc();
	/** \brief destructor */
	~Doc();
	/** \brief return root node for document */
	Node* getRoot() const { return root; }
	/** \brief return libxml DOM root node for document
	 *  this is used in output writers for reconstruction */
	xmlDocPtr getDOM() const { return dom; }
	/** \brief access to const iterators of nodes list */
	NodeVec::const_iterator getNodesIter() const { return nodes.begin(); }
	NodeVec::const_iterator getNodesIterEnd() const { return nodes.end(); }
};

}

#endif   /* ----- #ifndef SSDDOC_H  ----- */
