/* ===========================================================================
 *        Filename:  out_xupdate.h
 *     Description:  Header for the "xupdate" output format
 * 
 *         Version:  $Rev$
 *         Changed:  $Date$
 *         Licence:  GPL (read COPYING file for details)
 * 
 *          Author:  Erich Schubert (eS), erich@debian.org
 *                   Institut für Informatik, LMU München
 * ======================================================================== */
#ifndef OUT_XUPDATE_H
#define OUT_XUPDATE_H
#include "doc.h"
#include "diff.h"
#include <libxml/tree.h>

namespace SSD {
/** \brief XUpdate output writer class */
/** The XUpdate output writer is by far the most complex. We're trying to make
 *  a minimal edit script to execute the changes we feel that are necessary */
class XUpdateWriter {
private:
	/** \brief pointer to the current output serialization position */
	xmlNodePtr						output;
	/** \brief XML namespace for xupdate statements */
	xmlNsPtr						ns;
	/** \brief hashmap summarizing diff result */
	hashmap<xmlNodePtr, xmlNodePtr, hashfun<void*> >		map;
	/** \brief map nodes to index for move variables */
	hashmap<xmlNodePtr, int, hashfun<void*> >			map_index;
	/** \brief next number to be used for a move variable */
	int							index_num;
	/** \brief storage for subtress "cut" but not yet reinserted (i.e. store variable contents) */
	hashmap<int, xmlNodePtr, hashfun<int> >			map_move_subtrees;
	/** \brief to identify nodes ignored in the diff process */
	set<xmlNodePtr>						known;
	/** \brief map a node to its clone in the output */
	hashmap<xmlNodePtr, xmlNodePtr, hashfun<void*> >		map_clone;
	/** \brief map a cloned node back to its original node */
	hashmap<xmlNodePtr, xmlNodePtr, hashfun<void*> >		map_clone_back;
	/** \brief set of dummy nodes we need to clean afterwards */
	set<xmlNodePtr>						dummy_nodes;

	/** \brief initialize clone map (the clone is where we simulate the xupdate execution) */
	/** \param r1 root of original document
	 *  \param r3 root of cloned document */
	void		initCloneMap(xmlNodePtr r1, xmlNodePtr r3);
	/** \brief make a buffer to construct an xpath */
	/** \param buf buffer to be used
	 *  \param pos node the xpath should point to */
	void		makePathBuf(xmlBufferPtr buf, xmlNodePtr pos);
	/** \brief make an xpath to a node */
	/** \param pos node the xpath should point to */
	xmlChar*	makePath(xmlNodePtr pos);
	/** \brief recursive diff procedure */
	/** \param pos1 Position in "old" first document
	 *  \param pos2 Position in second ("target") document
	 *  \param pos3 Position in "current" document obtained by application of the xupdate rules written
	 *  \param addedpos Position we last added at (?? FIXME!)
	 *  \param descend flag if we should recursively descend further (?? FIXME! ) */
	void		recDiff(xmlNodePtr pos1, xmlNodePtr pos2, xmlNodePtr pos3, xmlNodePtr addedpos, int descend);
	/* writer functions for writing nodes to output */
	/** \brief write a store subtree statement */
	/** \param num variable number to use
	 *  \param path path to be used in the store command */
	xmlNodePtr	writeStoreSubtree(int num, const xmlChar* path);
	/** \brief write a delete subtree statement */
	/** \param path path to be used in the delete command */
	xmlNodePtr	writeDeleteSubtree(const xmlChar* path);
	/** \brief write an append statement */
	/** \param path path to be used in the append command */
	xmlNodePtr	writeAppend(const xmlChar* path);
	/** \brief write an insert-after statement */
	/** \param path path to be used in the insert-after command */
	xmlNodePtr	writeInsertAfter(const xmlChar* path);
	/** \brief write an insert-before statement */
	/** \param path path to be used in the insert-before command */
	xmlNodePtr	writeInsertBefore(const xmlChar* path);
	/** \brief write an value-of statement */
	/** \param where xupdate statement to be inserted into
	 *  \param path path (or more precisely, variable name) to be used in the value-of command */
	xmlNodePtr	writeValueOf(xmlNodePtr where, const xmlChar* path);
	/** \brief write an element xupdate statement */
	/** \param where xupdate statement to be inserted into
	 *  \param elem element name to be used in the element command */
	xmlNodePtr	writeElement(xmlNodePtr where, const xmlChar* elem);
	/** \brief write an text xupdate statement */
	/** \param where xupdate statement where text wrapper should be inserted */
	xmlNodePtr	writeText(xmlNodePtr where);
	/** \brief make an insert command, depending on insert_mode */
	/** \param insertpos current processing position
	 *  \param insert_mode which type of insert statement (append, insert-before, insert-after) to do */
	xmlNodePtr	doInsertCmd(xmlNodePtr insertpos, int insert_mode);
	/** \brief execute an insert command, depending on insert_mode */
	/** "execute" the xupdate statement we just (should) have written
	 *  \param insertpos current processing position
	 *  \param data data to be inserted
	 *  \param insert_mode which type of insert step (append, insert-before, insert-after) to execute */
	void		execInsert(xmlNodePtr insertpos, xmlNodePtr data, int insert_mode);
	/** \brief try to delete a text node out-of-sequence to avoid having to use dummy nodes */
	/** When we have a text node, an element, more text, and the element gets deleted, we need to
	 *  prevent the text nodes from collapsing. Therefore we insert a dummy node there.
	 *  but in the case where we're going to delete the following text node, too, we can just
	 *  do that deletion first, then delete the element without a dummy.
	 *  \param pos3 check if we have to delete this text node */
	bool		tryDeleteTextNode(xmlNodePtr pos3);
	/** \brief insert a dummy node if needed */
	/** The insertion of dummy nodes may be necessary to prevent text node collapsing when inserting text nodes.
	 *  \param insertpos current position we are at
	 *  \param insert_mode insertion mode we are in */
	void		insertDummyIfNeededForInsert(xmlNodePtr& insertpos, int& insert_mode);
	/** \brief insert a dummy node if needed */
	/** The insertion of dummy nodes may be necessary to prevent text node collapsing when deleting elements
	 *  \param deletepos current position we are at
	 *  \param insertpos insert script position we are at (correct? FIXME!)
	 *  \param insert_mode insertion mode we are in */
	void		insertDummyIfNeededForDelete(xmlNodePtr deletepos, xmlNodePtr& insertpos, int& insert_mode);
	/** \brief delete all dummy nodes (internal recursive call) */
	/** \param pos position to delete dummy nodes at */
	void		deleteDummyNodesRec(xmlNodePtr pos);
	/** \brief delete all dummy nodes */
	/** \param pos position to delete dummy nodes at */
	void		deleteDummyNodes(xmlNodePtr pos);
public:
	/** \brief result XUpdate document */
	xmlDocPtr	result;
	/** \brief calculate the XUpdate rules needed for transformation */
	/** \param doc1 first document
	 *  \param doc2 second document
	 *  \param diff difference search result */
	void		run(Doc& doc1, Doc& doc2, DiffDijkstra& diff);
	/** \brief Dump the result document */
	void		dump();
	/** \brief create the xupdate writer object */
			XUpdateWriter();
	/** \brief delete the xupdate writer object */
			~XUpdateWriter();
};
}
#endif
