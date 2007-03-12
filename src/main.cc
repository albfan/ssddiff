/* ===========================================================================
 *        Filename:  main.cc
 *     Description:  Main program for the xml diff demo app
 * 
 *         Version:  $Rev$
 *         Changed:  $Date$
 *         Licence:  GPL (read COPYING file for details)
 * 
 *          Author:  Erich Schubert (eS), erich@debian.org
 *                   Institut für Informatik, LMU München
 * ======================================================================== */
#include <iostream>
#include "doc.h"
#include "diff.h"
#include "out_xupdate.h"
#include "out_marked.h"
#include "out_merged.h"

#include "unistd.h"

using namespace SSD;

//#define DEFAULT_PATH BAD_CAST ".//*|.//text()"
//#define DEFAULT_PATH BAD_CAST ".//*|./text()"
#define DEFAULT_PATH "./node() | ./*/*"
//#define DEFAULT_PATH BAD_CAST "./*|./text()"
//#define DEFAULT_PATH BAD_CAST ".//*|.//text()|following-sibling::*"
//#define DEFAULT_PATH BAD_CAST "./following-sibling::*"

void usage(char* progname) {
	cerr << "Usage: " << endl;
	cerr << "  " << progname <<
#ifdef TRACING_ENABLED
		" [-t trace.dat]" <<
#endif
		" [-p xpath] [-w] document1.xml document2.xml" << endl;
#ifdef TRACING_ENABLED
	cerr << "    -t trace.dat        Dump search information for analysis" << endl;
#endif
	cerr << "    -w                  Process whitespace" << endl;
	cerr << "    -m                  Use 'merged' output format" << endl;
	cerr << "    -a                  Use 'marked' output format" << endl;
	cerr << "    -u                  Use 'xupdate' output format" << endl;
	cerr << "    -f                  Use fast mode (approximative)" << endl;
	cerr << "    -p xpath            Use a different xpath statement for structure" << endl;
	cerr << "    -p './/node()'      Use descendant relation" << endl;
	cerr << "    -p './node()'       Use child relation" << endl;
}

int main(int argc, char** argv) {
	Doc		doc1;
	Doc		doc2;

	int output = 0;
	char* xpath = DEFAULT_PATH;

	int option_char;
	while (1) {
		option_char = getopt(argc, argv, "famuwt:p:");
		if (option_char < 0) break;
		switch (option_char) {
#ifdef TRACING_ENABLED
			case 't': DiffDijkstra::setSearchTreeOutput(optarg); break;
#endif
			case 'p': xpath = optarg; break;
			case 'w': Doc::useWhitespace = true; break;
			case 'u': output = 0; break;
			case 'm': output = 1; break;
			case 'a': output = 2; break;
			case 'f': DiffDijkstra::fastApproximativeMode = true; break;
			case '?':
				usage(argv[0]);
				return(0);
				break;
		}
	}

	if (argc - optind != 2) {
		usage(argv[0]);
		return(0);
	}

	try {

		if (!doc1.loadXML(argv[optind])) {
			std::cerr << "Could not load: " << argv[1] << "." << std::endl;
			return(1);
		}
		if (!doc2.loadXML(argv[optind + 1])) {
			std::cerr << "Could not load: " << argv[2] << "." << std::endl;
			return(1);
		}
		doc1.processXPath(xpath);
		doc2.processXPath(xpath);

		DiffDijkstra	diff(doc1,doc2);

		// RelCount::dumpIndex(cout); cout << endl;

		diff.run();

		if (!diff.result) throw "Did not get a result, something is wrong.\n";

		switch (output) {
		case 0:
			{
				XUpdateWriter	out;
				std:cerr << "Warning: XUpdate writer doesn't support attribute changes yet." << std::endl;
				out.run(doc1, doc2, diff);
				out.dump();
			};
			break;
		case 1:
			{
				MergedWriter	out;
				out.run(doc1, doc2, diff);
				out.dump();
			};
			break;
		case 2:
			{
				MarkedWriter	out;
				std:cerr << "Warning: Marked writer doesn't support attribute changes yet." << std::endl;
				out.run(doc1, doc2, diff);
				out.dump();
			};
			break;
		}
	} catch (const char* error) {
		cerr << "An exception occurred: " << error << endl;
	}

	return 0;
}
/* vim:set noet sw=4 ts=4: */
