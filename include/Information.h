#ifndef CLASS_INFORMATION
#define CLASS_INFORMATION
#include"externalLibraries.h"
using namespace std;
/*******************************************************************
*CLASS:	Information
*Stores all the input parameters to gptool. At run-time there is one 
*instance of this class being shared by all other objects that does 
*various operations on the data. 

*Also has an member function that read from a gptool.in file and 
*intiliazes their variables. 
*******************************************************************/
class Information
{	
	
	public:
	
	//variables
	string			pulsarName;
	string			modeOperation; 		//PA or IA
	float			lowestFrequency;
	float			bandwidth;
	int			noOfChannels;
	int 			startChannel; 		//Channels before startChannel are ignored by the program.
	int 			stopChannel;		//Channels after stopChannel are ignored by the program.
	double			samplingInterval;	//In seconds
	double			periodInMs;		//Pulsar period
	int			periodInSamples;
	float			profileOffset;		//Phase (0 to 1) by which the profile will be shifted
	float 			dispersionMeasure;	//in pc/cc
	float			blockSizeSec;		//Size of each window in seconds
	long int		blockSizeSamples;	//Size of each window in number of samples
	int			sampleSizeBytes;	//Stores the size in bytes of each sample. Usual GMRT data is sampled to 2byte integers.
	char			doFixedPeriodFolding;	//0-> use polyco based folding 1-> use fixed period to fold
	int*			badChanBlocks;		//manually entered list of bad channel blocks
	int			nBadChanBlocks;		//number of such bad blocks
	unsigned short		meanval;
	int			shmID;			//1-> standard correlator shm; 2-> gptool file shm; 3-> gptool inline shm
	//Parameters for polyco folding
	char*			psrcatdbPath;		//Path to psrcat database.
	double			MJDObs;			//Mean Julian Day at the start of observation
	int			spanPolyco,maxHA; 	//time span (in minutes) of each polyco set
	int			nCoeffPolyco;		//number of coefficients in each polyco set
	double*			polycoTable;		//relevent parameters from polyco.dat
	int 			polycoRowIndex;		//index number of correct polyco set.
	char			sidebandFlag;		//A sideband flag of 0 indicates frequencies decreasing with channel number. 1 is for the reverse scenario.
	char*			filepath;		//In offline mode, the path of the raw data file.
	string		outputfilepath;	//path where filtered raw data is written out
	string			filename;		//Name of raw data file in offline mode
	char			noOfPol;		//Number of polarization channels. Usually 4.
	char			polarChanToDisplay;	//Number of polarization channels to display.
	//Various flags:
	
	char			doReadFromFile;		//0-> Real time processing;	1-> Reading from file	
	char			doManualMode;		//0-> Automatic mode;		1-> Manual mode
	char			doWindowDelay;		//0-> No delay processing	1-> Adds artificial waiting time after each window to emulate real time 
	char			doWriteFullDM;		//1-> Write dedispersed series
	char			doWriteFiltered2D;	//1-> Write filtered 2D data
	char			doPolarMode;		//1-> Polarization data
	char			doReplaceByMean;	//1-> Flagged samples replaced by value of central tendency (either median or mode depending on algorithm chosen)
	char			doFilteringOnly;	//1-> Will perform filtering and write out filtered 2D time freq data	
	char			refFrequency;		//Dedisperse w.r.t 1-> Highest frequency 0-> Lowest frequency
	char			doUseTempo2;		//Sets the flag to decide whether to use tempo2 or tempo1
	char			isInline;		//inline mode of gptool - read and write to SHM
	char			doFRB;			//inline mode of gptool for FRB pipeline
	//Time filtering information:
	char			doTimeFlag;		//0-> Time flagging off		1-> Time flagging on
	char			timeFlagAlgo;		//1-> Histogram based		2-> MAD based
	char			doTimeClip;		//0-> Time sample clipping off	1-> On
	char			doMultiPointFilter;	//0-> single point filter	1-> Multi-point filter
	char			doUseNormalizedData;	//0-> w/o normalization 	1-> w normalizations
	float			timeCutOffToRMS;	//Cutoff to rms ratio
	float*			cutoff;			//multi point rms cutoffs for time filtering stored here
	char			doWriteTimeFlags;	//0-> Flag file not written	1-> File flag written
	float			smoothFlagWindowLength;
	float			concentrationThreshold;
	char 			doFlagWidthThreshold;
	int 			flagWidthOn;
	int			flagWidthOff;
	char 			doSubbandFiltering;
	int			nSubFilt;
	int			nSubDetect;
	//Channel filtering information:
	char			doChanFlag;		//0-> Chan flagging off		1-> Channel flagging on
	char			chanFlagAlgo;		//1-> Histogram based		2-> MAD based
	char			doChanClip;		//0-> Channel clipping off	1-> On
	char			bandshapeToUse;		//1-> Mean Bandshape		2-> Normalized bandshape	3->Mean-to-rms bandshape 4-> Both 2 and 3
	char			doZeroDMSub;		//0-> No zero DM subtraction 1-> On
	char			doRunFilteredMode;	//0-> Off 1-> Filtered mode run with no filtering
	float			chanCutOffToRMS;	//Cutoff to rms ratio
	char			doWriteChanFlags;	//0-> Flag file not written	1-> File flag written
	int			smoothingWindowLength;	//The window length of moving median smoothing.
	char			flagOrder; 		//1->channel first;2->time first;3->independent	
	char			normalizationProcedure; //1-> Cumulative bandshape, 2-> Externally supplied bandshape.dat
	double 			startTime;		//For file read the start time (used to skip some initial data blocks
	double			startBlockIndex;	//Block index corresponding to the start time
	//Functions:
	double stringToDouble(const std::string& s);	//Converts strings to double, used to take input from .in file
	void readGptoolInputFile();			//Function to read from .in file
	short int checkGptoolInputFileVersion();	//Checks if gptool.in has the latest version formatting.
	void reformatGptoolInputFile();			//reformats gptool.in file to latest version
	void cutOffCalculations();			//calculates the double,triple and quadrapule point cutoffs
	void display();					//Displays all input information to the user
	void writeWpmonIn();				//Writes out a gptool.in sample when it is not found.
	void writeInfFile();				//Writes out INF file to be used by presto
	void displayNoOptionsHelp();			//Displays possible ways to run gptool
	void genMJDObs();				//Generates mean julian day at the start of observation
	void getPsrcatdbPath();				//Gets the location of psrcat database.
	void genPolycoTempo1();				//Fires tempo1 to generate polyco.dat
	void genPolycoTempo2();				//Fires tempo2 to generate polyco.dat
	void loadPolyco();				//loads information from polyco.dat to double* polycoTable
	void checkPulsarName();				//Verifies if the pulsar name is valid
	void autoDM();					//Gets dispersion measure from psr cat
	void calculateCutoff();
	void errorChecks(); 		//Checks for error in the .in file
	void fillParams();
	void parseManFlagList(std::string& s);	//parses list of bad sub-bands
};
#endif
