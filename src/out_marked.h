/* ===========================================================================
 *        Filename:  out_marked.h
 *     Description:  Header for the "marked" output format
 * 
 *         Version:  $Rev$
 *         Changed:  $Date$
 *         Licence:  GPL (read COPYING file for details)
 * 
 *          Author:  Erich Schubert (eS), erich@debian.org
 *                   Institut für Informatik, LMU München
 * ======================================================================== */
#ifndef OUT_MARKED_H
#define OUT_MARKED_H
#include "doc.h"
#include "diff.h"

namespace SSD {
/** \brief Output writer for the "marked" output format */
/** The marked output format consists of two xml documents:
 *  copies of the source files, but with nodes "annotated" by
 *  numbers identifying which nodes are matched with which other nodes.
 *  
 *  This format probably is the most useful for debugging the diff
 *  application, but also for writing custom output writers, since parsing
 *  this output is by far the easiest. */
class MarkedWriter {
private:
	/** \brief first document */
	SSD::Doc *doc1;
	/** \brief second document */
	SSD::Doc *doc2;
public:
	/** \brief create new merged writer */
	MarkedWriter() : doc1(NULL), doc2(NULL) {};
	/** \brief destriy marked writer object */
	~MarkedWriter() {};
	/** \brief run marked writer */
	/** Generate output in "marked" format
	 *  \param d1 first document
	 *  \param d2 second document
	 *  \param diff diff result
	 */
	void run(SSD::Doc& d1, SSD::Doc& d2, SSD::DiffDijkstra& diff);
	/** \brief dump marked results */
	void dump();
};

}
#endif
