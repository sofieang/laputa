#ifndef __APP_H__
#define __APP_H__


#include "Prefix.h"
#include "MultiBatch.h"
#include <assert.h>


#include "UserInterfaceItems.h"
#include "UserInterface.h"
#include "SocietyView.h"
#include "Distribution.h"
#include "DistributionView.h"
#include "Simulation.h"
#include "StatisticsView.h"
#include "gsl/gsl_rng.h"
#include "gsl/gsl_math.h"
#include <stdio.h>
#include <string>
#include "Files.h"

using namespace std;

#define LAPUTA_VERSION 170
#define MIN_LAPUTA_VERSION 150

// tools
#define TOOL_SELECT 0
#define TOOL_ADD_INQUIRER 1
#define TOOL_ADD_LINK 2

// selection tool setting bits
#define SELECT_INQUIRERS 1
#define SELECT_INTERNAL_LINKS 2
#define SELECT_EXTERNAL_LINKS 4

// undo
#define MAX_UNDOS 32

//-----------------------------------------------------------------------------------------------------------------------

class UserInterface;

// Application class - only one instance may exist, and is stored as the global variable "app"
// Handles initialisation and general tasks about the program and its t_interface
class App {
friend class UserInterface;
friend class DialogFactory;
public:

	//constructor
	App(t_int argc, char** argv, const string& inDataPath, const string& inDocsPath);

	// start program
	t_int run(void);

	// current society setup
	SocietySetup* getSocietySetup(void) {return &curSocietySetup;}

	// what simulations are active?
	Simulation *getCurSimulation(void) {return &curSimulation;}
	BatchSimulation *getCurBatchSimulation(void) {return &curBatchSimulation;}

	// which is the current tool?
	t_int getTool(void) {return tool;}
	void setTool(t_int t);

	// selection tool settings
	t_int getSelectionSettings(void) {return selectionToolSettings;}
	void setSelectionSetting(t_int setting);
	void clearSelectionSetting(t_int setting);
	void toggleSelectionSetting(t_int setting);

	// modal dialog factory
	DialogFactory df;
	void centerWindow(Fl_Window* wnd);

	// undo & redo, cut-copy-paste
	void doUndo(void);
	void doRedo(void);
	void resetUndo(void);
	SocietyFragment* getClipboard(void) {return clipboard;}
	void setClipboard(SocietyFragment *f) {
		if(clipboard) delete clipboard;
		clipboard = f;
	}

	// tooltips
	void toggleTooltips(void);

	// file handling
	void newSociety(void);
	void openSociety(void);
	void saveSociety(void);
	void saveSocietyAs(void);
	void exportStatistics(void);
	void quit(void);
	void touchFile(void);
	void saveSocietyToFile(const char* filename);
	void loadSocietyFromFile(const char* filename);

	// menus
	void adjustMacOSMenus(void);

  // file name access
  string dataFile(const char *filename);
  string docsFile(const char *filename);

  // paths
  string dataPath;
  string docsPath;

	// ui variables
	UserInterface *ui;
	t_int tool, selectionToolSettings;

	// current society setup (used for creting new inquirers/links)
	SocietySetup curSocietySetup;

	// variables for dialogs
	SocietySetup workingSetup;

	// file we are working with
	char currentFile[FL_PATH_MAX];
	bool fileSaved;

	// current simulation, batch simulation in progress
	Simulation curSimulation;
	BatchSimulation curBatchSimulation;
	MultiBatch curMultiBatch;

	// clipboard
	SocietyFragment* clipboard;

	// Undo/Redo list
  vector<Society*> undos;
	t_int curUndoIndex;
};




// global variables
extern App *app;
extern gsl_rng *rng;





#endif
