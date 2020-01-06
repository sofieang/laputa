#ifndef __BATCHSIMULATION_H__
#define __BATCHSIMULATION_H__

#include "Prefix.h"
#include "Simulation.h"
#include "Topology.h"

// how many steps to take per simulation call?
#define BATCH_SIMULATION_TIMEOUT 1.0

// max stages in a batch simulation
#define MAX_BATCH_STAGES 4

class BatchStatistics {
public:
	t_float totalEValue, totalEValueDelta;
	t_float totalEValueS, totalEValueDeltaS;
	t_float totalPolarisation, totalPolarisationDelta;
	t_float totalPolarisationS, totalPolarisationDeltaS;
	vector<t_float> avgEValueOverTime;
	t_float avgMessagesSentTotal, avgMessagesSentPerInquirer;
	t_float avgInquiryResultsTotal, avgInquiryResultsPerInquirer;
	vector<t_float> degrees[3];

	// bandwagon measurement
	t_float avgBWToPEffect, avgBWToNotPEffect, avgBWToPProb, avgBWToNotPProb;
    
	// block to store topology data
	vector<NetworkTopology> topologies;
	t_int societiesPerTopology;
	bool recordTopologies;

    // block to store history of all trials & inquirers over time
	StatisticsBlock eValuesOverTime;   // inquirers x time x trials
	t_int timePerEValueStat, societiesPerEValueStat;
	bool recordEValueStats;

};

// Batch Simulation class - keeps track of statistics during a batch simulation

class MultiBatch;

class BatchSimulation {
public:
	// constructor & destructor
	BatchSimulation();
	BatchSimulation(const BatchSimulation& bs1, const BatchSimulation& bs2, t_float v);
	BatchSimulation(TiXmlElement* xml);
	~BatchSimulation();
	BatchSimulation& operator=(const BatchSimulation& bs);
	void setDefault(MultiBatch* mb = 0);
	TiXmlElement* toXML(const char* name = 0);
	string getDescription(void);
	
	// run simulation
	void process(void);
	void setupTrials(void);
	
	// save statistics to file
	void saveStatisticsToFile(const char* filename);
	
	// get a string comparing this batch to another
	string compareTo(const BatchSimulation& bs);
	
	// batch simulation variables
	t_int nTrials = 0, nSteps[MAX_BATCH_STAGES] = { 0, 0, 0, 0 }, nStages = 0;
	t_int curTrial = 0, curStage = 0;
	
	// society setup to use
	SocietySetup setup[MAX_BATCH_STAGES];
	
	// society template
	Society *templateSociety;
	
	// simulation structure, used for each society generated
	Simulation sim;
	
	// statistics variables
	BatchStatistics stats;

	// should we display the results obtained in the statistics window?
	bool displayResults;
	
	// do we need to update display to not seem irresponsible?
	bool timeOut;
	
	t_int totalSteps(void) {
		t_int s = nSteps[0];
		for(t_int i = 1; i < nStages; ++i) s += nSteps[i];
		return s;
	}
	
	t_int maxInquirers(void);
	void recordTrialEndStatistics(void);
	void recordFinalStatistics(void);
};


void BatchProcess(void *data);
void StartBatchSimulation(BatchSimulation* bs);
void BatchSimulationTimeout(void* data);


#endif
