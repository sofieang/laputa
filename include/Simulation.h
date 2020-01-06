#ifndef __SIMULATION_H__
#define __SIMULATION_H__

#include "Prefix.h"

#include "SocietySetup.h"
#include "Society.h"
#include "StatisticsBlock.h"

#include <vector>
#include <set>

// how many steps to take per simulation call?
#define STEPS_PER_SIMULATION_STEP 1500
//#define STATISTICS_RESOLUTION 100

// application methods for evalue calculation
#define APPLY_INDIVIDUALLY 0
#define APPLY_TO_AVERAGE 1
#define APPLY_TO_MAJORITY 2

// log handling
#define LOG_NONE 0
#define LOG_SUMMARY 1
#define LOG_STANDARD 2
#define LOG_DETAILED 3



#define LOG_BUFFER_SIZE 0x7FFF

//-----------------------------------------------------------------------------------------------------------------------

class ValuationMethod {
public:
	t_int applicationMethod;
	t_float eValues[3];
    t_float exponent;
	t_float majorityAmt, majorityPCert, majorityNotPCert;
	bool amtStrictlyGreater, blfPStrictlyGreater, blfNotPStrictlyLess;
};

//-----------------------------------------------------------------------------------------------------------------------



// Simulation class - keeps track of statistics during a single simulation

class Simulation {
public:
	// constructor & destructor
	Simulation();
	Simulation(const Simulation& sim1, const Simulation& sim2, t_float v);
	Simulation(TiXmlElement* xml);
	~Simulation() { delete[] logMsg; }
	string compareTo(const Simulation& sim);
	string getDescription(void);

	// XML
	TiXmlElement* toXML(void) const;

	// simulation functions
	void step(t_int nStepsToTake = 1, t_int timePerEValue = 1);
	void setTime(t_int t);
	t_int getTime(void) {return curStep;}
	void stepTime(void);

	// reset simulation variables
	void reset(void);

	// fill out simulation window
	void setSimulationWindowFrom(void);

	// Calculation of epistemic value
	inline t_float instantEValue(void);
	t_float individualEValue(t_float blf);
	t_float instantPolarisation(t_float ev);

	// write message in log
	void addToLog(const char* msg);
	void writeLog(void);

	// which society are we simulating?
	Society* soc;

	// simulation variables
	t_int curStep;
	t_float eValue, eValueDelta, eValueTotal, eValueDeltaTotal, startEValue;
	t_float polarisation, polarisationDelta, startPolarisation;
	t_int msgSent, inqResults;

	// bandwagon measurement
	t_float bwTowardsP, bwTowardsNotP;
	t_int inqOverriddenTowardsP, inqOverriddenTowardsNotP;

	// block to enter individual eValues into
	StatisticsBlock eValuesOverTime;

	// how much to show in log
	t_int logLevel;

	// how to calculate evalue
	ValuationMethod val;

	// logging message being built
	char *logMsg;
	t_int logMsgSize;
};

// simulation idle functions
void RunSimulation(void *data);



#endif
