#ifndef __LINK_H__
#define __LINK_H__


#include "Prefix.h"

#include "SocietySetup.h"
#include "Trust.h"
#include "tinyxml.h"
#include <map>
#include <utility>
#define MSG_SAY_NOTHING 0
#define MSG_SAY_P 1
#define MSG_SAY_NOT_P 2


// Link - represents link between inquirers, and also handles saving and loading of said link.

class Link {
public:
	// constructor & destructor
	Link(t_int from = 0, t_int to = 0, SocietySetup *setup = 0);
	Link(t_int from, t_int to, const Link& l);
	Link(TiXmlElement* xml);
	~Link();
	Link& operator=(const Link& l);
	TiXmlElement* toXML(void) const;
	
	// merge with a new random link
	void mergeWithNew(SocietySetup *setup);
	
	// data
	TrustFunction trust;
	t_int source;
	t_int target;
	t_int message;
	t_float listenChance;
	t_float threshold;
	t_int lastUsed;
	LinkParameters* linkParams;
	
	bool updateTrust;
	
	// what new evidence is required to say anything?
	char evidencePolicy;
	bool countPriorAsEvidence;
	
};

typedef map<const unsigned t_int, Link, less<const unsigned t_int>, MEMORY_ALLOCATOR_LINK> LinkMap;
typedef LinkMap::iterator LinkIterator;
typedef LinkMap::const_iterator ConstLinkIterator;

// comparison
bool operator==(const Link& lhs, const Link& rhs);
inline bool operator != (const Link& lhs, const Link& rhs) { return !(lhs == rhs); }



#endif