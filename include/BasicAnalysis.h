
#include"externalLibraries.h"
#include"Information.h"
using namespace std;

#ifndef CLASS_BASICANALYSIS
#define  CLASS_BASICANALYSIS
/*******************************************************************
*CLASS: BasicAnalysis
*Performs basic operations like bandshape and zeroDM time series 
*computation for a particular window and a particular polarization.
*These quantities are then used by objects of RFIFiltering class 
*to find and filter Radio Frequency Interference (RFI).
*******************************************************************/
class BasicAnalysis
{
	public:
	//static variables:
	
	static	Information	info;			//contains all input parameters
	static double**		smoothSumBandshape;	//Cumulative smooth bandshape for each polarization
	static double** 	sumBandshape;		//Cumulative bandshapes for each polaization
	static double**		cumSquareBandshape;	//Cumulative square of bandshape for each polarization
	static long long int** 	countBandshape;		//Number of sample that goes into each frequency bin (Cumulative) for each polaization
	static float**		externalBandshape; 	//Externally loaded bandshape
	long int		blockLength;	
	
	//variables:
	float 			cumBandshapeScale;		//Used to scale down the cumulative bandshape to avoid overflow issues
	int			polarIndex;			//Index of polarization to process.
	float			*rawData;			//The 2D time-frequency data 	
	short int		*filteredRawData;		//The Filtered 2D time-frequency data 
	unsigned char		*filteredRawDataChar;		//The Filtered 2D time-frequency data (Single byte)
	float			*zeroDM;			//Time series obtained by collapsing all frequency channels (Without dedispersion)
	float			*zeroDMUnfiltered;		//Time series obtained by collapsing all frequency channels (Without dedispersion), without filtering
	float			*bandshape;			//Mean bandshape obtained by collapsing all time samples.
	float			*meanToRmsBandshape;		//Mean to rms computed for each channel. (rms of time series for that channel)
	double			*squareBandshape;		//Mean of bandpass squared for each channel. (rms of time series for that channel)
	float			*smoothBandshape;		//smoothened bandshape obtained by moving mean or median
	float			*normalizedBandshape;		//bandshape normalized using smoothBandshape.
	float			*correlationBandshape;
	char			*headerInfo;			//corresponding header information - used only in INLINE mode
	//Minimum and maximum of each array. Used in plotting.	
	float 		minZeroDM;		
	float		maxZeroDM;
	float		maxBandshape;
	float		minBandshape;
	float		maxMeanToRmsBandshape;
	float		minMeanToRmsBandshape;
	float		minNormalizedBandshape;
	float		maxNormalizedBandshape;
	int 		curBandpassCount;				//No of time series in current bandshape;
	//Functions:
	BasicAnalysis(Information _info);
	BasicAnalysis(float* _rawData,int polarIndex_,long int _blockLength);
	~BasicAnalysis();
	void computeZeroDM(char* freqFlags);							//Computes zeroDM with given frequency flags
	void computeZeroDMNorm(char* freqFlags,float *cumulativeBandshape);
	void computeBandshape();								//Computes bandshape
	void computeBandshape(char* timeFlags);							//Computes bandshape with given time flags
	void calculateCumulativeBandshapes();							//Computes the global cumulative bandshapes
	void calculateMeanByRMSBandshape();
	void quicksort(float* x,long int first,long int last);					//Used to compute smooth bandshape using moving median 
	void smoothAndNormalizeBandshape(); 							//Smoothens and normalizes the bandshape
	void normalizeBandshape();
	void setNormalizedBandpass(float *smoothBandshapeRR,float *smoothBandshapeLL);		// Set bandshape using RR and LL bandshapes for cross terms in full stokes.
	void normalizeData();	
								//normalizes 2-D data
	void getFilteredRawData(char* timeFlags,char* freqFlags,float replacementValue);	//gets Filtered Raw Data.
	void getFilteredRawDataSmoothBshape(char* timeFlags,char* freqFlags);
	void getFilteredRawDataChar(char* timeFlags,char* freqFlags,float replacementValue);	//gets Filtered Raw Data (single byte version)
	void getFilteredRawDataSmoothBshapeChar(char* timeFlags,char* freqFlags);	
	void subtractZeroDM(char* freqFlags,float centralTendency);
	void writeBandshape(const char*  filename);						//Writes out the cumulative mean and rms a bandshape	
	void writeCurBandshape(const char* filename);						//Writes out current bandshape
	void writeFilteredRawData(const char*  filename);					//Writes out filtered 2D data
		
};
#endif CLASS_BASICANALYSIS
