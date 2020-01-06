#ifndef __BATCHSIMULATIONWINDOW_H__
#define __BATCHSIMULATIONWINDOW_H__

#include "UserInterfaceItems.h"
#include "CoordinateSelector.h"
#include "DistributionView.h"
#include "MultiBatch.h"

#define BS_FORM_AS_BEFORE 0
#define BS_FORM_SIMULATION 1
#define BS_FORM_MULTIBATCH 2
#define BS_FORM_PREVIEW 3
#define BS_FORM_GENERATE 4

class BatchSimulationWindow;
extern BatchSimulationWindow* batchSimulationWindow;

// multiple society simulation dialog
class BatchSimulationWindow : public DialogInterface {
public:
	BatchSimulationWindow(t_int X, t_int Y, t_int W, t_int H, const char* l = 0) : DialogInterface(X, Y, W, H, l) {batchSimulationWindow = this;}
	BatchSimulationWindow(t_int W, t_int H, const char* l = 0) : DialogInterface(W, H, l) {batchSimulationWindow = this;}
	
	void configure(t_int form = BS_FORM_AS_BEFORE);
	void configureStage(void);
	void configureNetwork(void);
	void configureInquirers(void);
	void configureLinks(void);
	void configureEValues(void);
	void configureRecord(void);
	
	void turnOffExcludedVariables(void);
	void turnOffEditing(void);
	
	void setStage(t_int s);
	void setNumStages(t_int s);
	void copyBatch(t_int from, t_int to);
	void saveBatchSimulation(void);
	void loadBatchSimulation(void);
	
	void activateRecursively(void);
	
	Fl_Button *buttonLoad = nullptr;
	Fl_Button *buttonSave = nullptr;
	Fl_Button *buttonCancel = nullptr;
	
	Fl_Tabs *groupTabs = nullptr;
	Fl_Group *groupStage = nullptr;
	Fl_Group *groupNetwork = nullptr;
	Fl_Group *groupInquirers = nullptr;
	Fl_Group *groupLinks = nullptr;
	Fl_Group *groupEValues = nullptr;
	Fl_Group *groupRecord = nullptr;
	
	Fl_Value_Input *inputNumTrials = nullptr;
	CoordinateSelector *selectorPosition = nullptr;
	Fl_Box *boxPosition = nullptr;
	
	Fl_Value_Input *inputStepsPerTrial = nullptr;			// stage tab
	
	Fl_Value_Input *inputPopulationMin = nullptr;			// network tab
	Fl_Value_Input *inputPopulationMax = nullptr;
	Fl_Check_Button *buttonVaryPopulation = nullptr;
	Fl_Value_Slider *sliderInitialPopulationPart = nullptr;
	Fl_Value_Slider *sliderGrowthBalance = nullptr;
	Fl_Check_Button *buttonVaryLinks = nullptr;
	Fl_Check_Button *buttonLimitLinksToOnePerPair = nullptr;
	Fl_Check_Button *buttonGrowPopulation = nullptr;
	Fl_Choice *choiceLinkApplication = nullptr;
	Fl_Choice *choiceLinkCountMethod = nullptr;
	DistributionView *viewPopulation = nullptr;
	DistributionView *viewLinkDensity = nullptr;
	Fl_Value_Input *inputLinkWeights[N_LINK_WEIGHT_FACTORS] = { nullptr, nullptr, nullptr, nullptr };
	Fl_Box* linkDensityViewLabel[3] = { nullptr, nullptr, nullptr };
	Fl_Group *groupGrowth = nullptr;
	Fl_Box *boxLinks = nullptr;
	Fl_Box *boxPopulation = nullptr;
	Fl_Group *groupWeights = nullptr;
	
	DistributionView *viewBelief = nullptr;		// inquirer tab
	Fl_Check_Button *buttonVaryBelief = nullptr;
	Fl_Box *boxBelief = nullptr;
	DistributionView *viewInquiryChance = nullptr;
	Fl_Check_Button *buttonVaryInquiryChance = nullptr;
	Fl_Box *boxInquiryChance = nullptr;
	DistributionView *viewInquiryAccuracy = nullptr;
	Fl_Check_Button *buttonVaryInquiryAccuracy = nullptr;
	Fl_Box *boxInquiryAccuracy = nullptr;
	MetaDistributionView *viewInquiryTrust = nullptr;
	Fl_Check_Button *buttonVaryInquiryTrust = nullptr;
	Fl_Box *boxInquiryTrust = nullptr;
	
	DistributionView *viewListenChance = nullptr;		// link tab
	Fl_Check_Button *buttonVaryListenChance = nullptr;
	Fl_Box *boxListenChance = nullptr;
	DistributionView *viewThreshold = nullptr;
	Fl_Check_Button *buttonVaryThreshold = nullptr;
	Fl_Box *boxThreshold = nullptr;
	MetaDistributionView *viewTrust = nullptr;
	Fl_Check_Button *buttonVaryTrust = nullptr;
	Fl_Box *boxTrust = nullptr;
	Fl_Round_Button* buttonEvidencePolicy[3] = { nullptr, nullptr, nullptr };
	Fl_Check_Button *buttonExcludePrior = nullptr;
	Fl_Group *boxEvidencePolicy = nullptr;
	Fl_Box *labelBox1 = nullptr, *labelBox2 = nullptr;
	
	Fl_Round_Button* btnInqIndividually = nullptr;				// e-value tab
	Fl_Round_Button* btnInqAverage = nullptr;
	Fl_Round_Button* btnInqMajority = nullptr;
	Fl_Button* btnAmtMethod = nullptr;
	Fl_Button* btnBlfPMethod = nullptr;
	Fl_Button* btnBlfNotPMethod = nullptr;
	Fl_Box* boxAmt = nullptr;
	Fl_Box* boxBlfP = nullptr;
	Fl_Box* boxBlfNotP = nullptr;
	Fl_Value_Input* inputMajorityAmount = nullptr;
	Fl_Value_Input* inputMajorityPCertainty = nullptr;
	Fl_Value_Input* inputMajorityNotPCertainty = nullptr;
	Fl_Value_Input* inputBeliefPValue = nullptr;
	Fl_Value_Input* inputBeliefNoneValue = nullptr;
	Fl_Value_Input* inputBeliefNotPValue = nullptr;
	Fl_Value_Input* inputExponent = nullptr;

	Fl_Check_Button* buttonRecordEValues = nullptr;				// record tab
	Fl_Value_Input* fieldSocietiesPerEValue = nullptr;
	Fl_Value_Input* fieldTimePerEValue = nullptr;
	Fl_Box* labelEValueSocieties = nullptr;
	Fl_Box* labelEValueTimeSteps = nullptr;
	Fl_Check_Button* buttonRecordTopologies = nullptr;
	Fl_Value_Input* fieldSocietiesPerTopology = nullptr;
	Fl_Box* labelTopologySocieties = nullptr;
	
	Fl_Spinner* inputNumStages = nullptr;								// stages
	Fl_Button* btnStage[4] = { nullptr, nullptr, nullptr, nullptr };
	Fl_Button* btnCopyStageToNext[3] = { nullptr, nullptr, nullptr };
	Fl_Button* btnCopyStageToPrev[3] = { nullptr, nullptr, nullptr };
	
	BatchSimulation* bsEdited = nullptr;
	BatchSimulation bs;
	MultiBatch* mb = nullptr;
	t_int dialogForm = -1;
	
	bool maxValid[MAX_BATCH_STAGES] = { false, false, false, false };	// should populationdistribution.max be displayed?
};


#endif