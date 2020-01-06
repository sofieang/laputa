#include "UserInterfaceItems.h"
#include <gsl/gsl_randist.h>
#include <gsl/gsl_cdf.h>
#include "tinyxml.h"
#include <gsl/gsl_math.h>
#include "Utility.h"
#include <FL/Fl.H>

#define INDEX_FILE "index.htm"

// globals for windows
SocietyWindow *societyWindow;
InquirerWindow *inquirerWindow;
CreateInquirerPrefsWindow *createInquirerPrefsWindow;
LinkWindow *linkWindow;
CreateLinkPrefsWindow *createLinkPrefsWindow;
DistributionWindow *distributionWindow;
MetaDistributionWindow *metaDistributionWindow;
SimulationWindow *simulationWindow;
BatchSimulationWindow *batchSimulationWindow;
ProgressWindow *progressWindow;
StatisticsWindow *statisticsWindow;
AboutWindow *aboutWindow;
InquirerParametersWindow *inquirerParametersWindow;
LinkParametersWindow *linkParametersWindow;
ManualWindow *manualWindow;
DistributionFreeformValuesWindow *distributionFreeformValuesWindow;
SetValuesWindow *setValuesWindow;
MultiBatchWindow *multiBatchWindow;
DoubleProgressWindow *doubleProgressWindow;
DegreeDistributionWindow *degreeDistributionWindow;
ExpressionWindow *expressionWindow;
ExportStatisticsWindow *exportStatisticsWindow;
ExportTopologyWindow *exportTopologyWindow;

const char *FollowHTMLLink(Fl_Widget *w, const char *uri);
const char *FollowHTMLLinkBlindly(Fl_Widget *w, const char *uri);

//-----------------------------------------------------------------------------------------------------------------------

void DialogInterface::showDialog(t_int dialog, const void *data) {
    app->df.showDialog(dialog, data);
    // mac os fuckery
#ifdef __APPLE__
    if(modal()) {
        hide();
    }
#endif
	app->df.callstack.push_back(pair<t_int, DialogInterface*>(dialog, this));
}

//-----------------------------------------------------------------------------------------------------------------------

void DialogInterface::closeDialog(bool ok, t_int msg) {
	t_int id = dialogId();
	DialogInterface* d = app->df.callstack.back().second;
	app->df.callstack.pop_back();
	app->df.closeDialog(id, ok);
	if(ok) d->answer(msg);
#ifdef __APPLE__
    d->show();
#endif
}

//-----------------------------------------------------------------------------------------------------------------------

void DialogInterface::answer(t_int answ) {
	// placeholder
}

//-----------------------------------------------------------------------------------------------------------------------

t_int DialogInterface::dialogId(void) {
	return app->df.callstack.back().first;
}

//-----------------------------------------------------------------------------------------------------------------------

t_int DialogInterface::handle(t_int evt) {
    t_int evt2 = Fl_Double_Window::handle(evt);
	if(evt2 == 0) {
        if(evt == FL_PUSH) {
            xDrag = Fl::event_x();
            yDrag = Fl::event_y();
			return 1;
        }
		else if (evt == FL_DRAG) {
			t_int dx = Fl::event_x() - xDrag, dy = Fl::event_y() - yDrag;
			position(x() + dx, y() + dy);
			return 1;
		}
		else if (evt == FL_SHORTCUT) {
			// catch escape key
			if (Fl::event_key() == FL_Escape) return 1;

		}
    }
    return evt2;
}

//-----------------------------------------------------------------------------------------------------------------------

void DialogFactory::showDialog(t_int d, const void *data) {
	switch(d) {
	case DIALOG_CREATE_INQUIRER_PREFS:
		// setup values in dialog
		app->workingSetup.inqParams = app->curSocietySetup.inqParams;
		createInquirerPrefsWindow->viewBelief->setDistribution(&app->workingSetup.inqParams.startBelief);
		createInquirerPrefsWindow->viewInquiryChance->setDistribution(&app->workingSetup.inqParams.inquiryChance);
		createInquirerPrefsWindow->viewInquiryAccuracy->setDistribution(&app->workingSetup.inqParams.inquiryAccuracy);
		createInquirerPrefsWindow->viewInquiryTrust->setMetaDistribution(&app->workingSetup.inqParams.inquiryTrust);
		createInquirerPrefsWindow->buttonIncludeInStatistics->value(app->workingSetup.includeInStatistics);
		createInquirerPrefsWindow->buttonUpdateInquiryTrust->value(app->workingSetup.updateInquiryTrust);
		createInquirerPrefsWindow->show();
		break;

	case DIALOG_CREATE_LINK_PREFS:
		// setup values in dialog
		app->workingSetup.linkParams = app->curSocietySetup.linkParams;
		createLinkPrefsWindow->viewListenChance->setDistribution(&app->workingSetup.linkParams.linkListenChance);
		createLinkPrefsWindow->viewThreshold->setDistribution(&app->workingSetup.linkParams.linkThreshold);
		createLinkPrefsWindow->viewTrust->setMetaDistribution(&app->workingSetup.linkParams.linkTrust);
		createLinkPrefsWindow->buttonUpdateTrust->value(app->workingSetup.updateTrust);
		createLinkPrefsWindow->buttonExcludePrior->value(app->workingSetup.countPriorAsEvidence);
		for(t_int i = 0; i < 3; ++i) {
			if(app->workingSetup.evidencePolicy == i) createLinkPrefsWindow->buttonEvidencePolicy[i]->value(1);
			else createLinkPrefsWindow->buttonEvidencePolicy[i]->value(0);
		}
		createLinkPrefsWindow->show();
		break;

	case DIALOG_DISTRIBUTION:
		distributionWindow->distributionViewEdited = (DistributionView*)data;
		distributionWindow->trustViewEdited = NULL;
		distributionWindow->configure(distributionWindow->distributionViewEdited->distributionShown);
		distributionWindow->show();
		break;

	case DIALOG_TRUSTFUNCTION:
		distributionWindow->distributionViewEdited = NULL;
		distributionWindow->trustViewEdited = (TrustView*)data;
		distributionWindow->configure(distributionWindow->trustViewEdited->getAverageTrustFunction());
		distributionWindow->show();

		break;

	case DIALOG_METADISTRIBUTION:
        metaDistributionWindow->metaDistributionViewEdited = (MetaDistributionView*)data;
		metaDistributionWindow->setMetaDistribution(((MetaDistributionView*)data)->getMetaDistribution());
		metaDistributionWindow->show();
		break;


	case DIALOG_SIMULATION:
		simulationWindow->templateSoc = new Society(*curSociety);
		app->curSimulation.soc = curSociety;
		app->curSimulation.reset();
		app->curSimulation.setSimulationWindowFrom();
		simulationWindow->show();
		break;

	case DIALOG_BATCH_SIMULATION:
	case DIALOG_BATCH_SIMULATION_MULTIBATCH:
	case DIALOG_BATCH_SIMULATION_PREVIEW:
	case DIALOG_BATCH_SIMULATION_GENERATE:
		batchSimulationWindow->bsEdited = (BatchSimulation*)data;
		batchSimulationWindow->bs = *batchSimulationWindow->bsEdited;
		batchSimulationWindow->bs.curStage = 0;
		if(curSociety->people.size() == 0) batchSimulationWindow->bs.setup[0].varyPopulation = true;
		if(d == DIALOG_BATCH_SIMULATION_MULTIBATCH || d == DIALOG_BATCH_SIMULATION_PREVIEW) batchSimulationWindow->mb = &multiBatchWindow->mb;
		else batchSimulationWindow->mb = NULL;
		if(d == DIALOG_BATCH_SIMULATION_PREVIEW) {
			batchSimulationWindow->selectorPosition->xVal->value(0.5);
			batchSimulationWindow->selectorPosition->yVal->value(0.5);
		}
		for(t_int i = 0; i < MAX_BATCH_STAGES; ++i) batchSimulationWindow->maxValid[i] = true;
		if(d == DIALOG_BATCH_SIMULATION) batchSimulationWindow->configure(BS_FORM_SIMULATION);
		else if(d == DIALOG_BATCH_SIMULATION_MULTIBATCH) batchSimulationWindow->configure(BS_FORM_MULTIBATCH);
		else if(d == DIALOG_BATCH_SIMULATION_PREVIEW) batchSimulationWindow->configure(BS_FORM_PREVIEW);
		else if(d == DIALOG_BATCH_SIMULATION_GENERATE) batchSimulationWindow->configure(BS_FORM_GENERATE);
		batchSimulationWindow->groupStage->show();
		batchSimulationWindow->show();
		break;

	case DIALOG_PROGRESS:
		societyWindow->view->deselectAll();
		progressWindow->bs = (BatchSimulation*)data;
		progressWindow->barProgress->value(0);
		progressWindow->show();
		break;

	case DIALOG_STATISTICS:
		statisticsWindow->bsShown = (BatchSimulation*)data;
		statisticsWindow->fillOutStatistics();
		statisticsWindow->show();
		break;

	case DIALOG_ABOUT:
		// show window
		aboutWindow->show();
		break;

	case DIALOG_INQUIRER_PARAMETERS:
		// setup values in dialog
		if(societyWindow->view->getSelectedInquirers().size()) {
			app->workingSetup.inqParams = *(curSociety->people[*(societyWindow->view->getSelectedInquirers().begin())].inqParams);
			app->workingSetup.setInquirerParametersWindowFrom();
			inquirerParametersWindow->show();
		}
		break;

	case DIALOG_LINK_PARAMETERS:
		if(societyWindow->view->getSelectedLinks().size()) {
			app->workingSetup.linkParams = *(curSociety->getLink(societyWindow->view->getSelectedLinks().begin()->first, societyWindow->view->getSelectedLinks().begin()->second)->linkParams);
			app->workingSetup.setLinkParametersWindowFrom();
			linkParametersWindow->show();
		}
		break;

	case DIALOG_MANUAL:
		manualWindow->configure((const char*)data);
		manualWindow->show();
		break;

	case DIALOG_DISTRIBUTION_FREEFORM_VALUES:
		distributionFreeformValuesWindow->listValues->clear();
		for(t_int i = 0; i < distributionWindow->view->curDistribution.dFf.values.size(); ++i) {
			char str[64];
			t_float v = ((t_float)i / (t_float)(distributionWindow->view->curDistribution.dFf.values.size() - 1)) * (distributionWindow->view->curDistribution.max -
				distributionWindow->view->curDistribution.min) + distributionWindow->view->curDistribution.min;
			sprintf(str, "[%.3f] \t %.4f", v, distributionWindow->view->curDistribution.dFf.values[i]);
			distributionFreeformValuesWindow->listValues->add(str);
		}

		distributionFreeformValuesWindow->fieldValue->deactivate();
		distributionFreeformValuesWindow->show();
		break;

	case DIALOG_SET_VALUES:
		setValuesWindow->sliderEdited = (Fl_Value_Slider*)data;
		setValuesWindow->inputValue->label(setValuesWindow->sliderEdited->label());
		setValuesWindow->inputValue->value(setValuesWindow->sliderEdited->value());
		Fl::focus(setValuesWindow->inputValue);
		setValuesWindow->show();
		break;

	case DIALOG_MULTIBATCH:
		multiBatchWindow->mbEdited = &app->curMultiBatch;
		multiBatchWindow->mb = *multiBatchWindow->mbEdited;
		if(curSociety->people.size() == 0) for(t_int i = 0; i < 4; ++i) multiBatchWindow->mb.batches[i].setup[0].varyPopulation = true;
		multiBatchWindow->configure();
		multiBatchWindow->show();
		break;

	case DIALOG_DOUBLE_PROGRESS:
		societyWindow->view->deselectAll();
		doubleProgressWindow->mb = (MultiBatch*)data;
		doubleProgressWindow->barProgress->value(0);
		{
			char str[48] = "1 / ";
			strcat(str, IntToString(doubleProgressWindow->mb->stepsAtoB * doubleProgressWindow->mb->stepsAtoC));
			doubleProgressWindow->outputSecondProgress->value(str);
		}
		doubleProgressWindow->show();
		break;

	case DIALOG_DEGREE_DISTRIBUTION:
        degreeDistributionWindow->configure((vector<t_float>*)data);
		degreeDistributionWindow->show();
		break;
	case DIALOG_EXPRESSION:
		expressionWindow->configure((Distribution*)data);
		expressionWindow->show();
		break;
	case DIALOG_EXPORT_STATISTICS:
		exportStatisticsWindow->configure((BatchSimulation*)data);
		exportStatisticsWindow->show();
		break;
	case DIALOG_EXPORT_TOPOLOGY:
		exportTopologyWindow->configure((BatchSimulation*)data);
		exportTopologyWindow->show();
		break;
	}
}

//-----------------------------------------------------------------------------------------------------------------------

void DialogFactory::closeDialog(t_int d, bool ok) {
	switch(d) {
	case DIALOG_CREATE_INQUIRER_PREFS:
		// set values
		if(ok) app->curSocietySetup.inqParams = app->workingSetup.inqParams;
		createInquirerPrefsWindow->hide();
		break;

	case DIALOG_CREATE_LINK_PREFS:
		if(ok) app->curSocietySetup.linkParams = app->workingSetup.linkParams;
		createLinkPrefsWindow->hide();
		break;

	case DIALOG_DISTRIBUTION:
		if(ok) {
			if(distributionWindow->distributionViewEdited) {
				distributionWindow->view->saveDistribution();
				distributionWindow->distributionViewEdited->redraw();
			}
		}

		distributionWindow->hide();
		break;

	case DIALOG_TRUSTFUNCTION:
		if(ok) {
			if(distributionWindow->trustViewEdited == inquirerWindow->viewTrust)
				inquirerWindow->setSelectedInquirerTrustFromDistribution(distributionWindow->view->curDistribution);
			else if(distributionWindow->trustViewEdited == linkWindow->viewTrust)
				linkWindow->setSelectedLinkTrustFromDistribution(distributionWindow->view->curDistribution);
			app->touchFile();
		}

		distributionWindow->hide();
		break;

	case DIALOG_METADISTRIBUTION:
		if(ok) {
			*(metaDistributionWindow->metaDistributionFieldEdited) = metaDistributionWindow->curMetaDistribution;
			metaDistributionWindow->metaDistributionViewEdited->setMetaDistribution(metaDistributionWindow->curMetaDistribution);
			metaDistributionWindow->metaDistributionViewEdited->redraw();
		}
		metaDistributionWindow->hide();
		break;

	case DIALOG_SIMULATION:
		if(Fl::has_idle(RunSimulation, app->getCurSimulation())) {
			Fl::remove_idle(RunSimulation, app->getCurSimulation());
			simulationWindow->buttonStep->activate();
			simulationWindow->buttonRun->activate();
			simulationWindow->buttonPause->deactivate();
			app->touchFile();
		}
		if(!ok) {
			// Restore society
			*curSociety = *(simulationWindow->templateSoc);
		}
		delete simulationWindow->templateSoc;
		simulationWindow->templateSoc = 0;
		simulationWindow->hide();
		break;

	case DIALOG_BATCH_SIMULATION:
	case DIALOG_BATCH_SIMULATION_MULTIBATCH:
	case DIALOG_BATCH_SIMULATION_PREVIEW:
	case DIALOG_BATCH_SIMULATION_GENERATE:
		batchSimulationWindow->hide();
		if(ok) {
			// save simulation & start it
			*(batchSimulationWindow->bsEdited) = batchSimulationWindow->bs;
			if(batchSimulationWindow->dialogForm == BS_FORM_SIMULATION) batchSimulationWindow->bsEdited->displayResults = true;
			else if(batchSimulationWindow->dialogForm == BS_FORM_GENERATE) {
				// generate new society
				Society soc(&batchSimulationWindow->bs.setup[0], curSociety);
				soc.organise();
				*curSociety = soc;
				societyWindow->view->redraw();

				// update number fields
				societyWindow->view->updateStatistics();
				app->touchFile();
			}
			if(batchSimulationWindow->dialogForm == BS_FORM_SIMULATION) StartBatchSimulation(batchSimulationWindow->bsEdited);
		}
		break;

	case DIALOG_PROGRESS:
		if(!ok) {
			Fl::remove_idle(BatchProcess, progressWindow->bs);
			// restore society
			*curSociety = *progressWindow->bs->templateSociety;
		}
		progressWindow->hide();

		break;


	case DIALOG_STATISTICS:
		statisticsWindow->hide();
		break;

	case DIALOG_ABOUT:
		aboutWindow->hide();
		break;

	case DIALOG_INQUIRER_PARAMETERS:
		if(ok) inquirerWindow->setSelectedInquirerParameters(app->workingSetup.inqParams);
		inquirerParametersWindow->hide();
		break;

	case DIALOG_LINK_PARAMETERS:
		if(ok) linkWindow->setSelectedLinkParameters(app->workingSetup.linkParams);
		linkParametersWindow->hide();
		break;

	case DIALOG_DISTRIBUTION_FREEFORM_VALUES:
		distributionFreeformValuesWindow->hide();
		break;

	case DIALOG_SET_VALUES:
		if(ok) {
			setValuesWindow->sliderEdited->value(setValuesWindow->inputValue->value());
			setValuesWindow->sliderEdited->do_callback();
		}
		setValuesWindow->hide();
		break;

	case DIALOG_MULTIBATCH:
		multiBatchWindow->hide();
		if(ok) {
			*multiBatchWindow->mbEdited = multiBatchWindow->mb;
      string filename = SaveFileDialog("Save Results As", "*.ods", "Untitled.ods");
			if (filename != "") {
				fl_filename_absolute(multiBatchWindow->mbEdited->filename, FL_PATH_MAX, filename.c_str());
				StartMultiBatchSimulation(multiBatchWindow->mbEdited);
			}
		}
		break;

	case DIALOG_DOUBLE_PROGRESS:
		if(!ok) {
			if(Fl::has_idle(MultiBatchProcess, doubleProgressWindow->mb)) Fl::remove_idle(MultiBatchProcess, doubleProgressWindow->mb);
			*curSociety = *doubleProgressWindow->mb->curBatch.templateSociety;
			if(doubleProgressWindow->mb) {
				delete [] doubleProgressWindow->mb->values;
				delete [] doubleProgressWindow->mb->titles;
			}
			doubleProgressWindow->mb = 0;
		}
		doubleProgressWindow->hide();
		if(ok) fl_alert("Metadistribution simulation complete. Results saved in file %s.", doubleProgressWindow->mb->filename);
		break;

	case DIALOG_DEGREE_DISTRIBUTION:
        degreeDistributionWindow->hide();
		break;


	case DIALOG_EXPRESSION:
		if(ok) expressionWindow->applyExpression();
		expressionWindow->hide();
		break;

	case DIALOG_EXPORT_STATISTICS:
		exportStatisticsWindow->hide();
		break;

	case DIALOG_EXPORT_TOPOLOGY:
		exportTopologyWindow->hide();
		break;
	}
}

//-----------------------------------------------------------------------------------------------------------------------

void SocietyWindow::adjustMenuShortcuts(void) {
	#ifdef _WINDOWS
	for(t_int i = 0; i < menuBar->size(); ++i) {
		if(menuBar->menu()[i].shortcut() & FL_META) menuBar->shortcut(i, (menuBar->menu()[i].shortcut() & ~FL_META) | FL_CTRL);
	}
	#endif
}

//-----------------------------------------------------------------------------------------------------------------------

void SocietyWindow::resize(int X, int Y, int W, int H) {
	Fl_Double_Window::resize(X, Y, W, H);
	view->resize(4, societyWindow->buttonCreateInquirer->y() + societyWindow->buttonCreateInquirer->h() + 4,
		W - (societyWindow->scrollbarVertical->w() + 8),
		H - (societyWindow->buttonCreateInquirer->y() + societyWindow->buttonCreateInquirer->h() + societyWindow->scrollbarHorizontal->h() + 8));
	scrollbarHorizontal->resize(view->x(), view->y() + view->h(), view->w(), scrollbarHorizontal->h());
	scrollbarVertical->resize(view->x() + view->w(), view->y(), scrollbarVertical->w(), view->h());

}

//-----------------------------------------------------------------------------------------------------------------------

t_int SimulationWindow::handle(t_int event) {
	static t_int dx, dy;
	static bool dragging = false;

	if(event == FL_PUSH) {
		dx = Fl::event_x();
		dy = Fl::event_y();
		// is event inside a child?
		for(t_int i = 0; i < children(); ++i) {
			if(Fl::event_inside(child(i))) {
				dragging = false;
				return Fl_Double_Window::handle(event);
			}
		}
		dragging = true;
		return 1;
	}
	else if(event == FL_DRAG && dragging) {
		position(x() + Fl::event_x() - dx, y() + Fl::event_y() - dy);
		redraw();
		return 1;
	}
	else return Fl_Double_Window::handle(event);
}

//-----------------------------------------------------------------------------------------------------------------------

void SimulationWindow::loadParameters(void) {
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
			fl_alert("This batch simulation was created with a too old version of Laputa.");
			return;
		}

		// read batch simulation's simulation field
		BatchSimulation bs(BatchSimulation(root->FirstChildElement("BATCH_SIMULATION")));
		app->getCurSimulation()->val = bs.sim.val;
		app->getCurSimulation()->reset();
	}
}


//-----------------------------------------------------------------------------------------------------------------------

void DistributionWindow::configure(Distribution *d) {

	// set sliders & controls
	sliderPtValue->range(d->min, d->max);
	sliderPtValue->value(d->dPt.value * (d->max - d->min) + d->min);
	sliderIntLower->range(d->min, d->max);
	sliderIntLower->value(d->dInt.lower * (d->max - d->min) + d->min);
	sliderIntUpper->range(d->min, d->max);
	sliderIntUpper->value(d->dInt.upper * (d->max - d->min) + d->min);
	sliderNrmMidpt->range(d->min, d->max);
	sliderNrmMidpt->value(d->dNrm.midpt * (d->max - d->min) + d->min);
	sliderNrmStddev->range(0.001, (d->max - d->min) * 2.0);
	sliderNrmStddev->value(d->dNrm.stddev * (d->max - d->min) + d->min);
	sliderBtAlpha->value(d->dBt.alpha);
	sliderBtBeta->value(d->dBt.beta);
	for(t_int i = 0; i < 5; ++i) boxLabel[i]->copy_label(DoubleToString((d->max - d->min) * i / 4.0 + d->min, 2));
	for(t_int i = 0; i < N_DISTRIBUTION_TYPES; ++i) sliderWeight[i]->value(d->weights[i]);

	// if discrete, user cannot choose resolution
	if(d->discreteParts) {
		inputResolution->value(d->dFf.values.size());
		inputResolution->deactivate();
	}
	else {
		inputResolution->value(d->dFf.values.size() - 1);
		inputResolution->activate();
	}

	// set view
	view->filter = DISTR_ALL;
	for(t_int i = 0; i < N_DISTRIBUTION_TYPES; ++i) if(d->weights[i] == 1.0) view->filter = i;
	if(view->filter == DISTR_ALL) choiceFilter->value(0);
	else choiceFilter->value(view->filter + 1);
	view->setDistribution(d);

}
//-----------------------------------------------------------------------------------------------------------------------

void DistributionWindow::configure(TrustFunction* f) {

	// make a distribution from trust function
	Distribution d(f);

	// set sliders & controls
	sliderPtValue->value(d.dPt.value);
	sliderIntLower->value(d.dInt.lower);
	sliderIntUpper->value(d.dInt.upper);
	sliderNrmMidpt->value(d.dNrm.midpt);
	sliderNrmStddev->value(d.dNrm.stddev);
	sliderBtAlpha->value(d.dBt.alpha);
	sliderBtBeta->value(d.dBt.beta);
	for(t_int i = 0; i < N_DISTRIBUTION_TYPES; ++i) sliderWeight[i]->value(d.weights[i]);
	inputResolution->value(d.dFf.values.size() - 1);
	inputResolution->deactivate();

	// set view
	choiceFilter->value(5);
	view->filter = DISTR_FREEFORM;
	view->setDistribution(d);
}

//-----------------------------------------------------------------------------------------------------------------------

void DistributionWindow::loadDistribution(void) {
  string filename = OpenFileDialog("Open Distribution", "*.distr");
	if (filename != "") {
		// open the file
		TiXmlDocument f(filename.c_str());
		bool loadSucceeded = f.LoadFile();
		if(!loadSucceeded) {
			fl_alert("Failed to load distribution file.");
			return;
		}
		TiXmlElement *root = f.RootElement();

		// check if distribution file
		if(strcmp("DISTRIBUTION_FILE", root->Value())) {
			fl_alert("This is not a valid distribution file.");
			return;
		}


		// check version
		t_int vers;
		root->QueryIntAttribute("VERSION", &vers);
		if(vers < MIN_LAPUTA_VERSION) {
			fl_alert("This distribution was created with a too old version of Laputa.");
			return;
		}


		// read distribution
		view->curDistribution = Distribution(root->FirstChildElement("DISTRIBUTION"));

		// update distribution window
		configure(&view->curDistribution);
	}
}

//-----------------------------------------------------------------------------------------------------------------------

void DistributionWindow::saveDistribution(void) {
  string filename = SaveFileDialog("Save Distribution As", "*.distr", "Untitled.distr");
  if (filename != "") {
		// create xml file
		TiXmlDocument f;
		TiXmlDeclaration* decl = new TiXmlDeclaration( "1.0", "UTF-8", "true" );
		f.LinkEndChild(decl);
		TiXmlElement *root = new TiXmlElement("DISTRIBUTION_FILE");
		root->SetAttribute("VERSION", LAPUTA_VERSION);
		f.LinkEndChild(root);
		root->LinkEndChild(view->curDistribution.toXML());

		// write out everything
		f.SaveFile(filename.c_str());
	}
}

//-----------------------------------------------------------------------------------------------------------------------

void DistributionWindow::setBetaDistributionInterface(t_int i) {
	if(i == 0) {
		// alpha/beta t_interface
		buttonAlphaBeta->value(1);
		buttonMeanDev->value(0);
		sliderBtAlpha->copy_label("Alpha");
		sliderBtAlpha->range(0.001, 10.0);
		sliderBtAlpha->value(view->curDistribution.dBt.alpha);
		sliderBtBeta->copy_label("Beta");
		sliderBtBeta->range(0.001, 10.0);
		sliderBtBeta->value(view->curDistribution.dBt.beta);

	}
	else {
		// mean/stddev t_interface
		buttonAlphaBeta->value(0);
		buttonMeanDev->value(1);
		sliderBtAlpha->copy_label("Mean");
		sliderBtAlpha->range(view->curDistribution.min + 0.001, view->curDistribution.max - 0.001);
		sliderBtAlpha->value(view->curDistribution.dBt.getMean() * (view->curDistribution.max - view->curDistribution.min) + view->curDistribution.min);
		sliderBtBeta->copy_label("StdDev");
		sliderBtBeta->range(0.001, (view->curDistribution.dBt.getMaxDev() - 0.001));
		sliderBtBeta->value(view->curDistribution.dBt.getStddev());
	}
}

//-----------------------------------------------------------------------------------------------------------------------

void DistributionWindow::setBetaDistributionFromMean(t_float v) {
	t_float tot = view->curDistribution.dBt.alpha + view->curDistribution.dBt.beta;
	view->curDistribution.dBt.alpha = v * tot;
	view->curDistribution.dBt.beta = (1.0 - v) * tot;

	// set new max dev
	sliderBtBeta->range(0.001, sqrt(v * (1.0 - v)) - 0.001);
	sliderBtBeta->value(view->curDistribution.dBt.getStddev());
}

//-----------------------------------------------------------------------------------------------------------------------


void DistributionWindow::setBetaDistributionFromDev(t_float v) {
	view->curDistribution.dBt.setFromMeanDev(view->curDistribution.dBt.getMean(), v);
}


//-----------------------------------------------------------------------------------------------------------------------

void DistributionWindow::answer(t_int answ) {
	view->redraw();
}

//-----------------------------------------------------------------------------------------------------------------------


void StatisticsWindow::fillOutStatistics(void) {
	// fill out diagram
	//viewDiagram->setData(bsShown->eValueOverTime, 1.0);
	for(t_int i = 0; i <= 4; ++i) {
		boxDiagramXValue[i]->copy_label(DoubleToString(i * bsShown->totalSteps() / 4.0, 1));
		boxDiagramYValue[4-i]->copy_label(DoubleToString(i / 4.0, 2));
	}


	// fill out collected statistics fields
	outputAvgEValue->value(DoubleToString(bsShown->stats.totalEValue, 4));
	outputEValueDelta->value(DoubleToString(bsShown->stats.totalEValueDelta, 4));
	outputPolarisation->value(DoubleToString(bsShown->stats.totalPolarisation, 4));
	outputPolarisationDelta->value(DoubleToString(bsShown->stats.totalPolarisationDelta, 4));
	outputNumSteps->value(IntToString(bsShown->totalSteps()));
	outputNumSocieties->value(IntToString(bsShown->nTrials));
	outputMessagesSentTotal->value(DoubleToString(bsShown->stats.avgMessagesSentTotal, 2));
	outputMessagesSentPerInquirer->value(DoubleToString(bsShown->stats.avgMessagesSentPerInquirer, 2));
	outputInquiryResultsTotal->value(DoubleToString(bsShown->stats.avgInquiryResultsTotal, 2));
	outputInquiryResultsPerInquirer->value(DoubleToString(bsShown->stats.avgInquiryResultsPerInquirer, 2));

	// fill out error margins
	choiceConfidenceLevel->value(1);
	computeConfidences(.95f);

	// fill out bandwagon data
	outputBWToPPerc->value(DoubleToString(bsShown->stats.avgBWToPProb * 100.0, 1));
	outputBWToPEffect->value(DoubleToString(bsShown->stats.avgBWToPEffect, 4));
	outputBWToNotPPerc->value(DoubleToString(bsShown->stats.avgBWToNotPProb * 100.0, 1));
	outputBWToNotPEffect->value(DoubleToString(bsShown->stats.avgBWToNotPEffect, 4));

	// turn on/off export
	if (bsShown->stats.eValuesOverTime.valid()) {
		buttonExportEValues->activate();
		choiceDiagramView->activate();
		setDiagramView(5);
	}
	else {
		buttonExportEValues->deactivate();
		choiceDiagramView->deactivate();
	}
	if (bsShown->stats.recordTopologies) buttonExportTopologies->activate();
	else buttonExportTopologies->deactivate();
}

//-----------------------------------------------------------------------------------------------------------------------

void StatisticsWindow::computeConfidences(t_float level) {
	t_float confSize = gsl_cdf_tdist_Pinv((1.0 + level) / 2.0, bsShown->nTrials - 1);
	t_float val = bsShown->stats.totalEValueS / sqrt((t_float)bsShown->nTrials) * confSize;
	outputAvgEValueMargin->value(DoubleToString(val));
	val = bsShown->stats.totalEValueDeltaS / sqrt((t_float)bsShown->nTrials) * confSize;
	outputEValueDeltaMargin->value(DoubleToString(val));
	val = bsShown->stats.totalPolarisationS / sqrt((t_float)bsShown->nTrials) * confSize;
	outputPolarisationMargin->value(DoubleToString(val));
	val = bsShown->stats.totalPolarisationDeltaS / sqrt((t_float)bsShown->nTrials) * confSize;
	outputPolarisationDeltaMargin->value(DoubleToString(val));
}

//-----------------------------------------------------------------------------------------------------------------------

void StatisticsWindow::setDiagramView(t_int v) {
	if (bsShown->stats.eValuesOverTime.valid()) {
		if(v == 5) {
			// show for all
			viewDiagram->setData(bsShown->stats.eValuesOverTime.average(DIM_X).average(DIM_Y), 1.0);
			viewDiagram->redraw();
			choiceDiagramView->value(5);
		}
		else {
			// show a quintile
			StatisticsBlock b = bsShown->stats.eValuesOverTime.partialYAverage(v / 5.0, (v + 1) / 5.0, v <= 2, v >= 2);
			viewDiagram->setData(b, 1.0);
			viewDiagram->redraw();
			choiceDiagramView->value(v);
		}
	}
}

//-----------------------------------------------------------------------------------------------------------------------

void DistributionFreeformValuesWindow::setDistributionValues(void) {
	t_float v = fieldValue->value();
	for(t_int i = 0; i < distributionWindow->view->curDistribution.dFf.values.size(); ++i) {
		if(distributionFreeformValuesWindow->listValues->selected(i + 1)) {
			distributionWindow->view->curDistribution.dFf.values[i] = v;
			char str[64];
			t_float v = ((t_float)i / (t_float)(distributionWindow->view->curDistribution.dFf.values.size() - 1)) * (distributionWindow->view->curDistribution.max -
				distributionWindow->view->curDistribution.min) + distributionWindow->view->curDistribution.min;
			sprintf(str, "[%.3f] \t %.4f", v, distributionWindow->view->curDistribution.dFf.values[i]);
			listValues->text(i + 1, str);

		}
	}
	distributionWindow->view->redraw();
}

//-----------------------------------------------------------------------------------------------------------------------

void DistributionFreeformValuesWindow::getDistributionValues(void) {
	t_float v = 0;
	t_int n = 0;
	for(t_int i = 0; i < distributionWindow->view->curDistribution.dFf.values.size(); ++i) {
		if(listValues->selected(i + 1)) {
			v += distributionWindow->view->curDistribution.dFf.values[i];
			++n;
		}
	}

	if(n == 0) fieldValue->deactivate();
	else {
		fieldValue->activate();
		fieldValue->value(v / n);
	}
}

//-----------------------------------------------------------------------------------------------------------------------

void MetaDistributionWindow::setMetaDistribution(MetaDistribution* md) {
	curMetaDistribution = *md;
	metaDistributionFieldEdited = md;

	viewZero->setDistribution(&curMetaDistribution.zero);
	viewOne->setDistribution(&curMetaDistribution.one);
	viewMixture->setDistribution(&curMetaDistribution.mixture);
	viewMetaDistribution->setMetaDistribution(curMetaDistribution);
	viewMetaDistribution->set_output();
	viewPreview->set_output();
	setPreview(0.5);
}

//-----------------------------------------------------------------------------------------------------------------------

void MetaDistributionWindow::setPreview(t_float v) {
	previewDistribution = curMetaDistribution.getDistribution(curMetaDistribution.mixture.getInverseCDFValue(v));
	viewPreview->setDistribution(&previewDistribution);
}

//-----------------------------------------------------------------------------------------------------------------------

void MetaDistributionWindow::answer(t_int answ) {
	viewMetaDistribution->setMetaDistribution(curMetaDistribution);
	setPreview(sliderParameter->value());
}

//-----------------------------------------------------------------------------------------------------------------------

void MultiBatchWindow::configure(void) {
	multiBatchWindow->inputStepsAtoB->value(multiBatchWindow->mb.stepsAtoB);
	multiBatchWindow->inputStepsAtoC->value(multiBatchWindow->mb.stepsAtoC);
	if(multiBatchWindow->mb.stepsAtoC == 1) {
		// one dimensional
		multiBatchWindow->inputStepsAtoC->deactivate();
		multiBatchWindow->btnSetBatchC->deactivate();
		multiBatchWindow->btnSetBatchD->deactivate();
		multiBatchWindow->btnOneDimension->value(1);
		multiBatchWindow->btnTwoDimensions->value(0);
		for(t_int i = 2; i < 8; ++i) multiBatchWindow->btnCopyBatch[i]->deactivate();
	}
	else {
		// two dimensional
		multiBatchWindow->inputStepsAtoC->activate();
		multiBatchWindow->btnSetBatchC->activate();
		multiBatchWindow->btnSetBatchD->activate();
		multiBatchWindow->btnOneDimension->value(0);
		multiBatchWindow->btnTwoDimensions->value(1);
		for(t_int i = 2; i < 8; ++i) multiBatchWindow->btnCopyBatch[i]->activate();
	}

}

//-----------------------------------------------------------------------------------------------------------------------

void MultiBatchWindow::showPreview(void) {
	mb.generateBatch(0.5, 0.5);
	showDialog(DIALOG_BATCH_SIMULATION_PREVIEW, &mb.curBatch);
}

//-----------------------------------------------------------------------------------------------------------------------

void MultiBatchWindow::loadMultibatch(void) {
  string filename = OpenFileDialog("Open Multibatch", "*.mbatch");

	if (filename != "") {
		// open the file
		TiXmlDocument f(filename.c_str());
		bool loadSucceeded = f.LoadFile();
		if(!loadSucceeded) {
			fl_alert("Failed to load multibatch file.");
			return;
		}
		TiXmlElement *root = f.RootElement();

		// check if distribution file
		if(strcmp("MULTIBATCH_FILE", root->Value())) {
			fl_alert("This is not a valid multibatch file.");
			return;
		}

		// check version
		t_int vers;
		root->QueryIntAttribute("VERSION", &vers);
		if(vers < MIN_LAPUTA_VERSION) {
			fl_alert("This multibatch was created with a different version of Laputa.");
			return;
		}

		// read distribution
		mb = MultiBatch(root->FirstChildElement("MULTIBATCH"));

		// update distribution window
		configure();
	}

}

//-----------------------------------------------------------------------------------------------------------------------

void MultiBatchWindow::saveMultibatch(void) {
  string filename = SaveFileDialog("Save Multibatch As", "*.mbatch", "Untitled.mbatch");
	if (filename != "") {
		// create xml file
		TiXmlDocument f;
		TiXmlDeclaration* decl = new TiXmlDeclaration( "1.0", "UTF-8", "true" );
		f.LinkEndChild(decl);
		TiXmlElement *root = new TiXmlElement("MULTIBATCH_FILE");
		root->SetAttribute("VERSION", LAPUTA_VERSION);
		f.LinkEndChild(root);
		root->LinkEndChild(mb.toXML());

		// write out everything
		f.SaveFile(filename.c_str());
	}
}

//-----------------------------------------------------------------------------------------------------------------------

void ManualWindow::configure(const char* file) {
	char fname[FL_PATH_MAX];
    if(file) strcpy(fname, app->docsFile(file).c_str());
    else strcpy(fname, app->docsFile(INDEX_FILE).c_str());

	view->link(FollowHTMLLinkBlindly);
	view->load(fname);
	view->link(FollowHTMLLink);
	outputFile->value(fname);
	pageHistory.clear();
	pageHistory.push_back(string(fname));
	if (strcmp(fname, app->docsFile(INDEX_FILE).c_str())) buttonHome->activate();
	else buttonHome->deactivate();
	buttonForward->deactivate();
	buttonBack->deactivate();
	curPage = 0;
}

//-----------------------------------------------------------------------------------------------------------------------
const char *FollowHTMLLink(Fl_Widget *w, const char *uri) {
	ManualWindow* m = (ManualWindow*)(((Fl_Help_View*)w)->window());
	const char *ext = strrchr(uri, '.');
	if(ext) {
		if(strcmp(ext, ".htm") == 0) {
			m->outputFile->value(uri);
			m->pageHistory.push_back(uri);
			++m->curPage;
			m->buttonBack->activate();
			m->buttonForward->deactivate();
			if (strcmp(uri, app->docsFile(INDEX_FILE).c_str())) m->buttonHome->activate();
			else m->buttonHome->deactivate();
		}
	}
	return uri;
}


//-----------------------------------------------------------------------------------------------------------------------

const char *FollowHTMLLinkBlindly(Fl_Widget *w, const char *uri) {
	return uri;
}

//-----------------------------------------------------------------------------------------------------------------------

void ManualWindow::goForward(void) {
	++curPage;
	outputFile->value(pageHistory[curPage].c_str());
	view->link(FollowHTMLLinkBlindly);
	view->load(pageHistory[curPage].c_str());
	view->link(FollowHTMLLink);
	if(curPage >= pageHistory.size() - 1) buttonForward->deactivate();
	if (strcmp(pageHistory[curPage].c_str(), app->docsFile(INDEX_FILE).c_str())) buttonHome->activate();
	else buttonHome->deactivate();
	buttonBack->activate();
}

//-----------------------------------------------------------------------------------------------------------------------


void ManualWindow::goBack(void) {
	--curPage;
	outputFile->value(pageHistory[curPage].c_str());
	view->link(FollowHTMLLinkBlindly);
	view->load(pageHistory[curPage].c_str());
	view->link(FollowHTMLLink);
	if(curPage <= 0) buttonBack->deactivate();
	if (strcmp(pageHistory[curPage].c_str(), app->docsFile(INDEX_FILE).c_str())) buttonHome->activate();
	else buttonHome->deactivate();
	buttonForward->activate();
}

//-----------------------------------------------------------------------------------------------------------------------

void ManualWindow::goHome(void) {
	outputFile->value(app->docsFile(INDEX_FILE).c_str());
	pageHistory.push_back(app->docsFile(INDEX_FILE).c_str());
	view->link(FollowHTMLLinkBlindly);
	view->load(app->docsFile(INDEX_FILE).c_str());
	view->link(FollowHTMLLink);
	++curPage;
	buttonBack->activate();
	buttonForward->deactivate();
	buttonHome->deactivate();
}

//-----------------------------------------------------------------------------------------------------------------------

void DegreeDistributionWindow::configure(vector<t_float> *degrees) {
	for(t_int i = 0; i < 3; ++i) {
		view[i]->setData(degrees[i]);
		sliderCutoff[i]->range(0, degrees[i].size() - 1);
		sliderCutoff[i]->value(0);
		for(t_int j = 0; j <= 4; ++j) {
			labelXAxis[i][j]->copy_label(view[i]->xLabel((t_float)j / 4.0));
			labelYAxis[i][j]->copy_label(view[i]->yLabel((t_float)j / 4.0));
		}
		FillOutCurveMenu(choiceFunction[i]);

		view[i]->curve = view[i]->findBestCurve();
		calculateFit(i, view[i]->curve.type);
		choiceFunction[i]->value(view[i]->curve.type);
	}
}

//-----------------------------------------------------------------------------------------------------------------------

void DegreeDistributionWindow::calculateFit(t_int pane, t_int f) {
	view[pane]->curve = view[pane]->fitToCurve(f);

	// fill out formula & parameters
	boxFormula[pane]->image(view[pane]->curve.getFormula());
	for(t_int j = 0; j < view[pane]->curve.nParameters(); ++j) {
		outputParameter[pane][j]->value(view[pane]->curve.getParameterValue(j));
		char str[64];
		strcpy(str, view[pane]->curve.getParameterName(j));
		strcat(str, " = ");
		outputParameter[pane][j]->copy_label(str);
		outputParameter[pane][j]->show();
	}
	for(t_int j = view[pane]->curve.nParameters(); j < 4; ++j) outputParameter[pane][j]->hide();

	// fill out error
	outputError[pane]->value(view[pane]->curveError(view[pane]->curve));

	// update
	view[pane]->redraw();
	boxFormula[pane]->redraw();
}

//-----------------------------------------------------------------------------------------------------------------------

void DegreeDistributionWindow::setCutoff(t_int pane, t_int cutoff) {
	view[pane]->range(cutoff, view[pane]->data.size());
	calculateFit(pane, choiceFunction[pane]->value());
}

//-----------------------------------------------------------------------------------------------------------------------

void DegreeDistributionWindow::fitToCutoff(t_int pane, t_int cutoff) {
	view[pane]->range(cutoff, view[pane]->data.size());
	view[pane]->curve = view[pane]->findBestCurve();
	calculateFit(pane, view[pane]->curve.type);
	choiceFunction[pane]->value(view[pane]->curve.type);
}

//-----------------------------------------------------------------------------------------------------------------------

void ExpressionWindow::configure(Distribution *distr) {
	d = distr;
	bufPDF.text("1.0");
	inputFormulaPDF->buffer(&bufPDF);
	bufCDF.text("t");
	inputFormulaCDF->buffer(&bufCDF);
	tabs->value(groupPDF);

}

//-----------------------------------------------------------------------------------------------------------------------

void ExpressionWindow::applyExpression(void) {
	if(tabs->value() == groupPDF) {
		for(t_int i = 0; i < d->dFf.values.size(); ++i) d->dFf.values[i] = inputFormulaPDF->evaluate(d->min + (t_float)i / (d->dFf.values.size() - 1) * (d->max - d->min));
		d->dFf.normalise();
	}
	else {
		t_float prev = 0;
		for(t_int i = 0; i < d->dFf.values.size(); ++i) {
			t_float v = inputFormulaCDF->evaluate(d->min + (t_float)i / (d->dFf.values.size() - 1) * (d->max - d->min));
			d->dFf.values[i] = (v - prev) * d->dFf.values.size();
			prev = v;
		}
		d->dFf.normalise();
	}
	d->dFf.cdfValidity = 0;
}

//-----------------------------------------------------------------------------------------------------------------------

void ExportStatisticsWindow::configure(BatchSimulation* b) {
	bs = b;

	choose(0, 0);
	choose(1, 1);
	choose(2, 2);
	buttonAverage[0]->value(0);
	buttonAverage[1]->value(0);
	buttonAverage[2]->value(1);
}

//-----------------------------------------------------------------------------------------------------------------------

void ExportStatisticsWindow::choose(t_int dim, t_int variable) {
	t_int openVariable;
	buttonVariable[dim][variable]->value(0);
	for(t_int i = 0; i < 3; ++i) if(buttonVariable[dim][i]->value()) openVariable = i;
	for(t_int i = 0; i < 3; ++i) {
		if(buttonVariable[dim][i]->value()) openVariable = i;
		if(i == variable) buttonVariable[dim][i]->value(1);
		else buttonVariable[dim][i]->value(0);
	}
	if (variable == 0) inputFrom[dim]->maximum(bs->stats.eValuesOverTime.width);
	if (variable == 1) inputFrom[dim]->maximum(bs->stats.eValuesOverTime.height);
	if (variable == 2) inputFrom[dim]->maximum(bs->stats.eValuesOverTime.depth);
	if(variable == 1) inputFrom[dim]->minimum(0);
	else inputFrom[dim]->minimum(1);
	inputTo[dim]->maximum(inputFrom[dim]->maximum());
	inputTo[dim]->minimum(inputFrom[dim]->minimum());
	inputFrom[dim]->value(inputFrom[dim]->minimum());
	inputTo[dim]->value(inputTo[dim]->maximum());
	if(variable == 1) buttonAverage[dim]->copy_label("Final");
	else  buttonAverage[dim]->copy_label("Average");

	// make sure each variable has only one dimension
	for(t_int j = 0; j < 3; ++j) {
		if(j != dim && buttonVariable[j][variable]->value()) {
			// use the empty spot
			choose(j, openVariable);
			break;
		}
	}
}

//-----------------------------------------------------------------------------------------------------------------------

bool ExportStatisticsWindow::save(void) {
	// get a place to save
  string filename = SaveFileDialog("Save Statistics As", "*.ods", "Untitled test results.ods") ;
	if (filename != "") {
		char relName[FL_PATH_MAX];

		// create file
		fl_filename_relative(relName, FL_PATH_MAX, filename.c_str());
		if(!exportToFile(relName)) {
			fl_alert("There was not memory enough to export in this format. Try exporting only a part of the data, averaging over some dimensions, or getting more memory.");
			return false;
		}
	}
	else return false;

	return true;
}

//-----------------------------------------------------------------------------------------------------------------------

bool ExportStatisticsWindow::exportToFile(const char* filename) {

	// get dimensions & inverse
	t_int dimensions[3], variables[3];
	bool average[3];
	for(t_int i = 0; i < 3; ++i) {
		if(buttonVariable[i][0]->value()) dimensions[DIM_X] = i;
		else if(buttonVariable[i][1]->value()) dimensions[DIM_Y] = i;
		else dimensions[DIM_Z] = i;
		if(buttonAverage[i]->value()) average[i] = true;
		else average[i] = false;
	}
	for(t_int i = 0; i < 3; ++i) {
		if(dimensions[i] == 0) variables[0] = i;
		else if(dimensions[i] == 1) variables[1] = i;
		else variables[2] = i;
	}

	// permute
	StatisticsBlock b = bs->stats.eValuesOverTime.permute(variables[DIM_X], variables[DIM_Y], variables[DIM_Z]);

	// extract part of data we need
	StatisticsBlock c;
	if(variables[0] == 1) c = b.extract(inputFrom[DIM_X]->value(), inputTo[DIM_X]->value(), inputFrom[DIM_Y]->value() - 1, inputTo[DIM_Y]->value(), inputFrom[DIM_Z]->value() - 1, inputTo[DIM_Z]->value());
	else if(variables[1] == 1) c = b.extract(inputFrom[DIM_X]->value() - 1, inputTo[DIM_X]->value(), inputFrom[DIM_Y]->value(), inputTo[DIM_Y]->value(), inputFrom[DIM_Z]->value() - 1, inputTo[DIM_Z]->value());
	else c = b.extract(inputFrom[DIM_X]->value(), inputTo[DIM_X]->value(), inputFrom[DIM_Y]->value() - 1, inputTo[DIM_Y]->value(), inputFrom[DIM_Z]->value(), inputTo[DIM_Z]->value());

	// average or pick out last moment as needed
	StatisticsBlock d, e, f;
	if(average[DIM_X]) {
		if(variables[DIM_X] == 1) d = c.extract(DIM_X, c.width - 1, c.width);
		d = c.average(DIM_X);
	}
	else d = c;
	c.free();
	if(average[DIM_Y]) {
		if(variables[DIM_Y] == 1) e = d.extract(DIM_Y, d.height - 1, d.height);
		else e = d.average(DIM_Y);
	}
	else e = d;
	d.free();
	if(average[DIM_Z]) {
		f = e.average(DIM_Z);
		if(variables[DIM_Z] == 1) f = e.extract(DIM_Z, e.depth - 1, e.depth);
		else f = e.average(DIM_Z);
	}
	else f = e;
	e.free();

	// set up a spreadsheet
	XMLData *xmlBlock = new XMLData [(f.width + 1) * (f.height + 1) * f.depth];
	if(!xmlBlock) return false;
	string *sheetNames = new string[f.depth];

	// write out titles
	f.setDimension(3);
	for(t_int k = 0; k < f.depth; ++k) {
		if(!average[DIM_X]) {
			for(t_int i = 0; i < f.width; ++i) {
				if(variables[DIM_X] == DIM_X) {
					char str[64] = "Inquirer ";
					strcat(str, IntToString(i + inputFrom[DIM_X]->value()));
					xmlBlock[k * (f.height + 1) * (f.width + 1) + i + 1].setString(str);
				}
				else if(variables[DIM_X] == DIM_Y) {
					char str[64] = "t = ";
					strcat(str, IntToString(i + inputFrom[DIM_X]->value()));
					xmlBlock[k * (f.height + 1) * (f.width + 1) + i + 1].setString(str);
				}
				else if(variables[DIM_X] == DIM_Z) {
					char str[64] = "Trial ";
					strcat(str, IntToString(i + inputFrom[DIM_X]->value()));
					xmlBlock[k * (f.height + 1) * (f.width + 1) + i + 1].setString(str);
				}
			}
		}
		else {
			if(variables[DIM_X] == 1) xmlBlock[k * (f.height + 1) * (f.width + 1) +1].setString("Final E-value");
			else xmlBlock[k * (f.height + 1) * (f.width + 1) +1].setString("Average E-value");
		}
		if(!average[DIM_Y]) {
			for(t_int i = 0; i < f.height; ++i) {
				if(variables[DIM_Y] == DIM_X) {
					char str[64] = "Inquirer ";
					strcat(str, IntToString(i + inputFrom[DIM_Y]->value()));
					xmlBlock[k * (f.height + 1) * (f.width + 1) + (i + 1) * (f.width + 1)].setString(str);
				}
				else if(variables[DIM_Y] == DIM_Y) {
					char str[64] = "t = ";
					strcat(str, IntToString(i + inputFrom[DIM_Y]->value()));
					xmlBlock[k * (f.height + 1) * (f.width + 1) + (i + 1) * (f.width + 1)].setString(str);
				}
				else if(variables[DIM_Y] == DIM_Z) {
					char str[64] = "Trial ";
					strcat(str, IntToString(i + inputFrom[DIM_Y]->value()));
					xmlBlock[k * (f.height + 1) * (f.width + 1) + (i + 1) * (f.width + 1)].setString(str);
				}
			}
		}
		else {
			if(variables[DIM_Y] == 1) xmlBlock[k * (f.height + 1) * (f.width + 1) + f.width + 1].setString("Final E-value");
			else xmlBlock[k * (f.height + 1) * (f.width + 1) + f.width + 1].setString("Average E-value");		}
		if(!average[DIM_Z]) {
			for(t_int i = 0; i < f.depth; ++i) {
				if(variables[DIM_Z] == DIM_X) {
					char str[64] = "Inquirer ";
					strcat(str, IntToString(i + inputFrom[DIM_Z]->value()));
					sheetNames[i] = string(str);
				}
				else if(variables[DIM_Z] == DIM_Y) {
					char str[64] = "t = ";
					strcat(str, IntToString(i + inputFrom[DIM_Z]->value()));
					sheetNames[i] = string(str);
				}
				else if(variables[DIM_Z] == DIM_Z) {
					char str[64] = "Trial ";
					strcat(str, IntToString(i + inputFrom[DIM_Z]->value()));
					sheetNames[i] = string(str);
				}
			}
		}
		else {
			if(variables[DIM_Z] == 1) sheetNames[0] = string("Final E-value");
			else sheetNames[0] = string("Average E-value");
		}
	}

	// fill out actual data
	for(t_int k = 0; k < f.depth; ++k) {
		for(t_int j = 0; j < f.height; ++j) {
			for(t_int i = 0; i < f.width; ++i) {
				if(f.v(i, j, k) == f.v(i, j, k)) xmlBlock[k * (f.height + 1) * (f.width + 1) + (j + 1) * (f.width + 1) + i + 1].setDouble(f.v(i, j, k));
			}
		}
	}

	// do the actual saving
	bool r = SaveDataAsSpreadsheet(xmlBlock, f.width + 1, f.height + 1, f.depth, sheetNames, filename);
	delete [] sheetNames;
	return r;
}

//-----------------------------------------------------------------------------------------------------------------------

void ExportTopologyWindow::configure(BatchSimulation* b) {
	bs = b;
	if (bs) {
		labelSaveAs->activate();
		buttonSingleFile->activate();
		buttonFolder->activate();
	}
	else {
		labelSaveAs->deactivate();
		buttonSingleFile->deactivate();
		buttonFolder->deactivate();
	}
}

//-----------------------------------------------------------------------------------------------------------------------

bool ExportTopologyWindow::save(void) {
	// get a place to save
  string filename;
  if(bs) {
    if(buttonSingleFile->value())
      filename = SaveFileDialog("Save Networks As", "*.net", "Untitled networks.net");
    else
      filename = SaveFileDialog("Save Networks As", "", "Untitled networks");
  }
  else filename = SaveFileDialog("Save Network As", "*.paj", "Untitled networks.paj");
	if (filename != "") {
		char relName[FL_PATH_MAX];

		// create file
		fl_filename_relative(relName, FL_PATH_MAX, filename.c_str());
		exportToFile(relName);
	}
	else return false;

	return true;
}

//-----------------------------------------------------------------------------------------------------------------------

bool ExportTopologyWindow::exportToFile(char* filename) {
	t_int wts;
	if (buttonWeightNone->value()) wts = WTS_NONE;
	else if (buttonWeightListenChance->value()) wts = WTS_LISTEN_CHANCE;
	else wts = WTS_TRUST;

	if (bs) {
		if (buttonSingleFile->value()) {
			// write a single file
			FILE *f = fopen(filename, "w");

			for (t_int i = 0; i < bs->stats.topologies.size(); ++i) {
				string s = string("*Network NW") + string(IntToString(i + 1)) + string("\r\n");
				s += bs->stats.topologies[i].description(fieldMinListenChance->value(), wts) + string("\r\n");
				fwrite(s.c_str(), 1, strlen(s.c_str()), f);
			}
			fclose(f);
		}
		else {
			// create a folder
			MakePathNative(filename);
			MakeDirectory(filename);
			string dir = string(filename) + string("/"), file;
			t_int i;
			for (i = strlen(filename) - 1; i >= 0; --i) if (filename[i] == '/' || filename[i] == '\\') break;
			file = string(filename + i + 1);

			// write files
			for (t_int i = 0; i < bs->stats.topologies.size(); ++i) {
				string numberedFileName = dir + file + " " + IntToString(i + 1) + ".net";
				MakePathNative(numberedFileName);
				FILE *f = fopen(numberedFileName.c_str(), "w");
				string top = bs->stats.topologies[i].description(fieldMinListenChance->value(), wts);
				fwrite(top.c_str(), 1, strlen(top.c_str()), f);
				fclose(f);
			}
		}
	}
	else {
		// save a single network in a single file
		FILE *f = fopen(filename, "w");
		NetworkTopology top(curSociety);
		string d = top.description(fieldMinListenChance->value(), wts);
		fwrite(d.c_str(), 1, strlen(d.c_str()), f);
		fclose(f);
	}
	return true;
}
