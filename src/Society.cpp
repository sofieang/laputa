#include "Society.h"
#include "App.h"
#ifndef _USE_MATH_DEFINES
#define _USE_MATH_DEFINES
#endif
#include <time.h>
#include <gsl/gsl_randist.h>
#include "tinyxml.h"
#include "Utility.h"

#define INQUIRER_CLOSENESS_PENALTY 20.0
#define ORGANISE_SOCIETY_TIME 2.0
#define MAX_MINIMIZER_STEPS 2000


// global variables
Society *curSociety;

t_float addWeight(t_float& w, t_float v, t_float totalWeight);
t_float removeWeight(t_float& w, t_float v, t_float totalWeight);



//-----------------------------------------------------------------------------------------------------------------------

t_float addWeight(t_float& w, t_float v, t_float totalWeight) {
	if(w > 0) {
		if(w + v < 0) totalWeight -= w;
		else totalWeight += v;
	}
	else if(w + v > 0) totalWeight += v + w;
	w += v;

	return totalWeight;
}

//-----------------------------------------------------------------------------------------------------------------------

t_float removeWeight(t_float& w, t_float v, t_float totalWeight) {
	if(w > 0) {
		if(w - v < 0) totalWeight -= w;
		else totalWeight -= v;
	}
	else if(w - v > 0) totalWeight += w - v;

	return totalWeight;
}

//-----------------------------------------------------------------------------------------------------------------------

void Society::adjustInquirerWeight(t_int inq, t_float adjustment) {
	assert(nInqsToUpdate <= people.size());
	totalWeight = addWeight(weights[inq], adjustment, totalWeight);
	if(!inqsToUpdateMatrix[inq]) {
		inqsToUpdate[nInqsToUpdate++] = inq;
		inqsToUpdateMatrix[inq] = true;
	}
}

//-----------------------------------------------------------------------------------------------------------------------

void Society::adjustWeightsForTarget(t_int target, SocietySetup* setup) {
	// adjust weight matrix for symmetry
	if(setup->linkWeights[WT_SYMMETRY] != 0) {
		for(vector<t_int >::iterator i = people[target].listeners.begin(); i != people[target].listeners.end(); ++i) {
			adjustInquirerWeight(*i, setup->linkWeights[WT_SYMMETRY]);
		}
	}

	// adjust weight matrix for transitivity
	if(setup->linkWeights[WT_TRANSITIVITY] != 0) {
		for(LinkIterator l = links.lower_bound(LBOUND(target)); l != links.upper_bound(UBOUND(target)); ++l) {
			for(LinkIterator l2 = links.lower_bound(LBOUND(l->second.source)); l2 != links.upper_bound(UBOUND(l->second.source)); ++l2) {
				if(l2->second.source != target) adjustInquirerWeight(l2->second.source, setup->linkWeights[WT_TRANSITIVITY]);
			}
		}
	}

	// if limited to one/pair, set weight to zero for inquirers already linked to this one
	if(setup->limitLinksToOnePerPair) {
		for(LinkIterator l = links.lower_bound(LBOUND(target)); l != links.upper_bound(UBOUND(target)); ++l) adjustInquirerWeight(l->second.source, -weights[l->second.source]);
	}

	// never pick oneself!
	adjustInquirerWeight(target, -weights[target]);
}

//-----------------------------------------------------------------------------------------------------------------------

void Society::recalculateWeights(SocietySetup *setup) {
	assert(nInqsToUpdate <= people.size());
	for(t_int i = 0; i < nInqsToUpdate; ++i) {
		t_int inq = inqsToUpdate[i];
		if(weights[inq] > 0) totalWeight -= weights[inq];
		weights[inq] = setup->linkWeights[WT_BASE] + setup->linkWeights[WT_CLUSTERING] * people[inq].nListeners;
		if(weights[inq] > 0) totalWeight += weights[inq];
		inqsToUpdateMatrix[inq] = false;
	}
	nInqsToUpdate = 0;
}

//-----------------------------------------------------------------------------------------------------------------------

t_int Society::getRandomSource(t_int to, SocietySetup* setup) {
	// go through list of inquirers and pick one
	assert(totalWeight > 0);
	t_float v = gsl_rng_uniform(rng) * totalWeight;
	t_int pick;
	for(pick = 0; pick < people.size(); ++pick) {
		if(v <= weights[pick]) break;
		if(weights[pick] > 0) v -= weights[pick];
	}
	assert(weights[pick] > 0);
	assert(pick < people.size());
	return pick;
}

//-----------------------------------------------------------------------------------------------------------------------

void Society::generateNewInquirers(SocietySetup *setup, t_int number) {
	people.clear();
	for(t_int i = 0; i < number; ++i) {
		people.push_back(Inquirer(100, 100, setup));
		weights[i] = setup->linkWeights[WT_BASE];
	}
	totalWeight = setup->linkWeights[WT_BASE] * number;
}

//-----------------------------------------------------------------------------------------------------------------------

void Society::generateNewLink(SocietySetup *setup) {
	// pick target
	t_int target;
	do {
		target = gsl_rng_uniform_int(rng, people.size());
	} while(setup->limitLinksToOnePerPair && people[target].nSources >= people.size() - 1);


	// pick source
	adjustWeightsForTarget(target, setup);
	t_int source = getRandomSource(target, setup);

	// create link
    addLink(source, target, setup);


	// restore weights
	recalculateWeights(setup);
}

//-----------------------------------------------------------------------------------------------------------------------

void Society::addNewInquirerWithLinks(SocietySetup *setup) {
	// add inquirer
	people.push_back(Inquirer(100, 100, setup));
	weights[people.size() - 1] = setup->linkWeights[WT_BASE];
	totalWeight += setup->linkWeights[WT_BASE];

	// find out how many links to add
	t_int nLinks;
	if(setup->linkDistributionMethod & LDM_TO_NUMBER_BIT) {
		if((setup->linkDistributionMethod & LDM_MASK) == LDM_TOTAL) nLinks = Round(setup->linkDensityDistribution.getRandomValue() * (1 - setup->initialPopulationPart));
		else nLinks = setup->linkDensityDistribution.getRandomValue();
		if(nLinks > (people.size() - 1) * 2) nLinks = (people.size() - 1) * 2;
	}
	else {
		if((setup->linkDistributionMethod & LDM_MASK) == LDM_TOTAL) nLinks = Round(setup->linkDensityDistribution.getRandomValue() * (1 - setup->initialPopulationPart));
		else if((setup->linkDistributionMethod & LDM_MASK) == LDM_PER_INQUIRER) {
			nLinks = Round(setup->linkDensityDistribution.getRandomValue() * (t_float)(people.size() - 1) * 2);
		}
		else nLinks = Round(gsl_ran_binomial(rng, setup->linkDensityDistribution.getRandomValue(), (people.size() - 1) * 2));
	}
	// limit links to maximum
	if((setup->growthBalance > 0.999 || setup->growthBalance < 0.999) && nLinks > people.size() - 1) nLinks = people.size() - 1;

	// add the links
	for(t_int i = 0; i < nLinks;) {
		if(gsl_rng_uniform(rng) < setup->growthBalance) {
			// add link to inquirer
			adjustWeightsForTarget(people.size() - 1, setup);
			t_int from = getRandomSource(people.size() - 1, setup);
			if(addLink(from, people.size() - 1, setup)) {
				++i;
				adjustInquirerWeight(from, setup->linkWeights[WT_CLUSTERING]);
			}
			recalculateWeights(setup);
		}
		else {
			// add link from inquirer
			t_int to = gsl_rng_uniform_int(rng, people.size() - 1);
			if(addLink(people.size() - 1, to, setup)) {
				++i;
				totalWeight = addWeight(weights[people.size() - 1], setup->linkWeights[WT_CLUSTERING], totalWeight);
			}
		}
	}
}

//-----------------------------------------------------------------------------------------------------------------------

t_int Society::rollNumberOfLinks(SocietySetup *setup) {
	t_int nLinks = 0;
	if(setup->linkDistributionMethod & LDM_TO_NUMBER_BIT) {
		if((setup->linkDistributionMethod & LDM_MASK) == LDM_TOTAL) nLinks = setup->linkDensityDistribution.getRandomValue();
		else for(t_int i = 0; i < people.size(); ++i) nLinks += setup->linkDensityDistribution.getRandomValue();
		if(nLinks > people.size() * (people.size() - 1)) nLinks = people.size() * (people.size() - 1);
	}
	else {
		if((setup->linkDistributionMethod & LDM_MASK) == LDM_TOTAL) nLinks = Round(setup->linkDensityDistribution.getRandomValue() * (t_float)people.size() * (t_float)(people.size() - 1));
		else if((setup->linkDistributionMethod & LDM_MASK) == LDM_PER_INQUIRER) {
			nLinks = 0;
			for(t_int i = 0; i < people.size(); ++i) nLinks += Round(setup->linkDensityDistribution.getRandomValue() * (t_float)(people.size() - 1));
		}
		else nLinks = Round(gsl_ran_binomial (rng, setup->linkDensityDistribution.getRandomValue(), people.size() * (people.size() - 1)));
	}
	return nLinks;
}

//-----------------------------------------------------------------------------------------------------------------------

void Society::generateFromSetup(SocietySetup *setup) {
	t_int pop = Round(setup->populationDistribution.getRandomValue());
	weights.resize(pop);
	inqsToUpdate.resize(pop);
	inqsToUpdateMatrix.resize(pop);
	nInqsToUpdate = 0;
	for(t_int i = 0; i < pop; ++i) inqsToUpdateMatrix[i] = false;
	generateNewInquirers(setup, Round(pop * setup->initialPopulationPart));

	// add initial links
	t_int nLinks = rollNumberOfLinks(setup);
	for(t_int i = 0; i < nLinks; ++i) generateNewLink(setup);

	// grow society to correct size
	for(t_int i = people.size(); i < pop; ++i) addNewInquirerWithLinks(setup);
}

//-----------------------------------------------------------------------------------------------------------------------

void Society::generateLinksFromSetup(SocietySetup *setup) {
	// remove all existing links
	links.clear();
	for(t_int i = 0; i < people.size(); ++i) {
		people[i].listeners.clear();
		people[i].nListeners = people[i].nSources = 0;
	}
	weights.resize(people.size());
	for(t_int i = 0; i < people.size(); ++i) weights[i] = setup->linkWeights[WT_BASE];
	totalWeight = people.size() * setup->linkWeights[WT_BASE];
	inqsToUpdate.resize(people.size());
	inqsToUpdateMatrix.resize(people.size());
	nInqsToUpdate = 0;
	for(t_int i = 0; i < people.size(); ++i) inqsToUpdateMatrix[i] = false;

	// roll new links
	t_int nLinks = rollNumberOfLinks(setup);
	for(t_int i = 0; i < nLinks; ++i) generateNewLink(setup);
}

//-----------------------------------------------------------------------------------------------------------------------
Society::Society(SocietySetup *setup, Society *tmpl) {
	if(setup) {
		// society is made from a setup structure
		if(setup->varyPopulation) generateFromSetup(setup);
		else {
			// copy society from template and vary where applicable
			*this = *tmpl;
			for(t_int i = 0; i < people.size(); ++i) {
				// is inquirer variable?
				if(people[i].inqParams) {
					// vary inquirer properties
					if(people[i].inqParams->varyStartBelief == VARY_INDIVIDUALLY) people[i].belief = people[i].inqParams->startBelief.getRandomValue();
					else if(setup->inqParams.varyStartBelief == VARY_GLOBALLY && people[i].inqParams->varyStartBelief != KEEP_CONSTANT)
						people[i].belief = setup->inqParams.startBelief.getRandomValue();
					if(people[i].inqParams->varyInquiryChance == VARY_INDIVIDUALLY) people[i].inquiryChance = people[i].inqParams->inquiryChance.getRandomValue();
					else if(setup->inqParams.varyInquiryChance == VARY_GLOBALLY && people[i].inqParams->varyInquiryChance != KEEP_CONSTANT)
						people[i].inquiryChance = setup->inqParams.inquiryChance.getRandomValue();
					if(people[i].inqParams->varyInquiryAccuracy == VARY_INDIVIDUALLY) people[i].inquiryAccuracy = people[i].inqParams->inquiryAccuracy.getRandomValue();
					else if(setup->inqParams.varyInquiryAccuracy == VARY_GLOBALLY && people[i].inqParams->varyInquiryAccuracy != KEEP_CONSTANT)
						people[i].inquiryAccuracy = setup->inqParams.inquiryAccuracy.getRandomValue();
					if(people[i].inqParams->varyInquiryTrust == VARY_INDIVIDUALLY) people[i].inqParams->inquiryTrust.setTrustFunctionToRandom(&people[i].inquiryTrust);
					else if(setup->inqParams.varyInquiryTrust == VARY_GLOBALLY && people[i].inqParams->varyInquiryTrust != KEEP_CONSTANT)
						setup->inqParams.inquiryTrust.setTrustFunctionToRandom(&people[i].inquiryTrust);
				}
				else {
					// just follow setup parameters
					if(setup->inqParams.varyStartBelief == VARY_GLOBALLY) people[i].belief = setup->inqParams.startBelief.getRandomValue();
					if(setup->inqParams.varyInquiryChance == VARY_GLOBALLY) people[i].inquiryChance = setup->inqParams.inquiryChance.getRandomValue();
					if(setup->inqParams.varyInquiryAccuracy == VARY_GLOBALLY) people[i].inquiryAccuracy = setup->inqParams.inquiryAccuracy.getRandomValue();
					if(setup->inqParams.varyInquiryTrust == VARY_GLOBALLY) setup->inqParams.inquiryTrust.setTrustFunctionToRandom(&people[i].inquiryTrust);
				}
			}

			// vary link properties
			if(setup->varyLinks) generateLinksFromSetup(setup);
			else for(LinkIterator link = links.begin(); link != links.end(); ++link) {
				// change properties of the links we have
				if(link->second.linkParams) {
					// according to individual parameters
					if(link->second.linkParams->varyListenChance == VARY_INDIVIDUALLY)
						link->second.listenChance = link->second.linkParams->linkListenChance.getRandomValue();
					else if (setup->linkParams.varyListenChance == VARY_GLOBALLY && link->second.linkParams->varyListenChance != KEEP_CONSTANT)
						link->second.listenChance = setup->linkParams.linkListenChance.getRandomValue();
					if(link->second.linkParams->varyThreshold == VARY_INDIVIDUALLY)
						link->second.threshold = link->second.linkParams->linkThreshold.getRandomValue();
					else if (setup->linkParams.varyThreshold == VARY_GLOBALLY && link->second.linkParams->varyThreshold != KEEP_CONSTANT)
						link->second.threshold = setup->linkParams.linkThreshold.getRandomValue();
					if(link->second.linkParams->varyTrust == VARY_INDIVIDUALLY)
						link->second.linkParams->linkTrust.setTrustFunctionToRandom(&link->second.trust);
					else if (setup->linkParams.varyTrust == VARY_GLOBALLY && link->second.linkParams->varyTrust != KEEP_CONSTANT)
						setup->linkParams.linkTrust.setTrustFunctionToRandom(&link->second.trust);
				}
				else {
					// according to setup
					if (setup->linkParams.varyListenChance == VARY_GLOBALLY) link->second.listenChance = setup->linkParams.linkListenChance.getRandomValue();
					if (setup->linkParams.varyThreshold == VARY_GLOBALLY) link->second.threshold = setup->linkParams.linkThreshold.getRandomValue();
					if (setup->linkParams.varyTrust == VARY_GLOBALLY) setup->linkParams.linkTrust.setTrustFunctionToRandom(&link->second.trust);
				}
			}
		}
	}
	else {
		// empty society
		people.clear();
		links.clear();
	}
}

//-----------------------------------------------------------------------------------------------------------------------

Society::Society(const Society& s) {
    people.clear();
	links.clear();
	people = s.people;
	for (ConstLinkIterator l = s.links.begin(); l != s.links.end(); ++l) links.insert(*l);
}

//-----------------------------------------------------------------------------------------------------------------------

Society& Society::operator=(const Society& s) {
	people.clear();
	links.clear();
	people = s.people;
    for(ConstLinkIterator l = s.links.begin(); l != s.links.end(); ++l) links.insert(*l);
	return *this;
}

//-----------------------------------------------------------------------------------------------------------------------

Society::~Society() {
	people.clear();
	links.clear();
}

//-----------------------------------------------------------------------------------------------------------------------

void Society::recalculateListeners(void) {
	for(t_int i = 0; i < people.size(); ++i) {
		people[i].nListeners = 0;
		people[i].nSources = 0;
		people[i].listeners.clear();
	}
	for(LinkIterator l = links.begin(); l != links.end(); ++l) {
		people[l->second.source].listeners.push_back(l->second.target);
		++people[l->second.source].nListeners;
		++people[l->second.target].nSources;
	}
}

//-----------------------------------------------------------------------------------------------------------------------


void Society::addInquirer(t_float x, t_float y, SocietySetup *setup) {
	// add inquirer
	people.push_back(Inquirer(x, y, setup));
}

//-----------------------------------------------------------------------------------------------------------------------

bool Society::addLink(const t_int source, const t_int target, SocietySetup *setup) {
	// is there a link here already?
	LinkIterator l = links.find(COUPLE(source, target));
	if(l != links.end()) {
		if(setup->limitLinksToOnePerPair) return false;
		else {
			// merge with a new link
			l->second.mergeWithNew(setup);
			return true;
		}
	}
	else {
		// make a new link based on current setup
		links.insert(pair<const int, Link>(COUPLE(source, target), Link(source, target, setup)));

		// add to source's listener list
		people[source].listeners.push_back(target);
		++people[source].nListeners;
		++people[target].nSources;
		return true;
	}
}

//-----------------------------------------------------------------------------------------------------------------------

void Society::removeLink(t_int src, t_int target) {
	LinkIterator l(links.find(COUPLE(src, target)));
	links.erase(l);

	// remove from to source's listener list
	people[src].listeners.erase(std::find(people[src].listeners.begin(), people[src].listeners.end(), target));
	--people[src].nListeners;
	--people[target].nSources;
}

//-----------------------------------------------------------------------------------------------------------------------

Link* Society::getLink(t_int source, t_int target) {
	LinkIterator i(links.find(COUPLE(source, target)));
	if(i == links.end()) return NULL;
	else return &i->second;
}

//-----------------------------------------------------------------------------------------------------------------------

void Society::evolve(Simulation *sim) {
	// Init statistics variables
	t_float v = 0;
	t_int p = 0, notp = 0, total = 0;

	// Calculate every inquirer's new degree of belief
	for(t_int i = 0; i < people.size(); ++i) people[i].doInquiry(sim, this, i);


	// record messages for links
	for(LinkIterator l = links.begin(); l != links.end(); ++l) {
		if(l->second.evidencePolicy == NEW_EVIDENCE_INQUIRY) {
			if(people[l->second.source].message == MSG_SAY_P && people[l->second.source].newBelief >= l->second.threshold) l->second.message = MSG_SAY_P;
			else if(people[l->second.source].message == MSG_SAY_NOT_P && people[l->second.source].newBelief <= 1.0 - l->second.threshold) l->second.message = MSG_SAY_NOT_P;
		}
		else if(l->second.evidencePolicy == NEW_EVIDENCE_ANY) {
			if(people[l->second.source].newBelief > people[l->second.source].belief && people[l->second.source].newBelief >= l->second.threshold) l->second.message = MSG_SAY_P;
			else if(people[l->second.source].newBelief < people[l->second.source].belief && people[l->second.source].newBelief <= 1.0 - l->second.threshold) l->second.message = MSG_SAY_NOT_P;
		}
	}


	// Update inquirers to new values
	for(t_int i = 0; i < people.size(); ++i) {
		// update belief value
		people[i].belief = people[i].newBelief;

		// record statistics for this person
		if(people[i].includeInStatistics) {
			++total;
			if(sim->val.applicationMethod == APPLY_INDIVIDUALLY) v += sim->individualEValue(people[i].newBelief.v());
			else if(sim->val.applicationMethod == APPLY_TO_AVERAGE) v += people[i].newBelief.v();
			else {
				if(people[i].newBelief.v() > 0.5) {
					if(sim->val.blfPStrictlyGreater && people[i].newBelief.v() > sim->val.majorityPCert)  ++p;
					else if(people[i].newBelief.v() >= sim->val.majorityPCert) ++p;
				}
				else {
					if(sim->val.blfNotPStrictlyLess && people[i].newBelief.v() < sim->val.majorityNotPCert)  ++notp;
					else if(people[i].newBelief.v() <= sim->val.majorityNotPCert) ++notp;
				}
			}
		}
	}

	// record total statistics
	if(sim->val.applicationMethod == APPLY_INDIVIDUALLY) sim->eValue = v / (t_float)total;
	else if(sim->val.applicationMethod == APPLY_TO_AVERAGE) sim->eValue = sim->individualEValue(v / (t_float)total);
	else {
		if(sim->val.amtStrictlyGreater) {
			if((t_float)p > (t_float)total * sim->val.majorityAmt) sim->eValue = sim->val.eValues[2];
			else if((t_float)notp > (t_float)total * sim->val.majorityAmt) sim->eValue = sim->val.eValues[0];
			else sim->eValue = sim->val.eValues[1];
		}
		else {
			if((t_float)p >= (t_float)total * sim->val.majorityAmt) sim->eValue = sim->val.eValues[2];
			else if((t_float)notp >= (t_float)total * sim->val.majorityAmt) sim->eValue = sim->val.eValues[0];
			else sim->eValue = sim->val.eValues[1];
		}
	}
	sim->eValueDelta = sim->eValue - sim->startEValue;
}


//-----------------------------------------------------------------------------------------------------------------------

void Society::permuteInquirers(list<t_int> *inqsPerDegree, t_int maxDegree) {
	for(t_int i = 0; i <= maxDegree; ++i) {
		list<t_int > newOrder;
		t_int n = inqsPerDegree[i].size();
		while(n > 0) {
			t_int k = gsl_rng_uniform_int(rng, n);
			list<t_int >::iterator it = inqsPerDegree[i].begin();
			for(t_int j = 0; j < k; ++j) ++it;
			newOrder.push_back(*it);
			inqsPerDegree[i].erase(it);
			--n;
		}
		inqsPerDegree[i].swap(newOrder);
	}
}

//-----------------------------------------------------------------------------------------------------------------------

void Society::placeInquirers(vector<t_int> *inqsPerDegree, t_int maxDegree, t_float x, t_float y) {
	t_float r = 0;
	for(t_int i = maxDegree; i >= 0; --i) {
		// get size of this circle
		t_float minrad;
		if(inqsPerDegree[i].size() <= 1) minrad = 0;
		else if(inqsPerDegree[i].size() <= 4) minrad = 4 * INQUIRER_CIRCLE_SIZE * 3 / (2 * M_PI);
		else minrad = inqsPerDegree[i].size() * INQUIRER_CIRCLE_SIZE * 3 / (2 * M_PI);
		if(r < minrad) r = minrad;
		t_float v = gsl_rng_uniform(rng) * 2 * M_PI / inqsPerDegree[i].size();
		vector<t_int>::iterator it = inqsPerDegree[i].begin();
		for(t_int j = 0; j < inqsPerDegree[i].size(); ++j) {
			people[*it].x = x + cos(v) * r;
			people[*it].y = y - sin(v) * r;
			v += 2 * M_PI / inqsPerDegree[i].size();
			++it;
		}

		r += INQUIRER_CIRCLE_SIZE * 3;
	}
}

//-----------------------------------------------------------------------------------------------------------------------

void Society::organise(void) {
	// find midpt
	t_float xMid = societyWindow->view->xMid, yMid = societyWindow->view->yMid;

	// calculate degrees of all inquirers;
	t_int* degree = new t_int[people.size()];
	t_int maxDegree = 0;
	for(t_int i = 0; i < people.size(); ++i) {
		degree[i] = people[i].nListeners;
		if(degree[i] > maxDegree) maxDegree = degree[i];
	}
	for(const auto& l : links) {
		++degree[l.second.target];
		if(degree[l.second.target] > maxDegree) maxDegree = degree[l.second.target];
	}

	// order by degree
	vector<t_int> * inqsPerDegree = new vector<t_int> [maxDegree + 1];
	for(t_int i = 0; i < people.size(); ++i) inqsPerDegree[degree[i]].push_back(i);
	for(t_int i = 0; i <= maxDegree;) {
		if(inqsPerDegree[i].size() == 0) {
			--maxDegree;
			for (t_int j = i; j <= maxDegree; ++j) {
				inqsPerDegree[j] = inqsPerDegree[j + 1];
			}
		}
		else ++i;
	}

	placeInquirers(inqsPerDegree, maxDegree, xMid, yMid);


	// clean up
	delete [] degree;
	delete [] inqsPerDegree;
}

//-----------------------------------------------------------------------------------------------------------------------

void Society::getDegrees(t_int dir, vector<t_float>& degs) {
	t_int maxDegree = 0;

	for(t_int i = 0; i < degs.size(); ++i) degs[i] = 0;
	for(t_int i = 0; i < people.size(); ++i) {
		t_int d = 0;
		if(dir != DD_OUT) d += people[i].nSources;
		if(dir != DD_IN) d += people[i].nListeners;
		if(d > maxDegree) maxDegree = d;
		if(maxDegree >= degs.size()) {
			t_int prevSize = degs.size();
			degs.resize(maxDegree + 16);
			for(t_int j = prevSize; j < degs.size(); ++j) degs[j] = 0;
		}
		degs[d] += 1.0;
	}

	// pad degrees with zeros
	degs.resize(maxDegree * 1.5 + 1);
	for(t_int i = maxDegree + 1; i < degs.size(); ++i) degs[i] = 0;
}

//-----------------------------------------------------------------------------------------------------------------------

SocietyFragment* Society::extract(const set<t_int>& inqs, const set<pair<t_int, t_int> >& links) {

	// create a new society fragment
	SocietyFragment *f = new SocietyFragment;
	f->indices.reserve(inqs.size());
	f->inquirers.reserve(inqs.size());
	f->links.reserve(links.size());

	// copy inquirers to fragment
	for(set<t_int>::const_iterator inq = inqs.begin(); inq != inqs.end(); ++inq) {
		f->indices.push_back(*inq);
		f->inquirers.push_back(people[*inq]);
	}

	// copy links to fragment
	for(set<pair<t_int, t_int> >::const_iterator link = links.begin(); link != links.end(); ++link) {
		f->links.push_back(*getLink(link->first, link->second));
	}

	return f;
}

//-----------------------------------------------------------------------------------------------------------------------


void Society::merge(SocietyFragment* f, pair<t_int, t_int>* newLinks) {
	t_int *indexRemap = new t_int[f->indices.size() + people.size()];
	for(t_int i = 0; i < f->indices.size() + people.size(); ++i) indexRemap[i] = i;

	// add new inquirers
	for(t_int i = 0; i < f->indices.size(); ++i) {
		indexRemap[f->indices[i]] = people.size();
		people.push_back(f->inquirers[i]);
	}

	// add new links
	for(t_int i = 0; i < f->links.size(); ++i) {
		if(indexRemap[f->links[i].source] < people.size() && indexRemap[f->links[i].target] < people.size()) {
			// add link
			Link* link = getLink(indexRemap[f->links[i].source], indexRemap[f->links[i].target]);
			if(!link) {
				// create a new link
				links[COUPLE(indexRemap[f->links[i].source], indexRemap[f->links[i].target])] = Link(indexRemap[f->links[i].source], indexRemap[f->links[i].target]);
				link = getLink(indexRemap[f->links[i].source], indexRemap[f->links[i].target]);
			}
			// fill out link's fields
			link->listenChance = f->links[i].listenChance;
			link->trust = f->links[i].trust;
			link->threshold = f->links[i].threshold;
			link->linkParams = f->links[i].linkParams;
		}
		if(newLinks) {
			newLinks[i].first = indexRemap[f->links[i].source];
			newLinks[i].second = indexRemap[f->links[i].target];
		}
	}
	delete [] indexRemap;
}

//-----------------------------------------------------------------------------------------------------------------------

void Society::saveToFile(const char* filename) {
	// create xml file, root & society
	TiXmlDocument f;
	TiXmlDeclaration* decl = new TiXmlDeclaration( "1.0", "UTF-8", "true" );
	f.LinkEndChild(decl);
	TiXmlElement *root = new TiXmlElement("SOCIETY_FILE");
	root->SetAttribute("VERSION", LAPUTA_VERSION);
	f.LinkEndChild(root);
	TiXmlElement *soc = new TiXmlElement("SOCIETY");
	root->LinkEndChild(soc);

	// add inquirers & links
	for(t_int i = 0; i < people.size(); ++i) {
		TiXmlElement *inq = people[i].toXML(i);
		soc->LinkEndChild(inq);
	}
	// add inquirers & links
	for(LinkIterator l = links.begin(); l != links.end(); ++l) {
		TiXmlElement *link = l->second.toXML();
		soc->LinkEndChild(link);
	}

	// write out everything
	f.SaveFile(filename);
}

//-----------------------------------------------------------------------------------------------------------------------

void Society::loadFromFile(const char* filename) {
	TiXmlDocument f(filename);
	bool loadSucceeded = f.LoadFile();
	if(!loadSucceeded) {
		fl_alert("Failed to load file.");
		return;
	}
	TiXmlElement *root = f.RootElement();

	// check if distribution file
	if(strcmp("SOCIETY_FILE", root->Value())) {
		fl_alert("This is not a valid society file.");
		return;
	}

	// check version
	t_int vers;
	root->QueryIntAttribute("VERSION", &vers);
	if(vers < MIN_LAPUTA_VERSION) {
		fl_alert("This society was created with a too old version of Laputa.");
		return;
	}

	TiXmlElement *soc = root->FirstChildElement("SOCIETY");
	people.clear();
	links.clear();

	// read inquirers
	for(TiXmlElement *inq = soc->FirstChildElement("INQUIRER"); inq != NULL; inq = inq->NextSiblingElement("INQUIRER")) {
		people.push_back(Inquirer(inq));
	}

	// read links
	for(TiXmlElement *l = soc->FirstChildElement("LINK"); l != NULL; l = l->NextSiblingElement("LINK")) {
		Link lnk(l);
		links.insert(pair<unsigned t_int, Link>(COUPLE(lnk.source, lnk.target), lnk));
	}

	recalculateListeners();

}

//-----------------------------------------------------------------------------------------------------------------------

bool operator==(const Society& lhs, const Society& rhs) {
	if (lhs.people.size() != rhs.people.size()) return false;
	if (lhs.links.size() != rhs.links.size()) return false;
	for (t_int i = 0; i < lhs.people.size(); ++i) if (lhs.people[i] != rhs.people[i]) return false;
	ConstLinkIterator lhsl = lhs.links.begin(), rhsl = rhs.links.begin();
	for (t_int i = 0; i < lhs.links.size(); ++i) if ((lhsl++)->second != (rhsl++)->second) return false;
	return true;
}
