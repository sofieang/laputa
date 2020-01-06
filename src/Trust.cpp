#include "Trust.h"
#include "App.h"
#include <FL/fl_draw.H>
#include <cmath>
#ifdef _WINDOWS
#include <xmmintrin.h>
#endif

//-----------------------------------------------------------------------------------------------------------------------

TrustFunction::TrustFunction() {
	view = 0;
	expValid = false;

	// align to 16 bytes
	if ((size_t)valueBlock & 0x0F) values = (float*)(((size_t)valueBlock | 0x0F) + 1);
	else values = valueBlock;
}
//-----------------------------------------------------------------------------------------------------------------------

TrustFunction::TrustFunction(const TrustFunction& tf) {
	// align to 16 bytes
	if ((size_t)valueBlock & 0x0F) values = (float*)(((size_t)valueBlock | 0x0F) + 1);
	else values = valueBlock;

	// copy
	for (t_int i = 0; i <= TRUST_FUNCTION_RESOLUTION; ++i) values[i] = tf.values[i];
	view = tf.view;
	expValue = tf.expValue;
	expValid = tf.expValid;
}

//-----------------------------------------------------------------------------------------------------------------------

TrustFunction::TrustFunction(TiXmlElement* xml) {
	view = 0;
	expValid = false;

	// align to 16 bytes
	if ((size_t)valueBlock & 0x0F) values = (float*)(((size_t)valueBlock | 0x0F) + 1);
	else values = valueBlock;

	// load
	stringstream ss(string(xml->GetText()));
	for(t_int i = 0; i <= TRUST_FUNCTION_RESOLUTION; ++i) ss >> values[i];
}

//-----------------------------------------------------------------------------------------------------------------------

TrustFunction::~TrustFunction(void) {
	if(view) view->removeTrustFunction(this);
}

//-----------------------------------------------------------------------------------------------------------------------

TrustFunction& TrustFunction::operator=(const TrustFunction& tf) {
	for (t_int i = 0; i <= TRUST_FUNCTION_RESOLUTION; ++i) values[i] = tf.values[i];
	view = tf.view;
	expValue = tf.expValue;
	expValid = tf.expValid;
	return *this;
}

//-----------------------------------------------------------------------------------------------------------------------

void TrustFunction::setFromPreset(t_int presetValue) {
	// uses the value, sharpness representation
	const t_float vals[8] = {.5f, 0, .05f, .25f, .5f, .75f, .95f, 1.0f};
	const t_float shrp[8] = { 0,  .5f, .5f, .5f, .5f, .5f, .5f, .5f};

	// fill out
	setValueSharpness(vals[presetValue], shrp[presetValue]);
}

//-----------------------------------------------------------------------------------------------------------------------

void TrustFunction::setValueSharpness(t_float val, t_float sharpness) {
	// Uses the Beta distribution, and calculates alpha, beta for the beta distribution from the formulas
	//    alpha = (0.1 + val * 99.9)^sharpness
	//    beta  = (0.1 + (1 - val) * 99.9)^sharpness
	// in order to make the distribution easier to visualize. val and sharpness are assumed to be in the [0, 1] range.
	t_float alpha = pow((t_float)(0.1 + val * 99.9), sharpness);
	t_float beta = pow((t_float)(0.1 + (1 - val) * 99.9), sharpness);

	// fill out
	for(t_int i = 0; i <= TRUST_FUNCTION_RESOLUTION; ++i)
		values[i] = AbnormalBeta((t_float)i / TRUST_FUNCTION_RESOLUTION * .998 + .001, alpha, beta);

	normalise();
}

//-----------------------------------------------------------------------------------------------------------------------

inline void TrustFunction::normalise(void) {
	t_float vTotal = values[0] * .5;
	for(t_int i = 1; i < TRUST_FUNCTION_RESOLUTION; ++i) vTotal += values[i];
	vTotal += values[TRUST_FUNCTION_RESOLUTION] * .5;
	if(vTotal > 0) {
		vTotal = TRUST_FUNCTION_RESOLUTION / vTotal;
		for(t_int i = 0; i <= TRUST_FUNCTION_RESOLUTION; ++i) values[i] *= vTotal;
	}
	else for(t_int i = 0; i <= TRUST_FUNCTION_RESOLUTION; ++i) values[i] = 1.0;

	expValid = false;
}

//-----------------------------------------------------------------------------------------------------------------------

float TrustFunction::getValue(float pos) {
	t_int i = pos * TRUST_FUNCTION_RESOLUTION;
	float dx = pos * TRUST_FUNCTION_RESOLUTION - (float)i, slope;
	if(i < TRUST_FUNCTION_RESOLUTION) slope = values[i + 1] - values[i];
	else slope = 0;

	return values[i] + dx * slope;
}

//-----------------------------------------------------------------------------------------------------------------------
#ifdef __SSE9__
// SSE optimised semi-assembler, since this is one of the functions the program spends the most time in
float TrustFunction::expectation(void) {
	// returns t_integral(r * fn(r)). Calculated by approximating the function to be linear between the points. It must be normalised!
	__m128 f0, f1, fSlope, rSlope, r0, v1, v2, v3, sum, half, third;
#ifdef _WINDOWS
	__declspec(align(16)) static float r0vec[TRUST_FUNCTION_RESOLUTION];
#endif
#ifdef __APPLE__
	static float r0vec[TRUST_FUNCTION_RESOLUTION] __attribute__((aligned(16)));
#endif
	static bool firstTime = true;

	if(firstTime) {
		// fill out vector of r0 values
		for(t_int i = 0; i < TRUST_FUNCTION_RESOLUTION; ++i) r0vec[i] = i * TRUST_FUNCTION_RESOLUTION_INV;
		firstTime = false;
	}

	if(!expValid) {
		// init values
//#ifdef _WINDOWS

		_mm_prefetch((char const*)r0vec, 0);
		_mm_prefetch((char const*)values, 0);
//#endif
		sum = _mm_setzero_ps();
		half = _mm_set1_ps(0.5);
		third = _mm_set1_ps(0.33333333333333333333333);
		rSlope = _mm_set1_ps(TRUST_FUNCTION_RESOLUTION_INV);
		f0 = _mm_load_ps(values);
		for(t_int i = 0; i < TRUST_FUNCTION_RESOLUTION; i += 4) {
			// get equation for trust function line
			f1 = _mm_load_ps(values + i + 4);
			fSlope = _mm_move_ss(f0, f1);
			fSlope = _mm_shuffle_ps(fSlope, fSlope, _MM_SHUFFLE(0, 3, 2, 1));
			fSlope = _mm_sub_ps(fSlope, f0);

			// add size of this segment to value
			r0 = _mm_load_ps(r0vec + i);				// r0 * f0
			v1 = _mm_mul_ps(r0, f0);

			v2 = _mm_mul_ps(r0, fSlope);				// (r0 * fslope + f0 * rSlope) / 2
			v3 = _mm_mul_ps(f0, rSlope);
			v2 = _mm_add_ps(v2, v3);
			v2 = _mm_mul_ps(v2, half);

			v3 = _mm_mul_ps(fSlope, rSlope);			// fslope * rSlope / 3
			v3 = _mm_mul_ps(v3, third);

			v1 = _mm_add_ps(v1, v2);					// add together everything
			v1 = _mm_add_ps(v1, v3);
			sum = _mm_add_ps(sum, v1);

			f0 = f1;
		}

		// add together sum
		sum = _mm_hadd_ps(sum, sum);
		sum = _mm_hadd_ps(sum, sum);
		_mm_store_ss(&expValue, sum);
		expValue *= TRUST_FUNCTION_RESOLUTION_INV;
		expValid = true;
	}

	return expValue;
}

#else

// C++ version of the same code
float TrustFunction::expectation(void) {
	// returns t_integral(r * fn(r)). Calculated by approximating the function to be linear between the points. It must be normalised!
	float val = 0, r0 = 0;
	const float rSlope = TRUST_FUNCTION_RESOLUTION_INV;

	if(!expValid) {
		for(t_int i = 0; i < TRUST_FUNCTION_RESOLUTION; ++i) {
			// get equation for trust function line
			float f0 = values[i];
			float fslope = values[i + 1] - f0;

			// add size of this segment to value
			val += r0 * f0 + (r0 * fslope + f0 * rSlope) * .5 + fslope * rSlope * .333333333333333333;

			// increment start of reliability line
			r0 += rSlope;

		}
		expValue = val * TRUST_FUNCTION_RESOLUTION_INV;
		expValid = true;
	}

	return expValue;
}

#endif

//-----------------------------------------------------------------------------------------------------------------------
// SSE optimised semi-assembler, since this is one of the functions the program spends the most time in
#ifdef __SSE9__
void TrustFunction::update(float belief, bool pTrue) {
	__m128 beliefVec, beliefVecInv, r, rInv, dr, vTotalVec, v1, v2, v3;
#ifdef _WINDOWS
	__declspec(align(16)) static const float rVec[4] = {0, TRUST_FUNCTION_RESOLUTION_INV, 2 * TRUST_FUNCTION_RESOLUTION_INV, 3 * TRUST_FUNCTION_RESOLUTION_INV};
	__declspec(align(16)) static const float rVecInv[4] = {1.0 , 1.0 - TRUST_FUNCTION_RESOLUTION_INV, 1.0 - 2 * TRUST_FUNCTION_RESOLUTION_INV, 1.0 - 3 * TRUST_FUNCTION_RESOLUTION_INV};
#endif
#ifdef __APPLE__
	static const float rVec[4] __attribute((aligned(16))) = {0, TRUST_FUNCTION_RESOLUTION_INV, 2 * TRUST_FUNCTION_RESOLUTION_INV, 3 * TRUST_FUNCTION_RESOLUTION_INV};
	static const float rVecInv[4] __attribute((aligned(16))) = {1.0 , 1.0 - TRUST_FUNCTION_RESOLUTION_INV, 1.0 - 2 * TRUST_FUNCTION_RESOLUTION_INV, 1.0 - 3 * TRUST_FUNCTION_RESOLUTION_INV};
#endif
	float vTotal;

	// init
	beliefVec = _mm_set1_ps(belief);
	beliefVecInv = _mm_set1_ps(1.0 - belief);
	if(pTrue) {
		r = _mm_load_ps(rVec);
		rInv = _mm_load_ps(rVecInv);
		dr = _mm_set1_ps(TRUST_FUNCTION_RESOLUTION_INV * 4.0);
	}
	else {
		r = _mm_load_ps(rVecInv);
		rInv = _mm_load_ps(rVec);
		dr = _mm_set1_ps(-TRUST_FUNCTION_RESOLUTION_INV * 4.0);
	}

	// update trust values
	vTotalVec = _mm_setzero_ps();
	for(t_int i = 0; i < TRUST_FUNCTION_RESOLUTION; i += 4) {
		v1 = _mm_load_ps(values + i);
		v2 = _mm_mul_ps(r, beliefVec);
		v3 = _mm_mul_ps(rInv, beliefVecInv);
		v2 = _mm_add_ps(v2, v3);
		v1 = _mm_mul_ps(v1, v2);
		_mm_store_ps(values + i, v1);
		vTotalVec = _mm_add_ps(vTotalVec, v1);

		// update r
		r = _mm_add_ps(r, dr);
		rInv = _mm_sub_ps(rInv, dr);
	}

	// get total
	vTotalVec = _mm_hadd_ps(vTotalVec, vTotalVec);
	vTotalVec = _mm_hadd_ps(vTotalVec, vTotalVec);
	_mm_store_ss(&vTotal, vTotalVec);

	// take care of last value
	if(pTrue) values[TRUST_FUNCTION_RESOLUTION] *= belief;
	else values[TRUST_FUNCTION_RESOLUTION] *= 1.0 - belief;

	// only use half of first & last values in total
	vTotal += values[TRUST_FUNCTION_RESOLUTION] * .5;
	vTotal -= values[0] * .5;

	// normalise
	if(vTotal > 0) {
		vTotal = TRUST_FUNCTION_RESOLUTION / vTotal;
		vTotalVec = _mm_set1_ps(vTotal);
		for(t_int i = 0; i < TRUST_FUNCTION_RESOLUTION; i += 4) {
			v1 = _mm_load_ps(values + i);
			v1 = _mm_mul_ps(v1, vTotalVec);
			_mm_store_ps(values + i, v1);
		}
		values[TRUST_FUNCTION_RESOLUTION] *= vTotal;
	}
	else for(t_int i = 0; i <= TRUST_FUNCTION_RESOLUTION; ++i) values[i] = 1.0;

	// mark up expectation as needing to be recalculated
	expValid = false;
}

#else
// C++ version of the same code
void TrustFunction::update(float belief, bool pTrue) {
	float negBelief = 1.0 - belief, r, dr;

	if(pTrue) {
		r = 0;
		dr = TRUST_FUNCTION_RESOLUTION_INV;
	}
	else {
		r = 1.0;
		dr = -TRUST_FUNCTION_RESOLUTION_INV;
	}

	// first value
	values[0] *= r * belief + (1.0 - r) * negBelief;
	float vTotal = values[0] * .5;
	r += dr;

	// mid-lying values
	for(t_int t = 1; t < TRUST_FUNCTION_RESOLUTION; ++t) {
		values[t] *= r * belief + (1.0 - r) * negBelief;
		vTotal += values[t];
		r += dr;
	}

	// last value
	values[TRUST_FUNCTION_RESOLUTION] *= r * belief + (1.0 - r) * negBelief;
	vTotal += values[TRUST_FUNCTION_RESOLUTION] * .5;

	// normalise
	if(vTotal > 0) {
		vTotal = TRUST_FUNCTION_RESOLUTION / vTotal;
		for(t_int t = 0; t <= TRUST_FUNCTION_RESOLUTION; ++t) values[t] *= vTotal;
	}
	else for(t_int t = 0; t <= TRUST_FUNCTION_RESOLUTION; ++t) values[t] = 1.0;

	// mark up expectation as needing to be recalculated
	expValid = false;
}

#endif

//-----------------------------------------------------------------------------------------------------------------------

TiXmlElement* TrustFunction::toXML(void) const {
	TiXmlElement *tf = new TiXmlElement("TRUST_FUNCTION");
	tf->SetAttribute("RESOLUTION", TRUST_FUNCTION_RESOLUTION);
	stringstream ss;
	for(t_int i = 0; i <= TRUST_FUNCTION_RESOLUTION; ++i) {
		ss << values[i];
		if(i != TRUST_FUNCTION_RESOLUTION) ss << " ";
	}
	TiXmlText* txt = new TiXmlText(ss.str().c_str());
	tf->LinkEndChild(txt);

	return tf;
}

//-----------------------------------------------------------------------------------------------------------------------

TrustView::TrustView(t_int x, t_int y, t_int w, t_int h, const char *label) : Fl_Button(x, y, w, h, label) {
    img = new unsigned char [w * h * 3];
}

//-----------------------------------------------------------------------------------------------------------------------

TrustView::~TrustView(void) {
	removeAllTrustFunctions();
	delete [] img;
}

//-----------------------------------------------------------------------------------------------------------------------

void TrustView::setTrustFunction(TrustFunction *f) {
	functions.insert(f);
	redraw();
}

//-----------------------------------------------------------------------------------------------------------------------

void TrustView::removeTrustFunction(TrustFunction *f) {
	functions.erase(f);
}

//-----------------------------------------------------------------------------------------------------------------------

void TrustView::removeAllTrustFunctions(void) {
	std::set<TrustFunction*>::iterator fn(functions.begin());
	while(fn != functions.end()) {
		(*fn)->view = NULL;
		++fn;
	}
	functions.clear();
}

//-----------------------------------------------------------------------------------------------------------------------

TrustFunction* TrustView::getAverageTrustFunction(void) {
	for(t_int i = 0; i <= TRUST_FUNCTION_RESOLUTION; ++i) avgTrustFunction.values[i] = 0;
	std::set<TrustFunction*>::iterator fn(functions.begin());
	while(fn != functions.end()) {
		for(t_int i = 0; i <= TRUST_FUNCTION_RESOLUTION; ++i) avgTrustFunction.values[i] += (*fn)->values[i] / functions.size();
		++fn;
	}
	return &avgTrustFunction;
}

//-----------------------------------------------------------------------------------------------------------------------

void TrustView::drawColumn(t_int xVal, t_float ht0, t_float ht1, t_float amt, bool sel)  {
	t_float val, slope = ht1 - ht0;
	for(t_int i = 0; i < h(); ++i)  {
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
			img[(i * w() + xVal) * 3 + 1] -= (t_int)(val * amt * 128.0);
			img[(i * w() + xVal) * 3 + 2] -= (t_int)(val * amt * 255.0);
		}
		else {
			img[(i * w() + xVal) * 3 + 1] -= (t_int)(val * amt * 255.0);
			img[(i * w() + xVal) * 3 + 2] -= (t_int)(val * amt * 255.0);
		}
	}
}

//-----------------------------------------------------------------------------------------------------------------------

void TrustView::draw(void) {
	// is control active?
	if(active() && functions.size()) {
		// clear
		if(value()) for(t_int i = 0; i < w() * h(); ++i) {
			img[i * 3] = img[i * 3 + 1] = 0xFF;
			img[i * 3 + 2] = 0x00;
		}
		else memset(img, 0xFF, h() * w() * 3);

		// draw composite of trust functions
		std::set<TrustFunction*>::iterator fn = functions.begin();
		while(fn != functions.end()) {
			t_float ht0, ht1;
			ht0 = h() * (1.0 - (*fn)->getValue(0) / DISTRIBUTION_VIEW_HEIGHT);
			for(t_int xVal = 0; xVal < w(); ++xVal) {
				ht1 = h() * (1.0 - (*fn)->getValue((t_float)(xVal + 1) / (t_float)w()) / DISTRIBUTION_VIEW_HEIGHT);
				drawColumn(xVal, ht0, ht1, 1.0 / functions.size(), value() != false);
				ht0 = ht1;
			}
			++fn;
		}

		// draw image
		fl_draw_image(img, x(), y(), w(), h());
	}
	else {
		fl_draw_box(FL_FLAT_BOX, x(), y(), w(), h(), FL_WHITE);
		fl_font(FL_HELVETICA | FL_BOLD, 10);
		fl_color(FL_INACTIVE_COLOR);
		fl_draw("N/A", x() + w() / 2 - fl_width("N/A") / 2, y() + h() / 2 + 5);
	}
}

//-----------------------------------------------------------------------------------------------------------------------

void TrustView::deactivate() {
	removeAllTrustFunctions();
	Fl_Widget::deactivate();
}

//-----------------------------------------------------------------------------------------------------------------------

void TrustView::setFromDistribution(const Distribution& d) {
	std::set<TrustFunction*>::iterator fn(functions.begin());
	while(fn != functions.end()) {
		d.toTrustFunction(*fn);
		++fn;
	}
	redraw();
}

//-----------------------------------------------------------------------------------------------------------------------

bool operator==(const TrustFunction& lhs, const TrustFunction& rhs) {
    if (lhs.view != rhs.view) return false;
	if (lhs.expValue != rhs.expValue) return false;
	for (t_int i = 0; i <= TRUST_FUNCTION_RESOLUTION; ++i) if (lhs.values[i] != rhs.values[i]) return false;
	return true;
}
