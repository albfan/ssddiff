/* ===========================================================================
 *        Filename:  out_xupdate.cc
 *     Description:  the "xupdate" output format
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
#include "config.h"
#include "doc.h"
#include "diff.h"
#include <libxml/tree.h>
#include "util.h"
#include "out_xupdate.h"
#include "out_common.h"

using namespace SSD;
namespace SSD {

#define NAMESPACE_XUPDATE (xmlChar*)"http://www.xmldb.org/xupdate"
#define XUPDATE_VARIABLE  (xmlChar*)"variable"
#define XUPDATE_VARNAME   (xmlChar*)"name"
#define XUPDATE_VARSEL    (xmlChar*)"select"
#define XUPDATE_REMOVE    (xmlChar*)"remove"
#define XUPDATE_REMSEL    (xmlChar*)"select"
#define XUPDATE_APPEND    (xmlChar*)"append"
#define XUPDATE_APPSEL    (xmlChar*)"select"
#define XUPDATE_INSERTA   (xmlChar*)"insert-after"
#define XUPDATE_INSASEL   (xmlChar*)"select"
#define XUPDATE_INSERTB   (xmlChar*)"insert-before"
#define XUPDATE_INSBSEL   (xmlChar*)"select"
#define XUPDATE_VALUE     (xmlChar*)"value-of"
#define XUPDATE_VALUESEL  (xmlChar*)"select"
#define XUPDATE_ELEMENT   (xmlChar*)"element"
#define XUPDATE_ELEMNAME  (xmlChar*)"name"
#define XUPDATE_TEXT      (xmlChar*)"text"

#define DUMMY_TAG         (xmlChar*)"dummy"

#define A_INSERTED   (xmlChar*)"a-ins"
#define A_DELETED    (xmlChar*)"a-del"
#define A_MOVED_AWAY (xmlChar*)"a-m-away"
#define A_MOVED_HERE (xmlChar*)"a-m-here"
#define A_SEPARATOR (xmlChar*)";"

/* make an xpath describing the location of pos (helper) */
void XUpdateWriter::makePathBuf(xmlBufferPtr buf, xmlNodePtr pos) {
	const xmlChar* label=NULL;
	int count=1;
	/* get node label or use text() */
	if (pos == (xmlNodePtr) pos->doc) {
		label=BAD_CAST "";
	} else
	if (xmlNodeIsText(pos)) {
		label = (const xmlChar*) "text()";
		for (xmlNodePtr p = pos->prev; p; p=p->prev)
			if (xmlNodeIsText(p)) count++;
	} else {
		label = pos->name;
		for (xmlNodePtr p = pos->prev; p; p=p->prev)
			if (p->name && (strcmp((char*)label, (char*)p->name) == 0)) count++;
	}

	/* construct xpath */
	/* we are using two buffers here: in a stringstream we can add+convert numbers */
	ostringstream buffer;
	buffer << "/" << label << "[" << count << "]";
	/* we construct the buffer child-to-parent, thus use addhead */
	xmlBufferAddHead(buf, (xmlChar*) buffer.str().c_str(), buffer.str().length());
	/* ascend to parent */
	if (pos->parent && (pos->parent != (xmlNodePtr)pos->doc))
		makePathBuf(buf, pos->parent);
}

/* make an xpath describing the location of pos */
xmlChar* XUpdateWriter::makePath(xmlNodePtr pos) {
#ifdef CAREFUL
	if (!pos) throw "XUpdateWrite::makePath called with NULL pos";
#endif
	xmlBufferPtr buf = xmlBufferCreate();

	makePathBuf(buf, pos);

	xmlChar* result = (xmlChar*)strdup((char*)xmlBufferContent(buf));
	xmlBufferFree(buf);
	return result;
}

/* recursively initialize a map of nodes from one document to its cloned copy */
void XUpdateWriter::initCloneMap(xmlNodePtr r1, xmlNodePtr r3) {
	/* insert nodes into hashmap */
	map_clone.insert(make_pair(r1,r3));
	map_clone_back.insert(make_pair(r3,r1));

	/* recurse into children */
	xmlNodePtr pos1 = r1->children;
	xmlNodePtr pos3 = r3->children;
	while (pos1 && pos3) {
		initCloneMap(pos1, pos3);
		pos1 = pos1->next;
		pos3 = pos3->next;
	}
	/* FIXME: process attributes, ensure they have the same ordering */
}


#define INSERT_MODE_CHILD 1
#define INSERT_MODE_FOSIB 2
#define INSERT_MODE_PRSIB 3
#define INSERT_MODE_APPEND 4

/* print the command to store a subtree */
xmlNodePtr XUpdateWriter::writeStoreSubtree(int num, const xmlChar* path) {
	xmlNodePtr cmd = xmlNewNode(ns, XUPDATE_VARIABLE);
	std::ostringstream label;
	label << "m" << num;
	xmlSetProp(cmd,XUPDATE_VARNAME,(xmlChar*) label.str().c_str());
	xmlSetProp(cmd,XUPDATE_VARSEL, path);
	xmlAddChild(output,cmd);
	xmlNodeAddContent(output,(xmlChar*)"\n");
	return cmd;
}

xmlNodePtr XUpdateWriter::writeDeleteSubtree(const xmlChar* path) {
	xmlNodePtr cmd = xmlNewNode(ns, XUPDATE_REMOVE);
	xmlSetProp(cmd,XUPDATE_REMSEL,path);
	xmlAddChild(output,cmd);
	xmlNodeAddContent(output,(xmlChar*)"\n");
	return cmd;
}

xmlNodePtr XUpdateWriter::writeAppend(const xmlChar* path) {
	xmlNodePtr cmd = xmlNewNode(ns, XUPDATE_APPEND);
	xmlSetProp(cmd,XUPDATE_APPSEL,path);
	xmlAddChild(output,cmd);
	xmlNodeAddContent(output,(xmlChar*)"\n");
	return cmd;
}

xmlNodePtr XUpdateWriter::writeInsertAfter(const xmlChar* path) {
	xmlNodePtr cmd = xmlNewNode(ns, XUPDATE_INSERTA);
	xmlSetProp(cmd,XUPDATE_INSASEL,path);
	xmlAddChild(output,cmd);
	xmlNodeAddContent(output,(xmlChar*)"\n");
	return cmd;
}

xmlNodePtr XUpdateWriter::writeInsertBefore(const xmlChar* path) {
	xmlNodePtr cmd = xmlNewNode(ns, XUPDATE_INSERTB);
	xmlSetProp(cmd,XUPDATE_INSBSEL,path);
	xmlAddChild(output,cmd);
	xmlNodeAddContent(output,(xmlChar*)"\n");
	return cmd;
}

xmlNodePtr XUpdateWriter::writeValueOf(xmlNodePtr where, const xmlChar* path) {
	xmlNodePtr cmd = xmlNewNode(ns, XUPDATE_VALUE);
	xmlSetProp(cmd,XUPDATE_VALUESEL,path);
	xmlAddChild(where,cmd);
//	xmlNodeAddContent(where,(xmlChar*)"\n");
	return cmd;
}

xmlNodePtr XUpdateWriter::writeElement(xmlNodePtr where, const xmlChar* elem) {
	xmlNodePtr cmd = xmlNewNode(ns, XUPDATE_ELEMENT);
	xmlSetProp(cmd,XUPDATE_ELEMNAME,elem);
	xmlAddChild(where,cmd);
//	xmlNodeAddContent(where,(xmlChar*)"\n");
	return cmd;
}

xmlNodePtr XUpdateWriter::writeText(xmlNodePtr where) {
	xmlNodePtr cmd = xmlNewNode(ns, XUPDATE_TEXT);
	xmlAddChild(where,cmd);
//	xmlNodeAddContent(where,(xmlChar*)"\n");
	return cmd;
}

#define OUTPUT_NONE	0
#define OUTPUT_FIRST	1
#define OUTPUT_SECOND	2
#define OUTPUT_BOTH	3

/* make an insert command, dependant on the value of insert_mode */
xmlNodePtr XUpdateWriter::doInsertCmd(xmlNodePtr insertpos, int insert_mode) {
	xmlNodePtr cmd;
	if (!insertpos) throw "XUpdateWrite::doInsertCmd called with NULL insertpos";
	if (insert_mode == INSERT_MODE_PRSIB) {
		xmlChar* path = makePath(insertpos);
		cmd = writeInsertBefore(path);
		free(path);
	} else if (insert_mode == INSERT_MODE_FOSIB) {
		xmlChar* path = makePath(insertpos);
		cmd = writeInsertAfter(path);
		free(path);
	} else if (insert_mode == INSERT_MODE_APPEND) {
		xmlChar* path = makePath(insertpos);
		cmd = writeAppend(path);
		free(path);
	} else if (insert_mode == INSERT_MODE_CHILD) {
		if (insertpos->children) {
			xmlChar* path = makePath(insertpos->children);
			cmd = writeInsertBefore(path);
			free(path);
		} else {
			xmlChar* path = makePath(insertpos);
			cmd = writeAppend(path);
			free(path);
		}
	} else throw "Undefined insert_mode";
	return cmd;
}

/* execute an insert command, dependant on the value of insert_mode */
void XUpdateWriter::execInsert(xmlNodePtr insertpos, xmlNodePtr data, int insert_mode) {
	/* are we changing the root node? */
	if (insertpos == (xmlNodePtr) insertpos->doc) {
		xmlDocSetRootElement(insertpos->doc,data);
	/* other insertion modes */
	} else if (insert_mode == INSERT_MODE_PRSIB) {
		xmlAddPrevSibling(insertpos,data);
	} else if (insert_mode == INSERT_MODE_FOSIB) {
		xmlAddNextSibling(insertpos,data);
	} else if (insert_mode == INSERT_MODE_APPEND) {
		xmlAddChild(insertpos,data);
	} else if (insert_mode == INSERT_MODE_CHILD) {
		if (insertpos->children) {
			xmlAddPrevSibling(insertpos->children,data);
		} else {
			xmlAddChild(insertpos,data);
		}
	} else throw "Undefined insert_mode";
}

/* try to delete a text node "out of sequence" */
bool XUpdateWriter::tryDeleteTextNode(xmlNodePtr pos3) {
	if (!xmlNodeIsText(pos3)) return true;
	/* is this a node cloned from the second document? */
	if (map_clone_back.find(pos3) == map_clone_back.end()) return false;
	/* get same node in second document */
	xmlNodePtr pos2 = map_clone_back[pos3];
	/* check if we "know" this node */
	if (known.find(pos2) == known.end()) return false;
	/* find the node this will map to */
	if (map.find(pos2) == map.end()) {
		xmlChar* path = makePath(pos3);
		/* we need to store this value */
		writeDeleteSubtree(path);
		xmlUnlinkNode(pos3);
		free(path);
		known.erase(pos2);
		return true;
	}
	return false;
}

/* insert a dummy node if needed */
void XUpdateWriter::insertDummyIfNeededForInsert(xmlNodePtr &insertpos, int &insert_mode) {
	bool need_dummy = false;
	switch (insert_mode) {
		case INSERT_MODE_FOSIB:
		case INSERT_MODE_PRSIB:
			if (insertpos->next || xmlNodeIsText(insertpos->next))
				if (!tryDeleteTextNode(insertpos->next)) need_dummy = true;
			if (insertpos->prev || xmlNodeIsText(insertpos->prev))
				if (!tryDeleteTextNode(insertpos->prev)) need_dummy = true;
		break;
		case INSERT_MODE_CHILD:
			if (insertpos->children)
				if (xmlNodeIsText(insertpos->children))
					if (!tryDeleteTextNode(insertpos->children))
						need_dummy = true;
		break;
		case INSERT_MODE_APPEND:
			if (insertpos->children) {
				xmlNodePtr iter = insertpos->children;
				while (iter->next) iter = iter->next;
				if (xmlNodeIsText(iter))
					if (!tryDeleteTextNode(iter))
						need_dummy = true;
			}
		break;
		default:
			throw "Undefined insert_mode";
	}
	if (!need_dummy) return;
	
	xmlNodePtr cmd;
	cmd = doInsertCmd(insertpos,insert_mode);
	writeElement(cmd, DUMMY_TAG);
	xmlNodePtr dummy = xmlNewNode(NULL, DUMMY_TAG);
	execInsert(insertpos, dummy, insert_mode);
	dummy_nodes.insert(dummy);
	insertpos = dummy;
	if (insert_mode == INSERT_MODE_CHILD)  insert_mode = INSERT_MODE_PRSIB;
	if (insert_mode == INSERT_MODE_APPEND) insert_mode = INSERT_MODE_FOSIB;
}

void XUpdateWriter::insertDummyIfNeededForDelete(xmlNodePtr deletepos, xmlNodePtr& insertpos, int& insert_mode) {
	if (!deletepos->prev || !deletepos->next) return;
	if (xmlNodeIsText(deletepos->prev) && xmlNodeIsText(deletepos->next)) {
		/* try to solve by deleting text nodes first */
		if (tryDeleteTextNode(deletepos->prev)) return;
		if (tryDeleteTextNode(deletepos->next)) return;
		/* insert a dummy node */
		xmlNodePtr cmd;
		xmlChar* path = makePath(deletepos);
		cmd = writeInsertAfter(path);
		free(path);
		writeElement(cmd, DUMMY_TAG);
		xmlNodePtr dummy = xmlNewNode(NULL, DUMMY_TAG);
		xmlAddNextSibling(deletepos,dummy);
		dummy_nodes.insert(dummy);
/*		insertpos = dummy;
		insert_mode = INSERT_MODE_FOSIB;*/
	}
}

void XUpdateWriter::deleteDummyNodesRec(xmlNodePtr pos) {
	while (pos) {
		xmlNodePtr next = pos->next;
		if (dummy_nodes.find(pos) != dummy_nodes.end()) {
			xmlChar* path = makePath(pos);
			writeDeleteSubtree(path);
			/* unlink and free the nodes */
			xmlUnlinkNode(pos);
			xmlFreeNode(pos);
			free(path);
		} else {
			if (pos->children) deleteDummyNodesRec(pos->children);
		}
		pos = next;
	}
}

void XUpdateWriter::deleteDummyNodes(xmlNodePtr pos) {
	deleteDummyNodesRec(pos);
	dummy_nodes.clear();
}

// FIXME: i think i'll need special handling for the case of a nesting inversion
// i.e. <a><b/></a> -> <b><a/></b> with additional changes
void XUpdateWriter::recDiff(xmlNodePtr pos1, xmlNodePtr pos2, xmlNodePtr insertpos, xmlNodePtr addedpos, int output) {
	hash_map<xmlNodePtr, xmlNodePtr, hash<void*> >::iterator i;
	hash_map<xmlNodePtr, int,        hash<void*> >::iterator ix;
	int insert_mode = INSERT_MODE_CHILD;
	if (!pos1 && !pos2 && !insertpos) return;

	/* assure we have a useable inserting position */
	if (!insertpos) {
		xmlNodePtr iter = pos1;
		while (! (insertpos = map_clone[iter]) ) {
			iter = iter->parent;
			if (!iter) throw "iter got NULL, have no insert position - root node changed?";
		}
	}

	/* for a good output we need the longest common subsequence at each level.
	 * this is easier here, since each node can match only once, this
	 * boils down to longest-increasing-subsequence */
	vector<pair<xmlNodePtr, xmlNodePtr> > lcs;
	{
		vector<pair<xmlNodePtr, xmlNodePtr> > p1;
		vector<xmlNodePtr> p2;
		for (xmlNodePtr i=pos1; i; i=i->next) {
			hash_map<xmlNodePtr, xmlNodePtr, hash<void*> >::iterator f = map.find(i);
			if (f != map.end()) p1.push_back(make_pair(i,f->second));
		}
		for (xmlNodePtr i=pos2; i; i=i->next)
			if (known.find(i) != known.end()) p2.push_back(i);
		calcLIS(p1, p2, lcs);
	}
	vector<pair<xmlNodePtr, xmlNodePtr> >::iterator lcsi = lcs.begin();

	while (pos1 || pos2) {
		/* Longest-Subsequence checkpoints */
		xmlNodePtr check1 = NULL;
		xmlNodePtr check2 = NULL;
		if (lcsi != lcs.end()) {
			check1 = lcsi->first;
			check2 = lcsi->second;
			lcsi++;
		}

		/* process nodes in first document until LIS point */
		/* FIXME: new sequence needed:
		 * - Descend into nodes in first document if needed
		 * - Insert new nodes in second document
		 * - Delete nodes from first
		 * (this reduces amount of dummy nodes needed when in whitespace mode
		 */
		while (pos1 && (pos1 != check1)) {
			if (output & OUTPUT_FIRST) {
				/* ignored nodes are just dumped if applicable */
				if (known.find(pos1) == known.end()) {
					/* whitespace etc. in first document is ignored */
				} else {
					/* find clone position */
					xmlNodePtr clone = map_clone[pos1];

					i = map.find(pos1);
					if (i != map.end()) {
						// FIXME: descent into attributes, too.
						// FIXME: is this behaviour correct (i->second->children)?
						recDiff(pos1->children, i->second->children, clone, NULL, output & ~OUTPUT_SECOND);
						/* Nodes moved away */

						ix = map_index.find(pos1);
						if (ix == map_index.end()) {

							int num = index_num++;
							map_index.insert(make_pair(pos1, num));
							map_index.insert(make_pair(i->second, num));

							if ((pos1->parent == (xmlNodePtr)pos1->doc) || (map.find(pos1->parent) != map.end()))
								insertDummyIfNeededForDelete(clone, insertpos, insert_mode);
							/* we need to store this value */
							xmlChar* path = makePath(clone);
							writeStoreSubtree(num, path);
							/* FIXME: does the following part go here or outside? */
							/* do delete when root node, or parent is not deleted as well */
							if ((pos1->parent == (xmlNodePtr)pos1->doc) || (map.find(pos1->parent) != map.end())) {
								/* if needed, insert a dummy tag */
								writeDeleteSubtree(path);
								/* we can now unlink this subtree from the document */
								xmlUnlinkNode(clone);
								/* store it for later insertion */
								map_move_subtrees[num] = clone;
							} else {
								map_move_subtrees[num] = xmlCopyNode(clone,1);
								insertpos = clone;
								insert_mode = INSERT_MODE_FOSIB;
							}
							free(path);
						}
					} else {
						// FIXME: descent into attributes, too.
						// FIXME: is this descending behaviour correct?
						recDiff(pos1->children, NULL, clone, NULL, output & ~OUTPUT_SECOND);

						/* nodes removed altogether */
						/* do delete when root node, or parent is not deleted as well */
						if ((pos1->parent == (xmlNodePtr)pos1->doc) || (map.find(pos1->parent) != map.end())) {
							insertDummyIfNeededForDelete(clone, insertpos, insert_mode);
							xmlChar* path = makePath(clone);
							writeDeleteSubtree(path);
							/* unlink and free the nodes */
							xmlUnlinkNode(clone);
							xmlFreeNode(clone);
							free(path);
						}
					}
				}
			}
			pos1 = pos1->next;
		}
		
		while (pos2 && (pos2 != check2)) {
			if (output & OUTPUT_SECOND) {
				/* ignored nodes are just dumped if applicable */
				if (known.find(pos2) == known.end()) {
					/* ignore whitespace and similar nodes in second document */
					/* except when we are inserting subtrees or text would collapse */
					if (addedpos && !xmlNodeIsText(xmlGetLastChild(addedpos))) {
						xmlNodePtr newt = xmlCopyNode(pos2,0);
						xmlAddChild(addedpos,newt);
						xmlNodePtr copy3 = xmlCopyNode(pos2,0);
						execInsert(insertpos, copy3, insert_mode);
						insertpos = copy3; insert_mode = INSERT_MODE_FOSIB;
					}
				} else {
					i = map.find(pos2);
					if (i != map.end()) {
						/* Nodes moved here */
						int num=-1;
						ix = map_index.find(pos2);
						if (ix == map_index.end()) {
							xmlNodePtr clone = map_clone[i->second];
							num = index_num++;
							map_index.insert(make_pair(pos2, num));
							map_index.insert(make_pair(i->second, num));
							/* we have to select the other contents now, this is difficult, but in order to
							* have a reasonable variable ordering and execution sequence we need to do
							* it this way... */
							// FIXME: descent into attributes, too.
							// FIXME: is this behaviour correct (i->second->children)?
							recDiff(i->second->children, pos2->children, clone, NULL, output & ~OUTPUT_SECOND);
							insertDummyIfNeededForDelete(clone, insertpos, insert_mode);
							xmlChar* path = makePath(clone);
							/* we need to store this value */
							writeStoreSubtree(num, path);
							/* we can now delete this value */
							writeDeleteSubtree(path);
							free(path);
							/* we can now unlink this subtree from the document */
							xmlUnlinkNode(clone);
							/* store it for later insertion */
							map_move_subtrees[num] = clone;
						} else {
							num = ix->second;
						}
#ifdef CAREFUL
						if (num == -1) throw "Didn't get a valid move operation number.";
#endif
						xmlNodePtr clone2 = map_move_subtrees[num];
						if (xmlNodeIsText(clone2)) insertDummyIfNeededForInsert(insertpos, insert_mode);
						/* we can insert the previously stored contents here */
						xmlNodePtr cmd = doInsertCmd(insertpos, insert_mode);
						/* add value-of selection for previous contents */
						std::ostringstream label;
						label << "$m" << num;
						writeValueOf(cmd, (xmlChar*) label.str().c_str());
						/* re-insert the subtree here */
						execInsert(insertpos, clone2, insert_mode);
						insertpos = clone2; insert_mode = INSERT_MODE_FOSIB;
						//FIXME: handle attributes
						recDiff(i->second->children, pos2->children, clone2, NULL, output & ~OUTPUT_FIRST);
					} else {
						if (xmlNodeIsText(pos2)) insertDummyIfNeededForInsert(insertpos, insert_mode);
						/* if the parent was inserted as well, we can just add the new node as child */
						if (!(addedpos && !xmlNodeIsText(xmlGetLastChild(addedpos))) &&
							((pos2->parent == (xmlNodePtr)pos2->doc) || (map.find(pos2->parent) != map.end()))) {
							/* we can insert the previously stored contents here */
							xmlNodePtr cmd = doInsertCmd(insertpos, insert_mode);
							xmlNodePtr was_added = NULL;
							/* nodes inserted */
							if (xmlNodeIsText(pos2)) {
								xmlNodePtr elem = writeText(cmd);
								was_added = xmlCopyNode(pos2,0);
								xmlAddChild(elem,was_added);
							} else
								was_added = writeElement(cmd, pos2->name);
							//xmlNodePtr copy = xmlCopyNode(pos2,0);
							//xmlAddChild(cmd,copy);
							//xmlNodeAddContent(cmd,(xmlChar*)"\n");
							/* insert into reference tree */
							xmlNodePtr copy3 = xmlCopyNode(pos2,0);
							execInsert(insertpos,copy3,insert_mode);
							insertpos = copy3; insert_mode = INSERT_MODE_FOSIB;
							//FIXME: handle attributes
							recDiff(NULL, pos2->children, copy3, was_added, output & ~OUTPUT_FIRST);
						} else {
							xmlNodePtr newt = xmlCopyNode(pos2,0);
							xmlAddChild(addedpos,newt);
							xmlNodePtr copy3 = xmlCopyNode(pos2,0);
							execInsert(insertpos, copy3, insert_mode);
							insertpos = copy3; insert_mode = INSERT_MODE_FOSIB;
							//FIXME: handle attributes
							recDiff(NULL, pos2->children, copy3, newt, output & ~OUTPUT_FIRST);
						}
					}
				}
			}
			pos2 = pos2->next;
		}

		/* we now arrived at the LIS point */
		if (pos1 && pos2) {
			//xmlNodePtr copy = xmlCopyNode(pos2,0);
			//xmlAddChild(diff,copy);
			//attr_diff(copy, ns, pos1, pos2, map, known, output_only);
			insertpos = map_clone[pos1];
			insert_mode = INSERT_MODE_FOSIB;
			recDiff(pos1->children, pos2->children, insertpos, NULL, output);
			pos1 = pos1->next;
			pos2 = pos2->next;
		}
	}
}

XUpdateWriter::XUpdateWriter()
	: output(NULL), ns(NULL), map(), map_index(), index_num(0), map_move_subtrees(), known(), map_clone(), result(NULL) {
	result = xmlNewDoc((xmlChar*)"1.0");

	xmlNodePtr root = xmlNewNode(NULL,(xmlChar*)"modifications");
	xmlDocSetRootElement(result,root);
	xmlNodeAddContent(root,(xmlChar*)"\n");
	xmlSetProp(root,(xmlChar*)"version",(xmlChar*)"1.0");
	output=root;

	ns = xmlNewNs(root, NAMESPACE_XUPDATE,(xmlChar*)"xupdate");
	xmlSetNs(root,ns);
}

void XUpdateWriter::run(Doc& doc1, Doc& doc2, DiffDijkstra& diff) {
	/* build a map for lookups node-in-second -> node-in-first */
	for (NodeAssignments* iter = diff.result->ass; iter; iter = iter->next) {
		if (iter->n1 && iter->n2) {
			map.insert(make_pair((xmlNode*)iter->n1->data,(xmlNode*)iter->n2->data));
			map.insert(make_pair((xmlNode*)iter->n2->data,(xmlNode*)iter->n1->data));
		}
	}

	/* this only works because we know that sizeof(xmlNodePtr) == sizeof(void*) */
	set<xmlNodePtr>* k1 = (set<xmlNodePtr>*) &doc1.processed;
	set<xmlNodePtr>* k2 = (set<xmlNodePtr>*) &doc2.processed;

	set_union(k1->begin(), k1->end(), k2->begin(), k2->end(),
			inserter(known, known.begin()));

	/* make a working copy of the first document for diff construction */
	xmlDocPtr doc3 = xmlNewDoc((xmlChar*)"1.0");
	xmlNodePtr doc3root = xmlCopyNode(xmlDocGetRootElement(doc1.getDOM()),1);
	xmlDocSetRootElement(doc3,doc3root);
	/* make a mapping of original nodes to cloned copies */
	initCloneMap(xmlDocGetRootElement(doc1.getDOM()),doc3root);
	
	recDiff(xmlDocGetRootElement(doc1.getDOM()), xmlDocGetRootElement(doc2.getDOM()), (xmlNodePtr)doc3, NULL, OUTPUT_BOTH);

	deleteDummyNodes(doc3root);
	/* we can destroy doc3 now */
#if 1
	cout << "With the result document: " << endl;
	xmlDocDump(stdout,doc3);
#endif
	xmlFreeDoc(doc3);
}

void XUpdateWriter::dump() {
	xmlDocDump(stdout,result);
}

XUpdateWriter::~XUpdateWriter() {
	xmlFreeDoc(result); result = NULL, output = NULL;
}
} // namespace SSD
