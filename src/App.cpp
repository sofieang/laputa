#include "App.h"
#include <time.h>
#include <stdlib.h>

#ifdef __APPLE__
#include <Fl/x.h>
#endif

#include <FL/Fl_Tooltip.H>
#include <FL/Fl_PNG_Image.H>

#include "UserInterfaceItems.h"
#include "Utility.h"

// global variables
App *app;
gsl_rng *rng;


//-----------------------------------------------------------------------------------------------------------------------

App::App(t_int argc, char** argv, const string& inDataPath, const string& inDocsPath) : dataPath(inDataPath), docsPath(inDocsPath)
{
  // assign global pointer
  app = this;

  // x stuff
  Fl::visual(FL_DOUBLE|FL_INDEX);

	// create general random number generator
	rng = gsl_rng_alloc(gsl_rng_taus);
	gsl_rng_set(rng, clock());

  // set window icons
  Fl_Window::default_icon(new Fl_PNG_Image(app->dataFile("laputa.png").c_str()));
  Fl_Window::default_xclass("laputa");

	// create an empty society
	workingSetup.setDefault();
	curSocietySetup.setDefault();
	curSocietySetup.inqParams.varyStartBelief = curSocietySetup.inqParams.varyInquiryChance = curSocietySetup.inqParams.varyInquiryAccuracy =
		curSocietySetup.inqParams.varyInquiryTrust = curSocietySetup.inqParams.varyInquiryTrust = VARY_GLOBALLY;
	curSocietySetup.linkParams.varyListenChance = curSocietySetup.linkParams.varyTrust = curSocietySetup.linkParams.varyThreshold = VARY_GLOBALLY;
	curBatchSimulation.setDefault();
	curMultiBatch.setDefault();

	// register image types
	fl_register_images();

	// create fltk windows
	ui = new UserInterface;
	Fl::scheme("none");
	ui->make_windows();

	// turn on tooltips
	Fl_Tooltip::enable();
	Fl_Tooltip::delay(0.5);

	// set up user interface
 	societyWindow->buttonCreateInquirer->setIcons(app->dataFile("icon1up.png"), app->dataFile("icon1down.png"));
	societyWindow->buttonCreateLink->setIcons(app->dataFile("icon2up.png"), app->dataFile("icon2down.png"));
	societyWindow->buttonSelect->setIcons(app->dataFile("icon3up.png"), app->dataFile("icon3down.png"));
	societyWindow->buttonCreateInquirer->setPressed(true);
	societyWindow->adjustMenuShortcuts();
	societyWindow->view->setMidpoint(WORKSPACE_SIZE / 2, WORKSPACE_SIZE / 2);
	societyWindow->view->setZoom(1.0);
	societyWindow->view->updateStatistics();
#ifdef __APPLE__
	adjustMacOSMenus();
#endif
	clipboard = NULL;
	tool = TOOL_ADD_INQUIRER;
	inquirerWindow->configure();
	linkWindow->configure();

	// create simulation window
	simulationWindow->outputLog->buffer(new Fl_Text_Buffer(32668));
	simulationWindow->outputLog->textsize(10);
	simulationWindow->outputLog->textfont(FL_HELVETICA);
	curSimulation.soc = curSociety;
	curSimulation.reset();
	curSimulation.setSimulationWindowFrom();

	// create about dialog
	Fl_PNG_Image* img = new Fl_PNG_Image(app->dataFile("SplashScreen.png").c_str());
	aboutWindow->buttonAbout->image(img);

	// position windows
	t_int xScr, yScr, hScr, wScr;
	Fl::screen_xywh(xScr, yScr, wScr, hScr);
	societyWindow->resize(20, 40, wScr - linkWindow->w() - 50, hScr - 120);
  inquirerWindow->position(30 + societyWindow->w() + 10, 40);
  inquirerWindow->set_non_modal();
  linkWindow->position(30 + societyWindow->w() + 10, 40 + inquirerWindow->h() + 40);
  linkWindow->set_non_modal();
  simulationWindow->position(30 + societyWindow->w()/ 2 - simulationWindow->w() / 2, 60 + societyWindow->h() - simulationWindow->h() - 40);
  simulationWindow->set_non_modal();
  centerWindow(createInquirerPrefsWindow);
	centerWindow(createLinkPrefsWindow);
	centerWindow(inquirerParametersWindow);
	centerWindow(linkParametersWindow);
	centerWindow(distributionWindow);
	centerWindow(metaDistributionWindow);
	centerWindow(batchSimulationWindow);
	centerWindow(progressWindow);
	centerWindow(aboutWindow);
	centerWindow(distributionFreeformValuesWindow);
	centerWindow(statisticsWindow);
	centerWindow(setValuesWindow);
	centerWindow(multiBatchWindow);
	centerWindow(doubleProgressWindow);
	centerWindow(degreeDistributionWindow);
	centerWindow(expressionWindow);
	centerWindow(exportStatisticsWindow);
	centerWindow(exportTopologyWindow);

	// manual window
	manualWindow->resize(societyWindow->x() + 30, societyWindow->y() + 30, societyWindow->w() / 2, societyWindow->h() - 180);

	// file handling
	currentFile[0] = 0;
	fileSaved = true;

	// set selection settings
	selectionToolSettings = SELECT_INQUIRERS | SELECT_INTERNAL_LINKS | SELECT_EXTERNAL_LINKS;

	// undo
	resetUndo();

	// show windows
	societyWindow->show(argc, argv);
	inquirerWindow->show();
	societyWindow->show();
	linkWindow->show();
}

//-----------------------------------------------------------------------------------------------------------------------

void ShowAbout(Fl_Widget *w, void *data) {
	societyWindow->showDialog(DIALOG_ABOUT);
}

//-----------------------------------------------------------------------------------------------------------------------

void App::adjustMacOSMenus(void) {
	// adjust menus
#ifdef __APPLE__
	fl_mac_set_about(ShowAbout, 0);
	t_int idx = societyWindow->menuBar->find_index("Help/About Laputa...");
	societyWindow->menuBar->remove(idx);
	idx = societyWindow->menuBar->find_index("File/Save As...");
	societyWindow->menuBar->mode(idx, 0);
	idx = societyWindow->menuBar->find_index("File/Quit");
	societyWindow->menuBar->remove(idx);

	// move everything
	MoveAllWidgetsTo(societyWindow, 0, -25);
	societyWindow->view->size(societyWindow->view->w(), societyWindow->view->h() + 25);
	societyWindow->scrollbarVertical->size(societyWindow->scrollbarVertical->w(), societyWindow->scrollbarVertical->h() + 25);
	societyWindow->scrollbarHorizontal->position(societyWindow->scrollbarHorizontal->x(), societyWindow->scrollbarHorizontal->y() + 25);
#endif
}

//-----------------------------------------------------------------------------------------------------------------------

void App::centerWindow(Fl_Window* wnd) {
	// get screen size & window size
	t_int sx, ys, sw, sh;
	Fl::screen_xywh(sx, ys, sw, sh, 0);
	wnd->position(sw / 2 - wnd->w() / 2, sh / 3 - wnd->h() / 3);
}


//-----------------------------------------------------------------------------------------------------------------------

void App::setTool(t_int t) {
	if(tool != t) {
		if(tool == TOOL_SELECT) societyWindow->buttonSelect->setPressed(false);
		else if (tool == TOOL_ADD_INQUIRER) societyWindow->buttonCreateInquirer->setPressed(false);
		else if (tool == TOOL_ADD_LINK) societyWindow->buttonCreateLink->setPressed(false);
		tool = t;
		if(tool == TOOL_SELECT) societyWindow->buttonSelect->setPressed(true);
		else if (tool == TOOL_ADD_INQUIRER) societyWindow->buttonCreateInquirer->setPressed(true);
		else if (tool == TOOL_ADD_LINK) societyWindow->buttonCreateLink->setPressed(true);

		// turn off selections
		societyWindow->view->deselectAll();
	}
}

//-----------------------------------------------------------------------------------------------------------------------

void App::setSelectionSetting(t_int setting) {
	selectionToolSettings |= setting;
	if(selectionToolSettings & SELECT_INQUIRERS) const_cast<Fl_Menu_Item*>(societyWindow->buttonSelect->menu())[0].set();
	if(selectionToolSettings & SELECT_INTERNAL_LINKS) const_cast<Fl_Menu_Item*>(societyWindow->buttonSelect->menu())[1].set();
	if(selectionToolSettings & SELECT_EXTERNAL_LINKS) const_cast<Fl_Menu_Item*>(societyWindow->buttonSelect->menu())[2].set();
}

//-----------------------------------------------------------------------------------------------------------------------

void App::clearSelectionSetting(t_int setting) {
	selectionToolSettings &= ~setting;
	if(!(selectionToolSettings & SELECT_INQUIRERS)) const_cast<Fl_Menu_Item*>(societyWindow->buttonSelect->menu())[0].clear();
	if(!(selectionToolSettings & SELECT_INTERNAL_LINKS)) const_cast<Fl_Menu_Item*>(societyWindow->buttonSelect->menu())[1].clear();
	if(!(selectionToolSettings & SELECT_EXTERNAL_LINKS)) const_cast<Fl_Menu_Item*>(societyWindow->buttonSelect->menu())[2].clear();
}

//-----------------------------------------------------------------------------------------------------------------------

void App::toggleSelectionSetting(t_int setting) {
	if(setting == SELECT_INQUIRERS) {
		if(selectionToolSettings & SELECT_INQUIRERS) clearSelectionSetting(SELECT_INQUIRERS);
		else setSelectionSetting(SELECT_INQUIRERS);
	}
	else if(setting == SELECT_INTERNAL_LINKS) {
		if(selectionToolSettings & SELECT_INTERNAL_LINKS) clearSelectionSetting(SELECT_INTERNAL_LINKS | SELECT_EXTERNAL_LINKS);
		else setSelectionSetting(SELECT_INTERNAL_LINKS);
	}
	else if(setting == SELECT_EXTERNAL_LINKS) {
		if(selectionToolSettings & SELECT_EXTERNAL_LINKS) clearSelectionSetting(SELECT_EXTERNAL_LINKS);
		else setSelectionSetting(SELECT_INTERNAL_LINKS | SELECT_EXTERNAL_LINKS);
	}
}

//-----------------------------------------------------------------------------------------------------------------------

void App::newSociety(void) {
	if(!fileSaved) {
		// Ask if we should save file first
		t_int r = fl_choice("Current society has not been saved.\n"
                    "Would you like to save it now?",
                    "Cancel", "Save", "Don't Save");

		if (r == 1) saveSociety();
		else if(r == 2) fileSaved = true;
		else if(r == 0) return;
	}

	// create a new society
	if(fileSaved) {
		delete curSociety;
		curSociety = new Society;

		currentFile[0] = 0;
		resetUndo();

		// update count fields
		societyWindow->view->updateStatistics();

		// erase selection
		societyWindow->view->deselectAll();
		// update statistics
		curSimulation.reset();
	}
}

//-----------------------------------------------------------------------------------------------------------------------

void App::openSociety(void) {
	if(!fileSaved) {
		// Ask if we should save file first
		t_int r = fl_choice("Current society has not been saved.\n"
                    "Would you like to save it now?",
                    "Cancel", "Save", "Don't Save");

		if (r == 1) saveSociety();
		else if(r == 2) fileSaved = true;
		else if(r == 0) return;
	}
	if(fileSaved) {
	  string dstName = OpenFileDialog("Open Society", "*.soc");
		if (dstName != "") {
			// open the file
		  delete curSociety;
			curSociety = new Society;
			loadSocietyFromFile(dstName.c_str());
		}

		// update count fields
		societyWindow->view->updateStatistics();

		// erase selection
		societyWindow->view->deselectAll();

		// update statistics
		curSimulation.reset();

		// show society
		societyWindow->view->setZoomToFit();
		societyWindow->view->redraw();
	}
}

//-----------------------------------------------------------------------------------------------------------------------

void App::saveSociety(void) {
	if (currentFile[0] == '\0') {
    // No filename - get one!
		saveSocietyAs();
	}
	else saveSocietyToFile(currentFile);

	fileSaved = true;
}

//-----------------------------------------------------------------------------------------------------------------------

void App::saveSocietyAs(void) {
	char filename[FL_PATH_MAX] = "Untitled.soc";
	if(currentFile[0] != 0) strcpy(filename, currentFile);
  string dstFile = SaveFileDialog("Save Society As", ".*soc", filename);
  if (dstFile != "") saveSocietyToFile(dstFile.c_str());
	fileSaved = true;
}

//-----------------------------------------------------------------------------------------------------------------------

void App::quit(void) {
	if(!fileSaved) {
		// Ask if we should save file first
		t_int r = fl_choice("Current society has not been saved.\n"
                    "Would you like to save it now?",
                    "Cancel", "Save", "Don't Save");

		if (r == 1) saveSociety();
		else if(r == 2) fileSaved = true;
		else if(r == 0) return;
	}

	// quit
	if(fileSaved) exit(0);
}

//-----------------------------------------------------------------------------------------------------------------------

void App::saveSocietyToFile(const char* filename) {
	char relName[FL_PATH_MAX];

	fl_filename_relative(relName, FL_PATH_MAX, filename);
	curSociety->saveToFile(relName);
}

//-----------------------------------------------------------------------------------------------------------------------

void App::loadSocietyFromFile(const char* filename) {
	curSociety->loadFromFile(filename);
	strcpy(currentFile, filename);
	fileSaved = true;
	resetUndo();
	curSimulation.reset();
}

//-----------------------------------------------------------------------------------------------------------------------

void App::toggleTooltips(void) {
	Fl_Menu_Item *p = (Fl_Menu_Item*)societyWindow->menuBar->find_item("Help/Tooltips");
    if(Fl_Tooltip::enabled()) {
		Fl_Tooltip::disable();
		if(p) p->clear();
	}
	else {
		Fl_Tooltip::enable();
		if(p) p->set();
	}
}


//-----------------------------------------------------------------------------------------------------------------------

string App::docsFile(const char* filename) {
    return docsPath + string(filename);
}

//-----------------------------------------------------------------------------------------------------------------------

string App::dataFile(const char* filename) {
    return dataPath + string(filename);
}

//-----------------------------------------------------------------------------------------------------------------------

void App::touchFile(void) {
	// has an actual change occurred?
	if (*undos[curUndoIndex] == *curSociety) return;

	++curUndoIndex;
	if (curUndoIndex == MAX_UNDOS) {
		// move stuff in undo list if necessary
		delete undos[0];
		for (size_t i = 1; i < MAX_UNDOS; ++i) undos[i - 1] = undos[i];
		--curUndoIndex;
	}
	else {
		// delete undos after this one
		for (t_int i = curUndoIndex; i < undos.size(); ++i) delete undos[i];

		// expand undo list
		undos.resize((size_t)curUndoIndex + 1);
	}

	// store society
	undos[curUndoIndex] = new Society(*curSociety);
	fileSaved = false;

	// update menus
	t_int uidx = societyWindow->menuBar->find_index("Edit/Undo"), ridx = societyWindow->menuBar->find_index("Edit/Redo");
	societyWindow->menuBar->mode(uidx, societyWindow->menuBar->mode(uidx) &  ~FL_MENU_INACTIVE);
	societyWindow->menuBar->mode(ridx, societyWindow->menuBar->mode(ridx) | FL_MENU_INACTIVE);
}

//-----------------------------------------------------------------------------------------------------------------------

void App::doUndo(void) {
	if (curUndoIndex > 0) {
		// do the undo
		--curUndoIndex;
		delete curSociety;
		curSociety = new Society(*undos[curUndoIndex]);

		// update menus
		t_int uidx = societyWindow->menuBar->find_index("Edit/Undo"), ridx = societyWindow->menuBar->find_index("Edit/Redo");
		if (curUndoIndex == 0) societyWindow->menuBar->mode(uidx, societyWindow->menuBar->mode(uidx) | FL_MENU_INACTIVE);
		societyWindow->menuBar->mode(ridx, societyWindow->menuBar->mode(ridx) & ~FL_MENU_INACTIVE);

		// update everything else
		fileSaved = false;
		curSociety->recalculateListeners();
		societyWindow->view->updateStatistics();
		inquirerWindow->configure();
		linkWindow->configure();
		societyWindow->view->redraw();
	}
}

//-----------------------------------------------------------------------------------------------------------------------

void App::doRedo(void) {
	if (curUndoIndex < undos.size() - 1) {
		// do the undo
		++curUndoIndex;
		curSociety = new Society(*undos[curUndoIndex]);

		// update menus
		t_int uidx = societyWindow->menuBar->find_index("Edit/Undo"), ridx = societyWindow->menuBar->find_index("Edit/Redo");
		societyWindow->menuBar->mode(uidx, societyWindow->menuBar->mode(uidx) &  ~FL_MENU_INACTIVE);
		if (curUndoIndex == undos.size()) societyWindow->menuBar->mode(ridx, societyWindow->menuBar->mode(ridx) | FL_MENU_INACTIVE);

		// update everything else
		curSociety->recalculateListeners();
		societyWindow->view->updateStatistics();
		inquirerWindow->configure();
		linkWindow->configure();
		societyWindow->view->redraw();
		fileSaved = false;
	}
}
//-----------------------------------------------------------------------------------------------------------------------

void App::resetUndo(void) {
	for (t_int i = 0; i < undos.size(); ++i) delete undos[i];
	undos.resize(1);
	undos[0] = new Society(*curSociety);
	curUndoIndex = 0;
	t_int uidx = societyWindow->menuBar->find_index("Edit/Undo"), ridx = societyWindow->menuBar->find_index("Edit/Redo");
	societyWindow->menuBar->mode(uidx, societyWindow->menuBar->mode(uidx) | FL_MENU_INACTIVE);
	societyWindow->menuBar->mode(ridx, societyWindow->menuBar->mode(ridx) | FL_MENU_INACTIVE);

}

//-----------------------------------------------------------------------------------------------------------------------

t_int App::run(void) {
	return Fl::run();
}
