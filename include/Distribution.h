#ifndef __DISTRIBUTION_H__
#define __DISTRIBUTION_H__

#include "Prefix.h"
#include <gsl/gsl_rng.h>
#include <gsl/gsl_randist.h>
#include <gsl/gsl_cdf.h>
#include <vector>
#include <string>
#include <set>
#include <list>
#include "tinyxml.h"

using namespace std;

#define DISTR_POINT 0
#define DISTR_INTERVAL 1
#define DISTR_NORMAL 2
#define DISTR_BETA 3
#define DISTR_FREEFORM 4
#define DISTR_ALL 0xFFFF
#define N_DISTRIBUTION_TYPES 5

#define DEFAULT_DISTRIBUTION_SIZE 65

#define DEFAULT_INVERSE_CDF_STEPS 64


//-----------------------------------------------------------------------------------------------------------------------

// default distribution constants
enum {DF_DISTR_INTERVAL_WHOLE, DF_DISTR_INTERVAL_UPPER, DF_DISTR_INTERVAL_LOWER, DF_DISTR_BETA_NEUTRAL, DF_DISTR_BETA_UPPER, DF_DISTR_BETA_LOWER, N_DEFAULT_DISTRIBUTIONS};

class TrustFunction;

class PointDistribution {
public:
	t_float getPDFValue(t_float x) const;
	void getPDF(float* vals, t_int nVals) const;
	t_float getPDFValueDiscrete(t_float x, t_int parts) const;
	t_float getCDFValue(t_float x) const;
	t_float getCDFValueDiscrete(t_float x, t_int parts) const;

	t_float value;
};

inline bool operator==(const PointDistribution& lhs, const PointDistribution& rhs) { return lhs.value == rhs.value; }
inline bool operator!=(const PointDistribution& lhs, const PointDistribution& rhs) { return !(lhs == rhs); }

class IntervalDistribution {
public:
	t_float getPDFValue(t_float x) const;
	void getPDF(float* vals, t_int nVals) const;
	t_float getPDFValueDiscrete(t_float x, t_int parts) const;
	t_float getCDFValue(t_float x) const;
	t_float getCDFValueDiscrete(t_float x, t_int parts) const;

	t_float lower, upper;
};

inline bool operator==(const IntervalDistribution& lhs, const IntervalDistribution& rhs) { return lhs.lower == rhs.lower && lhs.upper == rhs.upper; }
inline bool operator!=(const IntervalDistribution& lhs, const IntervalDistribution& rhs) { return !(lhs == rhs); }

class NormalDistribution {
public:
	t_float getPDFValue(t_float x) const;
	void getPDF(float* vals, t_int nVals) const;
	t_float getPDFValueDiscrete(t_float x, t_int parts) const;
	t_float getCDFValue(t_float x) const;
	t_float getCDFValueDiscrete(t_float x, t_int parts) const;

	t_float midpt, stddev;
};

inline bool operator==(const NormalDistribution& lhs, const NormalDistribution& rhs) { return lhs.midpt == rhs.midpt && lhs.stddev == rhs.stddev; }
inline bool operator!=(const NormalDistribution& lhs, const NormalDistribution& rhs) { return !(lhs == rhs); }


class BetaDistribution {
public:
	t_float getPDFValue(t_float x) const;
	void getPDF(float* vals, t_int nVals) const;
	t_float getPDFValueDiscrete(t_float x, t_int parts) const;
	t_float getCDFValue(t_float x) const;
	t_float getCDFValueDiscrete(t_float x, t_int parts) const;

	t_float getMean(void) const;
	t_float getStddev(void) const;
	t_float getMaxDev(void) const;
	void setFromMeanDev(t_float mean, t_float stddev);

	t_float alpha, beta;
};

inline bool operator==(const BetaDistribution& lhs, const BetaDistribution& rhs) { return lhs.alpha == rhs.alpha && lhs.beta == rhs.beta; }
inline bool operator!=(const BetaDistribution& lhs, const BetaDistribution& rhs) { return !(lhs == rhs); }

class FreeformDistribution {
public:
	FreeformDistribution(void) {cdfValidity = 0;}

	t_float getPDFValue(t_float x) const;
	void getPDF(float* vals, t_int nVals) const;
	t_float getPDFValueDiscrete(t_float x, t_int parts) const;
	t_float getCDFValue(t_float x);
	t_float getCDFValueDiscrete(t_float x, t_int parts) const;
	void normalise(void);
	void normaliseAsDiscrete(void);
	void clear(void);
	void nudge(t_int d);
	inline void changed(void) {cdfValidity = 0;}

	vector<t_float> values;
	vector<t_float> cdf;
	t_int cdfValidity;
};

bool operator==(const FreeformDistribution& lhs, const FreeformDistribution& rhs);
inline bool operator!=(const FreeformDistribution& lhs, const FreeformDistribution& rhs) { return !(lhs == rhs); }


// Distribution class - represents a probability distribution
class Distribution {
public:
	// constructor & destructor
	Distribution(void);
	Distribution(TiXmlElement* xml);
	Distribution(const Distribution& l, const Distribution& r, t_float d);
	Distribution(const TrustFunction* f);
	Distribution(const char* name);

	// set distribution through dialog
	static bool setDistribution(Distribution& d);

	// get a random value in distribution
	t_float getRandomValue(void);

	// convert to trust function
	void toTrustFunction(TrustFunction* tf) const;
	void mergeTrustFunctionWith(TrustFunction* tf, float w) const;

    // interpolation between distributions
    void interpolate(const Distribution& l, const Distribution& r, t_float d);

	// access functions
	t_float getMin(void) const {return min;}
	t_float getMax(void) const {return max;}
	t_float getMean(void);
	t_float getStddev(void);
	t_float getPDFValue(t_float x) const;
	t_float getCDFValue(t_float x);
	t_float getInverseCDFValue(t_float v, t_int nSteps = DEFAULT_INVERSE_CDF_STEPS);

	// access functions
	void setMin(t_float v) {min = v;}
	void setMax(t_float v) {max = v;}
	void setResolution(t_int nValues);
	void setWeight(t_int w, t_float v);
	void setDiscreteRange(t_int minimum, t_int maximum);
	string getName(void);
	string getDescription(void);

	// draw t_into a memory block
	void draw(unsigned char* img, t_int w, t_int h, t_int filter, t_float amt, bool sel);
	void drawColumn(unsigned char* img, t_int width, t_int height, t_int xVal, t_float ht0, t_float ht1, t_float amt, bool sel);
	void drawDiscrete(unsigned char* img, t_int w, t_int h, t_int filter, t_float amt, bool sel);
	void drawRect(unsigned char* img, t_int w, t_int h, t_int x0, t_int x1, t_int y0, t_int y1, t_float amt, bool sel);

	// conversion to XML
	TiXmlElement* toXML(const char* name = 0) const;

	// min & max values
	t_float min, max;

	// how many pieces as discrete distribution? 0 = continuous
	t_int discreteParts;

	// distribution aspects
	PointDistribution dPt;
	IntervalDistribution dInt;
	NormalDistribution dNrm;
	BetaDistribution dBt;
	FreeformDistribution dFf;
	t_float weights[N_DISTRIBUTION_TYPES];


};

bool operator==(const Distribution& lhs, const Distribution& rhs);
inline bool operator!=(const Distribution& lhs, const Distribution& rhs) { return !(lhs == rhs); }


// Faster function for unnormalized beta distributions
float AbnormalBeta(float x, float alpha, float beta);

// Function to load default distributions
void LoadDefaultDistributions(const char* directory);

extern gsl_rng *rng;
extern vector<Distribution> defaultDistributions;

#endif
