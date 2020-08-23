using namespace std;
#include"externalLibraries.h"
#include <BasicAnalysis.h>
#include <RFIFiltering.h>
#include <BasicAnalysis.h>
#include <AdvancedAnalysis.h>
#ifndef CLASS_PLOT
#define CLASS_PLOT
class Plot
{
	public:
		static float*		profileAxis;
		static float* 		DMAxis;
		static float* 		timeAxis;
		static float*		bandshapeAxis;
		static Information	info;	
		static int		totalBlocks;
		Plot(Information info_,int totalBlocks_);
		void plotProfile(float *profile,float maxProfile,float minProfile,int k);
		void plotProfileUnfiltered(float *profile,float maxProfile,float minProfile);
		void plotFullDM(float *fullDM,float flMin,float flMax,int length,int k);
		void plotZeroDM(float *zeroDM,float zrMax,float zrMin,int length,int k);
		void plotBandshape(float *bandshape,float maxBandshape,float minBandshape,int k);
		void plotWaterfall(float* rawData,long int blockLength,float maxRawData,float minRawData);
		void plotFullDMCount(int* count,long int blockLength);
		void plotOtherBandshape(float *meanToRmsBandshape,float maxMeanToRmsBandshape,float minMeanToRmsBandshape,int k);
		void plotFullDMUnweighted(float* fullDM,int *count,long int length);
		void plotHistogram(float* histogram,float* histogramAxis,float histogramSize,float histMax);
		void plotChanFlags(char* chanFlag,int length);
		void plotTimeFlags(char* chanFlag,int length);
		void plotSmoothFlags(float* sFlags,long int blockLength);
		void plotDedispFlags(char* dedispFlags,int length);	
		void plotPolarLegend();

		void plotZeroDMSingleChannel(float *zeroDM,float zrMax,float zrMin,int length);
		void plotTimeFlagsSingleChannel(char* timeFlag,int length);
		void plotProfileSingleChannel(float *profile,float maxProfile,float minProfile);
		void plotProfileUnfilteredSingleChannel(float *profile,float maxProfile,float minProfile);	
		
		void plotAll(BasicAnalysis** basicAnalysis,AdvancedAnalysis** advancedAnalysis,RFIFiltering** rFIFilteringTime,RFIFiltering** rFIFilteringChan,int index);
		void plotAllNonPolar(BasicAnalysis* basicAnalysis,AdvancedAnalysis* advancedAnalysis,RFIFiltering* rFIFilteringTime,RFIFiltering* rFIFilteringChan,int index);
		void plotAllPolar(BasicAnalysis** basicAnalysis,AdvancedAnalysis** advancedAnalysis,int index);
		void plotAllSingleChannel(BasicAnalysis* basicAnalysis,AdvancedAnalysis* advancedAnalysis,RFIFiltering* rFIFilteringTime,RFIFiltering* rFIFilteringChan,int index);
		void plotBlockIndex(int index);
		void plotTitle();
		
		void foldingStartMessage();
};
#endif
