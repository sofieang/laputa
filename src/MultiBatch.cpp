#include "MultiBatch.h"
#include <FL/Fl.H>
#include "Utility.h"
#include "UserInterfaceItems.h"
#include <FL/filename.H>

//-----------------------------------------------------------------------------------------------------------------------

MultiBatch::MultiBatch(void) {
	for(t_int i = 0; i < 4; ++i) batches[i].displayResults = false;
	templateSociety = 0;
    values = 0;
    titles = 0;
}

//-----------------------------------------------------------------------------------------------------------------------

MultiBatch::MultiBatch(TiXmlElement* xml) {
	// read fields
	xml->QueryIntAttribute("STEPS_A_TO_B", &stepsAtoB);
	xml->QueryIntAttribute("STEPS_A_TO_C", &stepsAtoC);
	batches[BATCH_A] = BatchSimulation(xml->FirstChildElement("BATCH_A"));
	batches[BATCH_B] = BatchSimulation(xml->FirstChildElement("BATCH_B"));
	batches[BATCH_C] = BatchSimulation(xml->FirstChildElement("BATCH_C"));
	batches[BATCH_D] = BatchSimulation(xml->FirstChildElement("BATCH_D"));
    templateSociety = 0;
	values = 0;
	titles = 0;
 }

//-----------------------------------------------------------------------------------------------------------------------

void MultiBatch::generateBatch(t_float x, t_float y) {
	BatchSimulation bs1(batches[BATCH_A], batches[BATCH_B], x);
	if(stepsAtoC > 1) {
		BatchSimulation bs2(batches[BATCH_C], batches[BATCH_D], x);
		curBatch = BatchSimulation(bs1, bs2, y);
	}
	else curBatch = bs1;

	curBatch.templateSociety = new Society(*templateSociety);
	curBatch.curTrial = curBatch.sim.curStep = curBatch.curStage = 0;
}

//-----------------------------------------------------------------------------------------------------------------------

void MultiBatch::setDefault(void) {
	stepsAtoB = stepsAtoC = 10;
	if (templateSociety) delete templateSociety;
	templateSociety = new Society(*curSociety);
	for(t_int i = 0; i < 4; ++i) batches[i].setDefault(this);
	curBatch.setDefault(this);
}

//-----------------------------------------------------------------------------------------------------------------------

TiXmlElement* MultiBatch::toXML(const char *name) {
	TiXmlElement *xml;
	if(name) xml = new TiXmlElement(name);
	else xml = new TiXmlElement("MULTIBATCH");

	xml->SetAttribute("STEPS_A_TO_B", stepsAtoB);
	xml->SetAttribute("STEPS_A_TO_C", stepsAtoC);
	xml->LinkEndChild(batches[BATCH_A].toXML("BATCH_A"));
	xml->LinkEndChild(batches[BATCH_B].toXML("BATCH_B"));
	xml->LinkEndChild(batches[BATCH_C].toXML("BATCH_C"));
	xml->LinkEndChild(batches[BATCH_D].toXML("BATCH_D"));

	return xml;
}

//-----------------------------------------------------------------------------------------------------------------------

void MultiBatch::recordBatchStatistics(void) {
	values[stepsAtoC * stepsAtoB * 0 + stepsAtoB * yStep + xStep] = curBatch.stats.totalEValue;
	values[stepsAtoC * stepsAtoB * 1 + stepsAtoB * yStep + xStep] = curBatch.stats.totalEValueDelta;
	values[stepsAtoC * stepsAtoB * 2 + stepsAtoB * yStep + xStep] = curBatch.stats.totalPolarisation;
	values[stepsAtoC * stepsAtoB * 3 + stepsAtoB * yStep + xStep] = curBatch.stats.totalPolarisationDelta;
	titles[xStep + yStep * stepsAtoB] = curBatch.getDescription();
}

//-----------------------------------------------------------------------------------------------------------------------

void MultiBatch::process(void) {
	// run current batch
	Fl::add_timeout(BATCH_SIMULATION_TIMEOUT, BatchSimulationTimeout, &curBatch);
	curBatch.timeOut = false;
 	curBatch.process();

	if(curBatch.curTrial == curBatch.nTrials && curBatch.sim.curStep == curBatch.totalSteps() ) {
		// finished batch; record statistics
		recordBatchStatistics();

		// step to next batch
		++xStep;
		if(xStep == stepsAtoB) {
			++yStep;
			if(yStep == stepsAtoC) {
				// finished; write out results file
				Fl::remove_idle(MultiBatchProcess, this);
				saveStatisticsToFile();
				doubleProgressWindow->closeDialog(true);
				return;
			}
			else xStep = 0;
		}

		// create a new batch
		generateBatch((t_float)xStep / (t_float)(stepsAtoB - 1), (t_float)yStep / (t_float)(stepsAtoC - 1));

		// fill out progress
		string str = string(IntToString(xStep + yStep * stepsAtoB + 1)) + string(" / ") + string(IntToString(stepsAtoB * stepsAtoC));
		doubleProgressWindow->outputSecondProgress->value(str.c_str());
	}
}

//-----------------------------------------------------------------------------------------------------------------------


void MultiBatch::saveStatisticsToFile(void) {
	XMLData* data = new XMLData[(stepsAtoB) * (stepsAtoC) * (N_MULTIBATCH_VALUES + 1)];
	string ssNames[N_MULTIBATCH_VALUES + 1] = {"E-value", "E-value delta", "Polarisation", "Polarisation delta", "Parameters"};


	// fill out values
	for(t_int k = 0; k < N_MULTIBATCH_VALUES; ++k) {
		for(t_int j = 0; j < stepsAtoC; ++j) for(t_int i = 0; i < stepsAtoB; ++i)
			data[k * stepsAtoC * stepsAtoB + j * stepsAtoB + i].setDouble(values[k * stepsAtoC * stepsAtoB + j * stepsAtoB + i]);
	}

	// fill out titles
	for(t_int j = 0; j < stepsAtoC; ++j) for(t_int i = 0; i < stepsAtoB; ++i) data[4 * stepsAtoC * stepsAtoB + j * stepsAtoB + i].setString(titles[j * stepsAtoB + i]);

	// write to file
	SaveDataAsSpreadsheet(data, stepsAtoB, stepsAtoC, N_MULTIBATCH_VALUES + 1, ssNames, filename);
	delete [] data;
}

//-----------------------------------------------------------------------------------------------------------------------

void MultiBatchProcess(void* data) {
	// take a step
	MultiBatch* mb = (MultiBatch*)data;
	mb->process();
}


//-----------------------------------------------------------------------------------------------------------------------

void StartMultiBatchSimulation(MultiBatch* mb) {
	// create first batch
	mb->xStep = mb->yStep = 0;
	for(t_int i = 0; i < 4; ++i) mb->batches[i].sim.soc = curSociety;
	mb->curBatch = mb->batches[BATCH_A];
	mb->values = new t_float[mb->stepsAtoB * mb->stepsAtoC * 4];
	mb->titles = new string[mb->stepsAtoB * mb->stepsAtoC];
	if (mb->templateSociety) delete mb->templateSociety;
	mb->templateSociety = new Society(*curSociety);

	// start simulating
	societyWindow->showDialog(DIALOG_DOUBLE_PROGRESS, mb);
	Fl::add_idle(MultiBatchProcess, mb);
}
