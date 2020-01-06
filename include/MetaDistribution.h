#ifndef __METADISTRIBUTION_H__
#define __METADISTRIBUTION_H__

#include "Distribution.h"

// Metadistribution class - represents a probability distribution over probability distributions.
// Used to create random distributions.

class MetaDistribution {
    friend class MetaDistributionViewMiniature;
public:
	// constructor & destructor
	MetaDistribution(void);
	MetaDistribution(const MetaDistribution& m);
	MetaDistribution(const MetaDistribution& l, const MetaDistribution& r, t_float v);
	MetaDistribution(TiXmlElement* xml);
	~MetaDistribution();
	const MetaDistribution& operator=(const MetaDistribution& m)  {
        if(precalc) delete [] precalc;
		zero = m.zero;
		one = m.one;
		mixture = m.mixture;
        d = m.d;
		precalc = 0;
		return *this;
	}
	void setDefault(void);
	string getDescription(void);
    
    
	// get a random distribution
	const Distribution& getRandomDistribution(void);
	const Distribution& getDistribution(t_float weight);
	
	// handle generating trust functions
	void precalculateForTrust(void);
	void setTrustFunction(TrustFunction* tf, float v);
	void mergeTrustFunctionWith(TrustFunction* tf, float v, float amt);
	void setTrustFunctionToRandom(TrustFunction* tf) {setTrustFunction(tf, mixture.getRandomValue());}
	void mergeTrustFunctionWithRandom(TrustFunction* tf, float amt) {mergeTrustFunctionWith(tf, mixture.getRandomValue(), amt);}
	
	// saving
	TiXmlElement* toXML(string name) const;
	
	// the distributions that make up this metadistribution
	Distribution zero;
	Distribution one;
	Distribution mixture;
	
	// working distribution; used to send back a reference
	Distribution d;
	
	// precalculated values of distribution, for generating trust functions
	float *precalc;
};

// comparison
bool operator==(const MetaDistribution& lhs, const MetaDistribution& rhs);
inline bool operator!=(const MetaDistribution& lhs, const MetaDistribution& rhs) { return !(lhs == rhs); }


#endif
