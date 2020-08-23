using namespace std;
#include"externalLibraries.h"
#ifndef CLASS_RFIFILTERING
#define CLASS_RFIFILTERING
/*******************************************************************
CLASS: RFIFiltering
*Handles detecting and flagging Radio Frequency Interference (RFI) in
*input data.
*Contains methods to find the central tendency as well as the rms of 
*the current block. Can be expanded to include multiple algorithms
*to do so.
*It also generates and writes flags.
*******************************************************************/
class RFIFiltering
{
	
	public:
	float histogramInterval;
	float*	input;			//Input array from which outliers are to be detected
	int	inputSize;		//Size of input array
	float	inputMax;		//Maximum and minimum elements of the array. Used by certain algorithms.
	float	inputMin;		
	float	centralTendency;	//Computed central tendency of the input data.
	float	rms;			//Computer rms of the input data.
	float	cutoff;			//Threshold above which samples are flagged.
	float	cutoffToRms;		//Threshold to rms ratio.
	char*	flags;			//Array where flags are stored. 1 implies outlier. 0 implies normal.
	char*   flagsFilt;
	float*  sFlags;
	float*  smoothInput;
	//These variables are used when histogram based filtering is done
	float*	histogram;
	float*	histogramAxis;
	int 	histogramSize;
	int	histogramMax;
	
	
	RFIFiltering(float* input_,int inputSize_);			//Constructor	
	~RFIFiltering();						//Destructor
	void computeStatistics(int algorithmCode);			//Wrapper to call appropiate function to find the central tendency and rms
	void smoothFlags(int windowLength,float threshold);
	void smooth(int windowLength);
	void flagData();						//Function to generate flags once rms and central tendency has been found
	void multiPointFlagData(float* multiCutoff);
	void generateBlankFlags();					//Generates blank flags in case of no flagging
	void writeFlags(const char* fileName);				//Writes out flags to a file
	void writeFlags(const char* filename,char* startFlags,int nStartFlags,char* endFlags,int nEndFlags); //Writes out flags to a file with startFlags and endFlags appeneded at the beginning and end respectively.
	void generateManualFlags(int nBadChanBlocks,int* badChanBlocks,int offset);	//flags user specified blocks
	void flagWidthThreshold(int onWidth,int offWidth);
	void subbandFlagging(int nSub,int nSubdetect,int noOfChannels,int startChannel,int stopChannel,float *rawData,char *chanFlags);
	private:
	void histogramBased();						//Finds mode for central tendency and rms based on finding the distribution
	void MADBased();						//Finds median for central tendency and median absolute deviation for rms
	void quicksort(float* x,long int first,long int last);		//Sorting routine used by MADBased() to sort the input
	
};
#endif
