LAPUTA README

Last Updated Jan 5 2020


OVERVIEW
========
Laputa is a program designed for simulations in social epistemology. It is released under the GPL, v.3.
All code / data copyright is by the corresponding author(s). It is provided, together with its source code,
to facilitate research into the attainment and spread of knowledge in society. Laputa is dedicated to
the memory of the Vienna circle.


BUILDING
========
Building on both Windows and Linux uses CMake.

On Linux:
  mkdir build
  cd build
  cmake -DCMAKE_BUILD_TYPE=Release ..
  make
  sudo make install

On Windows:
  Use the CMake GUI to generate a VS project from CMakeLists.txt.


LIBRARIES
=========
Laputa relies on 7 external libraries, 4 of which are included as source, and 3 which are downloaded by
CMake:

FLTK: This is a cross-platform library for user interface design and handling, used in order to facilitate
ease of porting between different platforms such as Mac, Windows, and Linux. Laputa also uses the interface
builder (called "fluid") of FLTK. More information is available at http://www.fltk.org.

GSL: This is the GNU Scientific Library, an open sourced library for scientific calculations. Laputa uses
its random number generators,  its random distribution handling, and to some part its optimization functions.
It is however likely that more of the GSL may be used in the future, as more functionality is added to
Laputa. More information is available at http://www.gnu.org/software/gsl/.

muParserX: a library for parsing mathematical expressions. Just used when editing distributions at the moment,
but may be used more in the future.

mempool: A block allocator used for the Mac version. Included as source.

minizip: a library for opening and writing .zip files. This is necessary to save files in OpenDocument
format. Included as source.

TinyXML: an XML parser used for reading and writing files in XML format. More information at
http://sourceforge.net/projects/tinyxml/. Included as source.

zlib: Well known. It is included in FLTK, but for some reason still seems to be necessary to include
as source in the project.


SOURCE CODE
===========
The following is an overview of the source code for Laputa, meant to make it easier for developers to
familiarize themselves with the project. The program is built around an "App" class, which controls the
application interface, and a "Society" class, which controls the representation of a society. In addition
to these, there are numerous other classes which these classes use. The code uses global variables for
important objects that are accessed often (such as the application object, the current society, and
various windows) rather than the more politically correct singleton class construction.

Directories
-----------
/build: The target for all builds, both on Mac and Windows. Also contains the "non-code" data, i.e. the
folders "docs" and "data", which Laputa needs to run. In the Mac version, these are also included in
the .app bundle.

/src:     Contains the .h files.
/images:  Contains PhotoShop format images to facilitate easier editing of them. Images in .png and
          .jpg format are in the folder /build/data.
/lib:     Contains the external libraries Laputa needs to compile.
/src:     Contains the .cpp files.
/ui:      Contains files pertaining to the user interface, among which is "UserInterface.fld", which
          is used by the Fluid interface builder in FLTK.
/web:     Contains files used in the web site.

Source files
------------
Amount: Contains the class Amount, which can represent numbers in [0, 1] with much greater accuracy than
        standard IEEE floating point numbers can. Most importantly, the precision in an Amount is the same
        around 0 as it is around 1, which is crucial in order not to introduce asymmetries in when a
        degree of belief is rounded to 0 or to 1.
App: Contains the application class, and its instance "app", which controls various things related mainly
     to user interface and to preferences for tools (such as the "Create Inquirer" tool).
BatchSimulation: Contains the BatchSimulation class, which is used to generate random societies and run
                 simulations on them.
BatchSimulationWindow: Contains code for showing and interacting with the batch simulation dialog. In its
                       own file since that dialog is so complex.
CorrdinateSelector: A small widget that is used in the batch simulation window in order to pick two
                    parameters at once.
Distribution: Contains classes (Distribution, MetaDistribution, DistributionManager) which deal with random
              distributions. A MetaDistribution is a distribution over distributions, and the
              DistributionManager, through its instance "dm", is used to keep track of random distributions by
              name.
DistributionView: Contains classes that handle the drawing of random distributions and metadistributions,
                  and interaction with them.
ExpressionField: A widget that interfaces with the muParser.
Files: Utility functions to save data in OpenDocument spreadsheet format.
Inquirer: Contains the Inquirer class, which represents a bayesian inquirer in the model used. The method
          "doInquiry" implements the belief update model, as well as the investigative behaviour of the inquirer.
Inspectors: The windows used to inspect inquirers and links.
Link: Contains the Link class, which represents a communication link between inquirers.
main: As usual.
MetaDistribution: A class that stores 3 distributions, of which one determines a parameter. New distributions
                  can then be generated either randomly from that parameter, or by specifying the parameter
                  explicitly.
MultiBatch: Stores 4 batch simulations, and allows making new batch simulations by interpolating between these.
            Useful to be able to cover a whole parameter space.
Parameters: Contains the InquirerParameters and LinkParameters classes, which are contained in the Inquirer
             and Link classes, respectively. These describe the specific behaviour of the inquirer/link under
             batch simulations.
Simulation: Contains the Simulation and BatchSimulation objects, that are used to keep track of statistics
            collected during a simulation.
Society: Contains the Society object, and a helper object "SocietyFragment", which is used when copying and
         pasting parts of a society. The Society object contains the inquirers, and has methods that concern
         handling of these as groups. Most important is the "evolve" method, which calls the "doInquiry" methods
         of all inquirers, and then updates belief values. The society currently used is referenced through the
         global variable curSociety.
SocietySetup: The SocietySetup class describes settings for generating random societies, and is used for batch
              simulations.
SocietyView: This class handles drawing and interacting with the society object.
StatisticsBlock: Class for keeping track of statistics gathered from batch simulations. Allows some simple
                 operations, such as sorting and averaging, to be done on the data.
StatisticsView: This class draws bar or line graph statistics. Also contains code for interfacing with gsl
                to fit curves to the statistics block displayed.
ToolButton: a type of button used for the tools.
Topology: a small class to store the topology of a network. It is used when exporting data to Pajek.
Trust: Contains TrustFunction and TrustView. The first of these represents an inquirer's trust in her sources,
       and the second is used to display and interact with the first.
UserInterface: This file is generated by the FLTK "fluid" program. It should not be edited manually!
UserInterfaceItems: A file that contains all kinds of things pertaining to the user interface that did not fit
                    in anywhere else. Contains the classes for most windows/dialogs in the program, as well as
                    a DialogFactory, which is a class used to show and hide dialogs.
Utility: Some functions that don't really fit anywhere else.

Apologies for the messy code in some (actually a lot of) places. Laputa grew out of a prototype, and should
have been refactored or rewritten from scratch a long time ago. Who knows if that will happen sometime?
