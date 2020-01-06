#include "App.h"
#ifdef __APPLE__
#include <unistd.h>
#endif
#ifdef _WINDOWS
#include <cstdlib>
#include <direct.h>
#endif
#ifdef __linux__
#include <cstdlib>
#endif
#include <FL/filename.H>

//--------------------------------------------------------------------------------------------

void FindDirectories(const string& thisDirPath, const string& executableName, string& dataPath, string& docsPath) {
#ifdef __APPLE__
  dataPath = thisDirPath + executableName + string(DATA_PATH_PRIMARY);
  docsPath = thisDirPath + executableName + string(DOCS_PATH_PRIMARY);
#endif
#ifdef _WINDOWS
  dataPath = thisDirPath + string(DATA_PATH_PRIMARY);
  docsPath = thisDirPath + string(DOCS_PATH_PRIMARY);
#endif
#ifdef __linux__
  if(fl_filename_isdir(DATA_PATH_PRIMARY) && fl_filename_isdir(DOCS_PATH_PRIMARY)) {
    dataPath = string(DATA_PATH_PRIMARY);
    docsPath = string(DOCS_PATH_PRIMARY);
  }
  else {
    dataPath = thisDirPath + string(DATA_PATH_SECONDARY);
    docsPath = thisDirPath + string(DOCS_PATH_SECONDARY);
  }
#endif

  // Check that directories exist
  if(!fl_filename_isdir(dataPath.c_str())) {
#ifdef __linux__
    fl_alert("Cannot find the data directory. It must reside either in /usr/local/share/laputa/ or with the program.");
#else
    fl_alert("Cannot find the data directory. It must reside with the program.");
#endif
    exit(1);
  }
}

//--------------------------------------------------------------------------------------------

int main(int argc, char* argv[]) {
	// set working directory
 	char directoryPath[FL_PATH_MAX], executableName[FL_PATH_MAX];
  strcpy(directoryPath, argv[0]);
  strcpy(executableName, fl_filename_name(directoryPath));
  *(char*)fl_filename_name(directoryPath) = 0;

#ifdef __APPLE__
  // are we inside a bundle?
  if(!strcmp(directoryPath + strlen(directoryPath) - 16, "/Contents/MacOS/")) {
    directoryPath[strlen(directoryPath) - 16] = 0;
    strcpy(executableName, fl_filename_name(directoryPath));
	  *(char*)fl_filename_name(directoryPath) = 0;
    chdir(directoryPath);
  }
#endif

  // Find data directories and load default distributions
  string dataPath, docsPath;
  FindDirectories(directoryPath, executableName, dataPath, docsPath);
  string distrPath = dataPath + "distributions/";
	LoadDefaultDistributions(distrPath.c_str());

	// create society
	curSociety = new Society();

	// create & run application object
	app = new App(argc, argv, dataPath, docsPath);
	return app->run();
}

//--------------------------------------------------------------------------------------------
