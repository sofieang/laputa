#ifndef __SOCIETY_H__
#define __SOCIETY_H__


#include "Prefix.h"

// for encoding shorts as ints
#define COUPLE(b, a) (((const unsigned t_int)(a) << 16) | ((const unsigned t_int)(b) & 0xFFFF))
#define LBOUND(a) ((const unsigned t_int)(a) << 16)
#define UBOUND(a) (((const unsigned t_int)((a) + 1) << 16) - 1)


// degree distribution ids
#define DD_IN 0
#define DD_OUT 1
#define DD_TOTAL 2

#include "Inquirer.h"
#include <stdio.h>
#include <list>
#include <vector>
#include <utility>
#include <map>
#include <utility>

using namespace std;

class SocietyFragment;
class Simulation;

// Society class - a collection of inquirers. Also contains methods that access more than one
// inquirer/link at a time. The global variable "curSociety" points to the society currently
// being edited.

typedef map< const unsigned t_int, Link, less<const unsigned t_int>, MEMORY_ALLOCATOR_LINK > LinkMap;

class Society {
public:
	// constructor & destructor
	Society(SocietySetup *setup = 0, Society *tmpl = 0);
	Society& operator=(const Society& s);
    Society(const Society& s);
	~Society();

	// extraction & merging of fragments
	SocietyFragment* extract(const set<t_int>& inqs, const set<pair<t_int, t_int> >& links);
	void merge(SocietyFragment* f, pair<t_int, t_int>* newLinks = 0);
	void deleteInquirersAndLinks(const set<t_int>& inqs, const set<pair<t_int, t_int> >& links);

	// create a new inquirer from setup
	void addInquirer(t_float x, t_float y, SocietySetup *setup);

	// get size of society
	t_int nInquirers(void) {return people.size();}
	Inquirer& getInquirer(t_int i) {return people[i];}

	// create a new link from setup
	bool addLink(const t_int source, const t_int target, SocietySetup *setup);

	// methods for generating a society
	void generateFromSetup(SocietySetup *setup);
	void generateLinksFromSetup(SocietySetup *setup);
	t_int getRandomSource(t_int to, SocietySetup* setup);
	void generateNewInquirers(SocietySetup *setup, t_int number);
	void generateNewLink(SocietySetup *setup);
	void addNewInquirerWithLinks(SocietySetup *setup);
	void adjustWeightsForTarget(t_int target, SocietySetup* setup);
	void removeTargetWeightAdjustment(t_int target, SocietySetup* setup);
	void recalculateWeights(SocietySetup* setup);
	t_int rollNumberOfLinks(SocietySetup *setup);

	// run simulation one step
	void evolve(Simulation* sim);

	// organise society for visibility
	void organise(void);
	void placeInquirers(vector<t_int> *inqsPerDegree, t_int maxDegree, t_float x, t_float y);
	void permuteInquirers(list<t_int>* inqsPerDegree, t_int maxDegree);

	// get statistics from society
	void getDegrees(t_int dir, vector<t_float>& degs);
	void calculateDegrees(void) {for(t_int i = 0; i < 3; ++i) getDegrees(i, degrees[i]);}

	// load and save societies
	void saveToFile(const char* name);
	void loadFromFile(const char* name);

	// link network handling
	Link* getLink(t_int source, t_int target);
	void removeLink(t_int src, t_int target);
	void recalculateListeners(void);
	inline void adjustInquirerWeight(t_int inq, t_float adjustment);

	// count number of inquirers in statistics
	inline t_int peopleInStatistics(void) {
		t_int ppl = 0, n = people.size();
		for(t_int i = 0; i < n; ++i) if(people[i].includeInStatistics) ++ppl;
		return ppl;
	}

	// list of people in society
	vector<Inquirer> people;

	// map of links in society
	LinkMap links;

	// blocks used in generating new society
	vector<t_float> weights;
	t_float totalWeight;
	vector<t_int> inqsToUpdate;
	t_int nInqsToUpdate;
	vector<bool> inqsToUpdateMatrix;

	// vectors containing degree data
	vector<t_float> degrees[3];
};

// comparison
bool operator==(const Society& lhs, const Society& rhs);
inline bool operator != (const Society& lhs, const Society& rhs) { return !(lhs == rhs); }

// SocietyFragment - a part of a society. Used for copying and pasting.

class SocietyFragment {
public:
	vector<t_int> indices;
	vector<Inquirer> inquirers;
	vector<Link> links;
};

// global variables
extern Society *curSociety;


#endif
