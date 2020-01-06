#include "Distribution.h"
#include <stdio.h>
#include <string.h>
#include <assert.h>
#include "App.h"
#include <gsl/gsl_randist.h>
#include <gsl/gsl_sf_erf.h>
#include "Trust.h"
#include "Utility.h"

#define PT_DISTRIBUTION_PDF_HEIGHT 99999.9
#define MEAN_VALUE_ACCURACY 16384
vector<Distribution> defaultDistributions;

//-----------------------------------------------------------------------------------------------------------------------

float AbnormalBeta(float x, float alpha, float beta) {
	return powf(x, alpha - 1.0) * powf(1.0 - x, beta - 1.0);
}

//-----------------------------------------------------------------------------------------------------------------------

t_float PointDistribution::getPDFValue(t_float x) const {
	if (x == value) return PT_DISTRIBUTION_PDF_HEIGHT;
	else return 0;
}

//-----------------------------------------------------------------------------------------------------------------------

void PointDistribution::getPDF(float* vals, t_int nVals) const {
	for(t_int i = 0; i <= nVals; ++i) vals[i] = 0;
	t_int k = Round(value * nVals);
	if(k == 0 || k == nVals) vals[k] = 2.0 * nVals;
	else vals[k] = nVals;
}

//-----------------------------------------------------------------------------------------------------------------------

t_float PointDistribution::getPDFValueDiscrete(t_float x, t_int parts) const {
	t_float x0 = floor(x * parts) / (t_float)parts;
	t_float x1 = ceil(x * parts) / (t_float)parts;
	if(x1 <= x0) {
		if(x0 < 0.5) x1 += 1.0 / parts;
		else x0 -= 1.0 / parts;
	}
	if (value >= x0 && value <= x1) return PT_DISTRIBUTION_PDF_HEIGHT;
	else return 0;
}

//-----------------------------------------------------------------------------------------------------------------------

t_float PointDistribution::getCDFValue(t_float x) const {
	assert(x >= 0 && x <= 1);
	if (x < value) return 0;
	else return 1;
}

//-----------------------------------------------------------------------------------------------------------------------

t_float PointDistribution::getCDFValueDiscrete(t_float x, t_int parts) const {
	t_float x0 = floor(x * parts) / (t_float)parts, x1 = ceil(x * parts) / (t_float)parts;
	if(x1 <= x0) {
		if(x0 < 0.5) x1 += 1.0 / parts;
		else x0 -= 1.0 / parts;
	}
	t_float y0 = getCDFValue(x0), y1 = getCDFValue(x1);
	return y0 + (x - x0) * (y1 - y0) / (x1 - x0);
}

//-----------------------------------------------------------------------------------------------------------------------

t_float IntervalDistribution::getPDFValue(t_float x) const {
	assert(x >= 0 && x <= 1);
	if(x >= lower && x <= upper) return 1.0 / (upper - lower);
	else return 0;
}

//-----------------------------------------------------------------------------------------------------------------------

void IntervalDistribution::getPDF(float* vals, t_int nVals) const {
	t_int l = Round(lower * nVals), r = Round(upper * nVals);
	float v = nVals /((float)r - (float)l);
	for(t_int i = 0; i < l; ++i) vals[i] = 0;
	for(t_int i = l; i <= r; ++i) vals[i] = v;
	for(t_int i = r + 1; i <= nVals; ++i) vals[i] = 0;
}

//-----------------------------------------------------------------------------------------------------------------------

t_float IntervalDistribution::getPDFValueDiscrete(t_float x, t_int parts) const {
	t_float x0 = floor(x * parts) / (t_float)parts;
	t_float x1 = ceil(x * parts) / (t_float)parts;
	if(x1 <= x0) {
		if(x0 < 0.5) x1 += 1.0 / parts;
		else x0 -= 1.0 / parts;
	}
	if(x0 >= lower && x1 <= upper) return 1.0 / (upper - lower);
	else return 0;
}

//-----------------------------------------------------------------------------------------------------------------------

t_float IntervalDistribution::getCDFValue(t_float x) const {
	assert(x >= 0 && x <= 1);
	if(x < lower) return 0;
	else if (x > upper) return 1.0;
	else return (x - lower) / (upper - lower);
}

//-----------------------------------------------------------------------------------------------------------------------


t_float IntervalDistribution::getCDFValueDiscrete(t_float x, t_int parts) const {
	t_float x0 = floor(x * parts) / (t_float)parts, x1 = ceil(x * parts) / (t_float)parts;
	if(x1 <= x0) {
		if(x0 < 0.5) x1 += 1.0 / parts;
		else x0 -= 1.0 / parts;
	}
	t_float y0 = getCDFValue(x0), y1 = getCDFValue(x1);
	return y0 + (x - x0) * (y1 - y0) / (x1 - x0);
}

//-----------------------------------------------------------------------------------------------------------------------

t_float NormalDistribution::getPDFValue(t_float x) const {
	assert(x >= 0 && x <= 1);
	t_float tot = gsl_cdf_gaussian_P(1.0 - midpt, stddev) - gsl_cdf_gaussian_P(-midpt, stddev);

	return gsl_ran_gaussian_pdf(x - midpt, stddev) / tot;
}

//-----------------------------------------------------------------------------------------------------------------------

void NormalDistribution::getPDF(float* vals, t_int nVals) const {
	for(t_int i = 0; i <= nVals; ++i) vals[i] = getPDFValue((float)i / (float)nVals);
}

//-----------------------------------------------------------------------------------------------------------------------

t_float NormalDistribution::getPDFValueDiscrete(t_float x, t_int parts) const  {
	t_float x0 = floor(x * parts) / (t_float)parts;
	t_float x1 = ceil(x * parts) / (t_float)parts;
	if(x1 <= x0) {
		if(x0 < 0.5) x1 += 1.0 / parts;
		else x0 -= 1.0 / parts;
	}

	assert(x >= 0 && x <= 1);
	t_float tot = gsl_cdf_gaussian_P(1.0 - midpt, stddev) - gsl_cdf_gaussian_P(-midpt, stddev);

	return gsl_ran_gaussian_pdf((x0 + x1) / 2.0 - midpt, stddev) / tot;
}

//-----------------------------------------------------------------------------------------------------------------------

t_float NormalDistribution::getCDFValue(t_float x) const  {
	assert(x >= 0 && x <= 1);
	t_float tot = gsl_cdf_gaussian_P(1.0 - midpt, stddev) - gsl_cdf_gaussian_P(0 - midpt, stddev);

	return (gsl_cdf_gaussian_P(x - midpt, stddev) - gsl_cdf_gaussian_P(0 - midpt, stddev)) / tot;
}

//-----------------------------------------------------------------------------------------------------------------------


t_float NormalDistribution::getCDFValueDiscrete(t_float x, t_int parts) const  {
	t_float x0 = floor(x * parts) / (t_float)parts, x1 = ceil(x * parts) / (t_float)parts;
	if(x1 <= x0) {
		if(x0 < 0.5) x1 += 1.0 / parts;
		else x0 -= 1.0 / parts;
	}
	t_float y0 = getCDFValue(x0), y1 = getCDFValue(x1);
	return y0 + (x - x0) * (y1 - y0) / (x1 - x0);
}

//-----------------------------------------------------------------------------------------------------------------------

t_float BetaDistribution::getPDFValue(t_float x) const {
	return gsl_ran_beta_pdf(x, alpha, beta);
}

//-----------------------------------------------------------------------------------------------------------------------

void BetaDistribution::getPDF(float* vals, t_int nVals) const {
	vals[0] = AbnormalBeta(0.000001, alpha, beta);
	float v = vals[0] * 0.5;
	for(t_int i = 1; i < nVals; ++i) {
		vals[i] = AbnormalBeta((float)i / (float)nVals, alpha, beta);
		v += vals[i];
	}
	vals[nVals] = AbnormalBeta(.999999, alpha, beta);
	v += vals[nVals] * 0.5;
	v = (float)nVals / v;
	for(t_int i = 0; i <= nVals; ++i) vals[i] *= v;
}

//-----------------------------------------------------------------------------------------------------------------------

t_float BetaDistribution::getPDFValueDiscrete(t_float x, t_int parts) const {
	t_float x0 = floor(x * parts) / (t_float)parts;
	t_float x1 = ceil(x * parts) / (t_float)parts;
	if(x1 <= x0) {
		if(x0 < 0.5) x1 += 1.0 / parts;
		else x0 -= 1.0 / parts;
	}

	return gsl_ran_beta_pdf((x0 + x1) / 2.0, alpha, beta);
}

//-----------------------------------------------------------------------------------------------------------------------

t_float BetaDistribution::getCDFValue(t_float x) const {
	return gsl_cdf_beta_P(x, alpha, beta);
}

//-----------------------------------------------------------------------------------------------------------------------

t_float BetaDistribution::getCDFValueDiscrete(t_float x, t_int parts) const {
	t_float x0 = floor(x * parts) / (t_float)parts, x1 = ceil(x * parts) / (t_float)parts;
	if(x1 <= x0) {
		if(x0 < 0.5) x1 += 1.0 / parts;
		else x0 -= 1.0 / parts;
	}
	t_float y0 = getCDFValue(x0), y1 = getCDFValue(x1);
	return y0 + (x - x0) * (y1 - y0) / (x1 - x0);
}

//-----------------------------------------------------------------------------------------------------------------------


t_float BetaDistribution::getMean(void) const {
	return alpha / (alpha + beta);
}

//-----------------------------------------------------------------------------------------------------------------------

t_float BetaDistribution::getStddev(void) const {
	return alpha * beta / ((alpha + beta) * (alpha + beta) * (alpha + beta + 1));
}

//-----------------------------------------------------------------------------------------------------------------------

t_float BetaDistribution::getMaxDev(void) const {
	return sqrt(getMean() * (1.0 - getMean()));
}

//-----------------------------------------------------------------------------------------------------------------------

void BetaDistribution::setFromMeanDev(t_float mean, t_float stddev) {
	t_float v = mean * (1.0 - mean) / (stddev * stddev) - 1.0;
	alpha = mean * v;
	beta = (1.0 - mean) * v;
}

//-----------------------------------------------------------------------------------------------------------------------

t_float FreeformDistribution::getPDFValue(t_float x) const  {
	t_int i = x * (values.size() - 1);
	t_float over = x * (t_float)values.size() - i;
	assert(x >= 0 && x <= 1);
	if(i < values.size() - 1) return values[i] * (1.0 - over) + values[i + 1] * over;
	else return values[values.size() - 1];
}

//-----------------------------------------------------------------------------------------------------------------------

void FreeformDistribution::getPDF(float* vals, t_int nVals) const {
	// special handling of the case where nVals = values.size()
	if(nVals == values.size()) for(t_int i = 0; i <= nVals; ++i) vals[i] = values[i];
	else for(t_int i = 0; i <= nVals; ++i) vals[i] = getPDFValue((float)i / (float)nVals);
}

//-----------------------------------------------------------------------------------------------------------------------

t_float FreeformDistribution::getPDFValueDiscrete(t_float x, t_int parts) const {
	assert(parts == values.size());
	t_int i = x * parts;
	if(i < parts - 1) return values[i];
	else return values[parts - 1];
}

//-----------------------------------------------------------------------------------------------------------------------

t_float FreeformDistribution::getCDFValue(t_float x) {
	t_int i = floor(x * (values.size() - 1));
	t_float v;
	const t_float w = 0.5 / (values.size() - 1);
	assert(x >= 0 && x <= 1);

	// start with known cdf
	if(i < cdfValidity) v = cdf[i];
	else {
		if(cdfValidity) v = cdf[cdfValidity - 1];
		else v = 0;

		// add up whole values & store t_into cdf
		for(t_int j = cdfValidity; j < i; ++j) {
			v += (values[j] + values[j + 1]) * w;
			cdf[j] = v;
		}
		cdfValidity = i;
	}

	// add in partial value
	if(i < values.size() - 1) {
		t_float x0 = (t_float)i / (t_float)(values.size() - 1), x1 = x;
		t_float k = (values[i + 1] - values[i]) * (values.size() - 1), m = values[i] - k * x0;
		v += 0.5 * k * (x1 * x1 - x0 * x0) + m * x1 - m * x0;
	}
	return v;
}

//-----------------------------------------------------------------------------------------------------------------------

t_float FreeformDistribution::getCDFValueDiscrete(t_float x, t_int parts) const {
	t_int i = floor(x * parts);
	t_float v0 = 0;
	assert(x >= 0 && x <= 1);

	// add up whole values
	for(t_int j = 0; j < i; ++j) v0 += values[j];
	v0 /= parts;
	if(i < parts) {
		t_float v1 = v0 + values[i] / parts;

		// add in partial value
		t_float x0 = (t_float)i / (t_float)parts;
		t_float k = (v1 - v0) * parts;
		v0 += k * (x - x0);
	}
	return v0;
}

//-----------------------------------------------------------------------------------------------------------------------

void FreeformDistribution::normalise(void) {
	// check if < 0
	for(t_int i = 0; i < values.size(); ++i) if(values[i] < 0) values[i] = 0;

	// find total amount
	t_float amt = (values[0] + values[values.size() - 1]) * 0.5;
	for(t_int i = 1; i < values.size() - 1; ++i) amt += values[i];
	if(amt > 0) {
		amt = (values.size() - 1) / amt;
		for(t_int i = 0; i < values.size(); ++i) values[i] *= amt;
	}
	else for(t_int i = 0; i < values.size(); ++i) values[i] = 1.0;
}

//-----------------------------------------------------------------------------------------------------------------------

void FreeformDistribution::normaliseAsDiscrete(void) {
	// check if < 0
	for(t_int i = 0; i < values.size(); ++i) if(values[i] < 0) values[i] = 0;

	// find total amount
	t_float amt = 0;
	for(t_int i = 0; i < values.size(); ++i) amt += values[i];
	if(amt > 0) {
		amt = values.size() / amt;
		for(t_int i = 0; i < values.size(); ++i) values[i] *= amt;
	}
	else for(t_int i = 0; i < values.size(); ++i) values[i] = 1.0;
}

//-----------------------------------------------------------------------------------------------------------------------

void FreeformDistribution::clear(void) {
	for(t_int i = 0; i < values.size(); ++i) values[i] = 1.0;
}

//-----------------------------------------------------------------------------------------------------------------------

void FreeformDistribution::nudge(t_int d) {
	vector<t_float> v(values);
	for(t_int i = 0; i < values.size(); ++i) {
		t_int k = (i - d);
		if(k < 0) k += values.size();
		values[i] = v[k % values.size()];

	}
}

//-----------------------------------------------------------------------------------------------------------------------

bool operator==(const FreeformDistribution& lhs, const FreeformDistribution& rhs) {
	if (lhs.values.size() != rhs.values.size()) return false;
	if (lhs.cdf.size() != rhs.cdf.size()) return false;
	if (lhs.cdfValidity != rhs.cdfValidity) return false;
	return true;
}

//-----------------------------------------------------------------------------------------------------------------------

void Distribution::toTrustFunction(TrustFunction* tf) const {
	// mixed distribution
	float vals[TRUST_FUNCTION_RESOLUTION + 1];
	bool first = true;
	if(weights[DISTR_POINT] > 0) {
		dPt.getPDF(vals, TRUST_FUNCTION_RESOLUTION);
		for (t_int i = 0; i <= TRUST_FUNCTION_RESOLUTION; ++i) tf->values[i] = vals[i] * weights[DISTR_POINT];
		first = false;
	}
	if(weights[DISTR_INTERVAL] > 0) {
		dInt.getPDF(vals, TRUST_FUNCTION_RESOLUTION);
		if(first) {
			for (t_int i = 0; i <= TRUST_FUNCTION_RESOLUTION; ++i) tf->values[i] = vals[i] * weights[DISTR_INTERVAL];
			first = false;
		}
		else for (t_int i = 0; i <= TRUST_FUNCTION_RESOLUTION; ++i) tf->values[i] += vals[i] * weights[DISTR_INTERVAL];

	}
	if(weights[DISTR_NORMAL] > 0) {
		dNrm.getPDF(vals, TRUST_FUNCTION_RESOLUTION);
		if(first) {
			for (t_int i = 0; i <= TRUST_FUNCTION_RESOLUTION; ++i) tf->values[i] = vals[i] * weights[DISTR_NORMAL];
			first = false;
		}
		else for (t_int i = 0; i <= TRUST_FUNCTION_RESOLUTION; ++i) tf->values[i] += vals[i] * weights[DISTR_NORMAL];
	}
	if(weights[DISTR_BETA] > 0) {
		dBt.getPDF(vals, TRUST_FUNCTION_RESOLUTION);
		if(first) {
			for (t_int i = 0; i <= TRUST_FUNCTION_RESOLUTION; ++i) tf->values[i] = vals[i] * weights[DISTR_BETA];
			first = false;
		}
		else for (t_int i = 0; i <= TRUST_FUNCTION_RESOLUTION; ++i) tf->values[i] += vals[i] * weights[DISTR_BETA];
	}
	if(weights[DISTR_FREEFORM] > 0) {
		dFf.getPDF(vals, TRUST_FUNCTION_RESOLUTION);
		if(first) {
			for (t_int i = 0; i <= TRUST_FUNCTION_RESOLUTION; ++i) tf->values[i] = vals[i] * weights[DISTR_FREEFORM];
		}
		else for(t_int i = 0; i <= TRUST_FUNCTION_RESOLUTION; ++i) tf->values[i] += vals[i] * weights[DISTR_FREEFORM];
	}
	tf->expValid = false;
}

//-----------------------------------------------------------------------------------------------------------------------

void Distribution::mergeTrustFunctionWith(TrustFunction* tf, float w) const {
	TrustFunction tf2;
	toTrustFunction(&tf2);
	for(t_int i = 0; i <= TRUST_FUNCTION_RESOLUTION; ++i) tf->values[i] = tf->values[i] * (1.0 - w) + tf2.values[i] * w;
	tf->expValid = false;
}

//-----------------------------------------------------------------------------------------------------------------------

t_float Distribution::getPDFValue(t_float x) const {
	if(discreteParts) {
		// move x to midpt of block
		t_int i = x * discreteParts;
		if(i >= discreteParts) i = discreteParts - 1;
		x = (i + 0.5) / (t_float)discreteParts;
	}

	// return PDF value
	t_float v = 0;
	if(weights[DISTR_POINT] > 0) v += weights[DISTR_POINT] * dPt.getPDFValue(x);
	if(weights[DISTR_INTERVAL] > 0) v += weights[DISTR_INTERVAL] * dInt.getPDFValue(x);
	if(weights[DISTR_NORMAL] > 0) v += weights[DISTR_NORMAL] * dNrm.getPDFValue(x);
	if(weights[DISTR_BETA] > 0) v += weights[DISTR_BETA] * dBt.getPDFValue(x);
	if(weights[DISTR_FREEFORM] > 0) v += weights[DISTR_FREEFORM] * dFf.getPDFValue(x);
	return v;
}

//-----------------------------------------------------------------------------------------------------------------------

t_float Distribution::getCDFValue(t_float x) {
	if(discreteParts) {
		t_float v = 0;
		if(weights[DISTR_POINT] > 0) v += weights[DISTR_POINT] * dPt.getCDFValueDiscrete(x, discreteParts);
		if(weights[DISTR_INTERVAL] > 0) v += weights[DISTR_INTERVAL] * dInt.getCDFValueDiscrete(x, discreteParts);
		if(weights[DISTR_NORMAL] > 0) v += weights[DISTR_NORMAL] * dNrm.getCDFValueDiscrete(x, discreteParts);
		if(weights[DISTR_BETA] > 0) v += weights[DISTR_BETA] * dBt.getCDFValueDiscrete(x, discreteParts);
		if(weights[DISTR_FREEFORM] > 0) v += weights[DISTR_FREEFORM] * dFf.getCDFValueDiscrete(x, discreteParts);
		return v;
	}
	else {
		t_float v = 0;
		if(weights[DISTR_POINT] > 0) v += weights[DISTR_POINT] * dPt.getCDFValue(x);
		if(weights[DISTR_INTERVAL] > 0) v += weights[DISTR_INTERVAL] * dInt.getCDFValue(x);
		if(weights[DISTR_NORMAL] > 0) v += weights[DISTR_NORMAL] * dNrm.getCDFValue(x);
		if(weights[DISTR_BETA] > 0) v += weights[DISTR_BETA] * dBt.getCDFValue(x);
		if(weights[DISTR_FREEFORM] > 0) v += weights[DISTR_FREEFORM] * dFf.getCDFValue(x);
		return v;
	}

}

//-----------------------------------------------------------------------------------------------------------------------

t_float Distribution::getInverseCDFValue(t_float v, t_int nSteps) {
	t_float upper = 1, lower = 0;

	// adjust v to avoid extreme cases
	//if(v < 0.0000001) v = 0.0000001;
	//else if(v > 0.9999999) v = 0.9999999;

	// do a binary search
	for(t_int i = 0; i < nSteps; ++i) {
		t_float pos = (upper + lower) * 0.5, val = getCDFValue(pos);
		if(val < v) lower = pos;
		else if(val > v) upper = pos;
		else return pos;
	}
	return (upper + lower) * 0.5;
}

//-----------------------------------------------------------------------------------------------------------------------

t_float Distribution::getMean(void) {
	// calculate by integrating 1 - cdf(t).
	t_float integral = 0.5;
	for(t_int i = 1; i < MEAN_VALUE_ACCURACY; ++i) integral += getCDFValue((t_float)i / (t_float)MEAN_VALUE_ACCURACY);
	integral /= MEAN_VALUE_ACCURACY;
	assert(0 <= integral && integral <= 1);
	t_float v = 1.0 - integral;

	return v * (max - min) + min;
}

//-----------------------------------------------------------------------------------------------------------------------

t_float Distribution::getStddev(void) {
	// calculate by integrating 1 - t * cdf(t).
	t_float integral = 0.5;
	for(t_int i = 1; i < MEAN_VALUE_ACCURACY; ++i) integral += getCDFValue((t_float)i / (t_float)MEAN_VALUE_ACCURACY) * (t_float)i / (t_float)MEAN_VALUE_ACCURACY;
	integral /= MEAN_VALUE_ACCURACY;
	assert(0 <= integral && integral <= 1);
	t_float m = (getMean() - min) / (max - min);
	t_float v = sqrt((1 - 2 * integral) - m * m);


	return v * (max - min);
}

//-----------------------------------------------------------------------------------------------------------------------

Distribution::Distribution() {
	//fill out standard values
	min = 0;
	max = 1.0;
	dPt.value = 0.5;
	dInt.lower = 0;
	dInt.upper = 1.0;
	dNrm.midpt = 0.5;
	dNrm.stddev = 0.1;
	dBt.alpha = 2.0;
	dBt.beta = 2.0;
	dFf.values.resize(DEFAULT_DISTRIBUTION_SIZE);
	dFf.cdf.resize(DEFAULT_DISTRIBUTION_SIZE);
	for(t_int i = 0; i < dFf.values.size(); ++i) dFf.values[i] = 1.0;
	weights[DISTR_POINT] = weights[DISTR_NORMAL] = weights[DISTR_BETA] = weights[DISTR_FREEFORM] = 0;
	weights[DISTR_INTERVAL] = 1.0;
	discreteParts = 0;

}

//-----------------------------------------------------------------------------------------------------------------------

Distribution::Distribution(const TrustFunction* f) {
	//fill out standard values
	min = 0;
	max = 1.0;
	dPt.value = 0.5;
	dInt.lower = 0;
	dInt.upper = 1.0;
	dNrm.midpt = 0.5;
	dNrm.stddev = 0.1;
	dBt.alpha = 2.0;
	dBt.beta = 2.0;
	dFf.values.resize(TRUST_FUNCTION_RESOLUTION + 1);
	dFf.cdf.resize(TRUST_FUNCTION_RESOLUTION + 1);
	for(t_int i = 0; i <= TRUST_FUNCTION_RESOLUTION; ++i) dFf.values[i] = f->values[i];
	weights[DISTR_POINT] = weights[DISTR_NORMAL] = weights[DISTR_BETA] = weights[DISTR_INTERVAL] = 0;
	weights[DISTR_FREEFORM] = 1.0;
	discreteParts = 0;
}

//-----------------------------------------------------------------------------------------------------------------------

Distribution::Distribution(const Distribution& l, const Distribution& r, t_float d) {
	interpolate(l, r, d);
}

//-----------------------------------------------------------------------------------------------------------------------

void Distribution::interpolate(const Distribution& l, const Distribution& r, t_float d) {
	//fill out values
	min = l.min * (1.0 - d) + r.min * d;
	max = l.max * (1.0 - d) + r.max * d;
	dPt.value = l.dPt.value * (1.0 - d) + r.dPt.value * d;
	dInt.lower = l.dInt.lower * (1.0 - d) + r.dInt.lower * d;
	dInt.upper = l.dInt.upper * (1.0 - d) + r.dInt.upper * d;
	dNrm.midpt = l.dNrm.midpt * (1.0 - d) + r.dNrm.midpt * d;
	dNrm.stddev = l.dNrm.stddev * (1.0 - d) + r.dNrm.stddev * d;
	dBt.alpha = l.dBt.alpha * (1.0 - d) + r.dBt.alpha * d;
	dBt.beta = l.dBt.beta * (1.0 - d) + r.dBt.beta * d;
	discreteParts = Round(l.discreteParts * (1.0 - d) + r.discreteParts * d);

	// interpolate freeform distribution
	if(r.dFf.values.size() == l.dFf.values.size()) {
		dFf.values.resize(l.dFf.values.size());
		dFf.cdf.resize(l.dFf.values.size());
		for(t_int i = 0; i < dFf.values.size(); ++i) dFf.values[i] = l.dFf.values[i] * (1.0 - d) + r.dFf.values[i] * d;
	}
	else {
		// rescale freeform distributions to match
		t_int sz = Round(l.dFf.values.size() * (1.0 - d) + r.dFf.values.size() * d);
		dFf.values.resize(sz);
		dFf.cdf.resize(sz);
		for(t_int i = 0; i < dFf.values.size(); ++i) {
			dFf.values[i] = l.dFf.getPDFValue((t_float)i / (dFf.values.size() - 1)) * (1.0 - d) +
			r.dFf.getPDFValue((t_float)i / (dFf.values.size() - 1)) * d;
		}
	}
	dFf.changed();

	// adjust weights
	for(t_int i = 0; i < N_DISTRIBUTION_TYPES; ++i) weights[i] = l.weights[i] * (1.0 - d) + r.weights[i] * d;
}

//-----------------------------------------------------------------------------------------------------------------------

Distribution::Distribution(const char* name) {
	char filename[FL_PATH_MAX];
    strcpy(filename, name);
	strcat(filename, ".distr");

	TiXmlDocument f(filename);
	bool loadSucceeded = f.LoadFile();
	if(!loadSucceeded) {
		fl_alert("Failed to load distribution file %s", filename);
        return;
	}
	TiXmlElement *root = f.RootElement();

	// check if distribution file
	if(strcmp("DISTRIBUTION_FILE", root->Value())) {
		fl_alert("This is not a valid distribution file.");
		return;
	}


	// check version
	t_int vers;
	root->QueryIntAttribute("VERSION", &vers);
	if(vers < MIN_LAPUTA_VERSION) {
		fl_alert("This distribution was created with a too old version of Laputa.");
		return;
	}


	// read distribution
	*this = Distribution(root->FirstChildElement("DISTRIBUTION"));
}

//-----------------------------------------------------------------------------------------------------------------------

string Distribution::getName(void) {
	string name("Mixed distribution");
	if(weights[DISTR_POINT] == 1) name = "Point distribution";
	else if(weights[DISTR_INTERVAL] == 1) name = "Interval distribution";
	else if(weights[DISTR_NORMAL] == 1) name = "Normal distribution";
	else if(weights[DISTR_BETA] == 1) name = "Beta distribution";
	else if(weights[DISTR_FREEFORM] == 1) name = "Freeform distribution";
	else if(weights[DISTR_POINT] > 0.5) name = "Impure Point distribution";
	else if(weights[DISTR_INTERVAL] > 0.5) name = "Impure Interval distribution";
	else if(weights[DISTR_NORMAL] > 0.5) name = "Impure Normal distribution";
	else if(weights[DISTR_BETA] > 0.5) name = "Impure Beta distribution";
	else if(weights[DISTR_FREEFORM] > 0.5) name = "Impure Freeform distribution";

	return name;
}

//-----------------------------------------------------------------------------------------------------------------------

string Distribution::getDescription(void) {
	string dev = DoubleToString(getStddev());
	if(dev.size() == 0) dev = "0";
	return getName() + " (mean = " + DoubleToString(getMean()) + ", stdev = " + dev + ")";
}
//-----------------------------------------------------------------------------------------------------------------------

void Distribution::drawColumn(unsigned char* img, t_int width, t_int height, t_int xVal, t_float ht0, t_float ht1, t_float amt, bool sel)  {
	// reverse & scale heights
	ht0 = height * (1.0 - ht0);
	ht1 = height * (1.0 - ht1);

	// calculate slope in column
	t_float val, slope = ht1 - ht0;
	for(t_int i = 0; i < height; ++i)  {
		if(i + 1 <= ht0 && i + 1 <= ht1) {
			// empty; draw nothing
			val = 0;
		}
		else if(i >= ht0 && i >= ht1) {
			// draw as full
			val = 1.0;
		}
		else {
			// draw blended; classify pixel
			if(slope > 0) {
				// sloping downwards
				t_float base = ((t_float)i + 1.0 - ht0) / slope;
				t_float height = (t_float)i + 1.0 - ht0;
				val = base * height * .5;
				if(base > 1.0) val -= (base - 1.0) * ((t_float)i + 1.0 - ht1) * .5;
				if(height > 1.0) val -= (((t_float)i - ht0) / slope) * (height - 1.0) * .5;
			}
			else if(slope < 0) {
				// sloping upwards
				t_float base = (ht1 - (t_float)i - 1.0) / slope;
				t_float height = (t_float)i + 1.0 - ht1;
				val = base * height * .5;
				if(base > 1.0) val -= (base - 1.0) * ((t_float)i + 1.0 - ht0) * .5;
				if(height > 1.0) val -= ((ht1 - (t_float)i) / slope) * (height - 1.0) * .5;
			}
			else {
				// horizontal
				val = (t_float)i + 1.0 - ht0;
			}
		}

		// enter pixel
		if(val < 0) val = 0;
		if(val > 1.0) val = 1.0;
		if(sel) {
			img[(i * width + xVal) * 3 + 1] -= (t_int)(val * amt * 128.0);
			img[(i * width + xVal) * 3 + 2] -= (t_int)(val * amt * 255.0);
		}
		else {
			img[(i * width + xVal) * 3 + 1] -= (t_int)(val * amt * 255.0);
			img[(i * width + xVal) * 3 + 2] -= (t_int)(val * amt * 255.0);
		}
	}
}


//-----------------------------------------------------------------------------------------------------------------------

void Distribution::drawRect(unsigned char* img, t_int w, t_int h, t_int x0, t_int x1, t_int y0, t_int y1, t_float amt, bool sel) {
	// clip
	if(x0 < 0) x0 = 0;
	if(y0 < 0) y0 = 0;
	if(x1 >= w) x1 = w - 1;
	if(y1 >= h) y1 = h - 1;

	// draw
	for(t_int y = y0; y < y1; ++y) {
		for(t_int x = x0; x < x1; ++x) {
			if(sel) {
				img[(y * w + x) * 3 + 1] -= (t_int)(amt * 128.0);
				img[(y * w + x) * 3 + 2] -= (t_int)(amt * 255.0);
			}
			else {
				img[(y * w + x) * 3 + 1] -= (t_int)(amt * 255.0);
				img[(y * w + x) * 3 + 2] -= (t_int)(amt * 255.0);
			}

		}
	}
}

//-----------------------------------------------------------------------------------------------------------------------

void Distribution::drawDiscrete(unsigned char* img, t_int w, t_int h, t_int filter, t_float amt, bool sel) {
	t_float ht0 = 0, ht1 = 0, ht2;
	for(t_int xVal = 0; xVal < w; ++xVal) {
		if(filter == DISTR_POINT) ht2 = dPt.getCDFValueDiscrete((t_float)(xVal + 1) / (t_float)w, discreteParts);
		else if(filter == DISTR_INTERVAL) ht2 = dInt.getCDFValueDiscrete((t_float)(xVal + 1) / (t_float)w, discreteParts);
		else if(filter == DISTR_NORMAL) ht2 = dNrm.getCDFValueDiscrete((t_float)(xVal + 1) / (t_float)w, discreteParts);
		else if(filter == DISTR_BETA) ht2 = dBt.getCDFValueDiscrete((t_float)(xVal + 1) / (t_float)w, discreteParts);
		else if(filter == DISTR_FREEFORM) ht2 = dFf.getCDFValueDiscrete((t_float)(xVal + 1) / (t_float)w, discreteParts);
		else ht2 = getCDFValue((t_float)(xVal + 1) / w);

		if((xVal  * discreteParts / w) & 1) drawColumn(img, w, h, xVal, (ht1 - ht0) * w  / DISTRIBUTION_VIEW_HEIGHT, (ht2 - ht1) * w / DISTRIBUTION_VIEW_HEIGHT, amt, sel);
		else drawColumn(img, w, h, xVal, (ht1 - ht0) * w  / DISTRIBUTION_VIEW_HEIGHT, (ht2 - ht1) * w / DISTRIBUTION_VIEW_HEIGHT, amt * 0.75, sel);
		ht0 = ht1;
		ht1 = ht2;
	}

}

//-----------------------------------------------------------------------------------------------------------------------

void Distribution::draw(unsigned char* img, t_int w, t_int h, t_int filter, t_float amt, bool sel) {
	if(discreteParts > 0 && discreteParts < w) drawDiscrete(img, w, h, filter, amt, sel);
	else {
		t_float ht0 = 0, ht1 = 0, ht2;
		for(t_int xVal = 0; xVal < w; ++xVal) {
			if(filter == DISTR_POINT) ht2 = dPt.getCDFValue((t_float)(xVal + 1) / (t_float)w);
			else if(filter == DISTR_INTERVAL) ht2 = dInt.getCDFValue((t_float)(xVal + 1) / (t_float)w);
			else if(filter == DISTR_NORMAL) ht2 = dNrm.getCDFValue((t_float)(xVal + 1) / (t_float)w);
			else if(filter == DISTR_BETA) ht2 = dBt.getCDFValue((t_float)(xVal + 1) / (t_float)w);
			else if(filter == DISTR_FREEFORM) ht2 = dFf.getCDFValue((t_float)(xVal + 1) / (t_float)w);
			else ht2 = getCDFValue((t_float)(xVal + 1) / w);

			drawColumn(img, w, h, xVal, (ht1 - ht0) * w  / DISTRIBUTION_VIEW_HEIGHT, (ht2 - ht1) * w / DISTRIBUTION_VIEW_HEIGHT, amt, sel);
			ht0 = ht1;
			ht1 = ht2;
		}
	}
}


//-----------------------------------------------------------------------------------------------------------------------

Distribution::Distribution(TiXmlElement* xml) {

	xml->QueryFloatAttribute("MIN", &min);
	xml->QueryFloatAttribute("MAX", &max);
	xml->QueryIntAttribute("DISCRETE_PARTS", &discreteParts);
	xml->QueryFloatAttribute("WEIGHT_POINT", &weights[DISTR_POINT]);
	xml->QueryFloatAttribute("WEIGHT_INTERVAL", &weights[DISTR_INTERVAL]);
	xml->QueryFloatAttribute("WEIGHT_NORMAL", &weights[DISTR_NORMAL]);
	xml->QueryFloatAttribute("WEIGHT_BETA", &weights[DISTR_BETA]);
	xml->QueryFloatAttribute("WEIGHT_FREEFORM", &weights[DISTR_FREEFORM]);
	xml->QueryFloatAttribute("POINT_VALUE", &dPt.value);
	xml->QueryFloatAttribute("INTERVAL_LOWER", &dInt.lower);
	xml->QueryFloatAttribute("INTERVAL_UPPER", &dInt.upper);
	xml->QueryFloatAttribute("NORMAL_MIDPT", &dNrm.midpt);
	xml->QueryFloatAttribute("NORMAL_STDDEV", &dNrm.stddev);
	xml->QueryFloatAttribute("BETA_ALPHA", &dBt.alpha);
	xml->QueryFloatAttribute("BETA_BETA", &dBt.beta);
	t_int sz;
	xml->QueryIntAttribute("FREEFORM_RESOLUTION", &sz);
	dFf.values.resize(sz);
	stringstream ss(string(xml->GetText()));
	for(t_int i = 0; i < sz; ++i) ss >> dFf.values[i];
	dFf.cdf.resize(sz);

	// normalise, in case saving to xml has lost precision
	dFf.normalise();
}

//-----------------------------------------------------------------------------------------------------------------------

void Distribution::setResolution(t_int nValues) {
	assert(discreteParts == 0);
	Distribution d(*this);

	// resample
	dFf.values.resize(nValues);
	for(t_int i = 0; i < nValues; ++i)
		dFf.values[i] = d.dFf.getPDFValue((t_float)i / (nValues - 1));
	dFf.cdf.resize(nValues);
	dFf.normalise();
	dFf.changed();
}

//-----------------------------------------------------------------------------------------------------------------------

void Distribution::setDiscreteRange(t_int minimum, t_int maximum) {
	if(maximum >= minimum) {
		discreteParts = maximum - minimum + 1;
		min = minimum;
		max = maximum;

		// resample
		Distribution d(*this);
		dFf.values.resize(maximum - minimum + 1);
		for(t_int i = 0; i < maximum - minimum + 1; ++i)
			dFf.values[i] = d.dFf.getPDFValue((t_float)i / (maximum - minimum + 1));
		dFf.normalise();
	}
	else {
		discreteParts = 0;
		setResolution(DEFAULT_DISTRIBUTION_SIZE);
	}
}

//-----------------------------------------------------------------------------------------------------------------------

void Distribution::setWeight(t_int w, t_float v) {
	t_float wg[N_DISTRIBUTION_TYPES];
	t_float diff = v - weights[w], pTot = 1.0 - weights[w], tot = 0;

	// calculate new weights according to a Jeffrey conditionalisation
	for(t_int i = 0; i < N_DISTRIBUTION_TYPES; ++i) {
		if(i != w) wg[i] = weights[i] - diff * weights[i] / pTot;
		else wg[i] = v;
		if(wg[i] > 1.0) wg[i] = 1.0;
		if(wg[i] < 0) wg[i] = 0;
		tot += wg[i];
	}

	// make sure everything is nicely normalised
	if(tot > 0) for(t_int i = 0; i < N_DISTRIBUTION_TYPES; ++i) weights[i] = wg[i] / tot;
}

//-----------------------------------------------------------------------------------------------------------------------

t_float Distribution::getRandomValue(void) {
	t_float v = getInverseCDFValue(gsl_rng_uniform(rng));
	assert(v >= 0 && v <= 1.0);
	return v * (max - min) + min;
}

//-----------------------------------------------------------------------------------------------------------------------

TiXmlElement* Distribution::toXML(const char *name) const {
	TiXmlElement* xml;
	if(name) xml = new TiXmlElement(name);
	else xml = new TiXmlElement("DISTRIBUTION");

	// write parameters
	xml->SetDoubleAttribute("MIN", min);
	xml->SetDoubleAttribute("MAX", max);
	xml->SetAttribute("DISCRETE_PARTS", discreteParts);
	xml->SetDoubleAttribute("WEIGHT_POINT", weights[DISTR_POINT]);
	xml->SetDoubleAttribute("WEIGHT_INTERVAL", weights[DISTR_INTERVAL]);
	xml->SetDoubleAttribute("WEIGHT_NORMAL", weights[DISTR_NORMAL]);
	xml->SetDoubleAttribute("WEIGHT_BETA", weights[DISTR_BETA]);
	xml->SetDoubleAttribute("WEIGHT_FREEFORM", weights[DISTR_FREEFORM]);
	xml->SetDoubleAttribute("POINT_VALUE", dPt.value);
	xml->SetDoubleAttribute("INTERVAL_LOWER", dInt.lower);
	xml->SetDoubleAttribute("INTERVAL_UPPER", dInt.upper);
	xml->SetDoubleAttribute("NORMAL_MIDPT", dNrm.midpt);
	xml->SetDoubleAttribute("NORMAL_STDDEV", dNrm.stddev);
	xml->SetDoubleAttribute("BETA_ALPHA", dBt.alpha);
	xml->SetDoubleAttribute("BETA_BETA", dBt.beta);
	xml->SetAttribute("FREEFORM_RESOLUTION", dFf.values.size());
	stringstream ss;
	for(t_int i = 0; i < dFf.values.size(); ++i) {
		ss << dFf.values[i];
		if(i != dFf.values.size() - 1) ss << " ";
	}
	TiXmlText* txt = new TiXmlText(ss.str().c_str());
	xml->LinkEndChild(txt);

	return xml;
}

//-----------------------------------------------------------------------------------------------------------------------

bool operator==(const Distribution& lhs, const Distribution& rhs) {
	if (lhs.min != rhs.min) return false;
	if (lhs.max != rhs.max) return false;
	if (lhs.discreteParts != rhs.discreteParts) return false;
	if (lhs.dPt != rhs.dPt) return false;
	if (lhs.dInt != rhs.dInt) return false;
	if (lhs.dNrm != rhs.dNrm) return false;
	if (lhs.dBt != rhs.dBt) return false;
	if (lhs.dFf != rhs.dFf) return false;
	for (t_int i = 0; i < N_DISTRIBUTION_TYPES; ++i) if (lhs.weights[i] != rhs.weights[i]) return false;

	return true;
}

//-----------------------------------------------------------------------------------------------------------------------

void LoadDefaultDistributions(const char* directory) {
	const char* names[N_DEFAULT_DISTRIBUTIONS] = {	"interval_whole", "interval_upper", "interval_lower",
		"beta_neutral", "beta_upper", "beta_lower"};

	defaultDistributions.resize(N_DEFAULT_DISTRIBUTIONS);
	for(t_int i = 0; i < N_DEFAULT_DISTRIBUTIONS; ++i) {
        char str[FL_PATH_MAX];
        strcpy(str, directory);
        strcat(str, names[i]);
        defaultDistributions[i] = Distribution(str);
    }
}
