#ifndef __DISTRIBUTIONVIEW_H__
#define __DISTRIBUTIONVIEW_H__

#include "Prefix.h"

#include "Distribution.h"
#include <FL/Fl_Widget.H>

#define DISTRIBUTION_VIEW_HEIGHT 4.0

// Distribution view - shows a distribution. Clicking on this brings up the
// distribution editing window.


class DistributionView : public Fl_Button {
public:
	DistributionView(t_int x, t_int y, t_int w, t_int h, const char *label = 0);
	~DistributionView();

	// distribution setting
	void setDistribution(Distribution* d);
	Distribution* getDistribution(void) {return distributionShown;}

	// set & get filter
	void setFilter(t_int f) {filter = f; redraw();}
	t_int getFilter(void) {return filter;}

	// draw method
	void draw(void);

	// which distribution is shown?
	Distribution* distributionShown;

	// which aspects of the distribution are we displaying?
	t_int filter;

	// image
	unsigned char* img;

};

// Metadistribution view miniature - shows a metadistribution. Clicking on this brings up the
// metadistribution editing window.

class MetaDistributionView : public Fl_Button {
public:
	MetaDistributionView(t_int x, t_int y, t_int w, t_int h, const char *label = 0);
	~MetaDistributionView();

	// accessing metadistribution
	void setMetaDistribution(const MetaDistribution& m);
	void setMetaDistribution(MetaDistribution* m);
	MetaDistribution* getMetaDistribution(void) {return mdShown;}

	// overload drawing
	void draw(void);

	// metadistribution being shown
	MetaDistribution md;
	MetaDistribution* mdShown;

	// image
	unsigned char* img;

};




// Distribution view editable - shows a distribution and handles editing of said distribution.

class DistributionViewEditable : public DistributionView {

public:
	// constructor & destructor
	DistributionViewEditable(t_int x, t_int y, t_int w, t_int h, const char *label = 0) : DistributionView(x, y, w, h, label) {}

	// drawing
	void draw(void);

	// event handling
	t_int handle(t_int event);

	// set & get distribution to be edited
	Distribution* getDistribution() {return &curDistribution;}
	void setDistribution(const Distribution& d);
	void setDistribution(Distribution* d);
	void saveDistribution(void);

	// copy of distribution being edited
	Distribution curDistribution;

	// where was mouse last during drag?
	t_int xDrag = 0, yDrag = 0;
};















#endif
