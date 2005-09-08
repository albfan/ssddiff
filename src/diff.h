/* ===========================================================================
 *        Filename:  diff.h
 *     Description:  Diff classes
 * 
 *         Version:  $Rev$
 *         Changed:  $Date$
 *         Licence:  GPL (read COPYING file for details)
 * 
 *          Author:  Erich Schubert (eS), erich@debian.org
 *                   Institut für Informatik, LMU München
 * ========================================================================= */
#ifndef SSD_DIFF_H
#define SSD_DIFF_H
#include "node.h"
#include "doc.h"
#include <vector>
#include <map>
#include <set>
#include <fstream>

using namespace std;
using namespace __gnu_cxx;

namespace SSD {

/** \brief quasi-abstract class for a diff */
class Diff {
protected:
	/** \brief first document to be compared */
	Doc*	doc1;
	/** \brief second document to be compared */
	Doc*	doc2;
public:
	/**\brief (empty) constructor
	 * \param eins first document to be compared
	 * \param zwei second document to be compared */
	Diff(Doc& eins, Doc& zwei) : doc1(&eins), doc2(&zwei) {};
	/**\brief (virtual empty) destructor */
	virtual ~Diff() {};
	/**\brief (virtual abstract) run function
	 * \return true when not yet done */
	virtual bool run() = 0;
};

/** \brief a reference counting list carrying two Node* as data
 *  this is a reference counting list since we are not going to use
 *  it a true list, but it can grow into different directions. The real
 *  structure will look like a tree, but with the children pointing to its
 *  parents.
 *
 *  Once a leaf is closed it will be "destroyed", in turn collapsing
 *  parents if their refcount reaches zero.
 *
 *  if you want to keep a child, increase the refcount, if you
 *  do not need it any more, use 'if (na->release()) delete(na);'
 *  so it is correctly destroyed. */
class NodeAssignments {
public:
	/** \brief data: node in first document */
	Node* n1;
	/** \brief data: node in second document */
	Node* n2;
	/** \brief linked list structure */
	NodeAssignments* next;
	/** \brief reference counting for destruction */
	int refcount;

	/** \brief make a new list element */
	/** this will set the new elements refcount to 1 and increase the parents
	 *  \param nn1 first data part, a node from first document
	 *  \param nn2 second data part, a node from the second document
	 *  \param nnext parent list element */
	NodeAssignments(Node* nn1, Node* nn2, NodeAssignments* nnext);
	/** \brief release and return if needed to be destroyed
	 *  \return true when the object should be deleted now */
	bool release();
	/** \brief destructor */
	~NodeAssignments();
};

/** \brief state object for the Dijkstra search we are implementing */
/** Each open end in the Dijkstra Search is represented by such an object.
 *  By keeping a reference to the "node assignment", it will keep the
 *  matched-nodes list open as long as its needed */
class DiffDijkstraState {
public:
#ifdef VERBOSE_SEQCOUNT
	/** \brief for sequence counting - only used for tracing */
	int seq;
#endif
	/** \brief current costs of this state, needed for sorting */
	int cost;
	/** \brief how many nodes have been matched */
	int length;
	/** \brief number of retained relations */
	int retained;
	/** \brief state is a solution state */
	/** set if there are no unmatched nodes left,
	 *  so this is in fact a solution */
	bool complete;
	/* FIXME: replace this by an interator over a vector of nodes.
	 * Then we can reorder them and will not need the
	 * "next" pointer in the Node structure either */
	/** \brief current node being processed. */
	/** C++ will make this iterator class a pointer - 4 bytes is perfect */
	NodeVec::const_iterator iter;
	/** \brief Assignments made for the current state */
	NodeAssignments* ass;

	/** \brief current "credits" for this search state */
	RelCount* credit;

	/** \brief constructor for a new state
	 *
	 * \param c costs for the new state
	 * \param l "length" of this state, i.e. depth in search tree
	 * \param p "iterator" to the next node to be processed
	 * \param a list of node assignments with relcount already increased
	 * \param cr remaining credits for this state */
	DiffDijkstraState(int c, int l, int r, NodeVec::const_iterator p, NodeAssignments* a, RelCount* cr) :
#ifdef VERBOSE_SEQCOUNT
		seq(0),
#endif
		cost(c), length(l), retained(r),
		complete(false), iter(p), ass(a), credit(cr) {};
	/** \brief Destructor that releases referenced data */
	~DiffDijkstraState() { if (ass && ass->release()) delete(ass); if (credit) delete(credit); }
	/** \brief test if node n1 is assigned in the current state */
	/** \param n1 the node to be found
	 *  \return the node assignment if found */
	const NodeAssignments* findNodeAssignment1(Node* n1) const;
	/** \brief test if node n2 is assigned in the current state */
	/** \param n2 the node to be found
	 *  \return the node assignment if found */
	const NodeAssignments* findNodeAssignment2(Node* n2) const;
	/** \brief ordering operator used for sorting the working queue */
	/** A state is considered to be better if:
	 *  - his costs are lower or
	 *  - costs are equal, but less credits have yet been used
	 *  - costs and credits used are equal, but length is bigger
	 * \param other state to be compared with
	 * \return comparision result */
	bool operator<(const DiffDijkstraState& other) const {
		return (cost != other.cost) ? (cost < other.cost) :
		  (retained != other.retained) ? (retained > other.retained) :
		  (length > other.length);
	};
};

/** \brief sorting function used for sorting the state queue */
/** just using the ordering given by the states themselves */
struct DiffDijkstraStateQueue {
	/** \brief sorting function used for sorting the state queue */
	/** just using the ordering given by the states themselves */
	bool operator()(const DiffDijkstraState* s1, const DiffDijkstraState* s2) const {
		return *s1 < *s2;
	}
};

/** \brief Dijkstra search core object */
/** this object will control an Dijkstra search, managing the priority queue etc. */
class DiffDijkstra : public Diff {
private:
#ifdef VERBOSE_SEQCOUNT
	/** \brief sequence counting */
	int seq;
#endif
	/** \brief best retained value so far */
	int best_retained;
	/** \brief maximum number of relations that can be retained */
	int max_retained;
	/** \brief number of steps done (i.e. nodes expanded) in the search */
	int steps;
	/** \brief working queue */
	multiset<DiffDijkstraState*, DiffDijkstraStateQueue >	worklist;
	/** \brief try to make one more step */
	/** \return if successful or finished */
	bool            	step();
	/** \brief generate a new state object */
	/** \param state the previous (parent) state object
	 *  \param n1 the node in the first document newly matched
	 *  \param n2 the node in the second document newly matched */
	DiffDijkstraState* 	makeState(const DiffDijkstraState* state, NodeVec::const_iterator n1, Node* n2);

	/** \brief list of nodes from first document to be processed - will be resorted to optimize */
	NodeVec nodevec;

#ifdef TRACING_ENABLED
	/** \brief debugging output stream */
	static std::ofstream*	searchTreeOutputStream;
#endif
public:
	/** \brief if fast-mode should be used */
	static bool		fastApproximativeMode;
	/** \brief set to the result state object when finished */
	DiffDijkstraState* 	result;
	/** \brief constructor for the search */
	/** \param eins first document to be compared
	 *  \param zwei second document to be compared */
	DiffDijkstra(Doc& eins, Doc& zwei);
	/** \brief destructor, also destroying the result unless unset */
	~DiffDijkstra();
	/** \brief execute the search
	 *  \return true if not yet finished */
	bool run();
#ifdef TRACING_ENABLED
	/** \brief for trace output */
	static void setSearchTreeOutput(char* filename);
#endif
};

}
#endif /* DIFF_H */
