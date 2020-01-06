#ifndef __SOCIETYSETUP_H__
#define __SOCIETYSETUP_H__

#define MAX_INQUIRER_NAME_LENGTH 32

#include "Prefix.h"
#include "Parameters.h"
#include <set>

#define N_LINK_WEIGHT_FACTORS 4
#define WT_BASE 0
#define WT_SYMMETRY 1
#define WT_TRANSITIVITY 2
#define WT_CLUSTERING 3


#define LDM_TOTAL 0
#define LDM_PER_INQUIRER 1
#define LDM_PER_INQUIRER_SQUARED 2
#define LDM_MASK 3
#define LDM_TO_NUMBER_BIT 256

//-----------------------------------------------------------------------------------------------------------------------

//-----------------------------------------------------------------------------------------------------------------------

// SocietySetup - represents a setup for creating random societies. Used by the batch simulator.

class SocietySetup {
public:
	// how many in society?
	char varyPopulation;
	Distribution populationDistribution;
	t_float initialPopulationPart;
	t_float growthBalance;
	
	// parameters for inquirers
	InquirerParameters inqParams;
	
	// individual inquirer properties 
	bool includeInStatistics, updateInquiryTrust;
	
	// probability of link existing
	char varyLinks;
	Distribution linkDensityDistribution;
	bool limitLinksToOnePerPair;
	t_int linkDistributionMethod;
	t_float linkWeights[N_LINK_WEIGHT_FACTORS];
	
	// parameters for links
	LinkParameters linkParams;
	
	// individual link properties 
	t_int evidencePolicy;
	bool countPriorAsEvidence;
	bool updateTrust;
    
	// constructor
	SocietySetup() {setDefault();}
	SocietySetup(const SocietySetup& s1, const SocietySetup& s2, t_float v);
	SocietySetup(TiXmlElement* xml);
	TiXmlElement* toXML(t_int stage = 0) const;
	string getDescription(void);
	
	// set default values
	void setDefault(void);
	void setInquirerParametersWindowFrom(void);
	void setLinkParametersWindowFrom(void);	
	
	// precalculate distributions for a speedup
	void precalculate(void);
};

#endif