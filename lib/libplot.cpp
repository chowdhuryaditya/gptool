#include <Plot.h>

//Implementation of Plot methods begins
float*		Plot::profileAxis;
float* 		Plot::DMAxis;
float* 		Plot::timeAxis;
float*		Plot::bandshapeAxis;
Information	Plot::info;
int		Plot::totalBlocks;

Plot::Plot(Information info_,int totalBlocks_)
{
	info=info_;
	totalBlocks=totalBlocks_;
	profileAxis=new float[info.periodInSamples];
	for(int i=0;i<info.periodInSamples;i++)
		profileAxis[i]=i;
	DMAxis=new float[info.blockSizeSamples+1];
	for(long int i=0;i<info.blockSizeSamples+1;i++)
		DMAxis[i]=i;
	timeAxis=new float[info.blockSizeSamples+1];
	for(long int i=0;i<info.blockSizeSamples+1;i++)
		timeAxis[i]=i*info.samplingInterval*1000;
	bandshapeAxis=new float[info.stopChannel-info.startChannel];
	for(long int i=info.startChannel;i<info.stopChannel;i++)
		bandshapeAxis[i-info.startChannel]=i;
	cpgbeg(0, "?", 1, 1);
	cpgpap (0.0,0.618); //10.0
	cpgsch(0.85);
	if(info.doManualMode)
		cpgask(1);
	else
		cpgask(0);
}


void Plot::plotProfile(float *profile,float maxProfile,float minProfile,int k)
{ 
	cpgsci(1);
	if(!info.doPolarMode || (info.doPolarMode && info.polarChanToDisplay!=-1) )
		cpgsvp(0.68,0.98,0.07,0.22);		
	else
	{
		switch(k)
		{
			case 0:
				cpgsvp(0.65,0.98,0.07,0.17);	
				break;
			case 2:
				cpgsvp(0.65,0.98,0.18,0.28);	
				break;	
			case 1:
				cpgsvp(0.65,0.88,0.86,0.96);
				break;
			case 3:
				cpgsvp(0.65,0.88,0.75,0.85);
				break;	
		}	
	}

	cpgswin(0,info.periodInSamples,minProfile-(0.1)*(maxProfile-minProfile),maxProfile+(0.1)*(maxProfile-minProfile));
 
	if(!info.doPolarMode || (info.doPolarMode && info.polarChanToDisplay!=-1))
	{		
		cpgbox("BNST",0.0,0,"BCNSTV",0.0,0);
		cpgtext(info.periodInSamples/3.7,minProfile-(maxProfile-minProfile)*0.6,"Filtered Profile (bin index)");
		cpgsci(k);
	}
	else
	{
		switch(k)
		{
			case 0:
				cpgbox("BNST",0.0,0,"BCNVST",0.0,2);
				cpgtext(info.periodInSamples/2.7,minProfile-(maxProfile-minProfile)*0.75,"Profile");
				break;
			case 2:		
				cpgbox("BST",0.0,0,"BCNVST",0.0,2);
				break;
			case 1:
				cpgbox("BST",0.0,0,"BCNVST",0.0,2);
				cpgtext(info.periodInSamples/2.7,minProfile+(maxProfile-minProfile)*1.2,"Profile");
				break;
			case 3:
				cpgbox("BNST",0.0,0,"BCNVST",0.0,2);				
				break;
		}
		cpgsci(7-k);	
	}
	
	cpgline(info.periodInSamples,profileAxis,profile);
}

void Plot::plotProfileUnfiltered(float *profile,float maxProfile,float minProfile)
{ 
	cpgsci(1);	
	cpgsvp(0.68,0.98,0.80,0.95);		
	cpgswin(0,info.periodInSamples,minProfile-(0.1)*(maxProfile-minProfile),maxProfile+(0.1)*(maxProfile-minProfile));
 	cpgbox("BNST",0.0,0,"BCNSTV",0.0,0);
	cpgtext(info.periodInSamples/4.0,maxProfile+(maxProfile-minProfile)*0.3,"Unfiltered Profile (bin index)");
	cpgline(info.periodInSamples,profileAxis,profile);
}

void Plot::plotFullDM(float *fullDM,float flMax,float flMin,int length,int k)
{
  	cpgsci(1); 

	
	if(!info.doPolarMode || (info.doPolarMode && info.polarChanToDisplay!=-1))
	{
		cpgsvp(0.07,0.60,0.80,0.95);  
	}
	else
	{		
		if(k==0 || k==2)
			cpgsvp(0.07,0.55,0.75,0.85);  
		
		else
			cpgsvp(0.07,0.55,0.86,0.96); 		
	}
	cpgswin(0,info.blockSizeSamples+1, flMin-(0.1)*(flMax-flMin),flMax+(0.3)*(flMax-flMin));
	if(!info.doPolarMode || (info.doPolarMode && info.polarChanToDisplay!=-1))
	{
		cpgbox("BNST",0.0,0,"BCNSTV",0.0,0);
		cpgtext((info.blockSizeSamples+1)/6.0,flMin-(flMax-flMin)*0.57,"Dedispersed time series (Time sample index)");
		cpgsci(k);		
	}
	else
	{
		
		if(k==0 || k==2)
			cpgbox("BNST",0.0,0,"BCNSTV",0.0,0);				
		else
		{
			cpgbox("BST",0.0,0,"BCNSTV",0.0,0);
			cpgtext(info.blockSizeSamples/3.0,flMin+(flMax-flMin)*1.2,"Dedispersed time series");			
		}
		cpgsci(7-k);
	}
	cpgline(length,DMAxis,fullDM);
}

void Plot::plotOtherBandshape(float *bandshape,float maxBandshape,float minBandshape,int k)
{
  	cpgsci(1);  	
   	cpgsvp(0.88,0.98,0.27,0.72);
   	if(info.sidebandFlag)
  		cpgswin(minBandshape-0.1*(maxBandshape-minBandshape),maxBandshape+0.1*(maxBandshape-minBandshape),info.startChannel,info.stopChannel);
 	 else
		cpgswin(minBandshape-0.1*(maxBandshape-minBandshape),maxBandshape+0.1*(maxBandshape-minBandshape),info.stopChannel,info.startChannel);
  	cpgbox("BNCT",0.0,0,"CST",0.0,0);
  	cpgline(info.stopChannel-info.startChannel,&bandshape[info.startChannel],bandshapeAxis);
}

void Plot::plotZeroDM(float *zeroDM,float zrMax,float zrMin,int length,int k)
{
  	cpgsci(1);
	if(!info.doPolarMode || (info.doPolarMode && info.polarChanToDisplay!=-1))
  		cpgsvp(0.07,0.60,0.07,0.19);
	else
	{
		if(k==0 || k==2)
			cpgsvp(0.07,0.55,0.07,0.17);
		else
			cpgsvp(0.07,0.55,0.18,0.28);
			
	}
  	cpgswin(0,info.blockSizeSec*1000.0,zrMin-0.1*(zrMax-zrMin),zrMax+0.1*(zrMax-zrMin));
	if(!info.doPolarMode || (info.doPolarMode && info.polarChanToDisplay!=-1))
	{
  		cpgbox("BNST",0.0,0,"BCNSTV",0.0,0);
  		cpgmtxt("B",2.5,0.5,0.5,"zero DM time series (Time in ms)");
  		cpgsci(k);
	}
	else
	{
		if(k==0 || k==2)
		{
			cpgbox("BNST",0.0,0,"BCNSTV",0.0,0);
			cpgtext(info.blockSizeSamples/3.0,zrMin-(zrMax-zrMin)*0.8,"zero DM time series (Time in ms)");	
		}
		else		
			cpgbox("BST",0.0,0,"BCNSTV",0.0,0);  			
		
		cpgsci(7-k);
	}
  	cpgline(length,timeAxis,zeroDM);
}

void Plot::plotBandshape(float *bandshape,float maxBandshape,float minBandshape,int k)
{
	cpgsci(1);
	if(!info.doPolarMode || (info.doPolarMode && info.polarChanToDisplay!=-1))
  		cpgsvp(0.68,0.88,0.27,0.72);
	else
	{
		if(k==0 || k==2)
			cpgsvp(0.6,0.78,0.34,0.70);
		else
			cpgsvp(0.80,0.98,0.34,0.70);
	}
  	if(info.sidebandFlag)
  		cpgswin(minBandshape-0.1*(maxBandshape-minBandshape),maxBandshape+0.1*(maxBandshape-minBandshape),info.startChannel,info.stopChannel);
  	else
		cpgswin(minBandshape-0.1*(maxBandshape-minBandshape),maxBandshape+0.1*(maxBandshape-minBandshape),info.stopChannel,info.startChannel);
	if(!info.doPolarMode || (info.doPolarMode && info.polarChanToDisplay!=-1))
	{
		cpgbox("BMCST",0.0,0,"BNST",0.0,0);
  		cpglab("", "Channel number", "");
  		if(info.sidebandFlag==0)
  			cpgtext((maxBandshape-minBandshape)/2.0,info.stopChannel+0.1*(info.stopChannel-info.startChannel),"Bandshape");
  		else
  			cpgtext((maxBandshape-minBandshape)/2.0,info.startChannel-0.1*(info.stopChannel-info.startChannel),"Bandshape");
  		
		cpgsci(k);
	}
	else
	{
		if(k==0 || k==2)
		{
			cpgbox("BNCST",0.0,0,"BNST",0.0,0);
			cpgptxt(minBandshape+(maxBandshape-minBandshape)*1.1,info.startChannel+(info.stopChannel-info.startChannel)*0.35,90,0,"Bandshape");				
		}
		else
		{
			cpgbox("BNCST",0.0,0,"BST",0.0,0);  
					
		}
		
		cpgsci(7-k);
	}  
  	
  	cpgline(info.stopChannel-info.startChannel,&bandshape[info.startChannel],bandshapeAxis);
}

void Plot::plotPolarLegend()
{
	cpgsvp(0.90,0.98,0.72,0.96);	
	cpgswin(0,100,0,100);
	
	cpgsci(7);
	float timeX[2]={0,25};
	float timeY[2]={25,25};	
	cpgtext(30,timeY[0]-2,"Self A");
	cpgline(2,timeX,timeY);		

	cpgsci(5);
	timeY[0]=50;
	timeY[1]=timeY[0];	
	cpgtext(30,timeY[0]-2,"Self B");
	cpgline(2,timeX,timeY);	

	cpgsci(6);
	timeY[0]=75;
	timeY[1]=timeY[0];
	cpgtext(30,timeY[0]-2,"Cross A");
	cpgline(2,timeX,timeY);	

	cpgsci(4);
	timeY[0]=100;
	timeY[1]=timeY[0];	
	cpgtext(30,timeY[0]-2,"Cross B");
	cpgline(2,timeX,timeY);		
	
}
void Plot::plotWaterfall(float* rawData,long int blockLength,float maxRawData,float minRawData)
{

	/*float RL[]={-0.5, 0.0, 0.17, 0.33,  0.5, 0.67, 0.83, 1.0, 1.7};
	float RR[]={ 0.0, 0.0,  0.0,  0.0,  0.6,  1.0,  1.0, 1.0, 1.0};
	float RG[]={ 0.0, 0.0,  0.0,  1.0,  1.0,  1.0,  0.6, 0.0, 1.0};
	float RB[]={ 0.0, 0.3,  0.8,  1.0,  0.3,  0.0,  0.0, 0.0, 1.0};

	float contra = 1,bright = 0.5;
	cpgctab(RL, RR, RG, RB, 9, contra, bright);*/
	cpgsfs(1);
	if(!info.doPolarMode || (info.doPolarMode && info.polarChanToDisplay!=-1))
		cpgsvp(0.07,0.60,0.27,0.72);
	else
		cpgsvp(0.07,0.55,0.34,0.70);
	cpgswin(0, info.blockSizeSec*1000.0, info.lowestFrequency+info.startChannel*info.bandwidth/(float)info.noOfChannels,info.lowestFrequency+info.stopChannel*info.bandwidth/(float)info.noOfChannels);
	if(!info.sidebandFlag)	
		cpgswin(0, info.blockSizeSec*1000.0, info.lowestFrequency+(info.noOfChannels-info.stopChannel)*info.bandwidth/(float)info.noOfChannels,info.lowestFrequency+(info.noOfChannels-info.startChannel)*info.bandwidth/(float)info.noOfChannels);
	
  
	cpgbox("BCINST",0.0,0,"BCINST",0.0,0);
	cpglab("", "Frequency (Mhz)", "");
	float tr[]={0,0, info.samplingInterval*1000.0,info.lowestFrequency,info.bandwidth/(float)info.noOfChannels,0};
	if(!info.sidebandFlag)    
	{	
		tr[3]=info.lowestFrequency+info.bandwidth;
		tr[4]*=-1;
	}
 	cpgimag(rawData,info.noOfChannels,blockLength,info.startChannel+1,info.stopChannel,1,blockLength,minRawData,maxRawData,tr);
 
}
void Plot::plotFullDMCount(int* count,long int blockLength)
{

	/*float RL[]={-0.5, 0.0, 0.17, 0.33,  0.5, 0.67, 0.83, 1.0, 1.7};
	float RR[]={ 0.0, 0.0,  0.0,  0.0,  0.6,  1.0,  1.0, 1.0, 1.0};
	float RG[]={ 0.0, 0.0,  0.0,  1.0,  1.0,  1.0,  0.6, 0.0, 1.0};
	float RB[]={ 0.0, 0.3,  0.8,  1.0,  0.3,  0.0,  0.0, 0.0, 1.0};

	float contra = 1,bright = 0.5;
	cpgctab(RL, RR, RG, RB, 9, contra, bright);*/
	cpgsfs(1);
	cpgsvp(0.07,0.60,0.93,0.945);	
	cpgswin( 0,info.blockSizeSamples+1,1,2);
	
  
	float tr[]={0,0, 1,0,1,0};
	float* countfl=new float[blockLength];
	for(int i=0;i<blockLength;i++)
		countfl[i]=count[i];
	
 	cpgimag(countfl,1,blockLength,1,1,1,blockLength,0,info.noOfChannels,tr);
	delete[] countfl;
 
}
void Plot::plotHistogram(float* histogram,float* histogramAxis,float histogramSize,float histMax)
{
	float histMin=0;
  	cpgsvp(0.68,0.98,0.77,0.92);
  	cpgswin(-8,+15,histMin-(0.1)*(histMax-histMin),histMax+(0.1)*(histMax-histMin));
  	cpgbox("BNST",0.0,0,"BCNSTV",0.0,0);
  	cpgline(histogramSize,histogramAxis,histogram);
}

void Plot::plotChanFlags(char* chanFlag,int length)
{
 	cpgsvp(0.87,0.8825,0.27,0.72);
        if(info.sidebandFlag)  
        	cpgswin(0,1,0,info.stopChannel-info.startChannel);
	else
		cpgswin(0,1,info.stopChannel-info.startChannel,0);
        cpgrect(0,1,0,info.stopChannel-info.startChannel);
	float timeY[2];
	float timeX[2]={0,1};	
        for(int i=0;i<length;i++)
        {
          	if(chanFlag[i]==1)
          	{
            		timeY[0]=i;
            		timeY[1]=timeY[0];
            		cpgsci(0);
            		cpgline(2,timeX,timeY);
            		cpgsci(1);
          	}
        }
       
}

void Plot::plotTimeFlags(char* timeFlag,int length)
{
	if(info.smoothFlagWindowLength>0)
 		cpgsvp(0.07,0.60,0.20,0.21);
	else
		cpgsvp(0.07,0.60,0.20,0.22);
        cpgswin(0,info.blockSizeSamples+1,0, 1);
        cpgrect(0,length,0,1);
	float timeX[2];
	float timeY[2]={0,1};
        for(long int i=0;i<length;i++)
        {
          	if(timeFlag[i]==1)
          	{
            		timeX[0]=i;
            		timeX[1]=timeX[0];
            		cpgsci(0);
            		cpgline(2,timeX,timeY);
            		cpgsci(1);
          	}
        }
}

void Plot::plotSmoothFlags(float* sFlags,long int blockLength)
{

	/*float RL[]={-0.5, 0.0, 0.17, 0.33,  0.5, 0.67, 0.83, 1.0, 1.7};
	float RR[]={ 0.0, 0.0,  0.0,  0.0,  0.6,  1.0,  1.0, 1.0, 1.0};
	float RG[]={ 0.0, 0.0,  0.0,  1.0,  1.0,  1.0,  0.6, 0.0, 1.0};
	float RB[]={ 0.0, 0.3,  0.8,  1.0,  0.3,  0.0,  0.0, 0.0, 1.0};

	float contra = 1,bright = 0.5;
	cpgctab(RL, RR, RG, RB, 9, contra, bright);*/
	cpgsfs(1);
	cpgsvp(0.07,0.60,0.215,0.24);	
	cpgswin( 0,info.blockSizeSamples+1,1,2);
	
  
	float tr[]={0,0, 1,0,1,0};
	
 	cpgimag(sFlags,1,blockLength,1,1,1,blockLength,0,1,tr);
 
}
void Plot::plotDedispFlags(char* dedispFlags,int length)
{
 	cpgsvp(0.07,0.60,0.95,0.954);
        cpgswin(0,info.blockSizeSamples+1,0, 1);
        cpgrect(0,length,0,1);
	float timeX[2];
	float timeY[2]={0,1};
        for(long int i=0;i<length;i++)
        {
          	if(dedispFlags[i]==1)
          	{
            		timeX[0]=i;
            		timeX[1]=timeX[0];
            		cpgsci(0);
            		cpgline(2,timeX,timeY);
            		cpgsci(1);
          	}
        }
}

void Plot::plotBlockIndex(int index)
{
	cpgsci(1);
	cpgsvp(0.0,0.40,0.90,1.00);
	cpgswin(0,100,0,100);
	ostringstream text1;
	if(info.doReadFromFile)
		text1<<"Block: "<<info.startBlockIndex+index+1<<"/"<<totalBlocks<<" | Time: "<<setiosflags(ios::fixed) << setprecision(1)<<(info.startBlockIndex+index)*info.blockSizeSec<<"/"<<setiosflags(ios::fixed) << setprecision(1)<<(totalBlocks-1)*info.blockSizeSec<<" sec";
	else
		text1<<"Block: "<<index+1<<" | Time: "<<setiosflags(ios::fixed) << setprecision(1)<<(index+1)*info.blockSizeSec<<" sec";

	cpgtext(0,80,text1.str().c_str());

}

void Plot::plotTitle()
{
	cpgsci(8);
	cpgsvp(0.40,0.65,0.90,1.00);
	cpgswin(0,100,0,100);
	ostringstream text1;
	text1<<"GMRT Pulsar Tool ver 4.4.5";
	cpgtext(0,80,text1.str().c_str());
	cpgsci(1);

}

void Plot::foldingStartMessage()
{
	ostringstream text1;
	cpgsvp(0.68,0.98,0.80,0.95);		
	cpgswin(0,100,0,100);	
	text1<<"Profile folding will start once sufficient time";
	cpgtext(0,80,text1.str().c_str());

	ostringstream text2;
	text2<<"has elapsed so that all channels are";
	cpgtext(0,65,text2.str().c_str());

	ostringstream text3;
	text3<<"available to each dedispersed series bin.";
	cpgtext(0,50,text3.str().c_str());
	

}

//Single Channel mode methods
void Plot::plotZeroDMSingleChannel(float *zeroDM,float zrMax,float zrMin,int length)
{
  	cpgsci(1);
	cpgsvp(0.05,0.95,0.55,0.9);
  	cpgswin(0,info.blockSizeSec*1000.0,zrMin-0.1*(zrMax-zrMin),zrMax+0.1*(zrMax-zrMin));
  	cpgbox("BNST",0.0,0,"BCNSTV",0.0,0);
  	cpgmtxt("B",2.5,0.5,0.5,"Time series (Time in ms)");
  	cpgline(length,timeAxis,zeroDM);
}

void Plot::plotTimeFlagsSingleChannel(char* timeFlag,int length)
{
	cpgsvp(0.05,0.95,0.9,0.93);
        cpgswin(0,info.blockSizeSamples+1,0, 1);
        cpgrect(0,length,0,1);
	float timeX[2];
	float timeY[2]={0,1};
        for(long int i=0;i<length;i++)
        {
          	if(timeFlag[i]==1)
          	{
            		timeX[0]=i;
            		timeX[1]=timeX[0];
            		cpgsci(0);
            		cpgline(2,timeX,timeY);
            		cpgsci(1);
          	}
        }
}

void Plot::plotProfileSingleChannel(float *profile,float maxProfile,float minProfile)
{ 
	cpgsci(3);	
	cpgsvp(0.05,0.45,0.1,0.45);		
	cpgswin(0,info.periodInSamples,minProfile-(0.1)*(maxProfile-minProfile),maxProfile+(0.1)*(maxProfile-minProfile));
 	cpgbox("BCNST",0.0,0,"BCNSTV",0.0,0);
	cpgtext(info.periodInSamples/3.7,minProfile-(maxProfile-minProfile)*0.3,"Filtered Profile (bin index)");
	cpgline(info.periodInSamples,profileAxis,profile);
}


void Plot::plotProfileUnfilteredSingleChannel(float *profile,float maxProfile,float minProfile)
{ 
	cpgsci(1);	
	cpgsvp(0.5,0.95,0.1,0.45);		
	cpgswin(0,info.periodInSamples,minProfile-(0.1)*(maxProfile-minProfile),maxProfile+(0.1)*(maxProfile-minProfile));
 	cpgbox("BCNST",0.0,0,"BCNSTV",0.0,0);
	cpgtext(info.periodInSamples/3.7,minProfile-(maxProfile-minProfile)*0.3,"Unfiltered Profile (bin index)");
	cpgline(info.periodInSamples,profileAxis,profile);
}



void Plot::plotAllNonPolar(BasicAnalysis* basicAnalysis,AdvancedAnalysis* advancedAnalysis,RFIFiltering* rFIFilteringTime,RFIFiltering* rFIFilteringChan,int index)
{
	cpgpage();	
	if(advancedAnalysis->hasEnoughDedispersedData!=0)
	{		
		plotProfile(advancedAnalysis->curFoldedProfile,advancedAnalysis->maxProfile,advancedAnalysis->minProfile,3);
		cpgsci(1);
		plotProfileUnfiltered(advancedAnalysis->curFoldedProfileUnfiltered,advancedAnalysis->maxProfileUnfiltered,advancedAnalysis->minProfileUnfiltered);
	}	
	else
	{
		foldingStartMessage();
	}
	plotFullDM(advancedAnalysis->fullDMUnfiltered,advancedAnalysis->maxFullDMUnfiltered,advancedAnalysis->minFullDM,advancedAnalysis->length,1);
	plotFullDM(advancedAnalysis->fullDM,advancedAnalysis->maxFullDMUnfiltered,advancedAnalysis->minFullDM,advancedAnalysis->length,3);
	plotZeroDM(basicAnalysis->zeroDMUnfiltered,basicAnalysis->maxZeroDM,basicAnalysis->minZeroDM,basicAnalysis->blockLength,1);
	plotZeroDM(basicAnalysis->zeroDM,basicAnalysis->maxZeroDM,basicAnalysis->minZeroDM,basicAnalysis->blockLength,3);
	cpgsci(1);
	plotTimeFlags(rFIFilteringTime->flags,rFIFilteringTime->inputSize);
	if(info.smoothFlagWindowLength>0)
		plotSmoothFlags(rFIFilteringTime->sFlags,rFIFilteringTime->inputSize);
	plotDedispFlags(advancedAnalysis->dedispFlags,advancedAnalysis->length);
	plotFullDMCount(advancedAnalysis->count,advancedAnalysis->length);
	plotBandshape(basicAnalysis->bandshape,basicAnalysis->maxBandshape,basicAnalysis->minBandshape,1);
	
	
	if((int)info.bandshapeToUse==3 || (int)info.bandshapeToUse==4)
		plotOtherBandshape(basicAnalysis->meanToRmsBandshape,basicAnalysis->maxMeanToRmsBandshape,basicAnalysis->minMeanToRmsBandshape,1);
	else if((int)info.bandshapeToUse==2)
		plotOtherBandshape(basicAnalysis->normalizedBandshape,1.5,0.5,1);
	plotChanFlags(rFIFilteringChan->flags,rFIFilteringChan->inputSize);


	if((int)info.bandshapeToUse==2 || info.doUseNormalizedData)
		plotBandshape(basicAnalysis->smoothBandshape,basicAnalysis->maxBandshape,basicAnalysis->minBandshape,3);
		cpgsci(1);	
	
	plotWaterfall(basicAnalysis->rawData,basicAnalysis->blockLength,(basicAnalysis->maxZeroDM),(basicAnalysis->minZeroDM));
	/*if(info.timeFlagAlgo==1 && info.doTimeFlag)
		plotHistogram(rFIFilteringTime->histogram,rFIFilteringTime->histogramAxis,rFIFilteringTime->histogramSize,rFIFilteringTime->histogramMax);
	*/
	plotTitle();
	plotBlockIndex(index);
}

void Plot::plotAllPolar(BasicAnalysis** basicAnalysis,AdvancedAnalysis** advancedAnalysis,int index)
{
	cpgpage();	
	float minFullDM,maxFullDM,minProfile,maxProfile,maxRangeProfile,minBandshape,maxBandshape,maxZeroDM,minZeroDM;

	minFullDM=advancedAnalysis[0]->minFullDM < advancedAnalysis[2]->minFullDM?advancedAnalysis[0]->minFullDM:advancedAnalysis[2]->minFullDM;
	maxFullDM=advancedAnalysis[0]->maxFullDM > advancedAnalysis[2]->maxFullDM?advancedAnalysis[0]->maxFullDM:advancedAnalysis[2]->maxFullDM;
	
	minBandshape=basicAnalysis[0]->minBandshape < basicAnalysis[2]->minBandshape?basicAnalysis[0]->minBandshape:basicAnalysis[2]->minBandshape;
	maxBandshape=basicAnalysis[0]->maxBandshape > basicAnalysis[2]->maxBandshape?basicAnalysis[0]->maxBandshape:basicAnalysis[2]->maxBandshape;

	minZeroDM=basicAnalysis[0]->minZeroDM < basicAnalysis[2]->minZeroDM?basicAnalysis[0]->minZeroDM:basicAnalysis[2]->minZeroDM;
	maxZeroDM=basicAnalysis[0]->maxZeroDM > basicAnalysis[2]->maxZeroDM?basicAnalysis[0]->maxZeroDM:basicAnalysis[2]->maxZeroDM;

	for(int i=0;i<info.noOfPol;i+=2)
	{
		if(advancedAnalysis[i]->hasEnoughDedispersedData!=0)
		{			
			plotProfile(advancedAnalysis[i]->curFoldedProfile,advancedAnalysis[i]->maxProfile,advancedAnalysis[i]->minProfile,i);
		}	
		plotFullDM(advancedAnalysis[i]->fullDM,maxFullDM,minFullDM,advancedAnalysis[i]->length,i);
		plotZeroDM(basicAnalysis[i]->zeroDM,maxZeroDM,minZeroDM,basicAnalysis[i]->blockLength,i);
		plotBandshape(basicAnalysis[i]->bandshape,maxBandshape,minBandshape,i);
		cpgsci(1);		
	}

	minFullDM=advancedAnalysis[1]->minFullDM < advancedAnalysis[3]->minFullDM?advancedAnalysis[1]->minFullDM:advancedAnalysis[3]->minFullDM;
	maxFullDM=advancedAnalysis[1]->maxFullDM > advancedAnalysis[3]->maxFullDM?advancedAnalysis[1]->maxFullDM:advancedAnalysis[3]->maxFullDM;
	
	
	minBandshape=basicAnalysis[1]->minBandshape < basicAnalysis[3]->minBandshape?basicAnalysis[1]->minBandshape:basicAnalysis[3]->minBandshape;
	maxBandshape=basicAnalysis[1]->maxBandshape > basicAnalysis[3]->maxBandshape?basicAnalysis[1]->maxBandshape:basicAnalysis[3]->maxBandshape;

	minZeroDM=basicAnalysis[1]->minZeroDM < basicAnalysis[3]->minZeroDM?basicAnalysis[1]->minZeroDM:basicAnalysis[3]->minZeroDM;
	maxZeroDM=basicAnalysis[1]->maxZeroDM > basicAnalysis[2]->maxZeroDM?basicAnalysis[1]->maxZeroDM:basicAnalysis[3]->maxZeroDM;

	for(int i=1;i<info.noOfPol;i+=2)
	{
		if(advancedAnalysis[i]->hasEnoughDedispersedData!=0)
		{			
			plotProfile(advancedAnalysis[i]->curFoldedProfile,advancedAnalysis[i]->maxProfile,advancedAnalysis[i]->minProfile,i);
		}	
		plotFullDM(advancedAnalysis[i]->fullDM,maxFullDM,minFullDM,advancedAnalysis[i]->length,i);
		plotZeroDM(basicAnalysis[i]->zeroDM,maxZeroDM,minZeroDM,basicAnalysis[i]->blockLength,i);
		plotBandshape(basicAnalysis[i]->bandshape,maxBandshape,minBandshape,i);
		cpgsci(1);		
	}

	plotWaterfall(basicAnalysis[0]->rawData,basicAnalysis[0]->blockLength,(basicAnalysis[0]->maxZeroDM),(basicAnalysis[0]->minZeroDM));
	plotTitle();
	plotBlockIndex(index);
	plotPolarLegend();
}
void Plot::plotAllSingleChannel(BasicAnalysis* basicAnalysis,AdvancedAnalysis* advancedAnalysis,RFIFiltering* rFIFilteringTime,RFIFiltering* rFIFilteringChan,int index)
{
	cpgpage();
	plotTitle();
	plotBlockIndex(index);	
	plotZeroDMSingleChannel(basicAnalysis->zeroDMUnfiltered,basicAnalysis->maxZeroDM,basicAnalysis->minZeroDM,basicAnalysis->blockLength);
	plotTimeFlagsSingleChannel(rFIFilteringTime->flags,rFIFilteringTime->inputSize);
	plotProfileSingleChannel(advancedAnalysis->curFoldedProfile,advancedAnalysis->maxProfile,advancedAnalysis->minProfile);
	cpgsci(1);
	plotProfileUnfilteredSingleChannel(advancedAnalysis->curFoldedProfileUnfiltered,advancedAnalysis->maxProfileUnfiltered,advancedAnalysis->minProfileUnfiltered);
}
void Plot::plotAll(BasicAnalysis** basicAnalysis,AdvancedAnalysis** advancedAnalysis,RFIFiltering** rFIFilteringTime,RFIFiltering** rFIFilteringChan,int index)
{
	if(info.noOfChannels==1)
	{
		plotAllSingleChannel(basicAnalysis[0],advancedAnalysis[0],rFIFilteringTime[0],rFIFilteringChan[0],index);
	}
	else if(!info.doPolarMode)
		plotAllNonPolar(basicAnalysis[0],advancedAnalysis[0],rFIFilteringTime[0],rFIFilteringChan[0],index);
	else if(info.doPolarMode && info.polarChanToDisplay!=-1)
		plotAllNonPolar(basicAnalysis[info.polarChanToDisplay],advancedAnalysis[info.polarChanToDisplay],rFIFilteringTime[info.polarChanToDisplay],rFIFilteringChan[info.polarChanToDisplay],index);
	else
		plotAllPolar(basicAnalysis,advancedAnalysis,index);
	plotBlockIndex(index);
	
		
}

//implementation of Plot methods ends
