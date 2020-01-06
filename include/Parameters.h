#ifndef PARAMETERS_H
#define PARAMETERS_H

#include "MetaDistribution.h"
#include "tinyxml.h"

// variation policies for parameters
#define KEEP_CONSTANT 0
#define VARY_GLOBALLY 1
#define VARY_INDIVIDUALLY 2

// new evidence requirements
#define NEW_EVIDENCE_NONE 0
#define NEW_EVIDENCE_INQUIRY 1
#define NEW_EVIDENCE_ANY 2

// InquirerParameters - a collection of properties that affect how an inquirer is
// initialised in a random society.

class InquirerParameters {
public:
	InquirerParameters(void);
	InquirerParameters(TiXmlElement* xml);
	InquirerParameters(const InquirerParameters& ip1, const InquirerParameters& ip2, t_float v);
	void setDefault(void);
	TiXmlElement* toXML(void) const;
	string compareTo(const InquirerParameters& ip);
	string getDescription(void);

	// starting degree of belief distribution
	Distribution startBelief;
	
	// scientific activity distribution
	Distribution inquiryChance;
	
	// scientific accuracy distribution
	Distribution inquiryAccuracy;
	
	// scientific trust meta distribution
	MetaDistribution inquiryTrust;
	
	// flags for what to generate / vary
	char varyStartBelief, varyInquiryChance, varyInquiryAccuracy, varyInquiryTrust;
	

};

bool operator==(const InquirerParameters& lhs, const InquirerParameters& rhs);
inline bool operator != (const InquirerParameters& lhs, const InquirerParameters& rhs) { return !(lhs == rhs); }


// LinkParameters - a collection of properties that affect how a link is
// initialised in a random society.

class LinkParameters {
public:
	LinkParameters(void);
	LinkParameters(TiXmlElement* xml);
	LinkParameters(const LinkParameters& lp1, const LinkParameters& lp2, t_float v);
	void setDefault(void);
	TiXmlElement* toXML(void) const;
	string compareTo(const LinkParameters& lp);
	string getDescription(void);

	// probability of communication distribution
	Distribution linkListenChance;		

	// least degree of belief required to communicate distribution
	Distribution linkThreshold;

	// probability of communication being trusted
	MetaDistribution linkTrust;
	
	// flags for what to generate / vary
	char varyListenChance, varyThreshold, varyTrust;
	
	
};

bool operator==(const LinkParameters& lhs, const LinkParameters& rhs);
inline bool operator != (const LinkParameters& lhs, const LinkParameters& rhs) { return !(lhs == rhs); }

const char* EvidenceStr(t_int v);
t_int EvidenceValue(const char* c);

#endif
