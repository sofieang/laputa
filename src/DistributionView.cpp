//#include "DistributionView.h"
#include "App.h"
#include <FL/fl_draw.H>

#include <cmath>
#include "UserInterfaceItems.h"
#include "Utility.h"

#define METADISTRIBUTION_PIECES 16


//-----------------------------------------------------------------------------------------------------------------------

DistributionView::DistributionView(t_int x, t_int y, t_int w, t_int h, const char *label) : Fl_Button(x, y, w, h, label)  {
	filter = DISTR_ALL;
	img = new unsigned char [w * h * 3];
}

//-----------------------------------------------------------------------------------------------------------------------

DistributionView::~DistributionView()  {
	delete [] img;
}

//-----------------------------------------------------------------------------------------------------------------------

void DistributionView::setDistribution(Distribution* d) {
	distributionShown = d;
	redraw();
}

//-----------------------------------------------------------------------------------------------------------------------

void DistributionView::draw(void) {
	if(active()) {
		// create image
		if(value()) for(t_int i = 0; i < w() * h(); ++i) {
			img[i * 3] = img[i * 3 + 1] = 0xFF;
			img[i * 3 + 2] = 0x00;
		}
		else memset(img, 0xFF, h() * w() * 3);
		distributionShown->draw(img, w(), h(), filter, 1.0, value() != 0);

		// draw image
		fl_draw_image(img, x(), y(), w(), h());

		// draw type
		fl_font(FL_HELVETICA | FL_ITALIC, 10);
		fl_draw(distributionShown->getName().c_str(), x() + 2, y() + 2 - fl_descent() + fl_height());

		// mean & standard deviation
		char meanStr[32], stddevStr[32];
		wchar_t wstr = 0x03BC;
		fl_utf8fromwc(meanStr, 32, &wstr, 1);
		strcat(meanStr, " = ");
		strcat(meanStr, DoubleToString(distributionShown->getMean()));
		wstr = 0x03C3;
		fl_utf8fromwc(stddevStr, 32, &wstr, 1);
		strcat(stddevStr, " = ");
		t_float dev = distributionShown->getStddev();
		if(strlen(DoubleToString(dev))) strcat(stddevStr, DoubleToString(dev));
		else strcat(stddevStr, "0");
		fl_color(FL_BLACK);
		fl_draw(meanStr, x() + w() - 2 - fl_width(meanStr), y() - fl_descent() + fl_height());
		fl_draw(stddevStr, x() + w() - 2 - fl_width(stddevStr), y() - fl_descent() + 2 * fl_height());
	}
	else {
		// inactive
		//fl_draw_box(FL_FLAT_BOX, x(), y(), w(), h(), FL_WHITE);
	}
}

//-----------------------------------------------------------------------------------------------------------------------

MetaDistributionView::MetaDistributionView(t_int x, t_int y, t_int w, t_int h, const char *label) : Fl_Button(x, y, w, h, label)  {
	img = new unsigned char [w * h * 3];
}

//-----------------------------------------------------------------------------------------------------------------------

MetaDistributionView::~MetaDistributionView()  {
	delete [] img;
}

//-----------------------------------------------------------------------------------------------------------------------

void MetaDistributionView::setMetaDistribution(const MetaDistribution& m) {
	md = m;
	//mdShown = NULL;
	redraw();
}

//-----------------------------------------------------------------------------------------------------------------------

void MetaDistributionView::setMetaDistribution(MetaDistribution* m) {
	mdShown = m;
	md = *m;
	redraw();
}

//-----------------------------------------------------------------------------------------------------------------------

void MetaDistributionView::draw(void) {
	fl_push_clip(x(), y(), w(), h());

	// is control active?
	if(active()) {
		// clear area
		if(value()) for(t_int i = 0; i < w() * h(); ++i) {
			img[i * 3] = img[i * 3 + 1] = 0xFF;
			img[i * 3 + 2] = 0x00;
		}
		else memset(img, 0xFF, h() * w() * 3);

		// create image
		for(t_int i = 0; i <= METADISTRIBUTION_PIECES; ++i) {
			Distribution d = md.getDistribution(md.mixture.getInverseCDFValue((t_float)i / METADISTRIBUTION_PIECES));
			d.draw(img, w(), h(), DISTR_ALL, 1.0 / (METADISTRIBUTION_PIECES + 1), value() != false);
		}

		// draw image
		fl_draw_image(img, x(), y(), w(), h());
	}
	fl_pop_clip();
}

//-----------------------------------------------------------------------------------------------------------------------

void DistributionViewEditable::setDistribution(const Distribution& d) {
	distributionShown = &curDistribution;
	curDistribution = d;
	redraw();
}

//-----------------------------------------------------------------------------------------------------------------------

void DistributionViewEditable::setDistribution(Distribution* d) {
	curDistribution = *d;
	DistributionView::setDistribution(d);
}


//-----------------------------------------------------------------------------------------------------------------------

t_int DistributionViewEditable::handle(t_int event) {
	switch(filter) {
		case DISTR_POINT:
		case DISTR_INTERVAL:
		case DISTR_NORMAL:
			if(event == FL_PUSH || event == FL_DRAG) {
				t_int x0 = Fl::event_x() - x();
				if(x0 < 0) x0 = 0;
				if(x0 > w()) x0 = w() - 1;
				if(filter == DISTR_POINT) {
					curDistribution.dPt.value = (t_float)x0 / (t_float)(w() - 1);
					distributionWindow->sliderPtValue->value(curDistribution.dPt.value * (curDistribution.max - curDistribution.min) + curDistribution.min);
				}
				else if(filter == DISTR_INTERVAL) {
					t_float r = (curDistribution.dInt.upper - curDistribution.dInt.lower) * (w() - 1) / 2;
					curDistribution.dInt.lower = ((t_float)x0 - r) / (t_float)(w() - 1);
					curDistribution.dInt.upper = ((t_float)x0 + r) / (t_float)(w() - 1);
					if(curDistribution.dInt.lower < 0) curDistribution.dInt.lower = 0;
					if(curDistribution.dInt.upper > 1) curDistribution.dInt.upper = 1;
					if(curDistribution.dInt.upper < curDistribution.dInt.lower + 0.001) {
						if(curDistribution.dInt.upper > 0.5) curDistribution.dInt.lower = curDistribution.dInt.upper - 0.001;
						else curDistribution.dInt.upper = curDistribution.dInt.lower + 0.001;
					}
					distributionWindow->sliderIntLower->value(curDistribution.dInt.lower * (curDistribution.max - curDistribution.min) + curDistribution.min);
					distributionWindow->sliderIntUpper->value(curDistribution.dInt.upper * (curDistribution.max - curDistribution.min) + curDistribution.min);
				}
				else {
					curDistribution.dNrm.midpt = (t_float)x0 / (t_float)(w() - 1);
					distributionWindow->sliderNrmMidpt->value(curDistribution.dNrm.midpt);
				}

				redraw();
				return 1;
			}
			break;
		case DISTR_BETA:
			if(event == FL_PUSH || event == FL_DRAG) {
				t_int x0 = Fl::event_x() - x();
				if(x0 < 1) x0 = 1;
				if(x0 >= w() - 1) x0 = w() - 2;
				t_float mid = (t_float)x0 / (t_float)(w() - 1), total = curDistribution.dBt.alpha + curDistribution.dBt.beta;
				curDistribution.dBt.alpha = mid * total;
				curDistribution.dBt.beta = (1.0 - mid) * total;
				distributionWindow->sliderBtAlpha->value(curDistribution.dBt.alpha);
				distributionWindow->sliderBtBeta->value(curDistribution.dBt.beta);
				redraw();
				return 1;
			}
			break;
		case DISTR_FREEFORM:
			const t_float colWidth = (t_float)(w() - 1) / (t_float)(curDistribution.dFf.values.size() - 1);
			curDistribution.dFf.changed();
			switch(event) {
				case FL_PUSH: {
					xDrag = Fl::event_x() - x();
					yDrag = Fl::event_y() - y();
					t_int pos = (xDrag + colWidth / 2) * (curDistribution.dFf.values.size() - 1) / w(), ht = h() - (Fl::event_y() - y());
					if(ht < 0) ht = 0;
					curDistribution.dFf.values[pos] = (t_float)ht * DISTRIBUTION_VIEW_HEIGHT / (t_float)h();
					curDistribution.dFf.normalise();
					redraw();
					return 1;
				}
				case FL_DRAG: {
					// fill out in a line to new pt
					t_int xNew = Fl::event_x() - x();
					t_int yNew = Fl::event_y() - y();
					t_int iStart, iEnd;
					t_float vStart, vStep;
					if(xNew > xDrag) {
						// drawn to the right
						iStart = (xDrag + colWidth / 2) * (curDistribution.dFf.values.size() - 1) / w();
						iEnd = (xNew + colWidth / 2) * (curDistribution.dFf.values.size() - 1) / w();
						vStart = (1.0 - (t_float)yDrag / (t_float)h()) * DISTRIBUTION_VIEW_HEIGHT;
						if(iEnd == iStart) vStep = 0;
						else vStep =  ((1.0 - (t_float)yNew / (t_float)h()) * DISTRIBUTION_VIEW_HEIGHT - vStart) / (t_float)(iEnd - iStart);
					}
					else {
						// drawn to the left
						iStart = (xNew + colWidth / 2) * (curDistribution.dFf.values.size() - 1) / w();
						iEnd = (xDrag + colWidth / 2) * (curDistribution.dFf.values.size() - 1) / w();
						vStart = (1.0 - (t_float)yNew / (t_float)h()) * DISTRIBUTION_VIEW_HEIGHT;
						if(iEnd == iStart) vStep = 0;
						else vStep = ((1.0 - (t_float)yDrag / (t_float)h()) * DISTRIBUTION_VIEW_HEIGHT - vStart) / (t_float)(iEnd - iStart);
					}

					// fill out shape
					if(iStart < 0) iStart = 0;
					if(iEnd >= curDistribution.dFf.values.size()) iEnd = curDistribution.dFf.values.size() - 1;
					for(t_int i = iStart; i <= iEnd; ++i) {
						t_float v = vStart + vStep * (t_float)(i - iStart);
						if(v < 0) v = 0;
						if(v > DISTRIBUTION_VIEW_HEIGHT) v = DISTRIBUTION_VIEW_HEIGHT;
						curDistribution.dFf.values[i] = v;
					}
					curDistribution.dFf.normalise();

					// update
					xDrag = xNew;
					yDrag = yNew;
					redraw();
					return 1;
				}
			}
	}
	return 0;
}


//-----------------------------------------------------------------------------------------------------------------------

void DistributionViewEditable::draw(void) {
	// create image
	memset(img, 0xFF, h() * w() * 3);
	curDistribution.draw(img, w(), h(), filter, 1.0, value() != 0);

	// draw image
	fl_draw_image(img, x(), y(), w(), h());
}


//-----------------------------------------------------------------------------------------------------------------------


void DistributionViewEditable::saveDistribution(void) {
	*distributionShown = curDistribution;
}
