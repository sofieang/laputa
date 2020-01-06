#include "App.h"

#include "FL/fl_draw.H"

#include <cmath>

#define ARROW_LENGTH 10.0f
#define ARROW_WIDTH 5.0f
#define ARROW_BODY_WIDTH 3.0f

#define SELECTION_UPDATE_INTERVAL 0.1f

#ifdef _WINDOWS
#define COPY_KEY_PRESSED Fl::event_ctrl()
#else
#define COPY_KEY_PRESSED Fl::event_alt()
#endif

using namespace std;

// pass to DrawLink to draw in selection colours
#define DRAW_AS_SELECTED -1.0

// selection update callback
void IdleCallback(void* v);

// selection timer global
t_float selectionColour = 0;

//-----------------------------------------------------------------------------------------------------------------------

SocietyView::SocietyView(t_int x, t_int y, t_int w, t_int h, const char *label) : Fl_Widget(x, y, w, h, label)  {
	// init fields
	srcInquirer = targetInquirer = INQ_NONE;
	inquirerDragInProgress = linkDragInProgress = selectionDragInProgress = false;
	showInquirerNumbers = false;
	zoom = 1.0;

	// install timer
	Fl::add_timeout(SELECTION_UPDATE_INTERVAL, IdleCallback, (void*)this);
}

//-----------------------------------------------------------------------------------------------------------------------

void SocietyView::setZoomToDefault(int direction) {
	if(direction == 0) setZoom(1);
	else {
		t_float zsizes[13] = {8, 6, 4, 3, 2, 1.5, 1, 0.75, 0.5, 0.25, 0.10, 0.05, 0.05};
		for(t_int i = 1; i < 12; ++i) {
			if(zsizes[i] < zoom) {
				if(direction == 1) setZoom(zsizes[i - 1]);
				else setZoom(zsizes[i]);
				break;
			}
			else if (zsizes[i] == zoom) {
				if(direction == 1) setZoom(zsizes[i - 1]);
				else setZoom(zsizes[i + 1]);
				break;
			}
		}
	}
}

//-----------------------------------------------------------------------------------------------------------------------

void SocietyView::setZoomToFit(void) {
	if(curSociety->people.size()) {
		// find bounds of society
		t_float top = WORKSPACE_SIZE, left = WORKSPACE_SIZE, bottom = 0, right = 0;
		for(t_int i = 0; i < curSociety->people.size(); ++i) {
			if(curSociety->people[i].x < left) left = curSociety->people[i].x;
			if(curSociety->people[i].x > right) right = curSociety->people[i].x;
			if(curSociety->people[i].y < top) top = curSociety->people[i].y;
			if(curSociety->people[i].y > bottom) bottom = curSociety->people[i].y;
		}
		left -= INQUIRER_CIRCLE_SIZE * 3;
		top -= INQUIRER_CIRCLE_SIZE * 3;
		right += INQUIRER_CIRCLE_SIZE * 3;
		bottom += INQUIRER_CIRCLE_SIZE * 3;

		// set midpoint
		setMidpoint((left + right) / 2, (top + bottom) / 2);

		// scale
		t_float vScale = h() / (bottom - top), hScale = w() / (right - left);
		if(hScale < vScale) setZoom(hScale);
		else setZoom(vScale);
	}
}

//-----------------------------------------------------------------------------------------------------------------------

void SocietyView::setZoom(t_float z) {
	if(z > 8.0) z = 8.0;
	if(z < 0.05) z = 0.05;
	zoom = z;
	societyWindow->scrollbarHorizontal->scrollvalue(xViewToWorkspace(w() / 2 * z), w() / z, 0, WORKSPACE_SIZE);
	societyWindow->scrollbarVertical->scrollvalue(yViewToWorkspace(h() / 2 * z), h() / z, 0, WORKSPACE_SIZE);
	societyWindow->inputZoom->value(z * 100);

	// adjust buttons
	if(zoom == 8.0) societyWindow->buttonZoomIncrease->deactivate();
	else societyWindow->buttonZoomIncrease->activate();
	if(zoom == 0.05) societyWindow->buttonZoomDecrease->deactivate();
	else societyWindow->buttonZoomDecrease->activate();
	if(zoom == 1.0) societyWindow->buttonZoomDefault->deactivate();
	else societyWindow->buttonZoomDefault->activate();
}

//-----------------------------------------------------------------------------------------------------------------------

void SocietyView::setMidpoint(t_float x, t_float y) {
	xMid = x;
	yMid = y;
}

//-----------------------------------------------------------------------------------------------------------------------

void SocietyView::setInquirerColour(t_float belief) {
	if(belief < 0.5) {
		// red to yellow
		fl_color(255.0, 255.0 * (belief * 2.0), 0);
	}
	else {
		// yellow to green
		fl_color(255.0 * (2.0 - belief * 2.0), 255.0, 0);
	}
}


//-----------------------------------------------------------------------------------------------------------------------

t_float SocietyView::xViewToWorkspace(t_float xScr) {
	return (xScr - w() / 2) / zoom + xMid;
}

//-----------------------------------------------------------------------------------------------------------------------

t_float SocietyView::yViewToWorkspace(t_float yScr) {
	return (yScr - h() / 2) / zoom + yMid;
}

//-----------------------------------------------------------------------------------------------------------------------

t_float SocietyView::xWorkspaceToView(t_float xv) {
	return (xv - xMid) * zoom + w() / 2;
}

//-----------------------------------------------------------------------------------------------------------------------

t_float SocietyView::yWorkspaceToView(t_float yv) {
	return (yv - yMid) * zoom + h() / 2;
}

//-----------------------------------------------------------------------------------------------------------------------

void SocietyView::drawLink(t_float x0, t_float y0, t_float x1, t_float y1, t_float d, t_float strength, t_float trust, bool drawAsSelected) {
	t_float xSlope = (x1 - x0) / d, ySlope = (y1 - y0) / d;
	static char dashMt[6][3] = {1, 5, 0,
		2, 4, 0,
		3, 3, 0,
		4, 2, 0,
		5, 1, 0,
		0, 0, 0};

	// draw as selection?
	if(drawAsSelected) fl_color(selectionColour, selectionColour, selectionColour);
	else {
		if(trust < 0.5) {
			t_float c = 255.0 * trust * 2.0;
			fl_color(255, c, 0);
		}
		else {
			t_float c = 255.0 * (2.0 - trust * 2.0);
			fl_color(c, 255, 0);
		}
	}

	// draw line
	t_int str = strength * 6.0;
	if(str > 5) str = 5;
	fl_line_style(FL_SOLID, ARROW_BODY_WIDTH * zoom, dashMt[str]);
	fl_line(x0, y0, x1 - xSlope * 4 * zoom, y1 - ySlope * 4 * zoom);
	fl_line_style(FL_SOLID, ARROW_BODY_WIDTH * zoom, 0);

	// draw arrowhead
	fl_polygon(x1, y1,
			   x1 - xSlope * ARROW_LENGTH * zoom + ySlope * ARROW_WIDTH * zoom, y1 - ySlope * ARROW_LENGTH * zoom - xSlope * ARROW_WIDTH * zoom,
			   x1 - xSlope * ARROW_LENGTH * zoom - ySlope * ARROW_WIDTH * zoom, y1 - ySlope * ARROW_LENGTH * zoom + xSlope * ARROW_WIDTH * zoom);
}

//-----------------------------------------------------------------------------------------------------------------------

void SocietyView::draw(void) {
	char str[32];

	// clear
	fl_draw_box(FL_FLAT_BOX, x() + 1, y() + 1, w() - 1, h() - 1, FL_WHITE);

	// set clipping
	fl_push_clip(x() + 1, y() + 1, w() - 1, h() - 1);
	t_float xLeft = x() ;
	t_float yTop = y();

	// Draw links
	for(LinkIterator l = curSociety->links.begin(); l != curSociety->links.end(); ++l) {
		int src = l->second.source, tgt = l->second.target;
		t_float xOffset = 0, yOffset = 0;
		t_float x0 = xWorkspaceToView(curSociety->people[src].x), y0 = yWorkspaceToView(curSociety->people[src].y);
		t_float x1 = xWorkspaceToView(curSociety->people[tgt].x), y1 = yWorkspaceToView(curSociety->people[tgt].y);
		t_float dx = x1 - x0, dy = y1 - y0;
		t_float d = sqrt(dx * dx + dy * dy);

		// offset a bit if there are two arrows
		if(curSociety->getLink(tgt, src)) {
			xOffset = -dy / d * 3.0 * zoom;
			yOffset = dx / d * 3.0 * zoom;
		}

		// draw back arrowhead a bit
		d -= INQUIRER_CIRCLE_SIZE;

		// draw the actual links
		if(d > INQUIRER_CIRCLE_SIZE) {
			drawLink(xLeft + x0 + xOffset, yTop + y0 + yOffset, xLeft + x1 + xOffset - dx / d * INQUIRER_CIRCLE_SIZE * zoom,
				yTop + y1 + yOffset - dy / d * INQUIRER_CIRCLE_SIZE * zoom, d, l->second.listenChance,
				l->second.trust.expectation(), isLinkSelected(src, tgt));
		}
	}

	// Draw currently pulled link
	if(linkDragInProgress) {
		t_float dx = xDragEnd - xDragStart;
		t_float dy = yDragEnd - yDragStart;
		t_float d = sqrt(dx * dx + dy * dy);
		t_float x1 = xDragEnd, y1 = yDragEnd;

		if(targetInquirer != INQ_NONE && targetInquirer != srcInquirer) {
			// we have a target; pull back arrow a bit
			x1 -= dx / (d + 4.0) * INQUIRER_CIRCLE_SIZE;
			y1 -= dy / (d + 4.0) * INQUIRER_CIRCLE_SIZE;
			d -= INQUIRER_CIRCLE_SIZE + 4.0;
		}

		// draw
		if(d > INQUIRER_CIRCLE_SIZE) drawLink(xWorkspaceToView(xDragStart) + xLeft, yWorkspaceToView(yDragStart) + yTop,
													 xWorkspaceToView(x1) + xLeft,  yWorkspaceToView(y1) + yTop, d, 1.0, 1.0, true);
	}

	// Draw all inquirers
	for(t_int i = 0; i < curSociety->people.size(); ++i) {
		setInquirerColour(curSociety->people[i].belief.v());
		fl_line_style(FL_SOLID, 2 * zoom, 0);
		fl_pie(xWorkspaceToView(curSociety->people[i].x) + xLeft - (INQUIRER_CIRCLE_SIZE) * zoom, yWorkspaceToView(curSociety->people[i].y) + yTop - (INQUIRER_CIRCLE_SIZE) * zoom,
			(INQUIRER_CIRCLE_SIZE * 2) * zoom, (INQUIRER_CIRCLE_SIZE * 2) * zoom, 0, 360);
		fl_color(0, 0, 0);
		fl_line_style(FL_SOLID, 2 * zoom, 0);
		fl_circle(xWorkspaceToView(curSociety->people[i].x) + xLeft, yWorkspaceToView(curSociety->people[i].y) + yTop,
			INQUIRER_CIRCLE_SIZE * zoom);

		// does inquirer have a name?
		if(curSociety->people[i].name[0]) {
			fl_font(FL_HELVETICA | FL_BOLD, 10 * zoom);
			fl_draw(curSociety->people[i].name, xWorkspaceToView(curSociety->people[i].x) + xLeft - fl_width(curSociety->people[i].name) / 2,
				yWorkspaceToView(curSociety->people[i].y) + yTop + (INQUIRER_CIRCLE_SIZE + 13) * zoom);
		}

		// should we draw numbers?
		if(showInquirerNumbers) {
			fl_font(FL_HELVETICA, 9);
			sprintf(str, "%d", i + 1);
			fl_draw(str, xWorkspaceToView(curSociety->people[i].x) + xLeft - fl_width(str) / 2 + 1,
				yWorkspaceToView(curSociety->people[i].y) + yTop + 5);
		}

		// draw selection highlight
		if(isInquirerSelected(i) || i == srcInquirer || i == targetInquirer) {
			fl_color(selectionColour, selectionColour, selectionColour);
			fl_line_style(FL_SOLID, 2 * zoom, 0);
			fl_circle(xWorkspaceToView(curSociety->people[i].x) + xLeft, yWorkspaceToView(curSociety->people[i].y) + yTop,
				(INQUIRER_CIRCLE_SIZE + 3) * zoom);
		}
	}

	// draw selection rectangle
	if(selectionDragInProgress) {
		static char dashMt[3] = {3, 3, 0};
		fl_color(selectionColour, selectionColour, selectionColour);
		fl_line_style(FL_DASH, 0, dashMt);
		t_float x0, y0;
		if(xDragStart > xDragEnd) x0 = xDragEnd;
		else x0 = xDragStart;
		if(yDragStart > yDragEnd) y0 = yDragEnd;
		else y0 = yDragStart;

		fl_frame("AAAA", xWorkspaceToView(x0) + xLeft, yWorkspaceToView(y0) + yTop, fabs(xWorkspaceToView(xDragEnd)- xWorkspaceToView(xDragStart)), fabs(yWorkspaceToView(yDragEnd) - yWorkspaceToView(yDragStart)));
	}

	// restore line style
	fl_line_style(FL_SOLID, 0, 0);
	fl_pop_clip();

	// draw edge
	fl_frame("AAAA", x(), y(), w(), h());

}

//-----------------------------------------------------------------------------------------------------------------------

t_int SocietyView::getInquirerUnderPoint(t_float x, t_float y) {
	for(t_int i = curSociety->people.size() - 1; i >= 0; --i) {
		t_float dx = x - curSociety->people[i].x, dy = y - curSociety->people[i].y;
		t_float d = dx * dx + dy * dy;
		if(d <= INQUIRER_CIRCLE_SIZE * INQUIRER_CIRCLE_SIZE) return i;
	}

	return INQ_NONE;
}


//-----------------------------------------------------------------------------------------------------------------------

t_int SocietyView::pointInLine(t_float xPt, t_float yPt, t_float x0Line, t_float y0Line, t_float x1Line, t_float y1Line, t_float thickness) {
	// returns 0 for not in line, 1 for left of line, -1 for right of line
	t_float dx = x1Line - x0Line, dy = y1Line - y0Line;
	t_float d = sqrt(dx * dx + dy * dy);

	// get pt in line's coordinate system
	xPt -= x0Line;
	yPt -= y0Line;

	// check behind / in front of
	dx /= d;
	dy /= d;
	t_float dp = xPt * dx + yPt * dy;
	if(dp < 0 || dp > d) return 0;

	// check if sufficiently close
	t_float xNormal = dy, yNormal = -dx;
	dp = xPt * xNormal + yPt * yNormal;
	if(fabs(dp) <= thickness) {
		if(dp < 0) return 1;
		else return -1;
	}

	// not within line
	return 0;
}

//-----------------------------------------------------------------------------------------------------------------------

bool SocietyView::getLinkUnderPoint(t_float x, t_float y, t_int &src, t_int &target) {
	for(t_int i = curSociety->people.size() - 1; i >= 0 ; --i) {
		for(t_int j = i - 1; j >= 0; --j) {
			bool iToj = curSociety->getLink(i, j) != 0, jToi = curSociety->getLink(j, i) != 0;
			if(iToj && jToi) {
				// two lines, decide which is under point by checking side of middle
				t_int l = pointInLine(x, y, curSociety->people[i].x, curSociety->people[i].y, curSociety->people[j].x, curSociety->people[j].y, 7.0);
				if(l == 1) {
					src = i;
					target = j;
					return true;
				}
				else if(l == -1) {
					src = j;
					target = i;
					return true;
				}
			}
			else if(iToj || jToi) {
				if(pointInLine(x, y, curSociety->people[i].x, curSociety->people[i].y, curSociety->people[j].x, curSociety->people[j].y, 3.0)) {
					//inside single line between inquirers; which direction does it go in?
					if(iToj) {
						src = i;
						target = j;
					}
					else {
						src = j;
						target = i;
					}
					return true;
				}
			}
		}
	}

	// no link found under point
	src = INQ_NONE;
	target= INQ_NONE;
	return false;
}

//-----------------------------------------------------------------------------------------------------------------------

t_int SocietyView::handle(t_int event) {
	t_float xLeft = x();
	t_float yTop = y();
	t_int sel;

	switch(event) {
	case FL_PUSH:
		// pushed button in view, check tool
		switch(app->getTool()) {
		case TOOL_SELECT:
			if(Fl::event_shift()) {
				selectedLinksPrev = selectedLinks;
				selectedInquirersPrev = selectedInquirers;
			}
			else {
				selectedLinksPrev.clear();
				selectedInquirersPrev.clear();
			}

			// is this point in an inquirer?
			sel = getInquirerUnderPoint(xViewToWorkspace(Fl::event_x() - xLeft), yViewToWorkspace(Fl::event_y() - yTop));
			if(sel != INQ_NONE && (app->getSelectionSettings() & SELECT_INQUIRERS)) {
				// note starting point of drag
				xDragStart = xViewToWorkspace(Fl::event_x() - xLeft);
				yDragStart = yViewToWorkspace(Fl::event_y() - yTop);
				if(Fl::event_shift()) {
					if(isInquirerSelected(sel)) {
						deselectInquirer(sel);
					}
					else {
						selectInquirer(sel);
						inquirerDragInProgress = true;
						linkDragInProgress = selectionDragInProgress = copyDragInProgress = false;
					}
				}
				else {
					if(!isInquirerSelected(sel)) {
						selectedInquirers.clear();
						selectedLinks.clear();
					}
					inquirerDragInProgress = true;
					linkDragInProgress = selectionDragInProgress = copyDragInProgress = false;
					selectInquirer(sel);
				}
			}
			else {
				// is this point in a link?
				t_int src, target;
				if(getLinkUnderPoint(xViewToWorkspace(Fl::event_x() - xLeft), yViewToWorkspace(Fl::event_y() - yTop), src, target) && (app->getSelectionSettings() & (SELECT_INTERNAL_LINKS | SELECT_EXTERNAL_LINKS))) {
					if(Fl::event_shift()) toggleLinkSelection(src, target);
					else {
						selectedInquirers.clear();
						selectedLinks.clear();
						selectLink(src, target);
					}
				}
				else {
					// drag selection rectangle
					xDragStart = xDragEnd = xViewToWorkspace(Fl::event_x() - xLeft);
					yDragStart = yDragEnd = yViewToWorkspace(Fl::event_y() - yTop);
					selectionDragInProgress = true;
					inquirerDragInProgress = linkDragInProgress = copyDragInProgress = false;
					deselectAll();
				}
			}

			// update
			inquirerWindow->configure();
			linkWindow->configure();
			redraw();
			break;

		case TOOL_ADD_INQUIRER:
			curSociety->addInquirer(xViewToWorkspace(Fl::event_x() - xLeft), yViewToWorkspace(Fl::event_y() - yTop), &app->curSocietySetup);
			app->touchFile();
			updateStatistics();
			redraw();
			break;

		case TOOL_ADD_LINK:
			srcInquirer = targetInquirer = getInquirerUnderPoint(xViewToWorkspace(Fl::event_x() - xLeft), yViewToWorkspace(Fl::event_y() - yTop));
			if(srcInquirer != INQ_NONE) {
				// note starting point of drag
				xDragStart = curSociety->people[srcInquirer].x;
				xDragEnd = xViewToWorkspace(Fl::event_x() - xLeft);
				yDragStart = curSociety->people[srcInquirer].y;
				yDragEnd = yViewToWorkspace(Fl::event_y() - yTop);

				// mark dragging as in progress
				linkDragInProgress = true;
				inquirerDragInProgress = selectionDragInProgress = copyDragInProgress = false;
			}
			redraw();
			break;

		default:
			return 1;
		}
		break;

	case FL_DRAG:
		// update dragging position
		xDragEnd = xViewToWorkspace(Fl::event_x() - xLeft);
		yDragEnd = yViewToWorkspace(Fl::event_y() - yTop);

		// check tool
		switch(app->getTool()) {
		case TOOL_SELECT:
			// are we dragging inquirers?
			if(inquirerDragInProgress) {
				// should we make a copy?
				if(COPY_KEY_PRESSED && !copyDragInProgress) {
					dragCopySelection();
					copyDragInProgress = true;
				}

				// move inquirers
				set<t_int>::iterator iter(selectedInquirers.begin());
				while(iter != selectedInquirers.end()) {
					curSociety->people[*iter].x += xDragEnd - xDragStart;
					curSociety->people[*iter].y += yDragEnd - yDragStart;
					++iter;
				}

				// update variables
				xDragStart = xDragEnd;
				yDragStart = yDragEnd;
				app->touchFile();
			}
			else if(selectionDragInProgress) {
				// update which things are selected
				updateSelectionFromRect(xDragStart, yDragStart, xDragEnd, yDragEnd);
				inquirerWindow->configure();
				linkWindow->configure();
			}
			redraw();
			break;
		case TOOL_ADD_LINK:
			if(linkDragInProgress) {
				targetInquirer = getInquirerUnderPoint(xDragEnd, yDragEnd);
				if(targetInquirer != INQ_NONE) {
					// snap to inquirer
					xDragEnd = curSociety->people[targetInquirer].x;
					yDragEnd = curSociety->people[targetInquirer].y;
				}
				redraw();
			}
			break;
		default:
			return 1;
		}
		break;

	case FL_RELEASE:
		// what to do depends on the tool
		switch(app->getTool()) {
		case TOOL_SELECT:
			inquirerDragInProgress = selectionDragInProgress = copyDragInProgress = false;
			break;
		case TOOL_ADD_LINK:
			linkDragInProgress = false;
			if(srcInquirer != INQ_NONE && targetInquirer != INQ_NONE && srcInquirer != targetInquirer) {
				if(curSociety->getLink(srcInquirer, targetInquirer)) curSociety->removeLink(srcInquirer, targetInquirer);
				curSociety->addLink(srcInquirer, targetInquirer, &app->curSocietySetup);
				updateStatistics();
			}
			srcInquirer = targetInquirer = INQ_NONE;
            app->touchFile();
			redraw();
			break;
		default:
			return 1;
		}
		break;

	case FL_SHORTCUT:
		if(Fl::event_key() == FL_BackSpace || Fl::event_key() == FL_Delete) {
			// delete inquirer or link
			deleteSelectedInquirersAndLinks();
			app->touchFile();
			return 1;
		}
		else if(Fl::event_key() == '1') app->setTool(TOOL_ADD_INQUIRER);
		else if(Fl::event_key() == '2') app->setTool(TOOL_ADD_LINK);
		else if(Fl::event_key() == '3') app->setTool(TOOL_SELECT);
		else return Fl_Widget::handle(event);

	case FL_FOCUS:
		return 1;

	default:
		return Fl_Widget::handle(event);
	}
	return 1;
}

//-----------------------------------------------------------------------------------------------------------------------

void SocietyView::doCutSelection(void) {
	if(selectedInquirers.size() != 0 || selectedLinks.size() != 0) {
		app->setClipboard(curSociety->extract(selectedInquirers, selectedLinks));
		deleteSelectedInquirersAndLinks();
	}
}

//-----------------------------------------------------------------------------------------------------------------------

void SocietyView::doCopySelection(void) {
	if(selectedInquirers.size() != 0 || selectedLinks.size() != 0) app->setClipboard(curSociety->extract(selectedInquirers, selectedLinks));
}

//-----------------------------------------------------------------------------------------------------------------------

void SocietyView::doPaste(void) {
	SocietyFragment* f = app->getClipboard();
	if(f) {
		deselectAll();

		// move inquirers in fragment
		for(t_int i = 0; i < f->inquirers.size(); ++i) {
			f->inquirers[i].x += 16;
			f->inquirers[i].y += 15;
		}

		// merge fragment with society
		t_int sz = curSociety->people.size();
		pair<t_int, t_int> * newLinks = new pair<t_int, t_int> [f->links.size()];
		curSociety->merge(f, newLinks);

		// select new inquirers
		if(app->getSelectionSettings() & SELECT_INQUIRERS) 	for(t_int i = sz; i < curSociety->people.size(); ++i) selectInquirer(i);


		// select new links
		if(app->getSelectionSettings() & (SELECT_INTERNAL_LINKS | SELECT_EXTERNAL_LINKS)) {
			for(t_int i = 0; i < f->links.size(); ++i) {
				if(newLinks[i].first < curSociety->people.size() && newLinks[i].second < curSociety->people.size()) selectLink(newLinks[i].first, newLinks[i].second);
			}
		}
		delete [] newLinks;

		// update
		app->touchFile();
		curSociety->recalculateListeners();
		updateStatistics();
		inquirerWindow->configure();
		linkWindow->configure();
		redraw();
	}
}

//-----------------------------------------------------------------------------------------------------------------------

void SocietyView::dragCopySelection(void) {
	SocietyFragment* f = curSociety->extract(selectedInquirers, selectedLinks);
	deselectAll();
	t_int sz = curSociety->people.size();
	pair<t_int, t_int> * newLinks = new pair<t_int, t_int> [f->links.size()];
	curSociety->merge(f, newLinks);

	// select new inquirers
	if(app->getSelectionSettings() & SELECT_INQUIRERS) 	for(t_int i = sz; i < curSociety->people.size(); ++i) selectInquirer(i);


	// select new links
	if(app->getSelectionSettings() & (SELECT_INTERNAL_LINKS | SELECT_EXTERNAL_LINKS)) {
		for(t_int i = 0; i < f->links.size(); ++i) {
			if(newLinks[i].first < curSociety->people.size() && newLinks[i].second < curSociety->people.size()) selectLink(newLinks[i].first, newLinks[i].second);
		}
	}
	delete [] newLinks;

	// update
	curSociety->recalculateListeners();
	updateStatistics();
	inquirerWindow->configure();
	linkWindow->configure();
}

//-----------------------------------------------------------------------------------------------------------------------

void SocietyView::updateSelectionFromRect(t_float x0, t_float y0, t_float x1, t_float y1) {
	// reorder rect correctly
	if(x0 > x1) {
		t_float tmp = x1;
		x1 = x0;
		x0 = tmp;
	}
	if(y0 > y1) {
		t_float tmp = y1;
		y1 = y0;
		y0 = tmp;
	}

	// start with previous selection
	selectedInquirers = selectedInquirersPrev;
	selectedLinks = selectedLinksPrev;

	// check which inquirers/links are within rect & toggle
	for(t_int i = 0; i < curSociety->people.size(); ++i) {
		if(curSociety->people[i].x >= x0 && curSociety->people[i].y >= y0 && curSociety->people[i].x <= x1 && curSociety->people[i].y <= y1) {
			if(app->getSelectionSettings() & SELECT_INQUIRERS) toggleInquirerSelection(i);
			if(app->getSelectionSettings() & SELECT_EXTERNAL_LINKS) {
				// check links to & from this inquirer
				for(t_int j = 0; j < curSociety->people.size(); ++j) {
					Link* link = curSociety->getLink(i, j);
					if(link) {
						if(selectedLinksPrev.count(pair<t_int, t_int>(link->source, link->target))) deselectLink(link->source, link->target);
						else selectLink(link->source, link->target);
					}
					link = curSociety->getLink(j, i);
					if(link) {
						if(selectedLinksPrev.count(pair<t_int, t_int>(link->source, link->target))) deselectLink(link->source, link->target);
						else selectLink(link->source, link->target);
					}
				}
			}
			else if(app->getSelectionSettings() & SELECT_INTERNAL_LINKS) {
				// check if links to this inquirer are wholly inside as well
				LinkIterator link(curSociety->links.lower_bound(LBOUND(i)));
				while(link != (LinkIterator)curSociety->links.upper_bound(UBOUND(i))) {
					if(curSociety->people[link->second.source].x >= x0 && curSociety->people[link->second.source].y >= y0 && curSociety->people[link->second.source].x <= x1 && curSociety->people[link->second.source].y <= y1)
						toggleLinkSelection(link->second.source, i);
					++link;
				}
			}
		}
	}
}

//-----------------------------------------------------------------------------------------------------------------------

void SocietyView::selectAll(void) {
	if(app->getSelectionSettings() & SELECT_INQUIRERS) for(t_int i = 0; i < curSociety->people.size(); ++i) selectInquirer(i);
	if(app->getSelectionSettings() & (SELECT_INTERNAL_LINKS | SELECT_EXTERNAL_LINKS)) {
		for(t_int i = 0; i < curSociety->people.size(); ++i) {
			for(t_int j = i + 1; j < curSociety->people.size(); ++j) {
				if(curSociety->getLink(i, j)) selectLink(i, j);
				if(curSociety->getLink(j, i)) selectLink(j, i);
			}
		}
	}
	inquirerWindow->configure();
	linkWindow->configure();
}

//-----------------------------------------------------------------------------------------------------------------------

void SocietyView::deselectAll(void) {
	selectedInquirers.clear(); selectedLinks.clear();
	inquirerWindow->configure();
	linkWindow->configure();
}

//-----------------------------------------------------------------------------------------------------------------------

void SocietyView::deleteSelectedInquirersAndLinks(void) {
	// remove trust functions from views to avoid dangling pointers
	inquirerWindow->viewTrust->removeAllTrustFunctions();
	linkWindow->viewTrust->removeAllTrustFunctions();

	// create new linkless society
	Society* newSociety = new Society;

	t_int *indexRemap = new t_int[curSociety->people.size()];
	for(t_int i = 0, nNewInqs = 0; i < curSociety->people.size(); ++i) {
		indexRemap[i] = nNewInqs;
		if(!isInquirerSelected(i)) {
			newSociety->people.push_back(curSociety->people[i]);
			++nNewInqs;
		}
	}

	// add links to new society
	for(t_int i = 0; i < curSociety->people.size(); ++i) {
		if(!isInquirerSelected(i)) {
			for(t_int j = 0; j < curSociety->people.size(); ++j) {
				if(!isInquirerSelected(j)) {
					Link *l = curSociety->getLink(i, j);
					if(l != 0 && !isLinkSelected(i, j)) {
						// move this link over to new society
						newSociety->links[COUPLE(indexRemap[i], indexRemap[j])] = Link(indexRemap[i], indexRemap[j], *l);
					}
				}
			}
		}
	}
	delete curSociety;
	curSociety = newSociety;
	delete [] indexRemap;

	// update
	app->touchFile();
	deselectAll();
	curSociety->recalculateListeners();
	updateStatistics();
}

//-----------------------------------------------------------------------------------------------------------------------

void SocietyView::updateStatistics(void) {
	societyWindow->outputNumInquirers->value(curSociety->people.size());
	societyWindow->outputNumLinks->value(curSociety->links.size());

	if(curSociety->people.size()) societyWindow->buttonDegrees->activate();
	else societyWindow->buttonDegrees->deactivate();
}

//-----------------------------------------------------------------------------------------------------------------------

void IdleCallback(void* v) {
	static t_float t = 0;
	t += SELECTION_UPDATE_INTERVAL * 10.0f;

	// update selection color
	selectionColour = 128.0f + 127.0f * sin(t);

	// redraw Society view
	((SocietyView*)v)->redraw();

	// reinstall callback
	Fl::repeat_timeout(SELECTION_UPDATE_INTERVAL, IdleCallback, v);
}
