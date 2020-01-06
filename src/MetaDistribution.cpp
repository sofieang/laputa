#include "MetaDistribution.h"
#include <stdio.h>
#include <string.h>
#include <assert.h>
#include "App.h"
#include <gsl/gsl_randist.h>
#include <gsl/gsl_sf_erf.h>
#include "Trust.h"
#include "Utility.h"

//-----------------------------------------------------------------------------------------------------------------------

const Distribution& MetaDistribution::getDistribution(t_float weight)  {
	// create a new distribution
	d.interpolate(zero, one, weight);

	return d;
}

//-----------------------------------------------------------------------------------------------------------------------

const Distribution& MetaDistribution::getRandomDistribution(void) {
	// get random weight
	return getDistribution(mixture.getRandomValue());
}

//-----------------------------------------------------------------------------------------------------------------------

MetaDistribution::MetaDistribution(void) {
	precalc = 0;
}


//-----------------------------------------------------------------------------------------------------------------------

MetaDistribution::MetaDistribution(const MetaDistribution& m) {
	*this = m;
}

//-----------------------------------------------------------------------------------------------------------------------

MetaDistribution::~MetaDistribution() {
	if(precalc) delete [] precalc;
}

//-----------------------------------------------------------------------------------------------------------------------

MetaDistribution::MetaDistribution(const MetaDistribution& l, const MetaDistribution& r, t_float v) {
	zero = Distribution(l.zero, r.zero, v);
	one = Distribution(l.one, r.one, v);
	mixture = Distribution(l.mixture, r.mixture, v);
	precalc = 0;
}

//-----------------------------------------------------------------------------------------------------------------------

void MetaDistribution::setTrustFunction(TrustFunction* tf, float v) {
	if(precalc) {
		// uses the precalculated distributions to give a trust function
		t_int k = floor(v * TRUST_FUNCTION_RESOLUTION);
		if(k == TRUST_FUNCTION_RESOLUTION) --k;
		t_float over = v * TRUST_FUNCTION_RESOLUTION - k;
		float *lRow = precalc + k * (TRUST_FUNCTION_RESOLUTION + 1), *rRow = precalc + (k + 1) * (TRUST_FUNCTION_RESOLUTION + 1);
		for(t_int i = 0; i <= TRUST_FUNCTION_RESOLUTION; ++i) tf->values[i] = lRow[i] * (1.0 - over) + rRow[i] * over;
	}
	else {
		// do it the slow way
		d = Distribution(zero, one, v);
		d.toTrustFunction(tf);
	}
	tf->expValid = false;
}

//-----------------------------------------------------------------------------------------------------------------------

void MetaDistribution::mergeTrustFunctionWith(TrustFunction* tf, float v, float amt) {
	if(precalc) {
		// uses the precalculated distributions to give a trust function
		t_int k = floor(v * TRUST_FUNCTION_RESOLUTION);
		if(k == TRUST_FUNCTION_RESOLUTION) --k;
		t_float over = v * TRUST_FUNCTION_RESOLUTION - k;
		float *lRow = precalc + k * (TRUST_FUNCTION_RESOLUTION + 1), *rRow = precalc + (k + 1) * (TRUST_FUNCTION_RESOLUTION + 1);
		for(t_int i = 0; i <= TRUST_FUNCTION_RESOLUTION; ++i) {
			float val = lRow[i] * (1.0 - over) + rRow[i] * over;
			tf->values[i] = tf->values[i] * (1.0 - amt) + val * amt;
		}
	}
	else {
		d = Distribution(zero, one, v);
		d.mergeTrustFunctionWith(tf, amt);
	}
	tf->expValid = false;
}

//-----------------------------------------------------------------------------------------------------------------------

void MetaDistribution::precalculateForTrust(void) {
	if(precalc) delete [] precalc;
	precalc = new float[(TRUST_FUNCTION_RESOLUTION + 1) * (TRUST_FUNCTION_RESOLUTION + 1)];
	for(t_int j = 0; j <= TRUST_FUNCTION_RESOLUTION; ++j) {
		Distribution d(zero, one, (t_float)j / TRUST_FUNCTION_RESOLUTION);
		for(t_int i = 0; i <= TRUST_FUNCTION_RESOLUTION; ++i) precalc[j * (TRUST_FUNCTION_RESOLUTION + 1) + i] = d.getPDFValue((t_float)i / TRUST_FUNCTION_RESOLUTION);
	}
}

//-----------------------------------------------------------------------------------------------------------------------


void MetaDistribution::setDefault(void) {
	zero = Distribution(defaultDistributions[DF_DISTR_BETA_LOWER]);
	one = Distribution(defaultDistributions[DF_DISTR_BETA_UPPER]);
	mixture = Distribution(defaultDistributions[DF_DISTR_INTERVAL_WHOLE]);
	precalc = 0;
}

//-----------------------------------------------------------------------------------------------------------------------

MetaDistribution::MetaDistribution(TiXmlElement* xml) {
	mixture = Distribution(xml->FirstChildElement("MIXTURE"));
	zero = Distribution(xml->FirstChildElement("ZERO"));
	one = Distribution(xml->FirstChildElement("ONE"));
	precalc = 0;
}

//-----------------------------------------------------------------------------------------------------------------------

TiXmlElement* MetaDistribution::toXML(string name) const {
	TiXmlElement* xml = new TiXmlElement(name.c_str());
	xml->LinkEndChild(mixture.toXML("MIXTURE"));
	xml->LinkEndChild(zero.toXML("ZERO"));
	xml->LinkEndChild(one.toXML("ONE"));

	return xml;

}
//-----------------------------------------------------------------------------------------------------------------------

string MetaDistribution::getDescription(void) {
	string str("");
	str += string("[Lower = ") + zero.getDescription() + string("; ");
	str += string("Upper = ") + one.getDescription() + string("; ");
	str += string("Mixture = ") + mixture.getDescription() + string("]");

	return str;
}

//-----------------------------------------------------------------------------------------------------------------------

bool operator==(const MetaDistribution& lhs, const MetaDistribution& rhs) {
	if (lhs.zero != rhs.zero) return false;
	if (lhs.one != rhs.one) return false;
	if (lhs.mixture != rhs.mixture) return false;
	if (lhs.d != rhs.d) return false;
	return true;
}


//-----------------------------------------------------------------------------------------------------------------------
