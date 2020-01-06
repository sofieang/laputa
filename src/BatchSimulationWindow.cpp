#include "BatchSimulationWindow.h"
#include <FL/Fl_Widget.H>
#include "App.h"
#include "Utility.h"

//-----------------------------------------------------------------------------------------------------------------------

void BatchSimulationWindow::activateRecursively(void) {
	Fl_Double_Window::activate();
	for(t_int i = 0; i < children(); ++i) {
		child(i)->activate();
		Fl_Group* g1 = child(i)->as_group();
		if(g1) {
			for(t_int j = 0; j < g1->children(); ++j) {
				g1->child(j)->activate();
				Fl_Group* g2 = g1->child(j)->as_group();
				if(g2) {
					for(t_int k = 0; k < g2->children(); ++k) {
						g2->child(k)->activate();
						Fl_Group* g3 = g2->child(k)->as_group();
						if(g3) for(t_int l = 0; l < g3->children(); ++l) g3->child(l)->activate();
					}
				}
			}
		}
	}
}

//-----------------------------------------------------------------------------------------------------------------------

void BatchSimulationWindow::configure(t_int form) {
	if(form == BS_FORM_AS_BEFORE) form = dialogForm;
	else dialogForm = form;

	// activate everything by default
	activateRecursively();

	// trials / coordinate selector
	inputNumTrials->value(bs.nTrials);
	if(form == BS_FORM_SIMULATION || form == BS_FORM_MULTIBATCH) {
		boxPosition->hide();
		selectorPosition->hide();
		buttonCancel->show();
		buttonLoad->show();
		buttonSave->show();

	}
	else {
		if(form == BS_FORM_PREVIEW) {
			selectorPosition->show();
			boxPosition->show();
			if(mb->stepsAtoC == 1) selectorPosition->setDimensions(1);
			else selectorPosition->setDimensions(2);
			buttonCancel->hide();
			buttonLoad->hide();
			buttonSave->hide();
		}
		else {
			boxPosition->hide();
			selectorPosition->hide();
			buttonCancel->show();
			buttonLoad->show();
			buttonSave->show();
		}
	}
	if(form == BS_FORM_GENERATE) inputNumTrials->hide();
	else inputNumTrials->show();

	// show/hide tabs
	if (form == BS_FORM_GENERATE) {
		groupTabs->remove(groupStage);
		groupTabs->remove(groupEValues);
		groupTabs->remove(groupRecord);
	}
	else {
		groupTabs->insert(*groupStage, 0);
		groupTabs->add(groupEValues);
		if (form == BS_FORM_SIMULATION) groupTabs->add(groupRecord);
		else groupTabs->remove(groupRecord);
	}

	// stage selector
	if(form == BS_FORM_GENERATE) {
		// hide stage selectors
		inputNumStages->hide();
		for(t_int i = 0; i < MAX_BATCH_STAGES; ++i) btnStage[i]->hide();
		for(t_int i = 1; i < MAX_BATCH_STAGES; ++i) {
			btnCopyStageToNext[i - 1]->hide();
			btnCopyStageToPrev[i - 1]->hide();
		}
		MoveAllWidgetsTo(batchSimulationWindow, 20 - 50, 10);
		batchSimulationWindow->size(590, batchSimulationWindow->h());
	}
	else {
		batchSimulationWindow->size(640, batchSimulationWindow->h());
		MoveAllWidgetsTo(batchSimulationWindow, 20, 10);
		inputNumStages->value(bs.nStages);
		inputNumStages->show();
		for(t_int i = 0; i < MAX_BATCH_STAGES; ++i) btnStage[i]->show();
		for(t_int i = 0; i < bs.nStages; ++i) btnStage[i]->activate();
		for(t_int i = bs.nStages; i < MAX_BATCH_STAGES; ++i) btnStage[i]->deactivate();
		for(t_int i = 1; i < MAX_BATCH_STAGES; ++i) {
			if(i < bs.nStages) {
				btnCopyStageToNext[i - 1]->activate();
				btnCopyStageToPrev[i - 1]->activate();
			}
			else {
				btnCopyStageToNext[i - 1]->deactivate();
				btnCopyStageToPrev[i - 1]->deactivate();
			}
			btnCopyStageToNext[i - 1]->show();
			btnCopyStageToPrev[i - 1]->show();
		}
		for(t_int i = 0; i < MAX_BATCH_STAGES; ++i) btnStage[i]->value(0);
		btnStage[bs.curStage]->value(1);
	}

	// configure the tabs
	if(form != BS_FORM_GENERATE) configureStage();
	else groupTabs->remove(groupStage);
	configureNetwork();
	configureInquirers();
	configureLinks();
	if(form != BS_FORM_GENERATE) configureEValues();
	if (form == BS_FORM_SIMULATION) configureRecord();
}

//-----------------------------------------------------------------------------------------------------------------------

void BatchSimulationWindow::configureStage(void) {
	inputStepsPerTrial->value(bs.nSteps[bs.curStage]);

}

//-----------------------------------------------------------------------------------------------------------------------

void BatchSimulationWindow::configureNetwork(void) {

	// turn on/off population variation controls
	if(bs.setup[bs.curStage].varyPopulation) {
		buttonVaryPopulation->value(1);
		viewPopulation->setDistribution(&bs.setup[bs.curStage].populationDistribution);
		sliderInitialPopulationPart->value(1.0 - bs.setup[bs.curStage].initialPopulationPart);
		if(bs.setup[bs.curStage].initialPopulationPart == 1.0) {
			sliderInitialPopulationPart->deactivate();
			sliderGrowthBalance->deactivate();
		}
		sliderGrowthBalance->value(bs.setup[bs.curStage].growthBalance);
		inputPopulationMin->value(bs.setup[bs.curStage].populationDistribution.min);
		if(maxValid[bs.curStage]) inputPopulationMax->value(bs.setup[bs.curStage].populationDistribution.max);
		buttonGrowPopulation->value(bs.setup[bs.curStage].initialPopulationPart != 1.0);
	}
	else {
		buttonVaryPopulation->value(0);
		inputPopulationMin->deactivate();
		inputPopulationMax->deactivate();
		viewPopulation->deactivate();
		sliderInitialPopulationPart->deactivate();
		sliderGrowthBalance->deactivate();
		buttonGrowPopulation->value(0);
		buttonGrowPopulation->deactivate();
		boxPopulation->deactivate();
		boxLinks->deactivate();
		groupGrowth->deactivate();
		groupWeights->deactivate();
	}

	// turn on/off link variation controls
	if(bs.setup[bs.curStage].varyPopulation || bs.setup[bs.curStage].varyLinks) {
		buttonVaryLinks->value(1);
		viewLinkDensity->setDistribution(&bs.setup[bs.curStage].linkDensityDistribution);
		buttonLimitLinksToOnePerPair->value(bs.setup[bs.curStage].limitLinksToOnePerPair);
		if(buttonGrowPopulation->value() == 0) sliderInitialPopulationPart->deactivate();

		// set weights & distribution method
		for(t_int i = 0; i < N_LINK_WEIGHT_FACTORS; ++i) inputLinkWeights[i]->value(bs.setup[bs.curStage].linkWeights[i]);
		if(bs.setup[bs.curStage].linkDistributionMethod & LDM_TO_NUMBER_BIT) {
			choiceLinkCountMethod->value(1);
			if((bs.setup[bs.curStage].linkDistributionMethod & LDM_MASK) == LDM_PER_INQUIRER_SQUARED) bs.setup[bs.curStage].linkDistributionMethod = LDM_PER_INQUIRER | LDM_TO_NUMBER_BIT;
			((Fl_Menu_Item*)(choiceLinkApplication->menu()))[2].deactivate();
		}
		else {
			choiceLinkCountMethod->value(0);
			((Fl_Menu_Item*)(choiceLinkApplication->menu()))[2].activate();
		}
		choiceLinkApplication->value(bs.setup[bs.curStage].linkDistributionMethod & LDM_MASK);

		// determine & set scale of link distribution view
		t_float nLinks = 1.0;
		t_int digits = 2;
		if(bs.setup[bs.curStage].linkDistributionMethod & LDM_TO_NUMBER_BIT) {
			if((bs.setup[bs.curStage].linkDistributionMethod & LDM_MASK) == LDM_PER_INQUIRER) nLinks *= bs.setup[bs.curStage].populationDistribution.max - 1;
			else nLinks *= bs.setup[bs.curStage].populationDistribution.max * (bs.setup[bs.curStage].populationDistribution.max - 1);
			digits = 0;
		}
		if(!bs.setup[bs.curStage].limitLinksToOnePerPair) nLinks *= 2.0;
		linkDensityViewLabel[1]->copy_label(DoubleToString(0.5 * nLinks, digits));
		linkDensityViewLabel[2]->copy_label(DoubleToString(nLinks, digits));
		bs.setup[bs.curStage].linkDensityDistribution.max = nLinks;
	}
	else {
		buttonVaryLinks->value(0);
		buttonLimitLinksToOnePerPair->deactivate();
		viewLinkDensity->deactivate();
		for(t_int i = 0; i < N_LINK_WEIGHT_FACTORS; ++i) inputLinkWeights[i]->deactivate();
		choiceLinkApplication->deactivate();
	}
}

//-----------------------------------------------------------------------------------------------------------------------

void BatchSimulationWindow::configureInquirers(void) {

	// turn on/off starting belief controls
	if(bs.setup[bs.curStage].inqParams.varyStartBelief || bs.setup[bs.curStage].varyPopulation) {
		buttonVaryBelief->value(1);
		viewBelief->setDistribution(&bs.setup[bs.curStage].inqParams.startBelief);
	}
	else {
		buttonVaryBelief->value(0);
		viewBelief->deactivate();
		boxBelief->deactivate();
	}

	// turn on/off inquiry chance controls
	if(bs.setup[bs.curStage].inqParams.varyInquiryChance || bs.setup[bs.curStage].varyPopulation) {
		buttonVaryInquiryChance->value(1);
		viewInquiryChance->setDistribution(&bs.setup[bs.curStage].inqParams.inquiryChance);
	}
	else {
		buttonVaryInquiryChance->value(0);
		viewInquiryChance->deactivate();
		boxInquiryChance->deactivate();
	}

	// turn on/off inquiry accuracy controls
	if(bs.setup[bs.curStage].inqParams.varyInquiryAccuracy || bs.setup[bs.curStage].varyPopulation) {
		buttonVaryInquiryAccuracy->value(1);
		viewInquiryAccuracy->setDistribution(&bs.setup[bs.curStage].inqParams.inquiryAccuracy);
		viewInquiryAccuracy->activate();
		boxInquiryAccuracy->activate();
	}
	else {
		buttonVaryInquiryAccuracy->value(0);
		viewInquiryAccuracy->deactivate();
		boxInquiryAccuracy->deactivate();
	}

	// turn on/off inquiry trust controls
	if(bs.setup[bs.curStage].inqParams.varyInquiryTrust || bs.setup[bs.curStage].varyPopulation) {
		buttonVaryInquiryTrust->value(1);
		viewInquiryTrust->setMetaDistribution(&bs.setup[bs.curStage].inqParams.inquiryTrust);
		viewInquiryTrust->activate();
		boxInquiryTrust->activate();
	}
	else {
		buttonVaryInquiryTrust->value(0);
		viewInquiryTrust->deactivate();
		boxInquiryTrust->deactivate();
	}

}

//-----------------------------------------------------------------------------------------------------------------------

void BatchSimulationWindow::configureLinks(void) {
	// turn on/off listen chance controls
	if(bs.setup[bs.curStage].varyLinks || bs.setup[bs.curStage].varyPopulation || bs.setup[bs.curStage].linkParams.varyListenChance) {
		buttonVaryListenChance->value(1);
		viewListenChance->setDistribution(&bs.setup[bs.curStage].linkParams.linkListenChance);
	}
	else {
		buttonVaryListenChance->value(0);
		viewListenChance->deactivate();
		boxListenChance->deactivate();
	}

	// turn on/off link threshold controls
	if(bs.setup[bs.curStage].varyLinks || bs.setup[bs.curStage].varyPopulation || bs.setup[bs.curStage].linkParams.varyThreshold) {
		buttonVaryThreshold->value(1);
		viewThreshold->setDistribution(&bs.setup[bs.curStage].linkParams.linkThreshold);
	}
	else {
		buttonVaryThreshold->value(0);
		viewThreshold->deactivate();
		boxThreshold->deactivate();
	}

	// turn on/off link trust controls
	if(bs.setup[bs.curStage].varyLinks || bs.setup[bs.curStage].varyPopulation || bs.setup[bs.curStage].linkParams.varyTrust) {
		buttonVaryTrust->value(1);
		viewTrust->setMetaDistribution(&bs.setup[bs.curStage].linkParams.linkTrust);
		viewTrust->activate();
		boxTrust->activate();
	}
	else {
		buttonVaryTrust->value(0);
		viewTrust->deactivate();
		boxTrust->deactivate();
	}

	// set evidence policy
	if(bs.setup[bs.curStage].varyLinks || bs.setup[bs.curStage].varyPopulation) {
		for(t_int i = 0; i < 3; ++i) {
			if(bs.setup[bs.curStage].evidencePolicy == i) buttonEvidencePolicy[i]->value(1);
			else buttonEvidencePolicy[i]->value(0);
		}
		buttonExcludePrior->value(bs.setup[bs.curStage].countPriorAsEvidence);
	}
	else {
		for(t_int i = 0; i < 3; ++i) buttonEvidencePolicy[i]->value(0);
		buttonExcludePrior->value(0);
		buttonExcludePrior->deactivate();
		boxEvidencePolicy->deactivate();
		labelBox1->deactivate();
		labelBox2->deactivate();
	}

}


//-----------------------------------------------------------------------------------------------------------------------


void BatchSimulationWindow::configureEValues(void) {
	// set up evalues
	btnInqIndividually->value(0);
	btnInqAverage->value(0);
	btnInqMajority->value(0);
	if(bs.sim.val.blfPStrictlyGreater) btnBlfPMethod->copy_label(">");
	else btnBlfPMethod->copy_label(WCharToString(0x2265));
	if(bs.sim.val.blfNotPStrictlyLess) btnBlfNotPMethod->copy_label("<");
	else btnBlfNotPMethod->copy_label(WCharToString(0x2264));
	if(bs.sim.val.amtStrictlyGreater) btnAmtMethod->copy_label(">");
	else btnAmtMethod->copy_label(WCharToString(0x2265));
	inputMajorityAmount->value(bs.sim.val.majorityAmt * 100.0);
	inputMajorityPCertainty->value(bs.sim.val.majorityPCert * 100.0);
	inputMajorityNotPCertainty->value(bs.sim.val.majorityNotPCert * 100.0);
	inputBeliefPValue->value(bs.sim.val.eValues[2]);
	inputBeliefNoneValue->value(bs.sim.val.eValues[1]);
	inputBeliefNotPValue->value(bs.sim.val.eValues[0]);
    inputExponent->value(bs.sim.val.exponent);
	if(bs.sim.val.applicationMethod == APPLY_TO_MAJORITY) {
		btnInqMajority->value(1);
		boxAmt->activate();
		boxBlfP->activate();
		boxBlfNotP->activate();
		inputMajorityAmount->activate();
		inputMajorityPCertainty->activate();
		inputMajorityNotPCertainty->activate();
		btnAmtMethod->activate();
		btnBlfPMethod->activate();
		btnBlfNotPMethod->activate();
	}
	else {
		if(bs.sim.val.applicationMethod == APPLY_INDIVIDUALLY) btnInqIndividually->value(1);
		else if(bs.sim.val.applicationMethod == APPLY_TO_AVERAGE) btnInqAverage->value(1);
		boxAmt->deactivate();
		boxBlfP->deactivate();
		boxBlfNotP->deactivate();
		inputMajorityAmount->deactivate();
		inputMajorityPCertainty->deactivate();
		inputMajorityNotPCertainty->deactivate();
		btnAmtMethod->deactivate();
		btnBlfPMethod->deactivate();
		btnBlfNotPMethod->deactivate();
	}
}

//-----------------------------------------------------------------------------------------------------------------------

void BatchSimulationWindow::configureRecord(void) {
	buttonRecordEValues->value(bs.stats.recordEValueStats);
	buttonRecordTopologies->value(bs.stats.recordEValueStats);
	fieldSocietiesPerEValue->value(bs.stats.societiesPerEValueStat);
	fieldTimePerEValue->value(bs.stats.timePerEValueStat);
	fieldSocietiesPerTopology->value(bs.stats.societiesPerTopology);

	// turn on or off stuff
	if (bs.stats.recordEValueStats){
		fieldSocietiesPerEValue->activate();
		labelEValueSocieties->activate();
		fieldTimePerEValue->activate();
		labelEValueTimeSteps->activate();
	}
	else {
		fieldSocietiesPerEValue->deactivate();
		labelEValueSocieties->deactivate();
		fieldTimePerEValue->deactivate();
		labelEValueTimeSteps->deactivate();
	}
	if (bs.stats.recordTopologies){
		fieldSocietiesPerTopology->activate();
		labelTopologySocieties->activate();
	}
	else {
		fieldSocietiesPerTopology->deactivate();
		labelTopologySocieties->deactivate();
	}

	// make sure record tab is included
	groupTabs->add(groupRecord);

}

//-----------------------------------------------------------------------------------------------------------------------

void BatchSimulationWindow::turnOffEditing(void) {

}

//-----------------------------------------------------------------------------------------------------------------------

void BatchSimulationWindow::setStage(t_int s) {
	bs.curStage = s;
	configure();
}

//-----------------------------------------------------------------------------------------------------------------------

void BatchSimulationWindow::copyBatch(t_int from, t_int to) {
	bs.nSteps[to] = bs.nSteps[from];
	bs.setup[to] = bs.setup[from];
}


//-----------------------------------------------------------------------------------------------------------------------

void BatchSimulationWindow::setNumStages(t_int s) {
	bs.nStages = s;
	if(bs.curStage >= s) setStage(s - 1);
	configure();
}

//-----------------------------------------------------------------------------------------------------------------------


void BatchSimulationWindow::saveBatchSimulation(void) {
	string filename = SaveFileDialog("Save Batch As", "*.batch", "Untitled.batch");
	if (filename != "") {
		// create xml file
		TiXmlDocument f;
		TiXmlDeclaration* decl = new TiXmlDeclaration( "1.0", "UTF-8", "true" );
		f.LinkEndChild(decl);
		TiXmlElement *root = new TiXmlElement("BATCH_SIMULATION_FILE");
		root->SetAttribute("VERSION", LAPUTA_VERSION);
		f.LinkEndChild(root);
		root->LinkEndChild(bs.toXML());

		// write out everything
		f.SaveFile(filename.c_str());
	}
}

//-----------------------------------------------------------------------------------------------------------------------

void BatchSimulationWindow::loadBatchSimulation(void) {
	string filename = OpenFileDialog("Open Batch", "*.batch");
	if (filename != "") {
		// open the file
		TiXmlDocument f(filename.c_str());
		bool loadSucceeded = f.LoadFile();
		if(!loadSucceeded) {
			fl_alert("Failed to load batch simulation file.");
			return;
		}

		TiXmlElement *root = f.RootElement();

		// check if distribution file
		if(strcmp("BATCH_SIMULATION_FILE", root->Value())) {
			fl_alert("This is not a valid batch simulation file.");
			return;
		}

		// check version
		t_int vers;
		root->QueryIntAttribute("VERSION", &vers);
		if(vers < MIN_LAPUTA_VERSION) {
			fl_alert("This batch simulation was created with a different version of Laputa.");
			return;
		}

		// read batch simulation
		bs = BatchSimulation(root->FirstChildElement("BATCH_SIMULATION"));

		// update batch simulation window
		batchSimulationWindow->configure();
	}
}

//-----------------------------------------------------------------------------------------------------------------------
