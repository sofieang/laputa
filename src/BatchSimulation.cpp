#include "BatchSimulation.h"
#include "Utility.h"
#include "FL/fl_ask.H"
#include "UserInterfaceItems.h"

//-----------------------------------------------------------------------------------------------------------------------

BatchSimulation::BatchSimulation() {
	nTrials = 1000;
	for(t_int i = 0; i < MAX_BATCH_STAGES; ++i) nSteps[i] = 15;
	nStages = 1;
	curTrial = 0;
	curStage = 0;
    
	templateSociety = 0;
	displayResults = false;
	stats.recordEValueStats = stats.recordTopologies = false;
	stats.timePerEValueStat = stats.societiesPerEValueStat = stats.societiesPerTopology = 1;
}

//-----------------------------------------------------------------------------------------------------------------------

BatchSimulation::~BatchSimulation() {
	if (templateSociety) delete templateSociety;
	templateSociety = 0;
}

//-----------------------------------------------------------------------------------------------------------------------

BatchSimulation& BatchSimulation::operator=(const BatchSimulation& bs) {
	// check against self-assignment
	if (this == &bs) return *this;

	// release template society if necessary
	nTrials = bs.nTrials;
	for (t_int i = 0; i < MAX_BATCH_STAGES; ++i) nSteps[i] = bs.nSteps[i];
	nStages = bs.nStages;
	curTrial = bs.curTrial;
	curStage = bs.curStage;
	for (t_int i = 0; i < MAX_BATCH_STAGES; ++i) setup[i] = bs.setup[i];
	if (templateSociety) delete templateSociety;
	if(bs.templateSociety) templateSociety = new Society(*bs.templateSociety);
	else templateSociety = 0;
	sim = bs.sim;
	stats = bs.stats;
	displayResults = bs.displayResults;
	timeOut = bs.timeOut;
	return *this;
}

//-----------------------------------------------------------------------------------------------------------------------

void BatchSimulation::setDefault(MultiBatch* mb) {
    nTrials = 1000;
    if(templateSociety) delete templateSociety;
    templateSociety = 0;
	if(mb) if(mb->templateSociety) templateSociety = new Society(*mb->templateSociety);
    if(mb) displayResults = false;
    else displayResults = true;
    
	if(mb) sim.soc = curSociety;
	stats.eValuesOverTime.free();
    
	for(t_int i = 0; i < MAX_BATCH_STAGES; ++i) {
        nSteps[i] = 15;
		setup[i].setDefault();
		
		// turn off variation
		setup[i].inqParams.varyStartBelief = setup[i].inqParams.varyInquiryChance = setup[i].inqParams.varyInquiryAccuracy =
            setup[i].inqParams.varyInquiryTrust = setup[i].linkParams.varyListenChance = setup[i].linkParams.varyThreshold =
            setup[i].linkParams.varyTrust = KEEP_CONSTANT;
	}
	if(mb) stats.recordEValueStats = stats.recordTopologies = false;
    else stats.recordEValueStats = stats.recordTopologies = true;
	stats.timePerEValueStat = stats.societiesPerEValueStat = stats.societiesPerTopology = 1;

}

//-----------------------------------------------------------------------------------------------------------------------

BatchSimulation::BatchSimulation(const BatchSimulation& bs1, const BatchSimulation& bs2, t_float v) {
	templateSociety = 0;
	*this = bs1;
	nStages = Round(bs1.nStages * (1.0 - v) + (t_float)bs2.nStages * v);
	for(t_int i = 0; i < nStages; ++i) nSteps[i] = Round(bs1.nSteps[i] * (1.0 - v) + (t_float)bs2.nSteps[i] * v);
	nTrials = Round(bs1.nTrials * (1.0 - v) + (t_float)bs2.nTrials * v);
	sim = Simulation(bs1.sim, bs2.sim, v);
	for(t_int i = 0; i < nStages; ++i) setup[i] = SocietySetup(bs1.setup[i], bs2.setup[i], v);
	curStage = 0;
	stats.recordEValueStats = stats.recordTopologies = false;
	stats.timePerEValueStat = stats.societiesPerEValueStat = stats.societiesPerTopology = 1;
	assert(bs1.sim.soc == bs2.sim.soc);
}

//-----------------------------------------------------------------------------------------------------------------------

BatchSimulation::BatchSimulation(TiXmlElement* xml) {
	curTrial = 0;
	templateSociety = 0;
    
	// read fields
	xml->QueryIntAttribute("TRIALS", &nTrials);
	xml->QueryIntAttribute("STAGES", &nStages);
	xml->QueryIntAttribute("STEPS_0", &nSteps[0]);
	if(nStages > 1) xml->QueryIntAttribute("STEPS_1", &nSteps[1]);
	if(nStages > 2) xml->QueryIntAttribute("STEPS_2", &nSteps[2]);
	if(nStages > 3) xml->QueryIntAttribute("STEPS_3", &nSteps[3]);
    
	setup[0] = SocietySetup(xml->FirstChildElement("SOCIETY_SETUP_0"));
	if(nStages > 1) setup[1] = SocietySetup(xml->FirstChildElement("SOCIETY_SETUP_1"));
	if(nStages > 2) setup[2] = SocietySetup(xml->FirstChildElement("SOCIETY_SETUP_2"));
	if(nStages > 3) setup[3] = SocietySetup(xml->FirstChildElement("SOCIETY_SETUP_3"));
    
	sim = Simulation(xml->FirstChildElement("SIMULATION"));
	  
	curStage = 0;
	displayResults = false;
	stats.recordEValueStats = stats.recordTopologies = true;
	stats.timePerEValueStat = stats.societiesPerEValueStat = stats.societiesPerTopology = 1;
}

//-----------------------------------------------------------------------------------------------------------------------

void BatchSimulation::setupTrials(void) {
	// initialise statistics
	stats.totalEValue = stats.totalEValueDelta = 0;
	stats.totalEValueS = stats.totalEValueDeltaS = 0;
	stats.totalPolarisation = stats.totalPolarisationDelta = 0;
	stats.totalPolarisationS = stats.totalPolarisationDeltaS = 0;
	stats.avgMessagesSentTotal = stats.avgMessagesSentPerInquirer = 0;
	stats.avgInquiryResultsTotal = stats.avgInquiryResultsPerInquirer = 0;
	stats.avgBWToPEffect = stats.avgBWToNotPEffect = stats.avgBWToPProb = stats.avgBWToNotPProb = 0;
    
	// clean degree lists
	for (t_int i = 0; i < 3; ++i) stats.degrees[i].clear();
	
	// make block for eValue results
	if (displayResults && stats.recordEValueStats) {
		stats.eValuesOverTime.allocate(maxInquirers(), (totalSteps() + 1) / stats.timePerEValueStat, nTrials / stats.societiesPerEValueStat);
		if (!stats.eValuesOverTime.valid()) {
			stats.eValuesOverTime.free();
			fl_alert("Insufficient memory to store detailed results of batch simulation. Only rudimentary statistics will be available");
		}
		else stats.eValuesOverTime.clear();
	}
	
	// clear topology results
	stats.topologies.clear();
	if (displayResults && stats.recordTopologies) {
		stats.topologies.reserve(nTrials / stats.societiesPerTopology);
	}

	// do precalculations in setup
	for(t_int i = 0; i < MAX_BATCH_STAGES; ++i) setup[i].precalculate();
	
	// update progress bar
	if(displayResults) progressWindow->barProgress->value(0);
    
}

//-----------------------------------------------------------------------------------------------------------------------

t_int BatchSimulation::maxInquirers(void) {
	int maxInquirers = 0;
	if(!setup[0].varyPopulation) maxInquirers = templateSociety->people.size();
	for(t_int i = 0; i < nStages; ++i) if(setup[i].varyPopulation && setup[i].populationDistribution.max > maxInquirers) maxInquirers = setup[i].populationDistribution.max;
	return maxInquirers;
}

//-----------------------------------------------------------------------------------------------------------------------

void BatchSimulation::recordTrialEndStatistics(void) {
	// means & their squared values
	stats.totalEValue += sim.eValue;
	stats.totalEValueDelta += sim.eValueDelta;
	stats.totalEValueS += sim.eValue * sim.eValue;
	stats.totalEValueDeltaS += sim.eValueDelta * sim.eValueDelta;
	
	// polarisation measures & their squared values
	t_float p = sim.instantPolarisation(sim.eValue);
	stats.totalPolarisation += p;
	stats.totalPolarisationDelta += p - sim.startPolarisation;
	stats.totalPolarisationS += p * p;
	stats.totalPolarisationDeltaS += (p - sim.startPolarisation) * (p - sim.startPolarisation);
	
	// message and inquiry statistics
	stats.avgMessagesSentTotal += sim.msgSent;
	stats.avgMessagesSentPerInquirer += (t_float)sim.msgSent / (t_float)sim.soc->people.size();
	stats.avgInquiryResultsTotal += sim.inqResults;
	stats.avgInquiryResultsPerInquirer += (t_float)sim.inqResults / (t_float)sim.soc->people.size();
	
	// bandwagon measurement
	if(sim.inqResults > 0) {
		stats.avgBWToPProb += (t_float)sim.inqOverriddenTowardsP / (t_float)(sim.soc->people.size() * totalSteps());
		stats.avgBWToPEffect += sim.bwTowardsP / (t_float)(t_float)(sim.soc->people.size() * totalSteps());
		stats.avgBWToNotPProb += (t_float)sim.inqOverriddenTowardsNotP / (t_float)(t_float)(sim.soc->people.size() * totalSteps());
		stats.avgBWToNotPEffect += sim.bwTowardsNotP / (t_float)(t_float)(sim.soc->people.size() * totalSteps());
	}
	
	
	// record degrees
	if(displayResults) {
		for(t_int i = 0; i < 3; ++i) {
			sim.soc->getDegrees(i, sim.soc->degrees[i]);
			if (sim.soc->degrees[i].size() > stats.degrees[i].size()) {
				t_int oldsz = stats.degrees[i].size();
				stats.degrees[i].resize(sim.soc->degrees[i].size());
				for (t_int j = oldsz; j < stats.degrees[i].size(); ++j) stats.degrees[i][j] = 0;
			}
			for (t_int j = 0; j < sim.soc->degrees[i].size(); ++j) stats.degrees[i][j] += sim.soc->degrees[i][j];
		}
	}
}


//-----------------------------------------------------------------------------------------------------------------------

void BatchSimulation::recordFinalStatistics(void) {
	// calculate means & standard deviations
	stats.totalEValue /= (t_float)nTrials;
	stats.totalEValueDelta /= (t_float)nTrials;
	stats.totalEValueS = sqrt(stats.totalEValueS / (t_float)nTrials - stats.totalEValue * stats.totalEValue);
	stats.totalEValueDeltaS = sqrt(stats.totalEValueDeltaS / (t_float)nTrials - stats.totalEValueDelta * stats.totalEValueDelta);
	stats.totalPolarisation /= nTrials;
	stats.totalPolarisationDelta /= nTrials;
	stats.totalPolarisationS = sqrt(stats.totalPolarisationS / nTrials - stats.totalPolarisation * stats.totalPolarisation);
	stats.totalPolarisationDeltaS = sqrt(stats.totalPolarisationDeltaS / nTrials - stats.totalPolarisationDelta * stats.totalPolarisationDelta);
	
	stats.avgMessagesSentTotal /= (t_float)nTrials;
	stats.avgMessagesSentPerInquirer /= (t_float)nTrials;
	stats.avgInquiryResultsTotal /= (t_float)nTrials;
	stats.avgInquiryResultsPerInquirer /= (t_float)nTrials;
	stats.avgBWToPProb /= (t_float)nTrials;
	stats.avgBWToPEffect /= (t_float)nTrials;
	stats.avgBWToNotPProb /= (t_float)nTrials;
	stats.avgBWToNotPEffect /= (t_float)nTrials;
	
	// average degrees
	for (t_int i = 0; i < 3; ++i) for (t_int j = 0; j < stats.degrees[i].size(); ++j) stats.degrees[i][j] /= (t_float)nTrials;
	
}

//-----------------------------------------------------------------------------------------------------------------------

void BatchSimulation::process(void) {
	// loop until enough time has passed
	for(t_int stepsTaken = 0; (stepsTaken < STEPS_PER_SIMULATION_STEP) && (curTrial < nTrials) && !timeOut;) {
		// is this the first step of the trial?
		if(sim.curStep == 0) {
			// is this the first trial?
			if(curTrial == 0) setupTrials();
			
			// create a new society
			delete curSociety;
			curSociety = new Society(&setup[0], templateSociety);
            
			// set up simulation
			sim.reset();
			if (stats.eValuesOverTime.valid() && stats.recordEValueStats) {
				sim.eValuesOverTime = stats.eValuesOverTime;
				sim.eValuesOverTime.zOffset = curTrial / stats.societiesPerEValueStat;
				for(t_int i = 0; i < sim.soc->people.size(); ++i) sim.eValuesOverTime.v(i, 0, 0) = sim.individualEValue(sim.soc->people[i].belief.v());
			}

			// record topology
			if (stats.recordTopologies && (curTrial % stats.societiesPerTopology == 0)) stats.topologies.push_back(NetworkTopology(curSociety));
		}
		
		// run a number of steps
		t_int nStepsToTake = STEPS_PER_SIMULATION_STEP, endStep = nSteps[0];
		for(t_int i = 1; i <= curStage; ++i) endStep += nSteps[i];
		if(nStepsToTake + sim.curStep > endStep) nStepsToTake = endStep - sim.curStep;
		sim.step(nStepsToTake);
		stepsTaken += nStepsToTake;
		
		if(sim.curStep == endStep) {
			// increment stage
			if(++curStage == nStages) {
				recordTrialEndStatistics();
				
				// increment trial
				if(++curTrial == nTrials) {
					recordFinalStatistics();
					*sim.soc = *templateSociety;
                    
					if(displayResults) {
						// close progress dialog, restore society, and show statistics
						if(Fl::has_idle(BatchProcess, this)) Fl::remove_idle(BatchProcess, this);
						progressWindow->closeDialog(true);
						societyWindow->showDialog(DIALOG_STATISTICS, this);
					}
					else sim.soc->organise();
					return;
				}
				curStage = sim.curStep = 0;
			}
			else {
				// use previous society as template to make a new one
				*sim.soc = Society(&setup[curStage], sim.soc);

				// record new evalues, in case they have changed
				if (stats.eValuesOverTime.valid() && stats.recordEValueStats) {
					if ((curTrial % stats.societiesPerEValueStat == 0) && (sim.curStep % stats.timePerEValueStat == 0)) {
						for (t_int i = 0; i < sim.soc->people.size(); ++i) sim.eValuesOverTime.v(i, sim.curStep / stats.timePerEValueStat, 0) = sim.individualEValue(sim.soc->people[i].belief.v());
					}
				}
			}
		}
	}
	
	// update progress bar
	if(displayResults) progressWindow->barProgress->value((t_float)curTrial * 100.0 / (t_float)nTrials);
	else doubleProgressWindow->barProgress->value((t_float)curTrial * 100.0 / (t_float)nTrials);
	sim.soc->organise();
    societyWindow->view->redraw();
}

//-----------------------------------------------------------------------------------------------------------------------

TiXmlElement* BatchSimulation::toXML(const char *name) {
	TiXmlElement *xml;
	if(name) xml = new TiXmlElement(name);
	else xml = new TiXmlElement("BATCH_SIMULATION");
	xml->SetAttribute("TRIALS", nTrials);
	xml->SetAttribute("STAGES", nStages);
	xml->SetAttribute("STEPS_0", nSteps[0]);
	if(nStages > 0) xml->SetAttribute("STEPS_1", nSteps[1]);
	if(nStages > 1) xml->SetAttribute("STEPS_2", nSteps[2]);
	if(nStages > 2) xml->SetAttribute("STEPS_3", nSteps[3]);
	
	for(t_int i = 0; i < nStages; ++i) xml->LinkEndChild(setup[i].toXML(i));
	xml->LinkEndChild(sim.toXML());
	
	return xml;
}

//-----------------------------------------------------------------------------------------------------------------------

string BatchSimulation::getDescription(void) {
	string str("GENERAL PARAMETERS\r\n");
	str += string("Trials: ") + string(IntToString(nTrials)) + string("\r\n");
	str += string("Stages: ") + string(IntToString(nStages)) + string("\r\n");
	str += string("SIMULATION VARIABLES\r\n");
	str += sim.getDescription() + string("\r\n");
	for(t_int i = 0; i < nStages; ++i) {
		str += string("--------------------------\r\n");
		str += string("STAGE ") + string(IntToString(i + 1)) + string("\r\n");
		str += string("Steps: ") + string(IntToString(nSteps[i])) + string("\r\n");
		str += setup[i].getDescription() + string("\r\n");
	}
    
	return str;
}

//-----------------------------------------------------------------------------------------------------------------------

void BatchSimulationTimeout(void* data) {
	BatchSimulation* bs = (BatchSimulation*)data;
	bs->timeOut = true;
}

//-----------------------------------------------------------------------------------------------------------------------

void BatchProcess(void* data) {
	// take a step
	BatchSimulation* bs = (BatchSimulation*)data;
	Fl::add_timeout (BATCH_SIMULATION_TIMEOUT, BatchSimulationTimeout, bs);
	bs->timeOut = false;
 	bs->process();
}
//-----------------------------------------------------------------------------------------------------------------------

void StartBatchSimulation(BatchSimulation* bs) {
	bs->displayResults = true;
	societyWindow->showDialog(DIALOG_PROGRESS, bs);
	if (bs->templateSociety) delete bs->templateSociety;
	bs->templateSociety = new Society(*curSociety);
	bs->curTrial = bs->curStage = bs->sim.curStep = 0;
	Fl::add_idle(BatchProcess, bs);
}

//-----------------------------------------------------------------------------------------------------------------------
