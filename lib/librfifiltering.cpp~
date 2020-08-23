#include<RFIFiltering.h>
//implementation of RFIFiltering methods

/*******************************************************************
*CONSTRUCTOR: RFIFiltering::RFIFiltering(float* input_,int inputSize_)
*float* input_  : Array from which outliers are to be detected.
*int inputSize_ : Size of the input array
*******************************************************************/
RFIFiltering::RFIFiltering(float* input_,int inputSize_)
{
	input=input_;
	inputSize=inputSize_;
	flags=new char[inputSize];
	sFlags=new float[inputSize];
	flagsFilt=new char[inputSize];
	histogram=NULL;
	histogramAxis=NULL;
	smoothInput=NULL;
	generateBlankFlags();  	
}


/*******************************************************************
*DESTRUCTOR: RFIFiltering::~RFIFiltering()
*frees used memory
*******************************************************************/
RFIFiltering::~RFIFiltering()
{
	if(histogram!=NULL)
	{
		delete[] histogram;
		delete[] histogramAxis;
	}
	if(smoothInput!=NULL)
		delete[] smoothInput;
	delete[] flags;
	delete[] sFlags;
	delete[] flagsFilt;
}
/*******************************************************************
*FUNCTION: void RFIFiltering::computeStatistics(int algorithmCode)
*int algorithmCode: algorithm to use 
*Wrapper to select algorithm to find central tendency and rms.
*******************************************************************/
void RFIFiltering::computeStatistics(int algorithmCode)
{
	switch(algorithmCode)
	{
		case 1:
			histogramBased();
			break;
		case 2:
			MADBased();
			break;
	}
}
/*******************************************************************
*FUNCTION: void RFIFiltering::quicksort(float* x,long int first,long int last)
*sorts a given array using quicksort algorithm.
/*******************************************************************/
void RFIFiltering::quicksort(float* x,long int first,long int last)
{
    int pivot,j,i;
    float temp;

     if(first<last){
         pivot=first;
         i=first;
         j=last;

         while(i<j){
             while(x[i]<=x[pivot]&&i<last)
                 i++;
             while(x[j]>x[pivot])
                 j--;
             if(i<j){
                 temp=x[i];
                  x[i]=x[j];
                  x[j]=temp;
             }
         }

         temp=x[pivot];
         x[pivot]=x[j];
         x[j]=temp;
         quicksort(x,first,j-1);
         quicksort(x,j+1,last);

    }
}
/*******************************************************************
*FUNCTION: void RFIFiltering::MADBased()
*Median Absolute Deviation (MAD) based algorithm to calculate
*mean and rms of the underlying gaussian from which the dataset 
*is derived. It uses median to estimate the central tendency and
*MAD to estimate the variance. 
*******************************************************************/
void RFIFiltering::MADBased()
{
	float *ptrInput=input;
	float *tempInput=new float[inputSize];
	float *ptrTempInput=tempInput;
	/*Copies the input array to another array for sorting.
	*This is done to avoid scrambling the original array 
	*which may be in use by other objects*/
	for(long int i=0;i<inputSize;i++,ptrInput++,ptrTempInput++)
		*ptrTempInput=*ptrInput;

	quicksort(tempInput,0,inputSize-1); //sorts the entire array
	//median is calculated
	if(inputSize%2==0)
		centralTendency=(tempInput[inputSize/2-1]+tempInput[inputSize/2])/2.0;
	else
		centralTendency=tempInput[inputSize/2];
	ptrTempInput=tempInput;
	//Deviations from the median is calculated and its median used to estimate rms
	for(long int i=0;i<inputSize;i++,ptrTempInput++)
		*ptrTempInput=fabs(*ptrTempInput-centralTendency);
	quicksort(tempInput,0,inputSize-1);
	
	if(inputSize%2==0)
		rms=(tempInput[inputSize/2-1]+tempInput[inputSize/2])/2.0;
	else
		rms=tempInput[inputSize/2];
	rms=rms*1.4826;
	cutoff=rms*cutoffToRms;
	delete[] tempInput;
}
/*******************************************************************
*FUNCTION: void RFIFiltering::histogramBased()
*This algorithm bins datapoints to form a histogram of size histogramSize.
*It then uses the modal point of the histogram to estimate the central 
*tendency and then uses it along with the width at half maximum to find
*variance.
*******************************************************************/
void RFIFiltering::histogramBased()
{	
	float *ptrInput,interval;
	int modeHeight,modeIndex;
	int p;
	histogramInterval=(4.0*rms)/(pow(inputSize,1/3.0));
	interval=histogramInterval;
	histogramSize=(inputMax-inputMin)/(interval);
	histogramSize++;
	histogram=new float[histogramSize];
	histogramAxis=new float[histogramSize];
	char* ptrFlags;
	ptrFlags=flags;	
	for (int i=0;i<histogramSize;i++)
		 histogram[i]=0;
	ptrInput=&(input[0]);
	for(int i=0;i< inputSize;i++,ptrInput++,ptrFlags++)
		if(*ptrFlags==0)
			histogram[(int)((*ptrInput-inputMin)/interval)]++; //Computing histogram


	modeHeight=0;
	modeIndex=0;
	//finding modal point
	for (int i=0;i<histogramSize;i++)
	{
		if(histogram[i]>modeHeight)
		{
			modeHeight=histogram[i];
			modeIndex=i;
		}
	}
	if(modeHeight==0) 
		return;
	histogramMax=modeHeight;
	p=0;	
	while(histogram[modeIndex+p]>=histogram[modeIndex]/2.0) //finding index of half on the right
		p++;
	int q=0;
	while(histogram[modeIndex+q]>=histogram[modeIndex]/2.0) //finding index of half on the left
		q--;
	
	centralTendency=(modeIndex+0.5)*interval+inputMin;
	/*rms=(p*interval)/(sqrt(2*log(histogram[modeIndex]/(float)histogram[modeIndex+p])));
	rms+=(p*interval)/(sqrt(2*log(histogram[modeIndex]/(float)histogram[modeIndex-p])));
	rms/=2.0;*/
	rms=(p-q)*interval/2.355;
	for (int i=0;i<histogramSize;i++)
		histogramAxis[i]=(i*interval+inputMin-centralTendency)/rms;
	
	cutoff=rms*cutoffToRms;
		
}
/*******************************************************************
*FUNCTION: void RFIFiltering::smoothFlags()
*Generates a smooth version of the flags
*******************************************************************/
void RFIFiltering::smoothFlags(int windowLength,float threshold)
{
	float* ptrSmoothFlags;
	char* ptrFlags;
	ptrFlags=flags;	 
	ptrSmoothFlags=sFlags;
	float sum;
	int count;
	int s=windowLength/2;
	
	for(int i=0;i< inputSize;i++,ptrFlags++,ptrSmoothFlags++)
	{
		sum=0;
		count=0;
	       for(int j=-s;j<=s && i+j<inputSize;j++)	
	       {
	       		if(i+j<0)
	       			continue;
	       		sum+=(*(ptrFlags+j));
	       		count++;
	       }
	       *ptrSmoothFlags=1.0-(sum/count);
	}
	ptrFlags=flags;	 
	ptrSmoothFlags=sFlags;
	for(int i=0;i< inputSize;i++,ptrFlags++,ptrSmoothFlags++)
	{
		if(*ptrSmoothFlags<threshold)
		{
		 	for(int j=-s;j<=s && i+j<inputSize;j++)	
	       		{
	       			if(i+j<0)
	       				continue;
	       			*(ptrFlags+j)=1;
	       		}
	       		
	       	}
	       
	}
	
}
/*******************************************************************
*FUNCTION: void RFIFiltering::generateManualFlags(int nBadChanBlocks,int* badChanBlocks)
*Flags blocks specified by user (usually blocks of channels)
*******************************************************************/
void RFIFiltering::generateManualFlags(int nBadChanBlocks,int* badChanBlocks,int offset)
{
	for(int i=0;i<nBadChanBlocks;i++)
	{		
		for(int j=badChanBlocks[i*2];j<badChanBlocks[i*2+1];j++)
		{	
			if(j-offset<0 or j-offset>=inputSize)	
				continue;	
			flags[j-offset]=1;
		}
	}
}
/*******************************************************************
*FUNCTION: void RFIFiltering::generateBlankFlags()
*In cases when an array containing blank flags (all zeroes,no flagging)
*is required by other functions.
*******************************************************************/
void RFIFiltering::generateBlankFlags()
{
	float* ptrInput;
	char* ptrFlags,*ptrFlagsFilt;
	ptrFlags=flags;	 
	ptrFlagsFilt=flagsFilt;
	for(int i=0;i< inputSize;i++,ptrFlags++,ptrFlagsFilt++)
	{
		*ptrFlags=0;	
		*ptrFlagsFilt=0;
	}
}
/*******************************************************************
*FUNCTION: void RFIFiltering::flagData()
*Flagging of data based on its deviation from central tendency being
*above a certain threshold.
*******************************************************************/
void RFIFiltering::flagData()
{
	float* ptrInput;
	char* ptrFlags;
	ptrInput=input;
	ptrFlags=flags;
	cutoff=rms*cutoffToRms;
	for(int i=0;i< inputSize;i++,ptrInput++,ptrFlags++)
	{
		if(fabs(*ptrInput-centralTendency)>cutoff)
		{
			*ptrFlags=1;
		}
	}
}
/*******************************************************************
*FUNCTION: void RFIFiltering::
*
*******************************************************************/
void RFIFiltering::flagWidthThreshold(int onWidth,int offWidth)
{

	int window=(onWidth+offWidth*2+1)/2;
	int i,j,flagsum,start,stop;
	char *ptrFlags,*ptrFlagsFilt;
	ptrFlagsFilt=flagsFilt;
	ptrFlags=flags;

	i=0;
        
	while(i<inputSize)
	{
		if(*ptrFlags)
		{
			start=-window;
			stop=window;
			if(i+start<0)
				start=-i;
			if(i+stop>inputSize)
				stop=inputSize-i;
			flagsum=0;
			for(j=start;j<stop;j++)
				flagsum+=(*(ptrFlags+j));
			if(flagsum<=onWidth)
			{
				//j=0;
				//do
				//{
				*ptrFlagsFilt=1;					
				//ptrFlags++;
				//	ptrFlagsFilt++;
				//	i++;
				//	j++;
				//}while(j<onWidth && *ptrFlags);
			}
		}
		i++;
		ptrFlags++;
		ptrFlagsFilt++;
		
	} 
	//memcpy(flags,flagsTemp,inputSize);
	//delete[] flagsTemp;
}
/*******************************************************************
*FUNCTION: void RFIFiltering::
*
*******************************************************************/
void RFIFiltering::subbandFlagging(int nSub,int nSubdetect,int noOfChannels,int startChannel,int stopChannel,float *rawData,char *chanFlags)
{
	
	float *ptrRawData;
	char *ptrFlags,*ptrFlagsFilt,*ptrChanFlags;;
	float sum;
	ptrFlags=flags;
	ptrFlagsFilt=flagsFilt;
	int i,j,k,nChanSubband,nChan,sumflag,count;
	nChan=stopChannel-startChannel;
	nChanSubband=nChan/nSub;

	for(int i=0;i<inputSize;i++,ptrFlags++,ptrFlagsFilt++)
	{
		
		if(*ptrFlags && !(*ptrFlagsFilt))
		{
			ptrRawData=&rawData[i*noOfChannels+startChannel];
			ptrChanFlags=chanFlags;
			sumflag=0;
			for(j=0;j<nSub;j++)
			{
				sum=0;
				count=0;
				for(k=0;k<nChanSubband;k++,ptrRawData++,ptrChanFlags++)
				{
					sum+=(*ptrRawData)*(!(*ptrChanFlags));
					count+=(!(*ptrChanFlags));
				}
				sum/=count;
				if(count==0)
					continue;
				if(fabs(sum-centralTendency)>cutoff/sqrt(nSub))
					sumflag++;	
					
				if(sumflag>nSubdetect)
				{
					*ptrFlagsFilt=1;
					break;
				}
			//	if(sumflag+(nSub-j-1)<nSubdetect)
			//		break;
			}
				
		}
		
	}
}
/*******************************************************************
*FUNCTION: void RFIFiltering::generateBlankFlags()
*Flagging of data based on its deviation from central tendency being
*above a certain threshold.
*******************************************************************/
/*void RFIFiltering::multiPointFlagData(float* multiCutoff)
{
	float* ptrInput;
	char* ptrFlags;
	char thisFlag=0;
	short last=3;
	ptrInput=input;
	ptrFlags=flags;
	for(int i=0;i< inputSize;i++,ptrFlags++)
		*ptrFlags=0;
	ptrFlags=flags;
	for(int i=0;i< inputSize;i++,ptrInput++,ptrFlags++)
	{
		last=3;
		if(i+last>inputSize)
			last=inputSize-i;
		for(int k=0;k<last;k++)
		{
			thisFlag=1;
			for(int p=0;p<k;p++)
				if(thisFlag && (fabs(*(ptrInput+p)-centralTendency)>multiCutoff[k]*rms))
					thisFlag=1;
				else
					thisFlag=0;
			for(int p=0;p<k;p++)
			{
				if(thisFlag)
					*(ptrFlags+p)=1;
			}
		}
	}

}*/
void RFIFiltering::multiPointFlagData(float* multiCutoff)
{
	float* ptrInput;
	char* ptrFlags;
	ptrInput=input;
	ptrFlags=flags;
	for(int i=0;i< inputSize;i++,ptrFlags++)
		*ptrFlags=0;
	ptrFlags=flags;
	for(int i=0;i< inputSize-1;i++,ptrInput++,ptrFlags++)
	{
		if(fabs(*(ptrInput)-centralTendency)>multiCutoff[1]*rms && fabs(*(ptrInput+1)-centralTendency)>multiCutoff[1]*rms)
			*(ptrFlags)=*(ptrFlags+1)=1;
	}

}

/*******************************************************************
*FUNCTION: void RFIFiltering::writeFlags(const char* filename)
*const char* filename: Name of file to which flags are written.
*******************************************************************/
void RFIFiltering::writeFlags(const char* filename)
{
	FILE* flagFile;    
    	flagFile = fopen(filename, "ab");
        fwrite(flags, sizeof(char), inputSize, flagFile);    
    	fclose(flagFile);    	
}

/*******************************************************************
*FUNCTION: void RFIFiltering::writeFlags(const char* filename)
*const char* filename: Name of file to which flags are written.
*char* startFlags: Initial flags at the beginning 
*int nStartFlags: number of flags (1s) to write before flags
*char* endFlags: flags appeneded at the end
*int nEndFlags: number of flags (1s) to write after flags
*This is used to write channel flag file where some channels
*at the start and the end have been marked bad by the user. 
*******************************************************************/
void RFIFiltering::writeFlags(const char* filename,char* startFlags,int nStartFlags,char* endFlags,int nEndFlags)
{
	FILE* flagFile;    
    	flagFile = fopen(filename, "ab");
	fwrite(startFlags, sizeof(char), nStartFlags, flagFile);    
        fwrite(flags, sizeof(char), inputSize, flagFile);  
	fwrite(endFlags, sizeof(char), nEndFlags, flagFile);     
    	fclose(flagFile);    	
}


//implementation of RFIFiltering methods ends
