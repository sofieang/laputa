#ifndef __USERINTERFACEITEMS_H__
#define __USERINTERFACEITEMS_H__
#include "Prefix.h"

#include <FL/Fl.H>
#include <FL/Fl_Double_Window.H>
#include <FL/Fl_Sys_Menu_Bar.H>
#include <FL/Fl_Scroll.H>
#include <FL/Fl_Menu_Bar.H>
#include <FL/Fl_Button.H>
#include <FL/Fl_Spinner.H>
#include <FL/Fl_Group.H>
#include <FL/Fl_Round_Button.H>
#include <FL/Fl_Value_Slider.H>
#include <FL/Fl_Value_Output.H>
#include <FL/Fl_Check_Button.H>
#include <FL/Fl_Value_Input.H>
#include <FL/Fl_Progress.H>
#include <FL/Fl_Box.H>
#include <FL/Fl_Return_Button.H>
#include <FL/Fl_Text_Display.H>
#include <FL/Fl_Output.H>
#include <FL/Fl_Input.H>
#include <FL/fl_ask.H>
#include <FL/Fl_PNG_Image.H>
#include <FL/Fl_Menu_Item.H>
#include <FL/Fl_Help_Dialog.H>
#include <FL/Fl_Multi_Browser.H>
#include <FL/Fl_Tabs.H>
#include <FL/Fl_Choice.H>


#include "ToolButton.h"
#include "Distribution.h"
#include "MultiBatch.h"
#include "ExpressionField.h"


#include <deque>

// dialogs
#define DIALOG_CREATE_INQUIRER_PREFS 1
#define DIALOG_CREATE_LINK_PREFS 2
#define DIALOG_DISTRIBUTION 3
#define DIALOG_TRUSTFUNCTION 4
#define DIALOG_METADISTRIBUTION 5
#define DIALOG_SIMULATION 6
#define DIALOG_BATCH_SIMULATION 7
#define DIALOG_BATCH_SIMULATION_PREVIEW 8
#define DIALOG_BATCH_SIMULATION_MULTIBATCH 9
#define DIALOG_BATCH_SIMULATION_GENERATE 10
#define DIALOG_PROGRESS 11
#define DIALOG_STATISTICS 12
#define DIALOG_ABOUT 13
#define DIALOG_INQUIRER_PARAMETERS 14
#define DIALOG_LINK_PARAMETERS 15
#define DIALOG_MANUAL 16
#define DIALOG_DISTRIBUTION_FREEFORM_VALUES 17
#define DIALOG_SET_VALUES 18
#define DIALOG_MULTIBATCH 19
#define DIALOG_DOUBLE_PROGRESS 20
#define DIALOG_DEGREE_DISTRIBUTION 21
#define DIALOG_EXPRESSION 22
#define DIALOG_EXPORT_STATISTICS 23
#define DIALOG_EXPORT_TOPOLOGY 24
#define N_DIALOG_TYPES 24

class SocietyView;
class DistributionView;
class DistributionViewEditable;
class MetaDistribution;
class MetaDistributionView;
class StatisticsView;
class TrustView;
class CoordinateSelector;


// forward declarations of window classes
class SocietyWindow;
class InquirerWindow;
class CreateInquirerPrefsWindow;
class LinkWindow;
class CreateLinkPrefsWindow;
class DistributionWindow;
class MetaDistributionWindow;
class SimulationWindow;
class BatchSimulationWindow;
class ProgressWindow;
class StatisticsWindow;
class AboutWindow;
class InquirerParametersWindow;
class LinkParametersWindow;
class ManualWindow;
class DistributionFreeformValuesWindow;
class SetValuesWindow;
class MultiBatchWindow;
class DoubleProgressWindow;
class DegreeDistributionWindow;
class ExpressionWindow;
class ExportStatisticsWindow;
class ExportTopologyWindow;

// globals for windows
extern SocietyWindow *societyWindow;
extern InquirerWindow *inquirerWindow;
extern CreateInquirerPrefsWindow *createInquirerPrefsWindow;
extern LinkWindow *linkWindow;
extern CreateLinkPrefsWindow *createLinkPrefsWindow;
extern DistributionWindow *distributionWindow;
extern MetaDistributionWindow *metaDistributionWindow;
extern SimulationWindow *simulationWindow;
extern BatchSimulationWindow *batchSimulationWindow;
extern ProgressWindow *progressWindow;
extern StatisticsWindow *statisticsWindow;
extern AboutWindow *aboutWindow;
extern InquirerParametersWindow *inquirerParametersWindow;
extern LinkParametersWindow *linkParametersWindow;
extern ManualWindow *manualWindow;
extern DistributionFreeformValuesWindow *distributionFreeformValuesWindow;
extern SetValuesWindow *setValuesWindow;
extern MultiBatchWindow *multiBatchWindow;
extern DoubleProgressWindow *doubleProgressWindow;
extern DegreeDistributionWindow *degreeDistributionWindow;
extern ExpressionWindow *expressionWindow;
extern ExportStatisticsWindow *exportStatisticsWindow;
extern ExportTopologyWindow *exportTopologyWindow;

// app class
class App;
extern App* app;

// dialog t_interface for passing information back to windows opening a dialog
class DialogInterface : public Fl_Double_Window {
public:
    DialogInterface(t_int X, t_int Y, t_int W, t_int H, const char* l = 0) : Fl_Double_Window(X, Y, W, H, l) {}
    DialogInterface(t_int W, t_int H, const char* l = 0) : Fl_Double_Window(W, H, l) {}
	virtual void answer(t_int answ);
	void showDialog(t_int dialog, const void* data = 0);
	void closeDialog(bool ok, t_int msg = 0);
    t_int handle(t_int evt);
	t_int dialogId(void);

    t_int xDrag = 0, yDrag = 0;
};

// Main society view window
class SocietyWindow : public DialogInterface {
public:
	SocietyWindow(t_int X, t_int Y, t_int W, t_int H, const char* l = 0) : DialogInterface(X, Y, W, H, l) {societyWindow = this;}
	SocietyWindow(t_int W, t_int H, const char* l = 0) : DialogInterface(W, H, l) {societyWindow = this;}
	void adjustMenuShortcuts(void);
	void resize(int X, int Y, int W, int H);

	SocietyView *view = nullptr;
	Fl_Scrollbar *scrollbarHorizontal = nullptr;
	Fl_Scrollbar *scrollbarVertical = nullptr;

	Fl_Sys_Menu_Bar *menuBar = nullptr;

	Fl_Group *groupToolBar = nullptr;
	ToolButton *buttonSelect = nullptr;
	ToolButton *buttonCreateInquirer = nullptr;
	ToolButton *buttonCreateLink = nullptr;

	Fl_Value_Output *outputNumInquirers = nullptr;
	Fl_Value_Output *outputNumLinks = nullptr;
	Fl_Value_Input *inputZoom = nullptr;

	Fl_Button *buttonZoomToFit = nullptr;
	Fl_Button *buttonZoomDefault = nullptr;
	Fl_Button *buttonZoomIncrease = nullptr;
	Fl_Button *buttonZoomDecrease = nullptr;

	Fl_Button *buttonDegrees = nullptr;
};


// include inquirer & link inspectors
#include "Inspectors.h"

// Inquirer creation preferences window
class CreateInquirerPrefsWindow : public DialogInterface {
public:
	CreateInquirerPrefsWindow(t_int X, t_int Y, t_int W, t_int H, const char* l = 0) : DialogInterface(X, Y, W, H, l) {createInquirerPrefsWindow = this;}
	CreateInquirerPrefsWindow(t_int W, t_int H, const char* l = 0) : DialogInterface(W, H, l) {createInquirerPrefsWindow = this;}

	DistributionView *viewBelief = nullptr;
	DistributionView *viewInquiryChance = nullptr;
	DistributionView *viewInquiryAccuracy = nullptr;
	MetaDistributionView *viewInquiryTrust = nullptr;

	Fl_Check_Button *buttonUpdateInquiryTrust = nullptr;
	Fl_Check_Button *buttonIncludeInStatistics = nullptr;
};


// Link creation preferences windiw
class CreateLinkPrefsWindow : public DialogInterface {
public:
	CreateLinkPrefsWindow(t_int X, t_int Y, t_int W, t_int H, const char* l = 0) : DialogInterface(X, Y, W, H, l) {createLinkPrefsWindow = this;}
	CreateLinkPrefsWindow(t_int W, t_int H, const char* l = 0) : DialogInterface(W, H, l) {createLinkPrefsWindow = this;}

	DistributionView *viewListenChance = nullptr;
	DistributionView *viewThreshold = nullptr;
	MetaDistributionView *viewTrust = nullptr;
	Fl_Check_Button *buttonUpdateTrust = nullptr;

	Fl_Round_Button* buttonEvidencePolicy[3] = { nullptr, nullptr, nullptr };
	Fl_Check_Button *buttonExcludePrior = nullptr;

};

// distribution setup window
class DistributionWindow : public DialogInterface {
public:
	DistributionWindow(t_int X, t_int Y, t_int W, t_int H, const char* l = 0) : DialogInterface(X, Y, W, H, l) {distributionWindow = this;}
	DistributionWindow(t_int W, t_int H, const char* l = 0) : DialogInterface(W, H, l) {distributionWindow = this;}

	void configure(Distribution* d);
	void configure(TrustFunction* d);
	void answer(t_int answ);

	void loadDistribution(void);
	void saveDistribution(void);

	void setBetaDistributionInterface(t_int i);
	void setBetaDistributionFromMean(t_float v);
	void setBetaDistributionFromDev(t_float v);

	DistributionViewEditable* view = nullptr;
	Fl_Value_Slider* sliderPtValue = nullptr;
	Fl_Value_Slider* sliderIntLower = nullptr;
	Fl_Value_Slider* sliderIntUpper = nullptr;
	Fl_Value_Slider* sliderNrmMidpt = nullptr;
	Fl_Value_Slider* sliderNrmStddev = nullptr;
	Fl_Value_Slider* sliderBtAlpha = nullptr;
	Fl_Value_Slider* sliderBtBeta = nullptr;
	Fl_Light_Button* buttonAlphaBeta = nullptr;
	Fl_Light_Button* buttonMeanDev = nullptr;
	Fl_Value_Slider* sliderWeight[N_DISTRIBUTION_TYPES] = { nullptr, nullptr, nullptr, nullptr, nullptr };

	Fl_Spinner* inputResolution = nullptr;

	Fl_Choice* choiceFilter = nullptr;
	Fl_Button* buttonValues = nullptr;
	Fl_Button* buttonLoad = nullptr;
	Fl_Button* buttonSave = nullptr;

	Fl_Box* boxLabel[5] = { nullptr, nullptr, nullptr, nullptr, nullptr };

	DistributionView *distributionViewEdited = nullptr;
	TrustView *trustViewEdited = nullptr;
};

// metadistribution setup window
class MetaDistributionWindow : public DialogInterface {
public:
	MetaDistributionWindow(t_int X, t_int Y, t_int W, t_int H, const char* l = 0) : DialogInterface(X, Y, W, H, l) {metaDistributionWindow = this;}
	MetaDistributionWindow(t_int W, t_int H, const char* l = 0) : DialogInterface(W, H, l) {metaDistributionWindow = this;}
	void setMetaDistribution(MetaDistribution* md);
	void setPreview(t_float v);
	void answer(t_int answ);

	DistributionView *viewZero = nullptr;
	DistributionView *viewOne = nullptr;
	DistributionView *viewMixture = nullptr;
	MetaDistributionView *viewMetaDistribution = nullptr;
	DistributionView *viewPreview = nullptr;

	Fl_Slider *sliderParameter = nullptr;

	MetaDistribution curMetaDistribution;
	MetaDistribution* metaDistributionFieldEdited = nullptr;
	MetaDistributionView* metaDistributionViewEdited = nullptr;

	Distribution previewDistribution;

};


class SimulationWindow : public DialogInterface {
public:
	SimulationWindow(t_int X, t_int Y, t_int W, t_int H, const char* l = 0) : DialogInterface(X, Y, W, H, l) {simulationWindow = this;}
	SimulationWindow(t_int W, t_int H, const char* l = 0) : DialogInterface(W, H, l) {simulationWindow = this;}
	void loadParameters(void);

	Fl_Output *outputTime = nullptr;
	Fl_Output *outputEValue = nullptr;
	Fl_Output *outputEValueDelta = nullptr;
	Fl_Output *outputPolarisation = nullptr;
	Fl_Output *outputPolarisationDelta = nullptr;

	Fl_Button *buttonRun = nullptr;
	Fl_Button *buttonStep = nullptr;
	Fl_Button *buttonPause = nullptr;
	Fl_Button *buttonRewind = nullptr;

	Fl_Choice *choiceLogLevel = nullptr;

	Fl_Text_Display *outputLog = nullptr;

	// overloaded deactivate/activate
	void deactivate(void) {
		for(t_int i = 0; i < children(); ++i) child(i)->deactivate();
		Fl_Double_Window::deactivate();
	}
	void activate(void) {
		Fl_Double_Window::activate();
		for(t_int i = 0; i < children(); ++i) child(i)->activate();
	}

	// overloaded event handling to allow moving window
	t_int handle (t_int event);

	Society* templateSoc = nullptr;
};

// include batch simulation window header
#include "BatchSimulationWindow.h"

// progress bar for batch processing
class ProgressWindow : public DialogInterface {
public:
	ProgressWindow(t_int X, t_int Y, t_int W, t_int H, const char* l = 0) : DialogInterface(X, Y, W, H, l) {progressWindow = this;}
	ProgressWindow(t_int W, t_int H, const char* l = 0) : DialogInterface(W, H, l) {progressWindow = this;}

	Fl_Progress *barProgress = nullptr;

	BatchSimulation *bs = nullptr;
};

// Statistics dialog
class StatisticsWindow : public DialogInterface {
public:
	StatisticsWindow(t_int X, t_int Y, t_int W, t_int H, const char* l = 0) : DialogInterface(X, Y, W, H, l) {statisticsWindow = this;}
	StatisticsWindow(t_int W, t_int H, const char* l = 0) : DialogInterface(W, H, l) {statisticsWindow = this;}
	void fillOutStatistics(void);
	void computeConfidences(t_float level);
	void exportStatistics(void);
	void setDiagramView(t_int v);

	Fl_Box* boxDiagramXValue[5] = { nullptr, nullptr, nullptr, nullptr, nullptr };
	Fl_Box* boxDiagramYValue[5] = { nullptr, nullptr, nullptr, nullptr, nullptr };
	StatisticsView* viewDiagram = nullptr;

	Fl_Output *outputAvgEValue = nullptr;
	Fl_Output *outputAvgEValueMargin = nullptr;
	Fl_Output *outputEValueDelta = nullptr;
	Fl_Output *outputEValueDeltaMargin = nullptr;
	Fl_Output *outputPolarisation = nullptr;
	Fl_Output *outputPolarisationMargin = nullptr;
	Fl_Output *outputPolarisationDelta = nullptr;
	Fl_Output *outputPolarisationDeltaMargin = nullptr;

	Fl_Output *outputNumSocieties = nullptr;
	Fl_Output *outputNumSteps = nullptr;

	Fl_Choice *choiceConfidenceLevel = nullptr;

	Fl_Output *outputMessagesSentTotal = nullptr;
	Fl_Output *outputMessagesSentPerInquirer = nullptr;
	Fl_Output *outputInquiryResultsTotal = nullptr;
	Fl_Output *outputInquiryResultsPerInquirer = nullptr;

	Fl_Output *outputBWToPPerc = nullptr;
	Fl_Output *outputBWToNotPPerc = nullptr;
	Fl_Output *outputBWToPEffect = nullptr;
	Fl_Output *outputBWToNotPEffect = nullptr;

	Fl_Button *buttonExportEValues = nullptr;
	Fl_Button *buttonExportTopologies = nullptr;
	Fl_Choice *choiceDiagramView = nullptr;

	BatchSimulation* bsShown = nullptr;
};

// About dialog box
class AboutWindow : public DialogInterface {
public:
	AboutWindow(t_int X, t_int Y, t_int W, t_int H, const char* l = 0) : DialogInterface(X, Y, W, H, l) {aboutWindow = this;}
	AboutWindow(t_int W, t_int H, const char* l = 0) : DialogInterface(W, H, l) {aboutWindow = this;}

	Fl_Button *buttonAbout = nullptr;
};

// Inquirer parameter dialog
class InquirerParametersWindow : public DialogInterface {
public:
	InquirerParametersWindow(t_int X, t_int Y, t_int W, t_int H, const char* l = 0) : DialogInterface(X, Y, W, H, l) {inquirerParametersWindow = this;}
	InquirerParametersWindow(t_int W, t_int H, const char* l = 0) : DialogInterface(W, H, l) {inquirerParametersWindow = this;}

	DistributionView *viewBelief = nullptr;
	Fl_Box *boxBelief = nullptr;
	DistributionView *viewInquiryChance = nullptr;
	Fl_Box *boxInquiryChance = nullptr;
	DistributionView *viewInquiryAccuracy = nullptr;
	Fl_Box *boxInquiryAccuracy = nullptr;
	MetaDistributionView *viewInquiryTrust = nullptr;
	Fl_Box *boxInquiryTrust = nullptr;

	Fl_Slider* sliderVaryBelief = nullptr;
	Fl_Slider* sliderVaryInquiryChance = nullptr;
	Fl_Slider* sliderVaryVeracityChance = nullptr;
	Fl_Slider* sliderVaryInquiryTrust = nullptr;
};

// Link parameter dialog
class LinkParametersWindow : public DialogInterface {
public:
	LinkParametersWindow(t_int X, t_int Y, t_int W, t_int H, const char* l = 0) : DialogInterface(X, Y, W, H, l) {linkParametersWindow = this;}
	LinkParametersWindow(t_int W, t_int H, const char* l = 0) : DialogInterface(W, H, l) {linkParametersWindow = this;}

	DistributionView *viewListenChance = nullptr;
	Fl_Box *boxListenChance = nullptr;
	DistributionView *viewThreshold = nullptr;
	Fl_Box *boxThreshold = nullptr;
	MetaDistributionView *viewTrust = nullptr;
	Fl_Box *boxTrust = nullptr;

	Fl_Slider* sliderVaryListenChance = nullptr;
	Fl_Slider* sliderVaryTrust = nullptr;
	Fl_Slider* sliderVaryThreshold = nullptr;
};


// Manual dialog box
class ManualWindow : public DialogInterface {
public:
	ManualWindow(t_int X, t_int Y, t_int W, t_int H, const char* l = 0) : DialogInterface(X, Y, W, H, l) {manualWindow = this;}
	ManualWindow(t_int W, t_int H, const char* l = 0) : DialogInterface(W, H, l) {manualWindow = this;}

	Fl_Button* buttonBack = nullptr;
	Fl_Button* buttonForward = nullptr;
	Fl_Button* buttonHome = nullptr;
	Fl_Output* outputFile = nullptr;
	Fl_Help_View* view = nullptr;

	vector<string> pageHistory;
	int curPage = 0;

	void configure(const char* filename = 0);
	void goBack(void);
	void goForward(void);
	void goHome(void);

	void hide(void) {
		closeDialog(true);
		Fl_Double_Window::hide();
	}
	void resize(int x, int y, int w, int h) {
		outputFile->resize(outputFile->x(), outputFile->y(), w - outputFile->x() - 5, outputFile->h());
		view->resize(buttonBack->x(), buttonBack->y() + buttonBack->h() + 5, w - buttonBack->x() - 5, h - buttonBack->h() - buttonBack->y() - 10);
		Fl_Double_Window::resize(x, y, w, h);
	}
};


// Distribution windows values, for freeform distributions
class DistributionFreeformValuesWindow : public DialogInterface {
public:
	DistributionFreeformValuesWindow(t_int X, t_int Y, t_int W, t_int H, const char* l = 0) : DialogInterface(X, Y, W, H, l) {distributionFreeformValuesWindow = this;}
	DistributionFreeformValuesWindow(t_int W, t_int H, const char* l = 0) : DialogInterface(W, H, l) {distributionFreeformValuesWindow = this;}
	void setDistributionValues(void);
	void getDistributionValues(void);

	Fl_Value_Input* fieldValue = nullptr;
	Fl_Multi_Browser* listValues = nullptr;
};

class SetValuesWindow : public DialogInterface {
public:
	SetValuesWindow(t_int X, t_int Y, t_int W, t_int H, const char* l = 0) : DialogInterface(X, Y, W, H, l) {setValuesWindow = this;}
	SetValuesWindow(t_int W, t_int H, const char* l = 0) : DialogInterface(W, H, l) {setValuesWindow = this;}

	Fl_Value_Input* inputValue = nullptr;
	Fl_Value_Slider* sliderEdited = nullptr;
};

// Metabatch simulationwindow
class MultiBatchWindow : public DialogInterface {
public:
	MultiBatchWindow(t_int X, t_int Y, t_int W, t_int H, const char* l = 0) : DialogInterface(X, Y, W, H, l) {multiBatchWindow = this;}
	MultiBatchWindow(t_int W, t_int H, const char* l = 0) : DialogInterface(W, H, l) {multiBatchWindow = this;}
	void configure(void);
	void showPreview(void);
	void loadMultibatch(void);
	void saveMultibatch(void);

	Fl_Spinner* inputStepsAtoB = nullptr;
	Fl_Spinner* inputStepsAtoC = nullptr;

	Fl_Light_Button* btnOneDimension = nullptr;
	Fl_Light_Button* btnTwoDimensions = nullptr;

	Fl_Button* btnSetBatchA = nullptr;
	Fl_Button* btnSetBatchB = nullptr;
	Fl_Button* btnSetBatchC = nullptr;
	Fl_Button* btnSetBatchD = nullptr;

	Fl_Button* btnCopyBatch[8] = { nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr };

	Fl_Button* btnPreview = nullptr;

	MultiBatch* mbEdited = nullptr;
	MultiBatch mb;
};

// double progress bar for metabatch processing
class DoubleProgressWindow : public DialogInterface {
public:
	DoubleProgressWindow(t_int X, t_int Y, t_int W, t_int H, const char* l = 0) : DialogInterface(X, Y, W, H, l) {doubleProgressWindow = this;}
	DoubleProgressWindow(t_int W, t_int H, const char* l = 0) : DialogInterface(W, H, l) {doubleProgressWindow = this;}

	Fl_Progress *barProgress = nullptr;
	Fl_Output *outputSecondProgress = nullptr;

	MultiBatch *mb = nullptr;
};

// Window for showing data about degree distribution
class DegreeDistributionWindow : public DialogInterface {
public:
	DegreeDistributionWindow(t_int X, t_int Y, t_int W, t_int H, const char* l = 0) : DialogInterface(X, Y, W, H, l) {degreeDistributionWindow = this;}
	DegreeDistributionWindow(t_int W, t_int H, const char* l = 0) : DialogInterface(W, H, l) {degreeDistributionWindow = this;}

	void configure(vector<t_float> *deg);
	void calculateFit(t_int pane, t_int func);
	void setCutoff(t_int pane, t_int cutoff);
	void fitToCutoff(t_int pane, t_int cutoff);

	StatisticsView* view[3] = { nullptr, nullptr, nullptr };
	Fl_Box* labelXAxis[3][5] = { nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr };
	Fl_Box *labelYAxis[3][5] = { nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr };
	Fl_Choice *choiceFunction[3] = { nullptr, nullptr, nullptr };
	Fl_Value_Output *outputError[3] = { nullptr, nullptr, nullptr };
	Fl_Box *boxFormula[3] = { nullptr, nullptr, nullptr };
	Fl_Value_Output* outputParameter[3][4]{ nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr };
	Fl_Value_Slider* sliderCutoff[3] = { nullptr, nullptr, nullptr };
};

// Window for editing mathematical expressions
class ExpressionWindow : public DialogInterface {
public:
	ExpressionWindow(t_int X, t_int Y, t_int W, t_int H, const char* l = 0) : DialogInterface(X, Y, W, H, l) {expressionWindow = this;}
	ExpressionWindow(t_int W, t_int H, const char* l = 0) : DialogInterface(W, H, l) {expressionWindow = this;}

	void configure(Distribution *distr);
	void applyExpression(void);

	Fl_Tabs *tabs = nullptr;
	Fl_Group *groupPDF = nullptr;
	Fl_Group *groupCDF = nullptr;
	ExpressionField *inputFormulaPDF = nullptr;
	ExpressionField *inputFormulaCDF = nullptr;

	Fl_Text_Buffer bufPDF;
	Fl_Text_Buffer bufCDF;
	Distribution *d = nullptr;
};

// Window for exporting statistics
class ExportStatisticsWindow : public DialogInterface {
public:
	ExportStatisticsWindow(t_int X, t_int Y, t_int W, t_int H, const char* l = 0) : DialogInterface(X, Y, W, H, l) {exportStatisticsWindow = this;}
	ExportStatisticsWindow(t_int W, t_int H, const char* l = 0) : DialogInterface(W, H, l) {exportStatisticsWindow = this;}


	void configure(BatchSimulation *b);
	bool save(void);
	void choose(t_int dim, t_int variable);
	bool exportToFile(const char* filename);

	Fl_Round_Button* buttonVariable[3][3] = { nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr };
	Fl_Value_Input *inputFrom[3] = { nullptr, nullptr, nullptr };
	Fl_Value_Input *inputTo[3] = { nullptr, nullptr, nullptr };
	Fl_Check_Button *buttonAverage[3] = { nullptr, nullptr, nullptr };


	BatchSimulation *bs = nullptr;

};

class ExportTopologyWindow : public DialogInterface {
public:
	ExportTopologyWindow(t_int X, t_int Y, t_int W, t_int H, const char* l = 0) : DialogInterface(X, Y, W, H, l) { exportTopologyWindow = this; }
	ExportTopologyWindow(t_int W, t_int H, const char* l = 0) : DialogInterface(W, H, l) { exportTopologyWindow = this; }

	bool save(void);
	void configure(BatchSimulation *b);
	bool exportToFile(char* filename);

	Fl_Round_Button *buttonWeightNone = nullptr;
	Fl_Round_Button *buttonWeightListenChance = nullptr;
	Fl_Round_Button *buttonWeightTrust = nullptr;

	Fl_Value_Input *fieldMinListenChance = nullptr;

	Fl_Round_Button *buttonSingleFile = nullptr;
	Fl_Round_Button *buttonFolder = nullptr;
	Fl_Box *labelSaveAs = nullptr;

	BatchSimulation *bs = nullptr;

};



// Dialog Factory class. Shows and hides dialogs (they are created by the app object's
// initialisation method).

class DialogFactory {
friend class DialogInterface;
friend class UserInterface;
protected:
	void showDialog(t_int d, const void *data = 0);
	void closeDialog(t_int d, bool ok);

	deque<pair<t_int, DialogInterface*> > callstack;
};



#include "App.h"


#endif
