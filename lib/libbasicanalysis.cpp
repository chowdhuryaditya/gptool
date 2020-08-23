#include"BasicAnalysis.h"

using namespace std;


//implementation of BasicAnalysis methods begins
//Declaration of static variables

Information	BasicAnalysis::info;
double**	BasicAnalysis::smoothSumBandshape;
double** 	BasicAnalysis::sumBandshape;
double**	BasicAnalysis::cumSquareBandshape;
float**		BasicAnalysis::externalBandshape;
long long int** BasicAnalysis::countBandshape;


/*******************************************************************
*CONSTRUCTOR: BasicAnalysis::BasicAnalysis(Information _info)
*Information _info: All input parameters are contained in this object
*Initialization of static variables is done in this constructor
*******************************************************************/
BasicAnalysis::BasicAnalysis(Information _info)
{
	info=_info;
	//initializing other static variables
	smoothSumBandshape=new double*[info.noOfPol];
	sumBandshape=new double*[info.noOfPol];
	cumSquareBandshape=new double*[info.noOfPol];
	countBandshape=new long long int*[info.noOfPol];
	externalBandshape=new float*[info.noOfPol];
	for(int k=0;k<info.noOfPol;k++)
	{
		sumBandshape[k]=new double[info.noOfChannels];
		smoothSumBandshape[k]=new double[info.noOfChannels];
		cumSquareBandshape[k]=new double[info.noOfChannels];
		countBandshape[k]=new long long int[info.noOfChannels];
		externalBandshape[k]=new float[info.noOfChannels];
		for(int i=0;i<info.noOfChannels;i++)
			sumBandshape[k][i]=smoothSumBandshape[k][i]=cumSquareBandshape[k][i]=countBandshape[k][i]=0;
	}
	if(info.normalizationProcedure==2)
	{
		int count=0;
		std::ifstream f("bandshape.dat");
		std::string line;
		while (std::getline(f, line)) {
			count++;
			int index;
			float val;
			std::istringstream ss(line);
			ss >> index >> val;
			if(val==0)
				val=-1;
			externalBandshape[0][index]=val;
		}
		if(count<<info.noOfChannels)
		{
			cout<<"bandshape.dat does not contain "<<info.noOfChannels<<" number of channels"<<endl;
		}
	}
	filteredRawData=NULL;
	
}
/*******************************************************************
*FUNCTION: BasicAnalysis::BasicAnalysis(float* _rawData,int _blockIndex)
*float *_rawData 		:the 2-D raw data to process in the current block
*int _polarIndex		:index of polarization to process
*long int _blockLength		:length of current block
*Performs initialization of variables for current block
*******************************************************************/
BasicAnalysis::BasicAnalysis(float* _rawData,int _polarIndex,long int _blockLength)
{
	polarIndex=_polarIndex;
	blockLength=_blockLength;
	rawData=_rawData;
	bandshape=new float[info.noOfChannels];	
	correlationBandshape=new float[info.noOfChannels];	
	meanToRmsBandshape=new float[info.noOfChannels];
	squareBandshape=new double[info.noOfChannels];		
	normalizedBandshape=new float[info.noOfChannels];	
	zeroDM=new float[blockLength]; 
	zeroDMUnfiltered=new float[blockLength]; 
	cumBandshapeScale=1000.0;
	if(info.normalizationProcedure==2)
		smoothBandshape=externalBandshape[polarIndex];
	else
		smoothBandshape=new float[info.noOfChannels];
	filteredRawData=NULL;
	filteredRawDataChar=NULL;
	headerInfo=NULL;
	
}
/*******************************************************************
*DESTRUCTOR: BasicAnalysis::~BasicAnalysis()
*Frees memory occupied by various arrays
*******************************************************************/
BasicAnalysis::~BasicAnalysis()
{
	delete[] rawData;
	delete[] zeroDM;
	delete[] zeroDMUnfiltered;
	delete[] bandshape;
	delete[] correlationBandshape;
	delete[] meanToRmsBandshape;
	delete[] normalizedBandshape;
	delete[] squareBandshape;
	if(info.normalizationProcedure!=2)
		delete[] smoothBandshape;
	if(filteredRawData!=NULL)
		delete[] filteredRawData;
	if(headerInfo!=NULL)
		delete[] headerInfo;
	if(filteredRawDataChar!=NULL)
		delete[] filteredRawDataChar;
}


/*******************************************************************
*FUNCTION: void BasicAnalysis::computeZeroDM(char* freqFlags)
*char* freqFlags : The channels marked 1 in this array is not 
*included while computing zeroDM (or are clipped)
*For no flagging, a char* variable with all 0's is passed on to this 
*function
*Computes the time series by collapsing all frequency channels with
*no dedispersion (hence the name "zeroDM" series). While collapsing 
*the flagged channels are ignored or clipped.
*******************************************************************/
void BasicAnalysis::computeZeroDM(char* freqFlags)
{
	float*	ptrZeroDM;
	float*	ptrZeroDMUnfiltered;
	float* 	ptrRawData;
	float 	count=0;					//Stores the number of channels added to get each time sample	
	int 	startChannel=info.startChannel;
	int 	nChan= info.stopChannel-startChannel;		//Number of channels to use
	int 	endExclude=info.noOfChannels-info.stopChannel;	//Number of channels to exclude from the end of the band
	int 	l= blockLength;
	for(int i=0;i<nChan;i++)
		if(!freqFlags[i])
			count++;
	ptrRawData=rawData;
	ptrZeroDM=zeroDM;
	ptrZeroDMUnfiltered=zeroDMUnfiltered;
	maxZeroDM=0;
	minZeroDM=10000*nChan;	
			//This is done because there is no sample computed yet.		
	for(int i=0;i<l;i++,ptrZeroDM++,ptrZeroDMUnfiltered++)
	{		
		*ptrZeroDM=0;	
		*ptrZeroDMUnfiltered=0;		
		ptrRawData+=startChannel;			//startChannel number of channels skipped at the start of the band
		for(int j=0;j<nChan;j++,ptrRawData++)
		{
			*ptrZeroDMUnfiltered+=(*ptrRawData);
			if(!freqFlags[j])
				(*ptrZeroDM)+=(*ptrRawData);
		}
		ptrRawData+=endExclude;				//endExclude number of channels skipped at the end of the band
		(*ptrZeroDM)/=(float)count;			//Each sample averaged 
		(*ptrZeroDMUnfiltered)/=(float)nChan;
		//Calculating of minimum and maximum of zeroDM series
		if(*ptrZeroDM>maxZeroDM)
			maxZeroDM=*ptrZeroDM;
		if(*ptrZeroDM<minZeroDM)
			minZeroDM=*ptrZeroDM;
	}
}

/*******************************************************************
*FUNCTION: void BasicAnalysis::computeZeroDM(char* freqFlags)
*char* freqFlags : The channels marked 1 in this array is not 
*included while computing zeroDM (or are clipped)
*For no flagging, a char* variable with all 0's is passed on to this 
*function
*Computes the time series by collapsing all frequency channels with
*no dedispersion (hence the name "zeroDM" series). While collapsing 
*the flagged channels are ignored or clipped.
*******************************************************************/
void BasicAnalysis::computeZeroDMNorm(char* freqFlags,float *cumulativeBandshape)
{
	float*	ptrZeroDM;
	float* 	ptrRawData;
	float 	count=0;					//Stores the number of channels added to get each time sample	
	int 	startChannel=info.startChannel;
	int 	nChan= info.stopChannel-startChannel;		//Number of channels to use
	int 	endExclude=info.noOfChannels-info.stopChannel;	//Number of channels to exclude from the end of the band
	int 	l= blockLength;
	
	ptrRawData=rawData;
	ptrZeroDM=zeroDM;
	maxZeroDM=0;
	minZeroDM=10000*nChan;	
			//This is done because there is no sample computed yet.
	float *ptrSmoothBandshape;	
	
	for(int i=0;i<nChan;i++)
	{
		if(!freqFlags[i])
			count++;
	}
	//cout<<"Channel flags:"<<count<<endl;
	for(int i=0;i<l;i++,ptrZeroDM++)
	{		
		*ptrZeroDM=0;	
		ptrSmoothBandshape=&cumulativeBandshape[startChannel];	
		ptrRawData+=startChannel;			//startChannel number of channels skipped at the start of the band
		for(int j=0;j<nChan;j++,ptrRawData++,ptrSmoothBandshape++)
		{
			if(!freqFlags[j])
				(*ptrZeroDM)+=(*ptrRawData/ *ptrSmoothBandshape );
		}
		ptrRawData+=endExclude;				//endExclude number of channels skipped at the end of the band'
		(*ptrZeroDM)/=(float)count;
		if(*ptrZeroDM>maxZeroDM)
			maxZeroDM=*ptrZeroDM;
		if(*ptrZeroDM<minZeroDM)
			minZeroDM=*ptrZeroDM;
	}
	//cout<<"Min/Max:"<<minZeroDM<<","<<maxZeroDM<<endl;
}

/*******************************************************************
*FUNCTION: void BasicAnalysis::computeBandshape()
*Computes mean and mean-to-rms bandshape for the current block .
*It also calculates quantities to find the cumulative mean and 
*rms of the bandshape.
*******************************************************************/
void BasicAnalysis::computeBandshape()
{
	float	*ptrBandshape;
	double *ptrSquareBandshape;
	float	*ptrRawData;
	int 	startChannel=info.startChannel;
	int 	nChan= info.stopChannel-startChannel;				//Number of channels to use
	int 	endExclude=info.noOfChannels-info.stopChannel;			//Number of channels to exclude from the end
	int 	l= blockLength;
	ptrRawData=rawData;
	ptrBandshape=bandshape;
	ptrSquareBandshape=squareBandshape;

	//Intialization of bandshape
	for(int j=0;j<info.noOfChannels;j++,ptrBandshape++,ptrSquareBandshape++)
		*ptrBandshape=*ptrSquareBandshape=0;	
	for(int i=0;i<l;i++)
	{		
		ptrBandshape=&bandshape[startChannel]; 				//startChannel number of channels skipped at the start
		ptrSquareBandshape=&squareBandshape[startChannel];
		ptrRawData+=startChannel;
		for(int j=0;j<nChan;j++,ptrRawData++,ptrBandshape++,ptrSquareBandshape++)
		{
			(*ptrBandshape)+=(*ptrRawData);
			*ptrSquareBandshape+=(*ptrRawData)*(*ptrRawData);
		}
		ptrRawData+=endExclude;						//endExclude number of channels skipped at the end
		ptrBandshape+=endExclude;
		ptrSquareBandshape+=endExclude;
	}
	curBandpassCount=blockLength;
}


/*******************************************************************
*void BasicAnalysis::computeBandshape(char* timeFlags)
*char* timeFlags : Excludes the time samples for which timeFlag is 1
*This function is similar to void BasicAnalysis::computeBandshape(),
*the only difference being that timeFlags is used to ignore (or clip)
*bad time samples.
*******************************************************************/
void BasicAnalysis::computeBandshape(char* timeFlags)
{
	float *ptrBandshape;
	double *ptrSquareBandshape;
	float* 	ptrRawData;
	char*  	ptrTimeFlags;
	int 	startChannel=info.startChannel;
	int 	nChan= info.stopChannel-startChannel;
	int 	endExclude=info.noOfChannels-info.stopChannel;
	int 	l= blockLength;
	ptrRawData=rawData;
	ptrBandshape=bandshape;
	ptrTimeFlags=timeFlags;
	ptrSquareBandshape=squareBandshape;	
	//Initializing bandshape
	for(int j=0;j<info.noOfChannels;j++,ptrBandshape++,ptrSquareBandshape++)
		*ptrBandshape=*ptrSquareBandshape=0;		
			
	ptrTimeFlags=timeFlags;	
	//Finding number of time samples added to each channel bin
	curBandpassCount=0;
	for(int i=0;i<l;i++,ptrTimeFlags++)
		if(!(*ptrTimeFlags))		
			curBandpassCount++;
	ptrTimeFlags=timeFlags;	
	for(int i=0;i<l;i++,ptrTimeFlags++)
	{		
		ptrBandshape=&(bandshape[startChannel]);			//startChannel number of channels skipped at the start
		ptrSquareBandshape=&squareBandshape[startChannel];
		if(!(*ptrTimeFlags))
		{
			ptrRawData+=startChannel;			
			for(int j=0;j<nChan;j++,ptrRawData++,ptrBandshape++,ptrSquareBandshape++)
			{
				(*ptrBandshape)+=(*ptrRawData);	
				*ptrSquareBandshape+=(*ptrRawData)*(*ptrRawData);	
					
			}
			ptrRawData+=endExclude;					//endExclude number of channels skipped at the end
			ptrBandshape+=endExclude;
			ptrSquareBandshape+=endExclude;
		}
		else
			ptrRawData+=info.noOfChannels;				//skips entire time sample if flagged
	}
	ptrTimeFlags=timeFlags;	
	
}
void BasicAnalysis::calculateMeanByRMSBandshape()
{
	float *ptrBandshape,*ptrMeanToRmsBandshape;
	double *ptrSquareBandshape;
	int nChan= info.stopChannel-info.startChannel;
	ptrBandshape=&bandshape[info.startChannel];
	ptrSquareBandshape=&squareBandshape[info.startChannel];
	ptrMeanToRmsBandshape=&meanToRmsBandshape[info.startChannel];
	minMeanToRmsBandshape=minBandshape=1e8;
	maxMeanToRmsBandshape=maxBandshape=0;

	
	for(int j=0;j<nChan;j++,ptrBandshape++,ptrSquareBandshape++,ptrMeanToRmsBandshape++)
	{
		//Computation of mean bandshape:
		*ptrBandshape/=(float)curBandpassCount;
		if(*ptrBandshape>maxBandshape)
			maxBandshape=*ptrBandshape;
		if(*ptrBandshape<minBandshape)		
			minBandshape=*ptrBandshape;
		if(*ptrBandshape==0)
		{
			*ptrMeanToRmsBandshape=0;
			minMeanToRmsBandshape=0;
			continue;
		}
		//Computation of mean-to-rms bandshape:
		//if(*ptrMeanToRmsBandshape/curBandpassCount-pow(*ptrBandshape,2)<=0)
		//	cout<<"Something is funny!"<<j<<*ptrMeanToRmsBandshape<<","<<*ptrBandshape<<","<<curBandpassCount<<endl;
		*ptrMeanToRmsBandshape=(*ptrBandshape)/sqrt(*ptrSquareBandshape/curBandpassCount-pow(*ptrBandshape,2));
		
		
		if(*ptrMeanToRmsBandshape>maxMeanToRmsBandshape)
			maxMeanToRmsBandshape=*ptrMeanToRmsBandshape;
		if(*ptrMeanToRmsBandshape<minMeanToRmsBandshape)		
			minMeanToRmsBandshape=*ptrMeanToRmsBandshape;
		
	}
			
}

void BasicAnalysis::calculateCumulativeBandshapes()
{
	float *ptrBandshape,*ptrMeanToRmsBandshape;
	double *ptrSquareBandshape;
	int nChan= info.stopChannel-info.startChannel;
	ptrBandshape=&bandshape[info.startChannel];
	ptrSquareBandshape=&squareBandshape[info.startChannel];
	ptrMeanToRmsBandshape=&meanToRmsBandshape[info.startChannel];
	minMeanToRmsBandshape=minBandshape=1e8;
	maxMeanToRmsBandshape=maxBandshape=0;

	
	double *ptrSumBandshape=&sumBandshape[polarIndex][info.startChannel];
	double *ptrCumSquareBandshape=&cumSquareBandshape[polarIndex][info.startChannel];
	long long int *ptrCountBandshape=&countBandshape[polarIndex][info.startChannel];
	/*************************************************************************************
	*Calculation of cumulative values, current mean bandshape, mean-to-rms bandshape.
	*Also computes the max and min of each of these bandshapes.
	*These max-min values are used for plotting purposes.
	*The cumulative values are used to calculate the mean and the mean-to-rms bandshape
	*over the entire run of the program.
	**************************************************************************************/
	//cout<<*ptrCountBandshape<<","<<curBandpassCount<<endl;
	for(int j=0;j<nChan;j++,ptrBandshape++,ptrSumBandshape++,ptrCumSquareBandshape++,ptrSquareBandshape++,ptrCountBandshape++,ptrMeanToRmsBandshape++)
	{
		//Computation of cumulative values: 
		*ptrSumBandshape+=*ptrBandshape/cumBandshapeScale;
		*ptrCumSquareBandshape+=*ptrSquareBandshape/(cumBandshapeScale*cumBandshapeScale);
		*ptrCountBandshape+=curBandpassCount;	
		
		//Computation of mean bandshape:
		*ptrBandshape/=(float)curBandpassCount;
		if(*ptrBandshape>maxBandshape)
			maxBandshape=*ptrBandshape;
		if(*ptrBandshape<minBandshape)		
			minBandshape=*ptrBandshape;
		if(*ptrBandshape==0)
		{
			*ptrMeanToRmsBandshape=0;
			minMeanToRmsBandshape=0;
			continue;
		}
		//Computation of mean-to-rms bandshape:
		//if(*ptrMeanToRmsBandshape/curBandpassCount-pow(*ptrBandshape,2)<=0)
		//	cout<<"Something is funny!"<<j<<*ptrMeanToRmsBandshape<<","<<*ptrBandshape<<","<<curBandpassCount<<endl;
		*ptrMeanToRmsBandshape=(*ptrBandshape)/sqrt(*ptrSquareBandshape/curBandpassCount-pow(*ptrBandshape,2));
		
		
		if(*ptrMeanToRmsBandshape>maxMeanToRmsBandshape)
			maxMeanToRmsBandshape=*ptrMeanToRmsBandshape;
		if(*ptrMeanToRmsBandshape<minMeanToRmsBandshape)		
			minMeanToRmsBandshape=*ptrMeanToRmsBandshape;
		
	}
	if((int)info.bandshapeToUse==2 || info.doUseNormalizedData)
	{
		if(info.normalizationProcedure==1)
			smoothAndNormalizeBandshape();
		else
			normalizeBandshape();
	}
			
}
void BasicAnalysis::quicksort(float* x,long int first,long int last)
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
*FUNCTION: void BasicAnalysis::smoothAndNormalizeBandshape()
*Smoothens the mean bandshape and uses it to find the normalized 
*bandshape.
*The smooth bandshape here is used as an approximation to filtering
*response where smoothing ensures that the impact of RFI is minimized.
*Normilization inverts the effect of filter response to bring the power
*levels of each channel to roughly 1.
*Normalization helps in both time and frequency domain filtering.
*Refer to documentations on it for more details.
*******************************************************************/
void BasicAnalysis::smoothAndNormalizeBandshape() 
{
	int startChannel=info.startChannel;
	int nChan= info.stopChannel-startChannel;
	double *ptrSmoothSumBandshape=&smoothSumBandshape[polarIndex][startChannel];
	float *ptrSmoothBandshape=&smoothBandshape[startChannel];
	float *ptrNormalizedBandshape=&normalizedBandshape[startChannel];
	float *ptrBandshape=&bandshape[startChannel];	
	long long *ptrCountBandshape=&countBandshape[polarIndex][startChannel];
	for(int i=0;i<info.noOfChannels;i++)
		smoothBandshape[i]=0;
	int wSize=info.smoothingWindowLength/2;
	float* tempArray=new float[info.smoothingWindowLength];
	minNormalizedBandshape=maxNormalizedBandshape=0;
	
	for(int j=0;j<nChan;j++,ptrBandshape++,ptrSmoothBandshape++,ptrNormalizedBandshape++,ptrSmoothSumBandshape++,ptrCountBandshape++)
	{
		int tempCnt=0;	
		for(int i=-1*wSize;i<wSize;i++)
		{
			if(i+j>=nChan)
				break;
			if(i+j>=0)
			{
				tempArray[tempCnt]=*(ptrBandshape+i);
				tempCnt++;	
			}
		}
		quicksort(tempArray,0,tempCnt-1);
		*ptrSmoothBandshape=tempArray[tempCnt/2];
		
		*ptrSmoothSumBandshape+=(*ptrSmoothBandshape)*curBandpassCount/cumBandshapeScale;
		*ptrSmoothBandshape=(*ptrSmoothSumBandshape)*cumBandshapeScale/(*ptrCountBandshape);
		*ptrNormalizedBandshape=*ptrBandshape/(*ptrSmoothBandshape);
		if(*ptrNormalizedBandshape<minNormalizedBandshape)
			minNormalizedBandshape=*ptrNormalizedBandshape;
		if(*ptrNormalizedBandshape>maxNormalizedBandshape)
			maxNormalizedBandshape=*ptrNormalizedBandshape;
		if(*ptrSmoothBandshape==0)
		{
			*ptrSmoothBandshape=1;
			*ptrNormalizedBandshape=0;
		}
		
	}
	smoothBandshape[0]=-11205;
	delete[] tempArray;
}

/*******************************************************************
*FUNCTION: void BasicAnalysis::normalizeBandshape()
*Normalizes bandshape using externally supplied bandshape
*Normalization inverts the effect of filter response to bring the power
*levels of each channel to roughly 1.
*Normalization helps in both time and frequency domain filtering.
*Refer to documentations on it for more details.
*******************************************************************/
void BasicAnalysis::normalizeBandshape() 
{
	
	int nChan= info.stopChannel-info.startChannel;
	float *ptrSmoothBandshape=&smoothBandshape[info.startChannel];
	float *ptrNormalizedBandshape=&normalizedBandshape[info.startChannel];
	float *ptrBandshape=&bandshape[info.startChannel];
	for(int j=0;j<nChan;j++,ptrBandshape++,ptrSmoothBandshape++,ptrNormalizedBandshape++)
	{
		if(*ptrSmoothBandshape==0)
			*ptrNormalizedBandshape=0;
		*ptrNormalizedBandshape=*ptrBandshape/(*ptrSmoothBandshape);
		if(*ptrNormalizedBandshape<minNormalizedBandshape)
			minNormalizedBandshape=*ptrNormalizedBandshape;
		if(*ptrNormalizedBandshape>maxNormalizedBandshape)
			maxNormalizedBandshape=*ptrNormalizedBandshape;
	}
}

/*******************************************************************
*FUNCTION: void BasicAnalysis::NormalizeData()
*Normalizes data based on smooth bandshape.
*******************************************************************/
void BasicAnalysis::normalizeData()
{	
	int startChannel=info.startChannel;
	int nChan= info.stopChannel-startChannel;
	int endExclude=info.noOfChannels-info.stopChannel;
	float *ptrSmoothBandshape=&smoothBandshape[startChannel];
	float *ptrRawData=rawData;	
	for(int i=0;i<blockLength;i++)
	{							
		ptrRawData+=startChannel;
		ptrSmoothBandshape=&smoothBandshape[startChannel];
		for(int j=0;j<nChan;j++,ptrRawData++,ptrSmoothBandshape++)
			(*ptrRawData)=(*ptrRawData)/(*ptrSmoothBandshape);		
		ptrRawData+=endExclude;
		
	}
}
/*******************************************************************
*FUNCTION: void BasicAnalysis::setNormalizedBandpass(float *smoothBandshapeRR,float *smoothBandshapeLL)
*For full stokes data; the cross term bandshape = sqrt(bandshapeRR)*sqrt(bandshapeLL) [?].
*******************************************************************/
void BasicAnalysis::setNormalizedBandpass(float *smoothBandshapeRR,float *smoothBandshapeLL)
{	
	float *ptrSmoothBandshapeRR,*ptrSmoothBandshapeLL,*ptrSmoothBandshape;
	ptrSmoothBandshapeRR=smoothBandshapeRR;
	ptrSmoothBandshapeLL=smoothBandshapeLL;
	ptrSmoothBandshape=smoothBandshape;
	for(int j=0;j<info.noOfChannels;j++,ptrSmoothBandshapeRR++,ptrSmoothBandshapeLL++,ptrSmoothBandshape++)
		(*ptrSmoothBandshape)=sqrt(*ptrSmoothBandshapeRR)*sqrt(*ptrSmoothBandshapeLL);
	
}
/*******************************************************************
*FUNCTION: void BasicAnalysis::computeZeroDM(char* freqFlags)
*char* freqFlags : The channels marked 1 in this array is not 
*included while computing zeroDM (or are clipped)
*For no flagging, a char* variable with all 0's is passed on to this 
*function
*Subtracts the zero DM time series from each channel
*******************************************************************/
void BasicAnalysis::subtractZeroDM(char* freqFlags,float centralTendency)
{
	float*	ptrZeroDM;
	float*	ptrCorrelationBandshape;
	float*	ptrMeanToRmsBandshape;
	float*  ptrNormalizedBandshape;
	float* 	ptrRawData;
	float 	count=0;					//Stores the number of channels added to get each time sample	
	int 	startChannel=info.startChannel;
	int 	nChan= info.stopChannel-startChannel;		//Number of channels to use
	int 	endExclude=info.noOfChannels-info.stopChannel;	//Number of channels to exclude from the end of the band
	int 	l= blockLength;
	float 	zeroDMMean,zeroDMRMS;
	for(int i=0;i<nChan;i++)
		if(!freqFlags[i])
			count++;
	ptrZeroDM=zeroDM;
	zeroDMMean=0.0;	
	zeroDMRMS=0.0;
	for(int i=0;i<l;i++,ptrZeroDM++)
	{
		zeroDMMean+=*ptrZeroDM;
		zeroDMRMS+=(*ptrZeroDM)*(*ptrZeroDM);
	}
	zeroDMMean/=l;
	zeroDMRMS=zeroDMRMS/l-zeroDMMean*zeroDMMean;
	ptrZeroDM=zeroDM;
	ptrRawData=rawData;
	ptrCorrelationBandshape=correlationBandshape;
	for(int j=0;j<info.noOfChannels;j++,ptrCorrelationBandshape++)		
		*ptrCorrelationBandshape=0;
	for(int i=0;i<l;i++,ptrZeroDM++)
	{	
		ptrRawData+=startChannel;			//startChannel number of channels skipped at the start of the band
		ptrCorrelationBandshape=correlationBandshape+startChannel;
		ptrNormalizedBandshape=normalizedBandshape+startChannel;
		for(int j=0;j<nChan;j++,ptrRawData++,ptrCorrelationBandshape++,ptrNormalizedBandshape++)		
			*ptrCorrelationBandshape+=(*ptrZeroDM-zeroDMMean)*(*ptrRawData-*ptrNormalizedBandshape);
		ptrRawData+=endExclude;				//endExclude number of channels skipped at the end of the band
	}
	ptrZeroDM=zeroDM;
	ptrRawData=rawData;
	for(int i=0;i<l;i++,ptrZeroDM++)
	{	
		ptrRawData+=startChannel;			//startChannel number of channels skipped at the start of the band
		ptrCorrelationBandshape=correlationBandshape+startChannel;
		ptrNormalizedBandshape=normalizedBandshape+startChannel;
		ptrMeanToRmsBandshape=meanToRmsBandshape+startChannel;
		for(int j=0;j<nChan;j++,ptrRawData++,ptrNormalizedBandshape++,ptrCorrelationBandshape++,ptrMeanToRmsBandshape++)		
			*ptrRawData=(*ptrRawData-*ptrNormalizedBandshape)-(*ptrZeroDM-zeroDMMean)*(*ptrCorrelationBandshape/l)/zeroDMRMS+*ptrNormalizedBandshape;
		ptrRawData+=endExclude;				//endExclude number of channels skipped at the end of the band
	}
}
/*******************************************************************
*FUNCTION: void BasicAnalysis::getFilteredRawData(float replacementValue)
*float replacementValue - 0 or median/mode of zeroDM series
*Replaces flagged values by zero or median when 2d time-freq data
*is to be written out
*******************************************************************/
void BasicAnalysis::getFilteredRawData(char* timeFlags,char* freqFlags,float replacementValue)
{
	float* ptrRawData=rawData;
	char* ptrTimeFlags=timeFlags;
	char* ptrFreqFlags;
	
	long int pos;

	int startChannel=info.startChannel;
	int stopChannel=info.stopChannel;
	int totalChan=info.noOfChannels;
	int endExclude=info.noOfChannels-stopChannel;
	filteredRawData=new short int[blockLength*totalChan];
	short int* ptrFilteredRawData=filteredRawData;
	float meanval=info.meanval/replacementValue;
	for(long int i=0;i<blockLength;i++,ptrTimeFlags++)
	{
			ptrFreqFlags=freqFlags;	
			for(int j=0;j<startChannel;j++,ptrRawData++,ptrFilteredRawData++)
				*ptrFilteredRawData=(short int)(info.meanval);

			for(int j=startChannel;j<stopChannel;j++,ptrRawData++,ptrFreqFlags++,ptrFilteredRawData++)
			{

				if((!*ptrTimeFlags)&(!*ptrFreqFlags))
					*ptrFilteredRawData=(short int)round((*ptrRawData)*meanval);
				else
					*ptrFilteredRawData=(short int)(info.meanval);					
			}

			for(int j=stopChannel;j<totalChan;j++,ptrRawData++,ptrFilteredRawData++)
				*ptrFilteredRawData=(short int)(info.meanval);
	}
}

/*******************************************************************
*FUNCTION: void BasicAnalysis::getFilteredRawData(float replacementValue)
*float replacementValue - 0 or median/mode of zeroDM series
*Replaces flagged values by zero or median when 2d time-freq data
*is to be written out
*******************************************************************/
/**void BasicAnalysis::getFilteredRawData(char* timeFlags,char* freqFlags,float replacementValue)
{
	float* ptrRawData=rawData;
	char* ptrTimeFlags=timeFlags;
	char* ptrFreqFlags;
	
	long int pos;

	int startChannel=info.startChannel;
	int stopChannel=info.stopChannel;
	int totalChan=info.noOfChannels;
	int endExclude=info.noOfChannels-stopChannel;
	filteredRawData=new short int[blockLength*totalChan];
	short int* ptrFilteredRawData=filteredRawData;
	replacementValue=round(replacementValue);
	for(long int i=0;i<blockLength;i++,ptrTimeFlags++)
	{
			ptrFreqFlags=freqFlags;	
			for(int j=0;j<startChannel;j++,ptrRawData++,ptrFilteredRawData++)
				*ptrFilteredRawData=(short int)(replacementValue*info.meanval);

			for(int j=startChannel;j<stopChannel;j++,ptrRawData++,ptrFreqFlags++,ptrFilteredRawData++)
			{

				if((!*ptrTimeFlags)&(!*ptrFreqFlags))
					*ptrFilteredRawData=(short int)((*ptrRawData)*info.meanval);
				else
					*ptrFilteredRawData=(short int)(replacementValue*info.meanval);					
			}

			for(int j=stopChannel;j<totalChan;j++,ptrRawData++,ptrFilteredRawData++)
				*ptrFilteredRawData=(short int)(replacementValue*info.meanval);
	}
}**/
/*******************************************************************
*FUNCTION: void BasicAnalysis::getFilteredRawDataChar(float replacementValue)
*float replacementValue - 0 or median/mode of zeroDM series
*Replaces flagged values by zero or median when 2d time-freq data
*is to be written out
*******************************************************************/
void BasicAnalysis::getFilteredRawDataChar(char* timeFlags,char* freqFlags,float replacementValue)
{
	float* ptrRawData=rawData;
	char* ptrTimeFlags=timeFlags;
	char* ptrFreqFlags;
	
	long int pos;

	int startChannel=info.startChannel;
	int stopChannel=info.stopChannel;
	int totalChan=info.noOfChannels;
	int endExclude=info.noOfChannels-stopChannel;
	filteredRawDataChar=new unsigned char[blockLength*totalChan];
	unsigned char* ptrFilteredRawData=filteredRawDataChar;
	for(long int i=0;i<blockLength;i++,ptrTimeFlags++)
	{
			ptrFreqFlags=freqFlags;	
			for(int j=0;j<startChannel;j++,ptrRawData++,ptrFilteredRawData++)
				*ptrFilteredRawData=(unsigned char)(replacementValue*info.meanval);

			for(int j=startChannel;j<stopChannel;j++,ptrRawData++,ptrFreqFlags++,ptrFilteredRawData++)
			{

				if((!*ptrTimeFlags)&(!*ptrFreqFlags))
					*ptrFilteredRawData=(unsigned char)((*ptrRawData)*info.meanval);
				else
					*ptrFilteredRawData=(unsigned char)(replacementValue*info.meanval);					
			}

			for(int j=stopChannel;j<totalChan;j++,ptrRawData++,ptrFilteredRawData++)
				*ptrFilteredRawData=(unsigned char)(replacementValue*info.meanval);
	}
}
/*******************************************************************
*FUNCTION: void BasicAnalysis::getFilteredRawDataSmoothBshape(char* timeFlags,char* freqFlags)
*Replaces flagged values by the smooth bandshape value for that channel
*******************************************************************/
void BasicAnalysis::getFilteredRawDataSmoothBshape(char* timeFlags,char* freqFlags)
{
	float* ptrRawData=rawData;
	char* ptrTimeFlags=timeFlags;
	char* ptrFreqFlags;
	
	long int pos;

	int startChannel=info.startChannel;
	int stopChannel=info.stopChannel;
	int totalChan=info.noOfChannels;
	int endExclude=info.noOfChannels-stopChannel;
	filteredRawData=new short int[blockLength*totalChan];
	short int* ptrFilteredRawData=filteredRawData;
	float *ptrSmoothBandshape;
	for(long int i=0;i<blockLength;i++,ptrTimeFlags++)
	{
			ptrFreqFlags=freqFlags;	
			ptrSmoothBandshape=smoothBandshape;
			for(int j=0;j<startChannel;j++,ptrRawData++,ptrFilteredRawData++,ptrSmoothBandshape++)
				*ptrFilteredRawData=*ptrSmoothBandshape;

			for(int j=startChannel;j<stopChannel;j++,ptrRawData++,ptrFreqFlags++,ptrFilteredRawData++,ptrSmoothBandshape++)
			{

				if((!*ptrTimeFlags)&(!*ptrFreqFlags))
					*ptrFilteredRawData=(short int)(*ptrRawData);
				else
					*ptrFilteredRawData=(short int)(*ptrSmoothBandshape);					
			}

			for(int j=stopChannel;j<totalChan;j++,ptrRawData++,ptrFilteredRawData++,ptrSmoothBandshape++)
				*ptrFilteredRawData=*ptrSmoothBandshape;
	}
}
/*******************************************************************
*FUNCTION: void BasicAnalysis::getFilteredRawDataSmoothBshape(char* timeFlags,char* freqFlags)
*Replaces flagged values by the smooth bandshape value for that channel
*******************************************************************/
void BasicAnalysis::getFilteredRawDataSmoothBshapeChar(char* timeFlags,char* freqFlags)
{
	float* ptrRawData=rawData;
	char* ptrTimeFlags=timeFlags;
	char* ptrFreqFlags;
	
	long int pos;

	int startChannel=info.startChannel;
	int stopChannel=info.stopChannel;
	int totalChan=info.noOfChannels;
	int endExclude=info.noOfChannels-stopChannel;
	filteredRawDataChar=new unsigned char[blockLength*totalChan];
	unsigned char* ptrFilteredRawData=filteredRawDataChar;
	float *ptrSmoothBandshape;
	for(long int i=0;i<blockLength;i++,ptrTimeFlags++)
	{
			ptrFreqFlags=freqFlags;	
			ptrSmoothBandshape=smoothBandshape;
			for(int j=0;j<startChannel;j++,ptrRawData++,ptrFilteredRawData++,ptrSmoothBandshape++)
				*ptrFilteredRawData=(unsigned char)(*ptrSmoothBandshape);

			for(int j=startChannel;j<stopChannel;j++,ptrRawData++,ptrFreqFlags++,ptrFilteredRawData++,ptrSmoothBandshape++)
			{

				if((!*ptrTimeFlags)&(!*ptrFreqFlags))
					*ptrFilteredRawData=(unsigned char)(*ptrRawData);
				else
					*ptrFilteredRawData=(unsigned char)(*ptrSmoothBandshape);					
			}

			for(int j=stopChannel;j<totalChan;j++,ptrRawData++,ptrFilteredRawData++,ptrSmoothBandshape++)
				*ptrFilteredRawData=(unsigned char)(*ptrSmoothBandshape);
	}
}
/*******************************************************************
*FUNCTION: void BasicAnalysis::writeFilteredRawData(const char*  filename)
*const char*  fileName: Filename to write to.
*Writes out filtered rawdata where flagged samples have been replaced by 
*zeros
*******************************************************************/
void BasicAnalysis::writeFilteredRawData(const char*  filename)
{
	FILE* filteredRawDataFile;    
	filteredRawDataFile = fopen(filename, "ab");
	fwrite(filteredRawData, sizeof(short int), blockLength*info.noOfChannels, filteredRawDataFile); 
	fclose(filteredRawDataFile); 
}
/*******************************************************************
*FUNCTION: void BasicAnalysis::writeBandshape(char* filename)
*char* filename: Name of the file to which mean, mean smooth and rms bandshape will 
*be written out.
*Writes out the mean, mean smooth and rms bandshape (over the entire run of the program)
*to the specified file.
*******************************************************************/
void BasicAnalysis::writeBandshape(const char* filename)
{
	ofstream meanFile;
	meanFile.open(filename);
	double sMean,mean,rms;
	meanFile<<"# chan_no \t smooth_bshape \t mean_bshape \t rms_bshape"<<endl;
	for(int i=0;i<info.noOfChannels;i++)
	{
		if(countBandshape[polarIndex][i]!=0)		
		{
			sMean=smoothSumBandshape[polarIndex][i]/countBandshape[polarIndex][i];	
			mean=sumBandshape[polarIndex][i]/countBandshape[polarIndex][i];	
			rms=sqrt((cumSquareBandshape[polarIndex][i]/countBandshape[polarIndex][i])-pow(mean,2));	
		}		
		else
		{
			sMean=0;
			mean=0;
			rms=-1;
		}
		meanFile<<i<<" "<<sMean*cumBandshapeScale<<" "<<mean*cumBandshapeScale<<" "<<rms*cumBandshapeScale<<endl;	
			
	}
	meanFile.close();
}

/*******************************************************************
*FUNCTION: void BasicAnalysis::writeBandshape(char* filename)
*char* filename: Name of the file to which mean bandshape will 
*be written out.
*Writes out the mean bandshape (for current block)
*to the specified file.
*******************************************************************/
void BasicAnalysis::writeCurBandshape(const char* filename)
{
	ofstream meanFile;
	meanFile.open(filename,ios::app);
	meanFile<<blockLength<<" ";
	if((int)info.bandshapeToUse==2 || info.doUseNormalizedData)
		for(int i=0;i<info.noOfChannels;i++)
		{			
			if(smoothBandshape[i]==0)
				meanFile<<0.0<<" ";	
			else
				meanFile<<bandshape[i]/smoothBandshape[i]<<" ";			
		}	
	else	
		for(int i=0;i<info.noOfChannels;i++)
		{			
				meanFile<<bandshape[i]<<" ";				
		}	
	
	meanFile<<endl;
	meanFile.close();
}

//implementation of BasicAnalysis methods ends
