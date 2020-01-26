#ifndef __SOCIETYVIEW_H__
#define __SOCIETYVIEW_H__

#include "Prefix.h"

#include "Inquirer.h"
#include <FL/Fl_Widget.H>

#define INQ_NONE -1
#define LINK_NONE -1

#define WORKSPACE_SIZE 16384

// SocietyView - shows the social network, and also handles user t_interaction.

class SocietyView : public Fl_Widget {
public:
	// constructor & destructor
	SocietyView(t_int x, t_int y, t_int w, t_int h, const char *label = 0);

	// draw method
	void draw(void);

	// event handling
	t_int handle(t_int event);

	// delete selected inquirers or links
	void deleteSelectedInquirersAndLinks(void);

	// get & set selected links or inquirers
	void deselectInquirer(t_int inq) {selectedInquirers.erase(inq);}
	void selectInquirer(t_int inq)  {selectedInquirers.insert(inq);}
	void toggleInquirerSelection(t_int inq) {if(isInquirerSelected(inq)) deselectInquirer(inq); else selectInquirer(inq);}
	void deselectLink(t_int src, t_int target) {selectedLinks.erase(pair<t_int, t_int>(src, target));}
	void selectLink(t_int src, t_int target) {selectedLinks.insert(pair<t_int, t_int>(src, target));}
	void toggleLinkSelection(t_int src, t_int target) {if(isLinkSelected(src, target)) deselectLink(src, target); else selectLink(src, target);}
	bool isInquirerSelected(t_int inq) {return selectedInquirers.count(inq);}
	bool isLinkSelected(t_int src, t_int target) {return selectedLinks.count(pair<t_int, t_int>(src, target));}
	void selectAll(void);
	void deselectAll(void);
	void updateSelectionFromRect(t_float x0, t_float y0, t_float x1, t_float y1);
	set<t_int>& getSelectedInquirers(void) {return selectedInquirers;}
	set<pair<t_int, t_int> >& getSelectedLinks(void) {return selectedLinks;}

	// update the statistics bar
	void updateStatistics(void);

	// cut, copy, paste
	void doCutSelection(void);
	void doCopySelection(void);
	void dragCopySelection(void);
	void doPaste(void);

	// set display properties
	void setShowInqNumbers(bool on) {showInquirerNumbers = on;}
	void setShowInqNames(bool on) { showInquirerNames = on; }
	void setShowLinks(bool on) { showLinks = on; }
	void setZoom(t_float z);
	void setZoomToDefault(int direction);
	void setZoomToFit(void);
	void setMidpoint(t_float x, t_float y);

	void setInquirerColour(t_float belief);
	void drawLink(t_float x0, t_float y0, t_float x1, t_float y1, t_float d, t_float strength, t_float trust, bool drawAsSelected);
	t_int getInquirerUnderPoint(t_float x, t_float y);
	bool getLinkUnderPoint(t_float x, t_float y, t_int& src, t_int& target);
	t_int pointInLine(t_float xPt, t_float yPt, t_float x0Line, t_float y0Line, t_float x1Line, t_float y1Line, t_float thickness);


	// coordinate space conversion
	t_float xViewToWorkspace(t_float xScr);
	t_float yViewToWorkspace(t_float xScr);
	t_float xWorkspaceToView(t_float xv);
	t_float yWorkspaceToView(t_float yv);


	// selection & dragging variables
	set<t_int> selectedInquirers;
	set<t_int> selectedInquirersPrev;
	set<pair<t_int, t_int> > selectedLinks;
	set<pair<t_int, t_int> > selectedLinksPrev;
	bool inquirerDragInProgress, linkDragInProgress, selectionDragInProgress, copyDragInProgress;
	t_int srcInquirer, targetInquirer;
	t_float xDragStart, yDragStart, xDragEnd, yDragEnd;

	// display variables
	t_float zoom, xMid, yMid;

	// display settings
	bool showInquirerNumbers;
	bool showInquirerNames;
	bool showLinks;

	// timer callback for updating selections
	static void idleCallback(void* v);
};

#endif
