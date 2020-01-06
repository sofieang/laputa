#include "SocietySetup.h"
#include "App.h"
#include <time.h>
#include "Utility.h"
//-----------------------------------------------------------------------------------------------------------------------

SocietySetup::SocietySetup(const SocietySetup& s1, const SocietySetup& s2, t_float v) {
	*this = s1;
	populationDistribution = Distribution(s1.populationDistribution, s2.populationDistribution, v);
	linkDensityDistribution = Distribution(s1.linkDensityDistribution, s2.linkDensityDistribution, v);
	initialPopulationPart = s1.initialPopulationPart * (1.0 - v) + s2.initialPopulationPart * v;
	growthBalance = s1.growthBalance * (1.0 - v) + s2.growthBalance * v;
	for(t_int i = 0; i < N_LINK_WEIGHT_FACTORS; ++i) linkWeights[i] = s1.linkWeights[i] * (1.0 - v) + s2.linkWeights[i] * v;
	inqParams = InquirerParameters(s1.inqParams, s2.inqParams, v);
	linkParams = LinkParameters(s1.linkParams, s2.linkParams, v);
	if(v >= 0.5) {
		varyPopulation = s2.varyPopulation;
		varyLinks = s2.varyLinks;
		limitLinksToOnePerPair = s2.limitLinksToOnePerPair;
		linkDistributionMethod = s2.linkDistributionMethod;
	}
}

//-----------------------------------------------------------------------------------------------------------------------

SocietySetup::SocietySetup(TiXmlElement* xml) {
	// read fields
	if(strcmp(xml->Attribute("VARY_POPULATION"), "true") == 0) varyPopulation = true;
	else varyPopulation = false;
	xml->QueryFloatAttribute("INITIAL_POPULATION_PART", &initialPopulationPart);
	xml->QueryFloatAttribute("GROWTH_BALANCE", &growthBalance);
	if(strcmp(xml->Attribute("VARY_LINKS"), "true") == 0) varyLinks = true;
	else varyLinks = false;
	if(strcmp(xml->Attribute("INCLUDE_IN_STATISTICS"), "true") == 0) includeInStatistics = true;
	else includeInStatistics = false;
	if(strcmp(xml->Attribute("UPDATE_INQUIRY_TRUST"), "true") == 0) updateInquiryTrust = true;
	else updateInquiryTrust = false;
	if(strcmp(xml->Attribute("LIMIT_LINKS_TO_ONE_PER_PAIR"), "true") == 0) limitLinksToOnePerPair = true;
	else limitLinksToOnePerPair = false;
	if(strcmp(xml->Attribute("LINK_DISTRIBUTION_METHOD"), "total") == 0) linkDistributionMethod = LDM_TOTAL;
	else if(strcmp(xml->Attribute("LINK_DISTRIBUTION_METHOD"), "per inquirer") == 0) linkDistributionMethod = LDM_PER_INQUIRER;
	else linkDistributionMethod = LDM_PER_INQUIRER_SQUARED;
	if(strcmp(xml->Attribute("LINK_COUNT_METHOD"), "number") == 0) linkDistributionMethod |= LDM_TO_NUMBER_BIT;
	xml->QueryFloatAttribute("LINK_WEIGHT_BASE", &linkWeights[WT_BASE]);
	xml->QueryFloatAttribute("LINK_WEIGHT_SYMMETRY", &linkWeights[WT_SYMMETRY]);
	xml->QueryFloatAttribute("LINK_WEIGHT_TRANSITIVITY", &linkWeights[WT_TRANSITIVITY]);
	xml->QueryFloatAttribute("LINK_WEIGHT_CLUSTERING", &linkWeights[WT_CLUSTERING]);
	evidencePolicy = EvidenceValue(xml->Attribute("NEW_EVIDENCE_REQUIREMENT"));
	if(xml->Attribute("COUNT_PRIOR_AS_EVIDENCE")) {
		if(strcmp(xml->Attribute("COUNT_PRIOR_AS_EVIDENCE"), "true") == 0) countPriorAsEvidence = true;
		else countPriorAsEvidence = false;
	}
	else countPriorAsEvidence = true;
	if(strcmp(xml->Attribute("UPDATE_TRUST"), "true") == 0) updateTrust = true;
	else updateTrust = false;

	// read distributions
	populationDistribution = Distribution(xml->FirstChildElement("POPULATION_DISTRIBUTION"));
	linkDensityDistribution = Distribution(xml->FirstChildElement("LINK_DENSITY_DISTRIBUTION"));

	// read inquirer & link parameters
	inqParams = InquirerParameters(xml->FirstChildElement("INQUIRER_PARAMETERS"));
	linkParams = LinkParameters(xml->FirstChildElement("LINK_PARAMETERS"));
}

//-----------------------------------------------------------------------------------------------------------------------

string SocietySetup::getDescription(void) {
	string str("Population: ");
	str += string("Distribution: ") + populationDistribution.getDescription() + string("\r\n");
	str += string("Links: ") + string(IntToString(linkDensityDistribution.min)) + string(" - ") +  string(IntToString(linkDensityDistribution.max));
	if(linkDistributionMethod == LDM_TOTAL) str += string(" in total\r\n");
	else if (linkDistributionMethod == LDM_PER_INQUIRER) str += string(" per inquirer\r\n");
	else str += string(" per inquirer squared\r\n");
	str += string("Distribution: ") + linkDensityDistribution.getDescription() + string("\r\n");
	str += string("Weights: Base=") + string(DoubleToString(linkWeights[WT_BASE])) + ", " + string("Symmetry=") + string(DoubleToString(linkWeights[WT_SYMMETRY])) + string(", ") +
		string("Transitivity=") + string(DoubleToString(linkWeights[WT_TRANSITIVITY])) + string(", ") + string("Clustering=") + string(DoubleToString(linkWeights[WT_CLUSTERING])) + string("\r\n");
	str += inqParams.getDescription() + string("\r\n");
	str += linkParams.getDescription() + string("\r\n");
	return str;
}

//-----------------------------------------------------------------------------------------------------------------------

void SocietySetup::precalculate(void) {
	inqParams.inquiryTrust.precalculateForTrust();
	linkParams.linkTrust.precalculateForTrust();
}

//-----------------------------------------------------------------------------------------------------------------------

void SocietySetup::setDefault(void) {
	inqParams.setDefault();
	linkParams.setDefault();
	varyPopulation = false;
	varyLinks = false;
	includeInStatistics = updateInquiryTrust = true;
	evidencePolicy = NEW_EVIDENCE_NONE;
	countPriorAsEvidence = false;
	updateTrust = true;
	limitLinksToOnePerPair = true;
	initialPopulationPart = 1.0;
	growthBalance = 0.5;
	linkWeights[WT_BASE] = 1.0;
	for(t_int i = 1; i < N_LINK_WEIGHT_FACTORS; ++i) linkWeights[i] = 0;
	populationDistribution.setDiscreteRange(2, 20);
	linkDistributionMethod = LDM_PER_INQUIRER;
}

//-----------------------------------------------------------------------------------------------------------------------

void SocietySetup::setInquirerParametersWindowFrom(void) {

	// turn on/off starting belief controls
	inquirerParametersWindow->sliderVaryBelief->value(2 - inqParams.varyStartBelief);
	if(inqParams.varyStartBelief == VARY_INDIVIDUALLY) {
		inquirerParametersWindow->viewBelief->setDistribution(&inqParams.startBelief);
		inquirerParametersWindow->viewBelief->activate();
		inquirerParametersWindow->boxBelief->activate();
	}
	else {
		inquirerParametersWindow->viewBelief->deactivate();
		inquirerParametersWindow->boxBelief->deactivate();
	}

	// turn on/off inquiry chance controls
	inquirerParametersWindow->sliderVaryInquiryChance->value(2 - inqParams.varyInquiryChance);
	if(inqParams.varyInquiryChance == VARY_INDIVIDUALLY) {
		inquirerParametersWindow->viewInquiryChance->setDistribution(&inqParams.inquiryChance);
		inquirerParametersWindow->viewInquiryChance->activate();
		inquirerParametersWindow->boxInquiryChance->activate();
	}
	else {
		inquirerParametersWindow->viewInquiryChance->deactivate();
		inquirerParametersWindow->boxInquiryChance->deactivate();
	}

	// turn on/off inquiry veracity chance controls
	inquirerParametersWindow->sliderVaryVeracityChance->value(2 - inqParams.varyInquiryAccuracy);
	if(inqParams.varyInquiryAccuracy == VARY_INDIVIDUALLY) {
		inquirerParametersWindow->viewInquiryAccuracy->setDistribution(&inqParams.inquiryAccuracy);
		inquirerParametersWindow->viewInquiryAccuracy->activate();
		inquirerParametersWindow->boxInquiryAccuracy->activate();
	}
	else {
		inquirerParametersWindow->viewInquiryAccuracy->deactivate();
		inquirerParametersWindow->boxInquiryAccuracy->deactivate();
	}

	// turn on/off inquiry trust controls
	inquirerParametersWindow->sliderVaryInquiryTrust->value(2 - inqParams.varyInquiryTrust);
	if(inqParams.varyInquiryTrust == VARY_INDIVIDUALLY) {
		inquirerParametersWindow->viewInquiryTrust->setMetaDistribution(&inqParams.inquiryTrust);
		inquirerParametersWindow->viewInquiryTrust->activate();
		inquirerParametersWindow->boxInquiryTrust->activate();
	}
	else {
		inquirerParametersWindow->viewInquiryTrust->deactivate();
		inquirerParametersWindow->boxInquiryTrust->deactivate();
	}
}

//-----------------------------------------------------------------------------------------------------------------------

void SocietySetup::setLinkParametersWindowFrom(void) {
	// turn on/off listen chance controls
	linkParametersWindow->sliderVaryListenChance->value(2 - linkParams.varyListenChance);
	if(linkParams.varyListenChance == VARY_INDIVIDUALLY) {
		linkParametersWindow->viewListenChance->setDistribution(&linkParams.linkListenChance);
		linkParametersWindow->viewListenChance->activate();
		linkParametersWindow->boxListenChance->activate();

	}
	else {
		linkParametersWindow->viewListenChance->deactivate();
		linkParametersWindow->boxListenChance->deactivate();
	}

	// turn on/off link trust controls
	linkParametersWindow->sliderVaryTrust->value(2 - linkParams.varyTrust);
	if(linkParams.varyTrust == VARY_INDIVIDUALLY) {
		linkParametersWindow->viewTrust->setMetaDistribution(&linkParams.linkTrust);
		linkParametersWindow->viewTrust->activate();
		linkParametersWindow->boxTrust->activate();
	}
	else {
		linkParametersWindow->viewTrust->deactivate();
		linkParametersWindow->boxTrust->deactivate();
	}

	// turn on/off link threshold controls
	linkParametersWindow->sliderVaryThreshold->value(2 - linkParams.varyThreshold);
	if(linkParams.varyThreshold == VARY_INDIVIDUALLY) {
		linkParametersWindow->viewThreshold->setDistribution(&linkParams.linkThreshold);
		linkParametersWindow->viewThreshold->activate();
		linkParametersWindow->boxThreshold->activate();
	}
	else {
		linkParametersWindow->viewThreshold->deactivate();
		linkParametersWindow->boxThreshold->deactivate();
	}


}

//-----------------------------------------------------------------------------------------------------------------------

TiXmlElement* SocietySetup::toXML(t_int stage) const {
	char name[255];
	sprintf(name, "SOCIETY_SETUP_%d", stage);
	TiXmlElement *s =  new TiXmlElement(name);
	if(varyPopulation) s->SetAttribute("VARY_POPULATION", "true");
	else s->SetAttribute("VARY_POPULATION", "false");
	s->SetDoubleAttribute("INITIAL_POPULATION_PART", initialPopulationPart);
	s->SetDoubleAttribute("GROWTH_BALANCE", growthBalance);
	if(varyLinks) s->SetAttribute("VARY_LINKS", "true");
	else s->SetAttribute("VARY_LINKS", "false");
	if(includeInStatistics) s->SetAttribute("INCLUDE_IN_STATISTICS", "true");
	else s->SetAttribute("INCLUDE_IN_STATISTICS", "false");
	if(updateInquiryTrust) s->SetAttribute("UPDATE_INQUIRY_TRUST", "true");
	else s->SetAttribute("UPDATE_INQUIRY_TRUST", "false");
	if(limitLinksToOnePerPair) s->SetAttribute("LIMIT_LINKS_TO_ONE_PER_PAIR", "true");
	else s->SetAttribute("LIMIT_LINKS_TO_ONE_PER_PAIR", "false");
	if((linkDistributionMethod & LDM_MASK) == LDM_TOTAL) s->SetAttribute("LINK_DISTRIBUTION_METHOD", "total");
	else if((linkDistributionMethod & LDM_MASK) == LDM_PER_INQUIRER) s->SetAttribute("LINK_DISTRIBUTION_METHOD", "per inquirer");
	else s->SetAttribute("LINK_DISTRIBUTION_METHOD", "per inquirer squared");
	if(linkDistributionMethod & LDM_TO_NUMBER_BIT) s->SetAttribute("LINK_COUNT_METHOD", "number");
	else s->SetAttribute("LINK_COUNT_METHOD", "density");

	s->SetDoubleAttribute("LINK_WEIGHT_BASE", linkWeights[WT_BASE]);
	s->SetDoubleAttribute("LINK_WEIGHT_SYMMETRY", linkWeights[WT_SYMMETRY]);
	s->SetDoubleAttribute("LINK_WEIGHT_TRANSITIVITY", linkWeights[WT_TRANSITIVITY]);
	s->SetDoubleAttribute("LINK_WEIGHT_CLUSTERING", linkWeights[WT_CLUSTERING]);

	s->SetAttribute("NEW_EVIDENCE_REQUIREMENT", EvidenceStr(evidencePolicy));
	if(countPriorAsEvidence) s->SetAttribute("COUNT_PRIOR_AS_EVIDENCE", "true");
	else s->SetAttribute("COUNT_PRIOR_AS_EVIDENCE", "false");
	if(updateTrust) s->SetAttribute("UPDATE_TRUST", "true");
	else s->SetAttribute("UPDATE_TRUST", "false");


	// add distributions
	s->LinkEndChild(populationDistribution.toXML("POPULATION_DISTRIBUTION"));
	s->LinkEndChild(linkDensityDistribution.toXML("LINK_DENSITY_DISTRIBUTION"));

	// add inquirer & link parameters
	s->LinkEndChild(inqParams.toXML());
	s->LinkEndChild(linkParams.toXML());

	return s;
}

//-----------------------------------------------------------------------------------------------------------------------
