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
private:
	/** \brief the merged output document */
	/* FIXME: make public, so you can do more than dumping to stdout? */
	xmlDocPtr mergeddoc;
public:
	/** \brief create a new merged writer object */
	MergedWriter() : mergeddoc(NULL) {};
	/** \brief destroy the merged writer object */
	~MergedWriter();
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
