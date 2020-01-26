#ifndef __TRUST_H__
#define __TRUST_H__
#include "Prefix.h"
#include "FL/Fl_Button.H"
#include <pmmintrin.h>
#include "tinyxml.h"
#include <set>

#define TRUST_FUNCTION_RESOLUTION 48
#define TRUST_FUNCTION_RESOLUTION_INV (1.0f / (t_float)TRUST_FUNCTION_RESOLUTION)

// preset trust functions
#define TF_UNKNOWN 0
#define TF_NONE 1
#define TF_VERY_LOW 2
#define TF_LOW 3
#define TF_AVERAGE 4
#define TF_HIGH 5
#define TF_VERY_HIGH 6
#define TF_FULL 7

using namespace std;

class TrustView;
class Distribution;

// TrustFunction - represents a probability distribution over [0, 1], and contains
// methods for updating this distribution in light of new data.

class TrustFunction {
public:
	TrustFunction();
	TrustFunction(const TrustFunction& tf);
	TrustFunction(TiXmlElement* xml);
	~TrustFunction();

	// set to one of the preset values
	void setFromPreset(t_int presetValue);
	void setValueSharpness(t_float mean, t_float dev);
	inline void normalise(void);
	t_float denormaliseValue(t_float v);
	t_float normaliseValue(t_float v);

	// get t_interpolated value at a point
	float getValue(float pos);

	// copy operation
	TrustFunction& operator=(const TrustFunction& tf);

	// comparison
	bool operator==(const TrustFunction& tf) {
		for(t_int i = 0; i <= TRUST_FUNCTION_RESOLUTION; ++i) if(values[i] != tf.values[i]) return false;
		return true;
	}
	bool operator!=(const TrustFunction& tf) {
		for(t_int i = 0; i <= TRUST_FUNCTION_RESOLUTION; ++i) if(values[i] != tf.values[i]) return true;
		return false;
	}

	// expectation of trust function
	float expectation();

	// update trust function with regard to current belief & message
	void update(float belief, bool pTrue);

	// list of values
	float valueBlock[TRUST_FUNCTION_RESOLUTION + 8];
	float *values;

	// conversion to XML
	TiXmlElement* toXML(void) const;

	// link to trust view this function is shown in, if any
	TrustView* view;
	float expValue;
	bool expValid;
};

// comparison
bool operator==(const TrustFunction& lhs, const TrustFunction& rhs);
inline bool operator != (const TrustFunction& lhs, const TrustFunction& rhs) { return !(lhs == rhs); }

// TrustView - shows a trust function

class TrustView : public Fl_Button {
public:
	TrustView(t_int x, t_int y, t_int w, t_int h, const char *label = 0);
	~TrustView();

	void setTrustFunction(TrustFunction *f);
	void removeTrustFunction(TrustFunction *f);
	void removeAllTrustFunctions(void);

	void draw();
	void drawColumn(t_int xVal, t_float ht0, t_float ht1, t_float amt, bool sel);
	void deactivate();
	void setFromPreset(t_int presetValue);
	void setFromDistribution(const Distribution& d);
	TrustFunction* getAverageTrustFunction(void);

	// which trust functions are we showing?
	std::set<TrustFunction*> functions;
	TrustFunction avgTrustFunction;

	// image block
	unsigned char *img;

	// t_interface variables
	t_float xDrag, yDrag;
};


#endif
