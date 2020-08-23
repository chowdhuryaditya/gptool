using namespace std;
#include"externalLibraries.h"
#include"Information.h"
#ifndef CLASS_ADVANCEDANALYSIS
#define CLASS_ADVANCEDANALYSIS
/*******************************************************************
CLASS: AdvancedAnalysis
*This is the class that does all the final analysis on the data.
*It has a routine to dedisperses the data and then collapses 
*the frequency channels creating a Dedispersed time series (fullDM). 
*It also performs folding of the data by using defualt fixed period
*or by using a polyco file (not yet implemented).
*******************************************************************/
class AdvancedAnalysis
{
	public:
		//Static variables:
		static Information 	info;			//contains all input parameters
		static unsigned long*	delayTable;		//An array that stores the delay of each frequency channel
		static unsigned long	maxDelay;		//The maximum delay encountered
		static double		initPhasePolyco;	//Initial phase for polyco folding
		//The following three variables points to different arrays(variables), one for each polarization mode.
		static float** 		foldedProfile;		//Pointers to arrays containing the folded profiles
		static int**		countProfile;		//keeps track of number of data points in each bin	
		
		static double*		curPosMsStatic;		//Time upto which time series have been folded.
		static double		initLagMs;		//initial lag before folding is started.
		//The following three variables points to different arrays(variables), one for each polarization mode and are used to store unfiltered data.
		static float** 		foldedProfileUnfiltered;		//Pointers to arrays containing the folded profiles
		static int**		countProfileUnfiltered;			//keeps track of number of data points in each bin	

				
		//Variables:
		long int	blockIndex;		//Index of current block(window) being processed.
		int		polarIndex;		//Index of polarization to process. Decides foldedProfile, countProfile & curPosMsStatic indexes.
		double		curPosMs;		//Time upto which time series have been folded for the CURRENT polarization.
		int		polycoRowIndex;		//Row index of polycoTable for polyco based folding
		float*		rawData;		//2-D time frequency data to process
		long int 	length;			//length of time series to process
		float*		fullDM;			//Array that stores the dedispersed time series
		int*		count;			//Number of data points in each bin of fullDM array
		char*		dedispFlags;
		float*		excess;			//The excess from the previous block (details given later)
		int*		countExcess;		//Number of data points in each bin of excess array
		float*		curFoldedProfile;	//Mean Folded profile of the current polarization, upto the current window (used for plotting)
		char		hasEnoughDedispersedData; //Used to tell the folding routine when to start profile calculation.
		int		foldingStartIndex;	  ///Used to comunicate the start index of first block for which dedispersed data is available across all channels.
		//Maximum and minimum of the profile array and the fullDM array. Used for plotting.
		float 		maxProfile;		
		float 		minProfile;
		float		maxFullDM;
		float		minFullDM;
		
		//Following are the unfiltered counterpart of the above variables:

		float*		fullDMUnfiltered;			//Array that stores the dedispersed time series
		float*		excessUnfiltered;			//The excess from the previous block (details given later)
		int*		countUnfiltered;			//Number of data points in each bin of fullDM array
		int*		countExcessUnfiltered;		//Number of data points in each bin of excess array
		float*		curFoldedProfileUnfiltered;	//Mean Folded profile of the current polarization, upto the current window (used for plotting)
		float 		maxProfileUnfiltered;		
		float 		minProfileUnfiltered;
		float		maxFullDMUnfiltered;
		float		minFullDMUnfiltered;

		//Functions:
		AdvancedAnalysis(Information info_);	//Constructor for first intialization
		AdvancedAnalysis(long int blockIndex_,int polarIndex_,float* rawData_,long int length_); //constructor
		~AdvancedAnalysis();	//Destructor
		void calculateDelayTable();	//Calculates the delay table, a table containing shifts (in number of samples) of each channel.
		void calculateFullDM(char* timeFlag,char *freqFlag); //Calculates the dedispersed time series
		void calculateFullDM(short int* filteredRawData); //Calculates the dedispersed time series for replaced by median DM.
		void mergeExcess(float* excess_,int* countExcess_,float* excessUnfiltered_,int* countExcessUnfiltered_);
		void normalizeFullDM();
		void calculateProfile();	//Calculates the folded profile
		void writeProfile(const char* filename,const char* filenameUnfiltered);	//Writes out the folded profile
		void writeFullDM(const char* filename,const char* filenameUnfiltered);		//Writes out the dedispersed time series		
		void writeFullDMCount(const char*  filename);	//Writes out the number of samples in each dedispersed time series bin

		private:
		double calculateFixedPeriodPhase();		//Calculates phase of current sample for folding (based on a given fixed period)
		double calculatePolycoPhase();			//Calculates phase of current sample for folding (based on a polyCo file)
	
	
};
#endif
