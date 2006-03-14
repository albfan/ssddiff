/* ===========================================================================
 *        Filename:  out_merged.h
 *     Description:  Header for the "merged" output format
 * 
 *         Version:  $Rev$
 *         Changed:  $Date$
 *         Licence:  GPL (read COPYING file for details)
 * 
 *          Author:  Erich Schubert (eS), erich@debian.org
 *                   Institut für Informatik, LMU München
 * ======================================================================== */
#ifndef OUT_MERGED_H
#define OUT_MERGED_H
#include "doc.h"
#include "diff.h"
#include <libxml/tree.h>

namespace SSD {
/** \brief Output writer class for the "merged" format */
/** This format is the most experimental (and yet incomplete)
 *  but it may be one of the most useful formats. It tries to
 *  produce one file with annotations so that you can reconstruct
 *  both original formats from it, while being as close as possible
 *  to both of them (and in some cases still being valid!) */
class MergedWriter {
public:
	/** \brief the merged output document */
	xmlDocPtr mergeddoc;

private:
	/** \brief action detected by the analyzer */
	enum Action {
		NONE,
		INSERTED,
		DELETED,
		MOVEDAWAY,
		MOVEDHERE
	};
	/** \brief output strings for the defined actions */
	static xmlChar* stringsAction[];
	/** \brief output strings to refer to a specific action place */
	static xmlChar* stringsRefer[];
	/** \brief output for special operations */
	static xmlChar* stringsSpecial[];
	/** \brief helper to mark a node in a certain way */
	virtual void markNode(xmlNode* node, enum Action action, xmlNsPtr ns);

	/** \brief recursively calculate actions */
	void recCalcActions(xmlNodePtr diff, xmlNsPtr ns,
		xmlNodePtr p1, xmlNodePtr p2,
		hash_map<xmlNodePtr, xmlNodePtr, hash<void*> >& map,
		set<xmlNodePtr>& known, int output_only);
public:
	/** \brief create a new merged writer object */
	MergedWriter() : mergeddoc(NULL) {};
	/** \brief destroy the merged writer object */
	virtual ~MergedWriter();
	/** \brief generate output document */
	/** generate the merged output document
	 *  \param doc1 first source document
	 *  \param doc2 second source document
	 *  \param diff diff results */
	void run(SSD::Doc& doc1, SSD::Doc& doc2, SSD::DiffDijkstra& diff);
	/** \brief dump the merged document to stdout */
	void dump();
};

}
#endif
