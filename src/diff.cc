/* ===========================================================================
 *        Filename:  diff.c
 *     Description:  Find differences between two SSD trees.
 * 
 *         Version:  $Rev$
 *         Changed:  $Date$
 *         Licence:  GPL (read COPYING file for details)
 * 
 *          Author:  Erich Schubert (eS), erich@debian.org
 *                   Institut für Informatik, LMU München
 * ========================================================================= */
#include "diff.h"
#include <vector>

#ifdef VERBOSE
# include <iostream>
#endif

namespace SSD {

bool DiffDijkstra::fastApproximativeMode = false;

/* constructor, adding the root nodes */
DiffDijkstra::DiffDijkstra(Doc& eins, Doc& zwei)
: Diff(eins,zwei),
#ifdef VERBOSE_SEQCOUNT
		seq(0),
#endif
		steps(0), result(NULL) {
}

DiffDijkstra::~DiffDijkstra() {
	if (result) delete result;
}

int
process_relations(
		Node* n1, Node* n2, RelCount* rc,
		int dir, /* "up" or "down" relations */
		const DiffDijkstraState* state,
		map<NodeEqClass,int>** c /* return parameter: local credits */) {

	NodeVec* rn1;
	NodeVec* rn2;
	if (dir == 1) { rn1 = n1->reldown; rn2 = n2 ? n2->reldown : NULL; }
	if (dir == 2) { rn1 = n1->relup;   rn2 = n2 ? n2->relup   : NULL; }

	int cost=0;
	NodeVec::iterator i1, i2;
	/* local credits counter */
	map<NodeEqClass,int>* count = new map<NodeEqClass,int>;
	/* this set contains the nodes we must be related to in the second document */
	/* because we are related to them in the first */
	set<Node*> rel;

	/* first make a list of nodes we need to find in the second list */
	if (rn1)
	for (i1=rn1->begin(); i1 != rn1->end(); i1++) {
		const NodeAssignments* f = state->findNodeAssignment1(*i1);
		/* register node to be found */
		if (f) {
			/* dropping nodes always loses us the relation. */
			if (f->n2) rel.insert(f->n2);
			/* if f->n2 == NULL, the node was dropped, costs have been calculated before */
			/* since we calculate these when dropping the first node */
		} else {
			/* do this also when dropping n1? */
			/* still unmatched node, add to local credits */
			NodeEqClass cp(*i1);
			(*count)[cp] = 1 + (*count)[cp];
		}
	}
	
	/* now check for matching nodes in the second documents relation */
	if (n2 && rn2)
	for (i2=rn2->begin(); i2 != rn2->end(); i2++) {
		/* did we already map this node? */
		const NodeAssignments* f = state->findNodeAssignment2(*i2);
		if (f) {
			/* to which node did we map it? */
			set<Node*>::iterator f2 = rel.find(*i2);
			if (f2 != rel.end()) {
				/* we have the matching node on the other side, too - optimal */
				/* no costs, and one node/relation less to care for */
				rel.erase(f2);
			} else {
				/* we cannot retain this relation */
				if (dir == 1) {
					int lcost = rc->modify(RelEqClass(*n2,*i2), -1);
					cost += lcost;
				}
				if (dir == 2) {
					int lcost = rc->modify(RelEqClass(*i2,*n2), -1);
					cost += lcost;
				}
			}
		} else {
			/* this node is still unmatched, add to local credits */
			NodeEqClass cp(*i2);
			(*count)[cp] = -1 + (*count)[cp];
		}
	}
#ifdef I_HAVE_FOUND_A_WAY_TO_MAKE_THIS_WORK_PROPERLY
	if (!n2) {
		cost *=2;
		map<NodeEqClass,int>::iterator i3;
		for (i3 = count->begin(); i3 != count->end(); i3++) {
			i3->second *= 2;
		}
	}
#endif
	/* now process remaining nodes in the first document */
	for(set<Node*>::iterator i = rel.begin(); i != rel.end(); i = rel.begin()) {
		if (dir == 1) {
			int lcost = rc->modify(RelEqClass(*n1,*i ), n2 ? +1 : +2);
			cost += lcost;
		}
		if (dir == 2) {
			int lcost = rc->modify(RelEqClass(*i ,*n1), n2 ? +1 : +2);
			cost += lcost;
		}
		rel.erase(i);
	}

	/* we now have the expected (minimal) loss of relations in count */
	*c = count;
	return cost;
}

DiffDijkstraState*
DiffDijkstra::makeState(const DiffDijkstraState* state, NodeVec::const_iterator n1, Node* n2) {
	int cost=0;
	RelCount* rc = new RelCount(*(state->credit));
#ifdef VERBOSE_COSTS_2
	cout << "Calculating costs for matching " << (**n1) << " with ";
	if (n2) { cout << *n2; } else { cout << "NONE"; }
	cout << "." << endl;
#endif

//	NodeVec::iterator i1, i2;
	map<NodeEqClass,int>* count;
	cost += process_relations(*n1, n2, rc, 1, state, &count);
#ifdef VERBOSE_COSTS_3
	cout << "Costs after process_relations down " << cost << endl;
#endif

	map<NodeEqClass,int>::iterator i3;
	for (i3 = count->begin(); i3 != count->end(); i3++) {
		int lcost = rc->modify(RelEqClass(**n1,i3->first), i3->second);
		cost += lcost;
	}

#ifdef VERBOSE_COSTS_4
	cout << "Costs with remaining nodes: " << cost << endl;
#endif
	//count.clear();
	delete(count);
	/* up relations */
	cost += process_relations(*n1, n2, rc, 2, state, &count);
#ifdef VERBOSE_COSTS_4
	cout << "Costs after process_relations up: " << cost << endl;
#endif

	for (i3 = count->begin(); i3 != count->end(); i3++) {
		int lcost = rc->modify(RelEqClass(i3->first,**n1), i3->second);
		cost += lcost;
	}
	delete(count);

#ifdef VERBOSE_COSTS
	/* calc costs */
	if (n2)
		cout << "Cost for matching " << **n1 << " and " << *n2 << " is " << cost << endl;
	else 
		cout << "Cost for dropping " << **n1 << " is " << cost << endl;
#endif

	NodeAssignments* a = new NodeAssignments(*n1, n2, state->ass);

	NodeVec::const_iterator ni = n1; ni++;
	DiffDijkstraState* stat = new DiffDijkstraState(state->cost + cost,
		state->length + (n2?1:0), ni, a, rc);

#ifdef VERBOSE_SEQCOUNT
	stat->seq = seq; seq++;
#endif
#ifdef TRACING_ENABLED
	if (searchTreeOutputStream) {
		if (n2)
			*searchTreeOutputStream << "Add " << stat->seq << "," << **n1 << "," << *n2 << "," << stat->cost << endl;
		else
			*searchTreeOutputStream << "Add " << stat->seq << "," << **n1 << ",," << stat->cost << "," << endl;
	}
#endif
	return stat;
}

bool
DiffDijkstra::step() {
	if (worklist.empty()) return false;

	/* retrieve the current entry in the work list */
	DiffDijkstraState* current = *(worklist.begin());
	worklist.erase(worklist.begin());
#ifdef TRACING_ENABLED
	if (searchTreeOutputStream) {
		*searchTreeOutputStream << "Step " << ++steps << ": " << current->seq << "/" << seq << " (of " << worklist.size()+1 << ") cost "
			<< current->cost << ", len " << current->length << " ";
		if (current->ass && current->ass->n1)
			*searchTreeOutputStream << *(current->ass->n1);
		if (current->ass && current->ass->n2)
			*searchTreeOutputStream << ", " << *(current->ass->n2);
		*searchTreeOutputStream << endl;
	}
#endif

	if (current->iter == nodevec.end()) {
		result = current;
		return false;
	} else {
		/* pos1 now points to the next node in the doc1 tree */
		NodeEqClass cp(*(current->iter));
		NodeVec* n = &(doc2->index_by_label[cp]);

		/* add yet unmatched nodes to worklist as new steps */
		for (NodeVec::iterator i = n->begin(); i != n->end(); i++)
			if (*i && !current->findNodeAssignment2(*i))
				worklist.insert(makeState(current,current->iter, *i));
		worklist.insert(makeState(current,current->iter, NULL));
		/* delete current state */
		delete current;
		return true;
	}
}

bool
DiffDijkstra::run() {
	/* calculate the credits by using the relation count */
	RelCount* credit = new RelCount(doc1->relcount, doc2->relcount);
	//cerr << *credit << endl;

	/* sort nodes by occurrence, low occurrence comes first */
	/* this small trick showed a 3* improvement in the first test - 84 steps instead of 224 */
	if (1) {
		multimap< int, Node* > nsort;

		for (NodeVec::const_iterator i=doc1->getNodesIter(); i != doc1->getNodesIterEnd(); ++i) {
			int count = doc2->index_by_label[NodeEqClass(*i)].size();
			nsort.insert(pair<int, Node*>(count, *i));
		}

		nodevec.empty();
		for (multimap< int, Node*>::iterator i = nsort.begin(); i != nsort.end(); ++i) {
			nodevec.push_back(i->second);
		}
		nsort.empty();
	} else {
		/* just copy the first list. its private, so we can't copy it directly */
		for (NodeVec::const_iterator i=doc1->getNodesIter(); i != doc1->getNodesIterEnd(); ++i)
			nodevec.push_back(*i);
	}

	if (fastApproximativeMode) {
		/* make start state with credits */
		worklist.insert(new DiffDijkstraState(0,0,nodevec.begin(),NULL, credit));
		while (step()) {
			/* drop all other states except the first */
			multiset<DiffDijkstraState*, DiffDijkstraStateQueue >::iterator wi = worklist.begin();
			++wi;
			worklist.erase(wi, worklist.end());
		}
		/* drop any remaining element in the work queue */
		while (worklist.begin() != worklist.end()) {
			delete *(worklist.begin());
			worklist.erase(worklist.begin());
		}
	} else {
		/* start with the root node, node matched nodes yet */
		worklist.insert(new DiffDijkstraState(0,0,nodevec.begin(),NULL, credit));
		/* process next element while not finished */
		while (step()) {;};
		/* drop any remaining element in the work queue */
		while (worklist.begin() != worklist.end()) {
			delete *(worklist.begin());
			worklist.erase(worklist.begin());
		}
	}
	/* return "done" */
	return false;
}

#ifdef TRACING_ENABLED
/* setup and open the output stream for search tree dumping */
void DiffDijkstra::setSearchTreeOutput(char* filename) {
	if (searchTreeOutputStream) delete(searchTreeOutputStream);
	searchTreeOutputStream = new std::ofstream(filename);
	if (!searchTreeOutputStream->is_open()) {
		delete(searchTreeOutputStream);
		searchTreeOutputStream = NULL;
	}
}

/* initialize stream to no output */
std::ofstream* DiffDijkstra::searchTreeOutputStream = NULL;
#endif

NodeAssignments::NodeAssignments(Node* nn1, Node* nn2, NodeAssignments* nnext)
	: n1(nn1), n2(nn2), next(nnext), refcount(1) {
	if (next) { next->refcount++; }
}

/* release and destroy if needed */
bool NodeAssignments::release() {
	refcount--;
	return (refcount == 0);
}

NodeAssignments::~NodeAssignments() {
	/* destroy linked elements no longer referred to */
	if (next) {
		if (next->release()) {
			delete(next);
			next = NULL;
		}
	}
}

const NodeAssignments* DiffDijkstraState::findNodeAssignment1(Node* n1) const {
	NodeAssignments* iter = ass;
	while(iter) {
		if (iter->n1 == n1) return iter;
		iter = iter->next;
	}
	return NULL;
}

const NodeAssignments* DiffDijkstraState::findNodeAssignment2(Node* n2) const {
	NodeAssignments* iter = ass;
	while(iter) {
		if (iter->n2 == n2) return iter;
		iter = iter->next;
	}
	return NULL;
}

}
