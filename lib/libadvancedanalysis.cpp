//implementation of advancedAnalysis methods begins
#include"AdvancedAnalysis.h"
//Declaration of static variables.
Information AdvancedAnalysis::info;
unsigned long* AdvancedAnalysis::delayTable;
float** AdvancedAnalysis::foldedProfile;
int** AdvancedAnalysis::countProfile;
float** AdvancedAnalysis::foldedProfileUnfiltered;
int** AdvancedAnalysis::countProfileUnfiltered;
double* AdvancedAnalysis::curPosMsStatic;
unsigned long AdvancedAnalysis::maxDelay;
double AdvancedAnalysis::initLagMs=0.0;
double AdvancedAnalysis::initPhasePolyco=-1;
/*******************************************************************
*CONSTRUCTOR: AdvancedAnalysis::AdvancedAnalysis(Information _info)
*Information _info: All input parameters are contained in this object
*Computation of delay table and 
*initialization of static variables is done in this constructor
*******************************************************************/
AdvancedAnalysis::AdvancedAnalysis(Information info_)
{
	info=info_;
	delayTable=new unsigned long[info.noOfChannels];
	calculateDelayTable();
	foldedProfile=new float*[info.noOfPol];
	countProfile=new int*[info.noOfPol];
	curPosMsStatic=new double[info.noOfPol];
	
	foldedProfileUnfiltered=new float*[info.noOfPol];
	countProfileUnfiltered=new int*[info.noOfPol];
	//Initializing folded profiles and current positions, one for each polarization.
	for(int k=0;k<info.noOfPol;k++)
	{
		foldedProfile[k]=new float[info.periodInSamples];
		countProfile[k]=new int[info.periodInSamples];	
		curPosMsStatic[k]=0.0;
		
		foldedProfileUnfiltered[k]=new float[info.periodInSamples];
		countProfileUnfiltered[k]=new int[info.periodInSamples];	
		for(int i=0;i<info.periodInSamples;i++)
		{
			foldedProfile[k][i]=0;
			countProfile[k][i]=0;
			
			foldedProfileUnfiltered[k][i]=0;
			countProfileUnfiltered[k][i]=0;
		}
	}
	excess=NULL;
	countExcess=NULL;
	fullDM=NULL;
	count=NULL;
	curFoldedProfile=NULL;

	excessUnfiltered=NULL;
	countExcessUnfiltered=NULL;
	fullDMUnfiltered=NULL;
	countUnfiltered=NULL;
	curFoldedProfileUnfiltered=NULL;

	dedispFlags=NULL;
}
/*******************************************************************
*General comments about calculation of dedispersed time series:
*
*Dedispersion is done with respect to the lowest frequency.
*The lowest frequency is most delayed, hence positive delays must be
*added to all other channels to align them.
*
*While doing so the shifted position of some channels of 
*the time samples whose index is greater than length-maxDelay 
*will cross the last index of the fullDM array. These are then stored in
*the excess array and added with the dedispersed series of the
*next block. 
*******************************************************************/

/*******************************************************************
*CONSTRUCTOR: AdvancedAnalysis:AdvancedAnalysis(long int blockIndex_,int polarIndex_,unsigned short int* rawData_,long int length_,float* excess_,int* countExcess_)
*long int blockIndex_		:Index of current block(window) to process
*int polarIndex_		:Index of current polarization to process
*float* rawData_		:2-D raw data.
*long int length_		:length of time sample to dedisperse
*float* excess_			:excess from the previous block
*int* countExcess_		:number of data points in each bin of excess_ array
*******************************************************************/
AdvancedAnalysis::AdvancedAnalysis(long int blockIndex_,int polarIndex_,float* rawData_,long int length_)
{
	length=length_;	
	rawData=rawData_;
	blockIndex=blockIndex_;
	polarIndex=polarIndex_;	
	fullDM=new float[length+maxDelay];
	excess=new float[maxDelay];
	count=new int[length+maxDelay];
	countExcess=new int[maxDelay];
	curFoldedProfile=new float[info.periodInSamples];
	dedispFlags=new char[length];

	fullDMUnfiltered=new float[length+maxDelay];
	excessUnfiltered=new float[maxDelay];
	countUnfiltered=new int[length+maxDelay];
	countExcessUnfiltered=new int[maxDelay];
	curFoldedProfileUnfiltered=new float[info.periodInSamples];
	hasEnoughDedispersedData=1;
	foldingStartIndex=0;

	float* ptrFullDM=fullDM;
	float* ptrExcess=excess;
	int* ptrCount=count;
	int* ptrCountExcess=countExcess;
	char* ptrDedispFlags=dedispFlags;

	float* ptrFullDMUnfiltered=fullDMUnfiltered;
	float* ptrExcessUnfiltered=excessUnfiltered;
	int* ptrCountUnfiltered=countUnfiltered;
	int* ptrCountExcessUnfiltered=countExcessUnfiltered;
	
	//intializing fullDM and count
	for(int i=0;i<length+maxDelay;i++,ptrFullDM++,ptrCount++,ptrFullDMUnfiltered++,ptrCountUnfiltered++)
		*ptrFullDM=*ptrCount=*ptrFullDMUnfiltered=*ptrCountUnfiltered=0;
	
	//intializing dedispersion flag
	for(int i=0;i<length;i++,ptrDedispFlags++)
		*ptrDedispFlags=0;
	
	//excess_==NULL implies that this is the first block to get dedispersed	

	
	//copying of excess of last block to fullDM of this block
	/*
	memcpy(fullDM,excess_,maxDelay*sizeof(float));
	memcpy(count,countExcess_,maxDelay*sizeof(int));

	memcpy(fullDMUnfiltered,excessUnfiltered_,maxDelay*sizeof(float));
	memcpy(countUnfiltered,countExcessUnfiltered_,maxDelay*sizeof(int));
	*/
}
/*******************************************************************
*DESTRUCTOR: AdvancedAnalysis::~AdvancedAnalysis()
*frees up used memory.
*******************************************************************/
AdvancedAnalysis::~AdvancedAnalysis()
{
	delete[] curFoldedProfile;	
	delete[] fullDM;
	delete[] count;
	delete[] excess;
	delete[] countExcess;
	delete[] curFoldedProfileUnfiltered;	
	delete[] fullDMUnfiltered;
	delete[] countUnfiltered;
	delete[] excessUnfiltered;
	delete[] countExcessUnfiltered;
	delete[] dedispFlags;
	

}
/*******************************************************************
*FUNCTION: void AdvancedAnalysis::calculateDelayTable()
*Calculates the amount of shift of each channel with respect to the
*lowest frequency. This shift corrects for the smearing of pulse
*that occurs when the signal travels through interstellar plasma.
*Note that this function is called only once and the table calculated
*is stored in a static array delayTable.
*******************************************************************/
void AdvancedAnalysis::calculateDelayTable()
{
	int j;	
	float lowestFrequency=info.lowestFrequency;
	float KDM=4.148808*pow(10,3)*info.dispersionMeasure;
	float freqInterval=(info.bandwidth)/(float)(info.noOfChannels);
	lowestFrequency+=freqInterval/2.0;	
	
	for(int i=0;i<info.noOfChannels;i++)
	{
		if(info.sidebandFlag==1)	//if sidebandFlag is one(zero) then the first(last) channel is of the lowest frequency 
			j=i;
		else
			j=info.noOfChannels-i-1;
		if(info.refFrequency==0)
			delayTable[i]=(unsigned long)(((1.0/pow(lowestFrequency,2))-(1.0/pow(lowestFrequency+j*freqInterval,2)))*KDM/info.samplingInterval+0.5);
		else
			delayTable[i]=(unsigned long)((-(1.0/pow(lowestFrequency+j*freqInterval,2))+(1.0/pow(lowestFrequency+info.bandwidth,2)))*KDM/info.samplingInterval+0.5);
			
	}
	if(info.refFrequency==0)
	{
		if(info.sidebandFlag==1)
			maxDelay=delayTable[info.noOfChannels-1];
		else
			maxDelay=delayTable[0];	
	}
	else
	{
		if(info.sidebandFlag==0)
			maxDelay=-delayTable[info.noOfChannels-1];
		else
			maxDelay=-delayTable[0];
		for(int i=0;i<info.noOfChannels;i++)
			delayTable[i]+=maxDelay;
	}	
}
/*******************************************************************
*FUNCTION: AdvancedAnalysis::calculateFullDM(char* timeFlags,char* freqFlags)
*char* timeFlags	:Time samples marked 1 are ignored (or clipped).
*char* freqFlags	:Channels marked 1 are ignored (or clipped).
*Calculates the dedispersed time series.
*If time or channel or both filtering are turned off the corresponding
*arrays have all 0.
*******************************************************************/
void AdvancedAnalysis::calculateFullDM(char* timeFlags,char* freqFlags)
{
	
	float* ptrRawData=rawData;
	char* ptrTimeFlags=timeFlags;
	char* ptrFreqFlags;
	long int pos;

	int startChannel=info.startChannel;
	int stopChannel=info.stopChannel;
	int totalChan=info.noOfChannels;
	int endExclude=info.noOfChannels-stopChannel;

	for(long int i=0;i<length;i++,ptrTimeFlags++)
	{
		ptrRawData+=startChannel;
		ptrFreqFlags=freqFlags;	
			for(int j=startChannel;j<stopChannel;j++,ptrRawData++,ptrFreqFlags++)
			{

				pos=i+delayTable[j];	//shift to correct for dispersion.
				if(!(*ptrFreqFlags)&!(*ptrTimeFlags))
				{
					fullDM[pos]+=(*ptrRawData);
					count[pos]++;
				}
				else
				{
					fullDMUnfiltered[pos]+=(*ptrRawData);
					countUnfiltered[pos]++;
				}
			}
		
		ptrRawData+=endExclude;
	}
	float* ptrFullDM=fullDM;
	float* ptrFullDMUnfiltered=fullDMUnfiltered;
	int* ptrCount=count;
	int* ptrCountUnfiltered=countUnfiltered;
	

	for(int i=0;i<length+maxDelay;i++,ptrFullDM++,ptrFullDMUnfiltered++,ptrCount++,ptrCountUnfiltered++)
	{

		(*ptrFullDMUnfiltered)+=(*ptrFullDM);
		(*ptrCountUnfiltered)+=(*ptrCount);		
	}        

}
/*******************************************************************
*FUNCTION: AdvancedAnalysis::calculateFullDM(unsigned short int* filteredRawData)
*short int* filteredRawData - filtered raw data
*Calculates the dedispersed time series using the filtered raw data.
*******************************************************************/
void AdvancedAnalysis::calculateFullDM(short int* filteredRawData)
{
	

	
	long int pos;

	int startChannel=info.startChannel;
	int stopChannel=info.stopChannel;
	int totalChan=info.noOfChannels;
	int endExclude=info.noOfChannels-stopChannel;
	float* ptrRawData=rawData;
	short int* ptrFilteredRawData=filteredRawData;
	for(long int i=0;i<length;i++)
	{
			ptrFilteredRawData+=startChannel;	
			ptrRawData+=startChannel;	
			for(int j=startChannel;j<stopChannel;j++,ptrRawData++,ptrFilteredRawData++)
			{

				pos=i+delayTable[j];	//shift to correct for dispersion.
				
				fullDM[pos]+=(*ptrFilteredRawData);
				count[pos]++;
				
				
				fullDMUnfiltered[pos]+=(*ptrRawData)*info.meanval;
				countUnfiltered[pos]++;
			}
			ptrRawData+=endExclude;
			ptrFilteredRawData+=endExclude;
			
	}
}
void AdvancedAnalysis::mergeExcess(float* excess_,int* countExcess_,float* excessUnfiltered_,int* countExcessUnfiltered_)
{
	float* ptrFullDM=fullDM;
	float* ptrFullDMUnfiltered=fullDMUnfiltered;
	int* ptrCount=count;
	int* ptrCountUnfiltered=countUnfiltered;
	float* ptrExcess=excess_;
	float* ptrExcessUnfiltered=excessUnfiltered_;
	int* ptrCountExcess=countExcess_;
	int* ptrCountExcessUnfiltered=countExcessUnfiltered_;
	if(excess_)
	{
		for(int i=0;i<maxDelay;i++,ptrFullDM++,ptrFullDMUnfiltered++,ptrExcess++,ptrExcessUnfiltered++,ptrCount++,ptrCountUnfiltered++,ptrCountExcess++,ptrCountExcessUnfiltered++)
		{
			(*ptrFullDM)+=(*ptrExcess);
			(*ptrFullDMUnfiltered)+=(*ptrExcessUnfiltered);
			(*ptrCount)+=(*ptrCountExcess);
			(*ptrCountUnfiltered)+=(*ptrCountExcessUnfiltered);		
		}
	}
	memcpy(excess,&fullDM[length],maxDelay*sizeof(float));
	memcpy(excessUnfiltered,&fullDMUnfiltered[length],maxDelay*sizeof(float));
	memcpy(countExcess,&count[length],maxDelay*sizeof(int));	
	memcpy(countExcessUnfiltered,&countUnfiltered[length],maxDelay*sizeof(int));
}
void AdvancedAnalysis::normalizeFullDM()
{
	float* ptrFullDM;
	float* ptrFullDMUnfiltered;
	int* ptrCount;
	int* ptrCountUnfiltered;
	int nChan= info.stopChannel-info.startChannel;
	double samplingIntervalMs=info.samplingInterval*1000.0;
	
		/*******************************************************************
		*The following is the logic for folding to start. Folding is started
		*only when the pulse ariving on all channels is guaranteed, i.e
		*when maxDelay number of samples have elapsed. If the window-length
		*is less than maxDelay then maxDelay/length number of windows must
		*be skipped.
		*******************************************************************/
		if(length<maxDelay && ((blockIndex+1)*(long int)info.blockSizeSamples<maxDelay))
		{	
			hasEnoughDedispersedData=0;	
			if(polarIndex==0)
				initLagMs+=length*samplingIntervalMs;	
				
		}
		/*******************************************************************
		*The if clause determines if this is the first block for which
		*folding will be performed. In case window(block) length is greater
		*than maxDelay then it suffices to check if blockIndex==0.
		*if length is lesser than maxDelay then the logic checks if it is
		*the first block to process.
		*******************************************************************/
		else if(blockIndex==0 || (length<maxDelay && int((blockIndex+1)*length/maxDelay)==1 && (blockIndex+1)*length%maxDelay<length))
		{		
			/*******************************************************************
			*maxDelay%length number of samples in the first block to process is
			*skipped to ensure that atleast maxDelay number of time samples have
			*been recorded since the start of observation.
							*******************************************************************/			
			foldingStartIndex=maxDelay%length;	
			if(polarIndex==0)
				initLagMs+=samplingIntervalMs*foldingStartIndex;
		}
	
	/*******************************************************************
	*The dedispersed time series is averaged over the number of samples 
	*added in each bin. Minimum and maximum of the series is found.
	*These are used while plotting the series.
	*******************************************************************/
	ptrFullDM=fullDM;
	ptrFullDMUnfiltered=fullDMUnfiltered;
	ptrCount=count;
	ptrCountUnfiltered=countUnfiltered;
	minFullDM=minFullDMUnfiltered=33554434;
	maxFullDM=maxFullDMUnfiltered=0;
	char* ptrDedispFlags=dedispFlags;
	
	for(long int i=0;i<length;i++,ptrFullDM++,ptrCount++,ptrDedispFlags++,ptrFullDMUnfiltered++,ptrCountUnfiltered++)
	{	


		if(*ptrCount==0)
		{
			*ptrDedispFlags=1;
			*ptrFullDM=0;
			continue;
		}
		else
		{
			(*ptrFullDM)/=(*ptrCount);		
			if((*ptrCount)<=0.4*nChan)
				*ptrDedispFlags=1;
			else
			{
							
				if(*ptrFullDM>maxFullDM)
					maxFullDM=*ptrFullDM;
				if(*ptrFullDM<minFullDM)
					minFullDM=*ptrFullDM;
			}
		}

		if(*ptrCountUnfiltered==0)
		{
			*ptrFullDMUnfiltered=0;
		}
		else
		{
			(*ptrFullDMUnfiltered)/=(*ptrCountUnfiltered);
			if(*ptrFullDMUnfiltered>maxFullDMUnfiltered)
				maxFullDMUnfiltered=*ptrFullDMUnfiltered;
			if(*ptrFullDMUnfiltered<minFullDMUnfiltered)
				minFullDMUnfiltered=*ptrFullDMUnfiltered;
		}

		
		
	}
	
}
/*******************************************************************
*FUNCTION: double AdvancedAnalysis::calculatePolycoPhase()
*returns double phase: folding phase for current time sample
*between 0 and 1.
*computes the phase of the current time sample using a polyco table.
*******************************************************************/
double AdvancedAnalysis::calculatePolycoPhase()
{
	
	//Check the time of the current sample wrt polyco start time (dt)	
	double dt = (initLagMs+curPosMs)/60000.0+ (double)((info.MJDObs - info.polycoTable[polycoRowIndex*(3+info.nCoeffPolyco)+0]) * 1440);
	//Logic to check if the time difference has crossed the span of current set.
	
	while(dt > info.spanPolyco/2.0)
  	{
    		dt -= info.spanPolyco;
    		polycoRowIndex += 1;
  	}
  	//Find the folding phase for the current time
  	double* polycoSet=&(info.polycoTable[polycoRowIndex*(3+info.nCoeffPolyco)]);
  	double phase = (polycoSet[1] + (dt*60*polycoSet[2])); 	//Ref phase + 0th order frequency
  	for(int i=0;i<info.nCoeffPolyco;i++)  	
    		phase += polycoSet[3+i] * pow(dt,i);
  	
  	
  	double F_Temp;

	//The following statement ensures that the first sample of the time series 
	//goes to the first profile bin.
  	if(initPhasePolyco == -1)  	
  		initPhasePolyco = modf( (modf(phase , &F_Temp)+ 1.0) , &F_Temp);
	//Calculates the fractional phase w.r.t the intial phase
  	phase -= initPhasePolyco;
  	double frac_phase = modf((modf(phase , &F_Temp)+ 1.0) , &F_Temp) ;  	
  	return frac_phase;
}
/*******************************************************************
*FUNCTION: double AdvancedAnalysis::calculateFixedPeriodPhase()
*returns double phase: folding phase for current time sample
*between 0 and 1.
*******************************************************************/
double AdvancedAnalysis::calculateFixedPeriodPhase()
{
	double periodInMs=info.periodInMs;
	int noOfPeriods=(int)floor(curPosMs/periodInMs);
	return (curPosMs/periodInMs)-noOfPeriods;
}
/*******************************************************************
*FUNCTION: void AdvancedAnalysis::calculateProfile()
*Folds the dedispersed time series to get the pulsar profile.
*******************************************************************/
void AdvancedAnalysis::calculateProfile()
{
	//if hasEnoughDedispersedData=0 then folding must not begin, see comments in calculateFullDM() function.
	if(hasEnoughDedispersedData==0)
		return;
	int periodInSamples=info.periodInSamples;
	double offset=info.profileOffset;
	double samplingIntervalMs=info.samplingInterval*1000.0;
	double phase;
	char* ptrDedispFlags=&dedispFlags[foldingStartIndex];	
	float* ptrFullDM=&fullDM[foldingStartIndex];
	float* ptrFullDMUnfiltered=&fullDMUnfiltered[foldingStartIndex];
	int noOfPeriods,index;
	//finds the appropiate static variables for given polarization (polarIndex)
	
	float* thisFoldedProfile,*thisFoldedProfileUnfiltered;
	int* thisCountProfile,*thisCountProfileUnfiltered;	
	

	thisFoldedProfile=foldedProfile[polarIndex];
	thisCountProfile=countProfile[polarIndex];
	curPosMs=curPosMsStatic[polarIndex];
	if(curPosMs==0.0)
		cout<<endl<<"Started folding after a lag of "<<initLagMs<<" ms"<<endl;
	thisFoldedProfileUnfiltered=foldedProfileUnfiltered[polarIndex];
	thisCountProfileUnfiltered=countProfileUnfiltered[polarIndex];


	if(!info.doFixedPeriodFolding)
		polycoRowIndex= info.polycoRowIndex; 	

	for(long int i=foldingStartIndex;i<length;i++,ptrFullDM++,ptrFullDMUnfiltered++,ptrDedispFlags++,curPosMs+=samplingIntervalMs)
	{	
		
		if(info.doFixedPeriodFolding)	
			phase=calculateFixedPeriodPhase()+offset;
		else
			phase=calculatePolycoPhase()+offset;
		if(*ptrDedispFlags)
			continue;
		if(phase>=1)
			phase=phase-1;	
		//logic to convert phase to sample index with given offset.
		index=(int)(phase*periodInSamples+0.5);	
		if(index==periodInSamples)
			index=0;
		thisFoldedProfile[index]+=(*ptrFullDM);
		thisFoldedProfileUnfiltered[index]+=(*ptrFullDMUnfiltered);
		thisCountProfileUnfiltered[index]++;
		thisCountProfile[index]++;
		
		
		
	}

	curPosMsStatic[polarIndex]=curPosMs;
	//Calculates the folded profile generate till the current block. This is used to plot.
	minProfile=maxProfile=thisFoldedProfile[(int)(offset*periodInSamples)+1]/thisCountProfile[(int)(offset*periodInSamples)+1]; //The slightly non-intuitive index is given here to ensure that countprofile is not zero, which can be the case if the first element was used to initialize the minimum. Such a situation can arise in the intial blocks with a non-zero phase offset.
	minProfileUnfiltered=maxProfileUnfiltered=thisFoldedProfileUnfiltered[(int)(offset*periodInSamples)+1]/thisCountProfile[(int)(offset*periodInSamples)+1]; 
	float* ptrFoldedProfile=thisFoldedProfile;
	float* ptrCurFoldedProfile=curFoldedProfile;
	int* ptrCountProfile=thisCountProfile;
	float* ptrFoldedProfileUnfiltered=thisFoldedProfileUnfiltered;
	float* ptrCurFoldedProfileUnfiltered=curFoldedProfileUnfiltered;
	int* ptrCountProfileUnfiltered=thisCountProfileUnfiltered;
	for(int i=0;i<periodInSamples;i++,ptrFoldedProfile++,ptrCurFoldedProfile++,ptrCountProfile++,ptrFoldedProfileUnfiltered++,ptrCurFoldedProfileUnfiltered++,ptrCountProfileUnfiltered++)
	{
		if((*ptrCountProfile)!=0)
		{		
			*ptrCurFoldedProfile=(float)((*ptrFoldedProfile)/(*ptrCountProfile));
			if(*ptrCurFoldedProfile>maxProfile)
				maxProfile=*ptrCurFoldedProfile;
			if(*ptrCurFoldedProfile<minProfile)
				minProfile=*ptrCurFoldedProfile;
		}
		else
			*ptrCurFoldedProfile=0;
		if((*ptrCountProfileUnfiltered)!=0)
		{		
			*ptrCurFoldedProfileUnfiltered=(float)((*ptrFoldedProfileUnfiltered)/(*ptrCountProfileUnfiltered));
			if(*ptrCurFoldedProfileUnfiltered>maxProfileUnfiltered)
				maxProfileUnfiltered=*ptrCurFoldedProfileUnfiltered;
			if(*ptrCurFoldedProfileUnfiltered<minProfileUnfiltered)
				minProfileUnfiltered=*ptrCurFoldedProfileUnfiltered;
		}
		else
			*ptrCurFoldedProfileUnfiltered=0;
	}
}
/*******************************************************************
*FUNCTION: void AdvancedAnalysis::writeFullDM(const char*  filename,const char* filenameUnfiltered)
*const char*  fileName: Filename to write filtered fullDM series to.
*const char*  fileNameUnfiltered: Filename to write unfiltered fullDM series to.
*Writes out the dedispersed time series to a files
*******************************************************************/
void AdvancedAnalysis::writeFullDM(const char*  filename,const char* filenameUnfiltered)
{
	FILE* fullDMFile;    
    fullDMFile = fopen(filename, "ab");
    fwrite(&fullDM[foldingStartIndex], sizeof(float), length-foldingStartIndex, fullDMFile);    
    fclose(fullDMFile); 
    
    fullDMFile = fopen(filenameUnfiltered, "ab");
    fwrite(&fullDMUnfiltered[foldingStartIndex], sizeof(float), length-foldingStartIndex, fullDMFile);    
    fclose(fullDMFile); 
}

/*******************************************************************
*FUNCTION: void AdvancedAnalysis::writeFullDMCount(const char*  filename)
*const char*  fileName: Filename to write to.
*Writes out the bin count of dedispersed time series to a file
*******************************************************************/
void AdvancedAnalysis::writeFullDMCount(const char*  filename)
{
	FILE* fullDMCountFile;    
    fullDMCountFile = fopen(filename, "ab");
    fwrite(&count[foldingStartIndex], sizeof(int), length-foldingStartIndex, fullDMCountFile);    
    fclose(fullDMCountFile); 
}
/*******************************************************************
*FUNCTION: void AdvancedAnalysis::writeProfile(const char*  filename,const char* filenameUnfiltered)
*const char*  fileName: Filename to write filtered profile to.
*const char*  fileNameUnfiltered: Filename to write unfiltered profile to.
*Writes out the folded profiles to files.
*******************************************************************/
void AdvancedAnalysis::writeProfile(const char*  filename,const char* filenameUnfiltered)
{
	
	ofstream profileFile,profileFileUnfiltered;
	profileFile.open(filename);
	profileFile<<"#phase\tvalue"<<endl;
	for(int i=0;i<info.periodInSamples;i++)
	{
		profileFile<<i/float(info.periodInSamples)<<"\t"<<setprecision(20)<<" "<<curFoldedProfile[i]<<endl;
	}


	profileFileUnfiltered.open(filenameUnfiltered);
	profileFileUnfiltered<<"#phase\tvalue"<<endl;
	for(int i=0;i<info.periodInSamples;i++)
	{
		profileFileUnfiltered<<i/float(info.periodInSamples)<<"\t"<<setprecision(20)<<" "<<curFoldedProfileUnfiltered[i]<<endl;
	}
	
}

