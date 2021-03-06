/*************************************************************
The GMRT Pulsar Tool (gptool)

The tool has its foundation in pmon and then wpmon. It was 
written from scratch keeping in mind the need for it to run 
real time with the high data flow rate of the upgraded GMRT. 
This was done in consultation with Yashwant Gupta. This version
uses c++ constructs such as classes to steamline the code and
to make feature expansions easier. If you're working on it 
please keep in mind these two guidelines:

1> Continue using similar naming conventions.
2> This programs owes a lot of its speed to how it handles array
   access. Use pointers to access arrays instead of indexing 
   wherever an array is being accessed repeatedly in a loop. 
   You can find multiple examples of this throughout the code.

			-Aditya Chowdhury, 14th March 2016

************************************************************/


#include<externalLibraries.h>
#include<Information.h>
#include<BasicAnalysis.h>
#include<AquireData.h>
#include<RFIFiltering.h>
#include<Plot.h>
#include<AdvancedAnalysis.h>
using namespace std;

//Benchmark variables
double timeReadData=0.0;
double timeConvertToFloat=0.0;
double timeBandshape=0.0;
double timeNormalization=0.0;
double timeZeroDM=0.0;
double timeRFITimeStats=0.0;
double timeRFITimeFlags=0.0;
double timeRFITimeFlagsWrite=0.0;
double timeRFIChanStats=0.0;
double timeRFIChanFlag=0.0;
double timeRFIChanFlagsWrite=0.0;
double timeFullDMUnfilteredCalc=0.0;
double timeFullDMCalc=0.0;
double timeFullDMUnfilteredWrite=0.0;
double timeFullDMWrite=0.0;
double timeProfileCalc=0.0;
double timeProfileUnfilteredCalc=0.0;
double timePlot=0.0;
double timeThread1=0.0;
double timeThread2=0.0;
double timeThread3=0.0;
double timeThread4=0.0;
double timeThread5=0.0;
double timeThread6=0.0;
double timeWaitTime=0.0;
//End benchmark variables



char *startFlags;
char *endFlags;

static bool keepRunning = true;
class ThreadPacket
{
	public:
	int noOfPol;
	BasicAnalysis** 	basicAnalysis;
	AquireData*		aquireData;
	BasicAnalysis** 	basicAnalysisWrite;
	RFIFiltering** 		rFIFilteringTime;
	RFIFiltering** 		rFIFilteringChan;	
	AdvancedAnalysis** 	advancedAnalysis;
	AdvancedAnalysis** 	advancedAnalysisOld;
	ThreadPacket(int noOfPol_);
	~ThreadPacket();
	void copy(ThreadPacket* threadPacket);
	void copySelect(ThreadPacket* threadPacket);
	void freeMem();
};
ThreadPacket::ThreadPacket(int noOfPol_)
{
	noOfPol=noOfPol_;
	aquireData=NULL;
	basicAnalysis=new BasicAnalysis*[noOfPol];
	basicAnalysisWrite=new BasicAnalysis*[noOfPol];
	rFIFilteringChan=new RFIFiltering*[noOfPol];
	rFIFilteringTime=new RFIFiltering*[noOfPol];
	advancedAnalysis=new AdvancedAnalysis*[noOfPol];
	advancedAnalysisOld=new AdvancedAnalysis*[noOfPol];
	for(int k=0;k<noOfPol;k++)
	{
		basicAnalysis[k]=NULL;
		basicAnalysisWrite[k]=NULL;
		rFIFilteringTime[k]=NULL;
		rFIFilteringChan[k]=NULL;
		advancedAnalysis[k]=NULL;
		advancedAnalysisOld[k]=NULL;
	}
}
ThreadPacket::~ThreadPacket()
{
	if(aquireData!=NULL)
		delete aquireData;
	delete[] basicAnalysis;
	delete[] basicAnalysisWrite;
	delete[] advancedAnalysis;
	delete[] advancedAnalysisOld;
	delete[] rFIFilteringChan;
	delete[] rFIFilteringTime;
}
void ThreadPacket::copy(ThreadPacket* threadPacket)
{
	aquireData=threadPacket->aquireData;
	for(int k=0;k<noOfPol;k++)
	{
		basicAnalysis[k]=threadPacket->basicAnalysis[k];
		rFIFilteringTime[k]=threadPacket->rFIFilteringTime[k];
		rFIFilteringChan[k]=threadPacket->rFIFilteringChan[k];
		advancedAnalysis[k]=threadPacket->advancedAnalysis[k];
	}
}
void ThreadPacket::copySelect(ThreadPacket* threadPacket)
{
	for(int k=0;k<noOfPol;k++)
	{
		basicAnalysisWrite[k]=threadPacket->basicAnalysis[k];
		rFIFilteringTime[k]=threadPacket->rFIFilteringTime[k];
		rFIFilteringChan[k]=threadPacket->rFIFilteringChan[k];
		advancedAnalysis[k]=threadPacket->advancedAnalysis[k];
		advancedAnalysisOld[k]=threadPacket->advancedAnalysisOld[k];
	}
}
void ThreadPacket::freeMem()
{
	
		
	for(int i=0;i<noOfPol;i++)
	{

		if(basicAnalysisWrite[i]!=NULL)
			delete basicAnalysisWrite[i];
		if(advancedAnalysisOld[i]!=NULL)
			delete advancedAnalysisOld[i];
		if(rFIFilteringChan[i]!=NULL)
			delete rFIFilteringChan[i];
		if(rFIFilteringTime[i]!=NULL)
			delete rFIFilteringTime[i];

		basicAnalysisWrite[i]=NULL;		
		advancedAnalysisOld[i]=NULL;
		rFIFilteringChan[i]=NULL;
		rFIFilteringTime[i]=NULL;
	}

}

class Runtime
{
	public:
	Information info;
	int blockIndex;
	int totalBlocks;
	Plot* plot;
	char* blankTimeFlags;
	char* blankChanFlags;
	float *centralpass0,*centralpass1,*stdpass0,*stdpass1;
	float **cumulativeBandpass;
	char readDoneFlag;
	char readCompleteFlag;
	ThreadPacket** threadPacket;
	
	/**variables for inline mode of gptool**/
	Correlator* shmInterface;
	int nbuff;
	
	Runtime(Information info_,int nThreadMultiplicity_);
	~Runtime();
	void intializeFiles();
	void fillPipe();
	void loopThrough();
	void closePipe();
	void quickclosePipe();
	void action(int threadPacketIndex,int actionIndex);
	private:
	int nActions;
	int nThreadMultiplicity,nThreadMultiplicityTemp;
	char chanFirst;
	char hasReachedEof;
	
	void writeAll(ThreadPacket* threadPacket);
	void testStatistics(ThreadPacket* threadPacket);
	void displayBlockIndex(int blockIndex);
	void ioTasks(int threadPacketIndex);
	void plotTasks(int threadPacketIndex);
	void channelTasks(int threadPacketIndex,char dosecondpass);
	void timeTasks(int threadPacketIndex,char dosecondpass);
	void fullDMTask(int threadPacketIndex);
	void floatConversionTasks(int threadPacketIndex);
	void writeFlagStats(ThreadPacket* threadPacket);
	void copyToSHM(ThreadPacket* threadPacket);
};
Runtime::Runtime(Information info_,int nThreadMultiplicity_)
{
	nActions=5;
	nThreadMultiplicity=nThreadMultiplicity_;
	nThreadMultiplicityTemp=nThreadMultiplicity;
	info=info_;
	AquireData* aquireData=new AquireData(info);	
	if(!info.doReadFromFile)
	{
		aquireData->initializeSHM();
		if(info.isInline)
		{
			shmInterface=new Correlator(aquireData->dataHdr,aquireData->dataBuffer);
			shmInterface->initializeWriteSHM();
			nbuff=aquireData->nbuff;
		}
		else if(info.doFRB)
		{
			shmInterface=new Correlator(aquireData->dataHdr,aquireData->dataBuffer);
			shmInterface->initializeWriteSHM_FRB(0);
			nbuff=aquireData->nbuff;
		}
	}
	info.fillParams();
	if(info.isInline || info.doFRB || info.shmID>=4)
	{
		info.blockSizeSamples=(aquireData->info).blockSizeSamples;
		info.blockSizeSec=(aquireData->info).blockSizeSec;
	}
	AquireData::info=info;
	AquireData::curPos=long((info.startTime/info.samplingInterval))*info.noOfChannels*info.noOfPol* info.sampleSizeBytes;	
	AquireData::info.startTime=long(info.startTime/info.blockSizeSec)*info.blockSizeSec;
	info.display();

	threadPacket=new ThreadPacket*[nActions*nThreadMultiplicity];
	for(int i=0;i<nActions*nThreadMultiplicity;i++)
	{
		threadPacket[i]=new ThreadPacket(info.noOfPol);
	}

	centralpass0=new float[info.noOfPol];
	centralpass1=new float[info.noOfPol];
	stdpass0=new float[info.noOfPol];
	stdpass1=new float[info.noOfPol];
	cumulativeBandpass=new float*[info.noOfPol];
	for(int k=0;k<info.noOfPol;k++)
		cumulativeBandpass[k]=new float[info.noOfChannels];
	int totalBlocksNoOff=0;
	if(info.doReadFromFile)
	{
		double totalTime=(AquireData::eof*info.samplingInterval)/(info.noOfChannels*info.sampleSizeBytes*info.noOfPol);		
		if(info.startTime>totalTime)
		{
			cout<<endl<<endl<<"File contains "<<totalTime<<" seconds of data. Please give a starting time less than that"<<endl;
			exit(0);
		}
		double extraOffset=info.startTime*1000.0/info.periodInMs;
		extraOffset=(extraOffset-floor(extraOffset));
		info.profileOffset+=extraOffset;
		if(info.profileOffset>=1.000)
			info.profileOffset-=1.00;
		totalBlocks=ceil((totalTime-info.startTime)/(info.blockSizeSec));
		totalBlocksNoOff=ceil(totalTime/(info.blockSizeSec));
	}
	else
		totalBlocks=totalBlocksNoOff=0;
	
	if(info.smoothFlagWindowLength>0)
		info.smoothFlagWindowLength=(int)(info.smoothFlagWindowLength/info.samplingInterval);
	info.concentrationThreshold=1.0-info.concentrationThreshold/100.0;
	if(!info.doFilteringOnly)
		plot=new Plot(info,totalBlocksNoOff);
	else
		plot=NULL;
	
	for(int k=0;k<info.noOfPol;k++)
	{
		if(!info.doFilteringOnly)
			threadPacket[(nActions-1)*nThreadMultiplicity]->advancedAnalysisOld[k]=new AdvancedAnalysis(info);
		threadPacket[nActions-1]->basicAnalysis[k]=new BasicAnalysis(info);	
	}

	blankTimeFlags=new char[info.blockSizeSamples+1];
	blankChanFlags=new char[info.stopChannel-info.startChannel];

	for(int i=0;i<info.blockSizeSamples+1;i++)
		blankTimeFlags[i]=0;
	
	for(int i=0;i<info.stopChannel-info.startChannel;i++)
		blankChanFlags[i]=0;
	
	blockIndex=0;
	chanFirst=0;
	if((info.doTimeFlag && info.doChanFlag && (info.flagOrder==1)) || info.doUseNormalizedData ||info.doChanFlag)
		chanFirst=1;
	//cout<<"Reached end of Runtime()"<<endl; //DEBUG
	//omp_set_num_threads(2+3*nThreadMultiplicity);
	
	//omp_set_dynamic(0);
}
Runtime::~Runtime()
{
	delete[] threadPacket;
	if(plot!=NULL)
		delete plot;
	delete[] blankTimeFlags;
	delete[] blankChanFlags;
	//delete[] histogramInterval;
}
void Runtime::displayBlockIndex(int blockIndex)
{

	if(info.doReadFromFile)
			cout<<'\r'
	<<"Block:"<<(blockIndex)-nActions*nThreadMultiplicity+1<<" of "<<totalBlocks<<"                                  "<<std::flush;		
		else
			cout<<'\r'
	<<"Block:"<<(blockIndex)-nActions*nThreadMultiplicity+1<<"                                  "<<std::flush;

}
void Runtime::testStatistics(ThreadPacket* threadPacket) 
{
	for(int k=0;k<info.noOfPol;k++)
	{
		double rmsPreFlag,meanPreFlag,rmsPostFlag,meanPostFlag,sum3,skewnessPostFilt;
		float* ptrZeroDM;
		float* ptrZeroDMUnfiltered;
		char* ptrFlags;
		int count;
		int l=threadPacket->basicAnalysisWrite[k]->blockLength;
		ptrZeroDM=threadPacket->basicAnalysisWrite[k]->zeroDM;
		ptrZeroDMUnfiltered=threadPacket->basicAnalysisWrite[k]->zeroDMUnfiltered;
		ptrFlags=threadPacket->rFIFilteringTime[k]->flags;
			meanPreFlag=rmsPreFlag=meanPostFlag=rmsPostFlag=sum3=0.0;
			count=0;
		for(int i=0;i<l;i++,ptrZeroDM++,ptrZeroDMUnfiltered++,ptrFlags++)
		{
			meanPreFlag+=(*ptrZeroDMUnfiltered);
			rmsPreFlag+=((*ptrZeroDMUnfiltered)*(*ptrZeroDMUnfiltered));
			if(!(*ptrFlags))
			{
				meanPostFlag+=(*ptrZeroDM);
				rmsPostFlag+=(*ptrZeroDM)*(*ptrZeroDM);
				sum3+=(*ptrZeroDM)*(*ptrZeroDM)*(*ptrZeroDM);
				count++;
			}
		}
		
		meanPreFlag/=l;
		rmsPreFlag=sqrt((rmsPreFlag/l)-(meanPreFlag*meanPreFlag));

		meanPostFlag/=count;
		rmsPostFlag=sqrt((rmsPostFlag/count)-(meanPostFlag*meanPostFlag));
		skewnessPostFilt=(sum3/count-3*meanPostFlag*rmsPostFlag-pow(meanPostFlag,3))/pow(rmsPostFlag,3);
		ofstream statFile;
		if(info.doPolarMode)
		{
			ostringstream filename;		
			filename<<"stats"<<k+1<<".gpt";
			statFile.open(filename.str().c_str(),ios::app);
		}
		else
			statFile.open("stats.gpt",ios::app);
		statFile<<blockIndex-3<<"\t"<<threadPacket->rFIFilteringTime[k]->centralTendency<<"\t"<<meanPreFlag<<"\t"<<meanPostFlag<<"\t";
		statFile<<threadPacket->rFIFilteringTime[k]->rms<<"\t"<<rmsPreFlag<<"\t"<<rmsPostFlag<<"\t";	
		statFile<<(threadPacket->rFIFilteringTime[k]->centralTendency)/(threadPacket->rFIFilteringTime[k]->rms)<<"\t"<<meanPreFlag/rmsPreFlag<<"\t"<<meanPostFlag/rmsPostFlag;
		statFile<<"\t"<<skewnessPostFilt<<endl;
		statFile.close();
	}
}
void Runtime::writeFlagStats(ThreadPacket* threadPacket)
{
	for(int k=0;k<info.noOfPol;k++)
	{
		ofstream statFile;
		if(info.doPolarMode)
		{
			ostringstream filename;		
			filename<<"flag_stats"<<k+1<<".gpt";
			statFile.open(filename.str().c_str(),ios::app);
		}
		else
			statFile.open("flag_stats.gpt",ios::app);

		char* ptrTimeFlags=threadPacket->rFIFilteringTime[k]->flags;
		char* ptrChanFlags=threadPacket->rFIFilteringChan[k]->flags;
		int l=threadPacket->basicAnalysisWrite[k]->blockLength;
		float timePercent;
		timePercent=0;
		for(int i=0;i<l;i++,ptrTimeFlags++)		
			timePercent+=*ptrTimeFlags;
		timePercent*=100.0/l;
		for(int i=0;i<info.startChannel;i++)
			statFile<<100.0<<" ";
		for(int i=info.startChannel;i<info.stopChannel;i++,ptrChanFlags++)	
		{
			if(!(*ptrChanFlags))	
				statFile<<timePercent<<" ";
			else
				statFile<<100.0<<" ";
		}
		for(int i=info.stopChannel;i<info.noOfChannels;i++)
			statFile<<100.0<<" ";
		statFile<<endl;
	}
}
void Runtime::intializeFiles()
{
	if(info.doWriteFiltered2D)
	{
		ostringstream filename;	
		filename<<info.outputfilepath<<".gpt";
		ofstream filtered2DFile;			
		filtered2DFile.open(filename.str().c_str(),ios::out | ios::trunc);
		if (filtered2DFile.fail())
		{
			cout<<endl<<"ERROR: Cannot create outputfile. Possible permission issues?"<<endl;
			exit(1);
		}
		filtered2DFile.close();
	}

	if(!info.doPolarMode)
	{
			
		if(info.doWriteChanFlags && info.doChanFlag)
		{
			ofstream chanflagfile;	
			chanflagfile.open("chanflag.gpt",ios::out | ios::trunc);
			chanflagfile.close();
		}
		if(info.doWriteTimeFlags && info.doTimeFlag)
		{
			ofstream timeflagfile;	
			timeflagfile.open("timeflag.gpt",ios::out | ios::trunc);
			timeflagfile.close();
		}
		if(info.doWriteFullDM && !info.doFilteringOnly)
		{			

			ofstream fullDMfile;	
			fullDMfile.open("fullDM_filtered.gpt",ios::out | ios::trunc);
			fullDMfile.close();
			
			ofstream fullDMUnfilteredfile;	
			fullDMUnfilteredfile.open("fullDM_unfiltered.gpt",ios::out | ios::trunc);
			fullDMUnfilteredfile.close();
			
			ofstream fullDMCountfile;	
			fullDMCountfile.open("fullDMCount.gpt",ios::out | ios::trunc);
			fullDMCountfile.close();
		}
		
		ofstream statFile;
		statFile.open("stats.gpt",ios::out | ios::trunc);
		statFile<<"#window_indx\tmean_pred\tmean_pre\tmean_post\trms_pred\trms_pre\trms_post\tm/r_pred\tm/r_pre\tm/r_post"<<endl;
		statFile.close();

		ofstream intensityFile;
		intensityFile.open("intensity_summary.gpt",ios::out | ios::trunc);
		intensityFile<<"#First element of each line denotes the number of time samples in the block, followed by intensity in each channel"<<endl;
		intensityFile.close();
	
		ofstream flagStatFile;
		flagStatFile.open("flag_stats.gpt",ios::out | ios::trunc);
		flagStatFile<<"#Each line represents a seperate block. For the particular block, the line contains the percentage of flagged data in each channel"<<endl;
		flagStatFile.close();
		
	}
	else
	{
		for(int k=0;k<info.noOfPol;k++)
		{
			ostringstream filename;
			if(info.doWriteChanFlags && info.doChanFlag)
			{
				filename<<"chanflag"<<k+1<<".gpt";
				ofstream chanflagfile;	
				chanflagfile.open(filename.str().c_str(),ios::out | ios::trunc);
				chanflagfile.close();
			}
			if(info.doWriteTimeFlags && info.doTimeFlag)
			{
				filename.str("");
				filename.clear();
				filename<<"timeflag"<<k+1<<".gpt";	
				ofstream timeflagfile;	
				timeflagfile.open(filename.str().c_str(),ios::out | ios::trunc);
				timeflagfile.close();
			}
			if(info.doWriteFullDM && !info.doFilteringOnly)
			{
				filename.str("");
				filename.clear();
				filename<<"fullDM_filtered"<<k+1<<".gpt";
				ofstream fullDMfile;	
				fullDMfile.open(filename.str().c_str(),ios::out | ios::trunc);
				fullDMfile.close();
				
				filename.str("");
				filename.clear();
				filename<<"fullDM_unfiltered"<<k+1<<".gpt";
				ofstream fullDMUnfilteredfile;	
				fullDMUnfilteredfile.open(filename.str().c_str(),ios::out | ios::trunc);
				fullDMUnfilteredfile.close();
				
				filename.str("");
				filename.clear();
				filename<<"fullDMCount"<<k+1<<".gpt";
				ofstream fullDMCountfile;	
				fullDMCountfile.open(filename.str().c_str(),ios::out | ios::trunc);
				fullDMCountfile.close();
			}
			
			filename.str("");
			filename.clear();
			filename<<"stats"<<k+1<<".gpt";
			ofstream statFile;
			statFile.open(filename.str().c_str(),ios::out | ios::trunc);
			statFile<<"#window_index(1)\tmean_pred(2)\tmean_pre(3)\tmean_post(4)\trms_pred(5)\trms_pre(6)\trms_post(7)\tm/r_pred(8)\tm/r_pre(9)\tm/r_post(10)"<<endl;
			statFile.close();

			filename.str("");
			filename.clear();
			filename<<"intensity_summary"<<k+1<<".gpt";
			ofstream intensityFile;
			intensityFile.open(filename.str().c_str(),ios::out | ios::trunc);
			intensityFile<<"#First element of each line denotes the number of time samples in the block, followed by intensity in each channel";
			intensityFile.close();

		}
	}
	
}

void Runtime::writeAll(ThreadPacket *threadPacket)
{

	if(!info.doPolarMode)
	{
		
		if(info.doWriteFullDM && !info.doFilteringOnly)
		{
			
			timeFullDMWrite-=omp_get_wtime(); //benchmark
			threadPacket->advancedAnalysis[0]->writeFullDM("fullDM_filtered.gpt","fullDM_unfiltered.gpt");
			threadPacket->advancedAnalysis[0]->writeFullDMCount("fullDMCount.gpt");
			timeFullDMWrite+=omp_get_wtime(); //benchmark			
		}
		if(info.doWriteFiltered2D)
		{
			if(info.sampleSizeBytes==1)
			{
				ostringstream filename;	
				filename.str("");
				filename.clear();
				filename<<info.outputfilepath<<".gpt";
				FILE* filteredRawDataFile;    
	       			filteredRawDataFile = fopen(filename.str().c_str(), "ab");
				short int *ptrFilteredData=threadPacket->basicAnalysisWrite[0]->filteredRawData;
				long int size=(threadPacket->basicAnalysisWrite[0]->blockLength)*info.noOfChannels;
				unsigned char* tmp=new unsigned char[size];
				unsigned char* ptrtmp=tmp;
				for(int i=0;i<size;i++,ptrtmp++)
				{
					*ptrtmp=(unsigned char)(*ptrFilteredData++);	                      		
				}
				fwrite(tmp, sizeof(unsigned char),size, filteredRawDataFile);    
				delete[] tmp;
		                fclose(filteredRawDataFile); 
			}
			else
			{
				ostringstream filename;	
				filename<<info.outputfilepath<<".gpt";
				threadPacket->basicAnalysisWrite[0]->writeFilteredRawData(filename.str().c_str());
			}
		}
		if(info.isInline)
		{
			cout<<"nbuff :"<<nbuff<<endl;
			int size=info.blockSizeSamples*info.noOfChannels/nbuff;
			short int* filteredRawData=threadPacket->basicAnalysisWrite[0]->filteredRawData;
			char * headerInfo=threadPacket->basicAnalysisWrite[0]->headerInfo;
			cout<<"Start writing output Buffer"<<endl;
			for(int i=0;i<nbuff;i++)
			{
				shmInterface->writeToSHM(filteredRawData+i*size,headerInfo+i*4096);
			}
		}
		else if (info.doFRB)
		{
			unsigned char* filteredRawData=threadPacket->basicAnalysisWrite[0]->filteredRawDataChar;
			cout<<"Start writing output Buffer;nbuff"<<nbuff<<endl;
			for(int i=0;i<nbuff;i++)
			{
				shmInterface->writeToSHM_FRB(filteredRawData);
			}
		}
		threadPacket->basicAnalysisWrite[0]->writeCurBandshape("intensity_summary.gpt");		
		timeRFITimeFlagsWrite-=omp_get_wtime(); //benchmark
		if(info.doWriteTimeFlags && info.doTimeFlag)
			threadPacket->rFIFilteringTime[0]->writeFlags("timeflag.gpt");
		timeRFITimeFlagsWrite+=omp_get_wtime(); //benchmark
	
		timeRFIChanFlagsWrite-=omp_get_wtime(); //benchmark
		if(info.doWriteChanFlags && info.doChanFlag)
			threadPacket->rFIFilteringChan[0]->writeFlags("chanflag.gpt",startFlags,info.startChannel,endFlags,info.noOfChannels-info.stopChannel);
		timeRFIChanFlagsWrite+=omp_get_wtime(); //benchmark	
		
		
	}
	else
	{
		if(info.doWriteFiltered2D)
		{
			ostringstream filename;	
			filename.str("");
			filename.clear();
			filename<<info.outputfilepath<<".gpt";
			FILE* filteredRawDataFile;    
       			filteredRawDataFile = fopen(filename.str().c_str(), "ab");
			short int **ptrFilteredData=new short int*[info.noOfPol];
			for(int k=0;k<info.noOfPol;k++)
			{
				ptrFilteredData[k]=threadPacket->basicAnalysisWrite[k]->filteredRawData;
			}
			if(info.sampleSizeBytes==1)
			{
				char tmp;
				for(int i=0;i<(threadPacket->basicAnalysisWrite[0]->blockLength)*info.noOfChannels;i++)
				{
					for(int k=0;k<info.noOfPol;k++)
					{
						tmp=(char)(*ptrFilteredData[k]++);
		                      		fwrite(&tmp, sizeof(char), 1, filteredRawDataFile);    
					}
				}
			}
			else
			{
				//cout<<"Writing full polar data"<<endl;
				//cout<<"Block length: "<<(threadPacket->basicAnalysisWrite[0]->blockLength)<<endl;
				//cout<<"nSamples: "<<(threadPacket->basicAnalysisWrite[0]->blockLength)*info.noOfChannels<<endl;
				for(int i=0;i<(threadPacket->basicAnalysisWrite[0]->blockLength)*info.noOfChannels;i++)
				{

					for(int k=0;k<info.noOfPol;k++)
					{
		                      		fwrite(ptrFilteredData[k]++, sizeof(short int), 1, filteredRawDataFile);    
					}
				}
				//cout<<"Done writing"<<endl;
			}
                        fclose(filteredRawDataFile); 

		}

		for(int k=0;k<info.noOfPol;k++)
		{
			ostringstream filename;	
			ostringstream filenameUnfiltered;	
			timeFullDMWrite-=omp_get_wtime(); //benchmark
			if(info.doWriteFullDM && !info.doFilteringOnly)
			{
				filename<<"fullDM_filtered"<<k+1<<".gpt";				
				filenameUnfiltered<<"fullDM_unfiltered"<<k+1<<".gpt";	

				timeFullDMWrite-=omp_get_wtime(); //benchmark
				threadPacket->advancedAnalysis[k]->writeFullDM(filename.str().c_str(),filenameUnfiltered.str().c_str());
				timeFullDMWrite+=omp_get_wtime(); //benchmark	
				

				filename.str("");
				filename.clear();
				filename<<"fullDMCount"<<k+1<<".gpt";
				timeFullDMWrite-=omp_get_wtime(); //benchmark
				threadPacket->advancedAnalysis[k]->writeFullDMCount(filename.str().c_str());
				timeFullDMWrite+=omp_get_wtime(); //benchmark
				
				
				
			}
			
			/**
			filename.str("");
			filename.clear();
			filename<<"intensity_summary"<<k+1<<".gpt";
			threadPacket->basicAnalysis[0]->writeCurBandshape(filename.str().c_str());	
			**/
			timeFullDMWrite+=omp_get_wtime(); //benchmark
			
			filename.str("");
			filename.clear();
			filename<<"timeflag"<<k+1<<".gpt";	

			timeRFITimeFlagsWrite-=omp_get_wtime(); //benchmark
			if(info.doWriteTimeFlags && info.doTimeFlag)
				threadPacket->rFIFilteringTime[k]->writeFlags(filename.str().c_str());
			timeRFITimeFlagsWrite+=omp_get_wtime(); //benchmark
	
			filename.str("");
			filename.clear();
			filename<<"chanflag"<<k+1<<".gpt";


			timeRFIChanFlagsWrite-=omp_get_wtime(); //benchmark
			if(info.doWriteChanFlags && info.doChanFlag)
				threadPacket->rFIFilteringChan[k]->writeFlags(filename.str().c_str(),startFlags,info.startChannel,endFlags,info.noOfChannels-info.stopChannel);
			timeRFIChanFlagsWrite+=omp_get_wtime(); //benchmark	
		}
	}
	testStatistics(threadPacket);
	writeFlagStats(threadPacket);
}

double fillTime; //benchmark
void Runtime::fillPipe()
{	

	fillTime=omp_get_wtime(); //benchmark
	int index;
	readCompleteFlag=0;
	readDoneFlag=1;	
	
	for(int k=0;k<nActions;k++)
	{
		//cout<<blockIndex<<","<<k<<endl;
		#pragma omp parallel for schedule(dynamic, 1)
		for(int j=1;j<k+1;j++)
		{
		//	cout<<j*nThreadMultiplicity<<","<<j<<endl;
			action(j*nThreadMultiplicity,j);
		}
		
		while(!readCompleteFlag);
		if(k>=3)
		{
			for(int ipol=0;ipol<info.noOfPol;ipol++)
			{
				memcpy(cumulativeBandpass[ipol],threadPacket[4*nThreadMultiplicity-1]->basicAnalysis[ipol]->smoothBandshape,info.noOfChannels*sizeof(float));
			}
		}
		for(int i=0;i<nThreadMultiplicity;i++)
			threadPacket[0+i]->copySelect(threadPacket[(nActions-1)*nThreadMultiplicity+i]);		//ready blocks for plot and io

		for(int i=(nActions-1)*nThreadMultiplicity-1;i>=0;i--)
			threadPacket[i+nThreadMultiplicity]->copy(threadPacket[i]);
		
		readCompleteFlag=0;
		readDoneFlag=1;	
		blockIndex+=nThreadMultiplicity;	
		
	}
	

	threadPacket[(nActions-1)*nThreadMultiplicity]->advancedAnalysisOld=threadPacket[nThreadMultiplicity-1]->advancedAnalysis;
	fillTime=(omp_get_wtime()-fillTime)/(nThreadMultiplicity*nActions); //benchmark
}
int numberOfThreadRuns=0;		//benchmark
void Runtime::loopThrough()
{
	double startTime,timeP,time0,time1,time2,time3,time4,time5,timeNet; //benchmark variables 
	omp_set_nested(1);
	ofstream benchmarkfile;
	benchmarkfile.open("benchmark_threadtime_indv.gpt",ios::out | ios::trunc);
	/*while(!hasReachedEof)
	{
		time0= omp_get_wtime();
		timeThread1 -= omp_get_wtime();
		action(0,0);
		timeThread1+=omp_get_wtime();	
		time0= omp_get_wtime()-time0;	
		benchmarkfile<<time0/nThreadMultiplicity<<endl;
		for(int i=0;i<nThreadMultiplicity;i++)
		{
			ThreadPacket* thisThreadPacket=threadPacket[i];		
			delete thisThreadPacket->aquireData;
			thisThreadPacket->aquireData=NULL;
		}
	}*/
	
			while(!hasReachedEof)
			{			
				numberOfThreadRuns+=nThreadMultiplicity;
				startTime=omp_get_wtime();
				timeNet=omp_get_wtime();


				if(!info.doFilteringOnly) //full mode of gptool
				{
					#pragma omp parallel sections 
					{
			
						//#pragma omp section
						//{
						//	timeP= omp_get_wtime();
							
						//	timeP= omp_get_wtime()-timeP;	
						//}
						#pragma omp section
						{
							time1= omp_get_wtime();
							timeThread1 -= omp_get_wtime();
							action(1*nThreadMultiplicity,1);
							action(0,-1);								
							timeThread1+=omp_get_wtime();	
							time1= omp_get_wtime()-time1;			
						}
						#pragma omp section
						{
							time2= omp_get_wtime();
							timeThread2 -= omp_get_wtime();
							action(2*nThreadMultiplicity,2);				
							timeThread2+=omp_get_wtime();	
							time2= omp_get_wtime()-time2;		
						}
						#pragma omp section
						{
							time3= omp_get_wtime();
							timeThread3-=omp_get_wtime();
							action(3*nThreadMultiplicity,3);	
							timeThread3+=omp_get_wtime();	
							time3= omp_get_wtime()-time3;	
						}
						#pragma omp section
						{
							time4= omp_get_wtime();
							timeThread4-=omp_get_wtime();
							action(4*nThreadMultiplicity,4);
							timeThread4+=omp_get_wtime();	
							time4= omp_get_wtime()-time4;	
						}
					
				
					}
				}
				else
				{
					#pragma omp parallel sections //filtering only mode of gptool
					{
						#pragma omp section
						{
							time1= omp_get_wtime();
							timeThread1 -= omp_get_wtime();
							action(1*nThreadMultiplicity,1);
							timeThread1+=omp_get_wtime();	
							time1= omp_get_wtime()-time1;			
						}
						#pragma omp section
						{
							time2= omp_get_wtime();
							timeThread2 -= omp_get_wtime();
							action(2*nThreadMultiplicity,2);				
							timeThread2+=omp_get_wtime();	
							time2= omp_get_wtime()-time2;		
						}
						#pragma omp section
						{
							time3= omp_get_wtime();
							timeThread3-=omp_get_wtime();
							action(3*nThreadMultiplicity,3);	
							timeThread3+=omp_get_wtime();	
							time3= omp_get_wtime()-time3;	
						}
				
					}
				}
				while(!readCompleteFlag);

				for(int ipol=0;ipol<info.noOfPol;ipol++)
					memcpy(cumulativeBandpass[ipol],threadPacket[4*nThreadMultiplicity-1]->basicAnalysis[ipol]->smoothBandshape,info.noOfChannels*sizeof(float));

				for(int i=0;i<nThreadMultiplicity;i++)
					threadPacket[i]->freeMem();

				for(int i=0;i<nThreadMultiplicity;i++)
					threadPacket[0+i]->copySelect(threadPacket[(nActions-1)*nThreadMultiplicity+i]);		//ready blocks for plot and io

				for(int i=(nActions-1)*nThreadMultiplicity-1;i>=0;i--)
					threadPacket[i+nThreadMultiplicity]->copy(threadPacket[i]);

				threadPacket[(nActions-1)*nThreadMultiplicity]->advancedAnalysisOld=threadPacket[nThreadMultiplicity-1]->advancedAnalysis;
				
				readCompleteFlag=0;
				readDoneFlag=1;	
				if(!keepRunning)
				{			
					cout<<endl<<"Terminating program.."<<endl;
					break;
				}

				if(info.doWindowDelay && info.doReadFromFile) 
					while(omp_get_wtime()-startTime<info.blockSizeSec);
				timeNet=omp_get_wtime()-timeNet;
				benchmarkfile<<blockIndex<<" "<<timeP/nThreadMultiplicity<<" "<<time0/nThreadMultiplicity<<" "<<time1/nThreadMultiplicity<<" "<<time2/nThreadMultiplicity<<" "<<time3/nThreadMultiplicity<<" "<<time4/nThreadMultiplicity<<endl;

				blockIndex+=nThreadMultiplicity;
			}
		
		
		
		 
	
	benchmarkfile.close();
		
	
	
	
}


void Runtime::quickclosePipe()
{
	cout<<endl<<"Closing pipe.."<<endl;
	for(int k=0;k<nThreadMultiplicityTemp;k++)
	{
		displayBlockIndex(blockIndex+k);
		writeAll(threadPacket[k]);
		if(!info.doFilteringOnly)
		{
			timePlot-=omp_get_wtime();		
			plot->plotAll(threadPacket[k]->basicAnalysisWrite,threadPacket[k]->advancedAnalysis,threadPacket[k]->rFIFilteringTime,threadPacket[k]->rFIFilteringChan,blockIndex+k-nActions*nThreadMultiplicity);
			timePlot+=omp_get_wtime();
		}
	}
	if(info.doPolarMode)
	{	
		for(int k=0;k<info.noOfPol;k++)
		{
			//writing final bits
			ostringstream filename;
			ostringstream filenameUnfiltered;
			filename<<"bandshape"<<k+1<<".gpt";
			threadPacket[nThreadMultiplicityTemp-1]->basicAnalysisWrite[k]->writeBandshape(filename.str().c_str());
			filename.str("");
			filename.clear();
			
			if(!info.doFilteringOnly)
			{
				filename<<"profile_filtered"<<k+1<<".gpt";
				filenameUnfiltered<<"profile_unfiltered"<<k+1<<".gpt";
				threadPacket[nThreadMultiplicityTemp-1]->advancedAnalysis[k]->writeProfile(filename.str().c_str(),filenameUnfiltered.str().c_str());
			}
	
		}
	}
	else
	{
		
		threadPacket[nThreadMultiplicityTemp-1]->basicAnalysisWrite[0]->writeBandshape("bandshape.gpt");
		if(!info.doFilteringOnly)
			threadPacket[nThreadMultiplicityTemp-1]->advancedAnalysis[0]->writeProfile("profile_filtered.gpt","profile_unfiltered.gpt");		
	}
	//for(int j=0;j<nThreadMultiplicity;j++)
	//	threadPacket[j]->freeMem();
	for(int j=0;j<nThreadMultiplicity;j++)
	{
		for(int i=0;i<info.noOfPol;i++)
		{
			delete threadPacket[2*nThreadMultiplicity+j]->basicAnalysis[i];
			delete threadPacket[3*nThreadMultiplicity+j]->basicAnalysis[i];
			delete threadPacket[4*nThreadMultiplicity+j]->basicAnalysis[i];

			if(!threadPacket[3*nThreadMultiplicity+j]->rFIFilteringChan[i])
				delete threadPacket[3*nThreadMultiplicity+j]->rFIFilteringChan[i];
			if(!threadPacket[4*nThreadMultiplicity+j]->rFIFilteringChan[i])
				delete threadPacket[4*nThreadMultiplicity+j]->rFIFilteringChan[i];

			if(!threadPacket[4*nThreadMultiplicity+j]->rFIFilteringTime[i])
				delete threadPacket[4*nThreadMultiplicity+j]->rFIFilteringTime[i];
		
			
		}

	}
	//	for(int i=0;i<info.noOfPol;i++)
	//	{
	//		delete threadPacket[4*nThreadMultiplicity]->advancedAnalysisOld[i];
	//	}
	cout<<endl;
}


void Runtime::closePipe()
{
	//cout<<"Entered closePipe()"<<nThreadMultiplicityTemp<<","<<nThreadMultiplicity<<endl;
	for(int k=0;k<nThreadMultiplicity;k++)
	{
		displayBlockIndex(blockIndex+k);
		writeAll(threadPacket[k]);
		if(!info.doFilteringOnly)
		{
			timePlot-=omp_get_wtime();		
			plot->plotAll(threadPacket[k]->basicAnalysisWrite,threadPacket[k]->advancedAnalysis,threadPacket[k]->rFIFilteringTime,threadPacket[k]->rFIFilteringChan,blockIndex+k-nActions*nThreadMultiplicity);
			timePlot+=omp_get_wtime();
		}
	}
	int temp=nThreadMultiplicity;
	int nPlots=nThreadMultiplicity;
	blockIndex+=nThreadMultiplicity;
	for(int i=1;i<nActions;i++)
	{
		nThreadMultiplicity=nThreadMultiplicityTemp;
		action(i*temp,i);
		nThreadMultiplicity=temp;
		for(int j=i+1;j<nActions;j++)
		{
			action(j*nThreadMultiplicity,j);
		}
		
		if(i!=nActions-1)	//The last packets is retained in memory to print the final profile from.
		{
			for(int j=0;j<nThreadMultiplicity;j++)
				threadPacket[j]->freeMem();
		}

		//Packet transfers between different operations:

		for(int j=0;j<nThreadMultiplicity;j++)
			threadPacket[0+j]->copySelect(threadPacket[(nActions-1)*nThreadMultiplicity+j]);		//ready blocks for plot and io

		for(int j=(nActions-1)*nThreadMultiplicity-1;j>=0;j--)
			threadPacket[j+nThreadMultiplicity]->copy(threadPacket[j]);

		threadPacket[(nActions-1)*nThreadMultiplicity]->advancedAnalysisOld=threadPacket[nThreadMultiplicity-1]->advancedAnalysis;

		//Plotting ready packets:
		if(i==nActions-1)	//last chunck may not contain as many blocks
			nPlots=nThreadMultiplicityTemp;
		for(int k=0;k<nPlots;k++)
		{
			displayBlockIndex(blockIndex+k);
			writeAll(threadPacket[k]);
			if(!info.doFilteringOnly)
			{
				timePlot-=omp_get_wtime();	
				plot->plotAll(threadPacket[k]->basicAnalysisWrite,threadPacket[k]->advancedAnalysis,threadPacket[k]->rFIFilteringTime,threadPacket[k]->rFIFilteringChan,blockIndex+k-nActions*nThreadMultiplicity);
				timePlot+=omp_get_wtime();
			}
		}
		blockIndex+=nThreadMultiplicity;
	}

	if(info.doPolarMode)
	{	
		for(int k=0;k<info.noOfPol;k++)
		{
			//writing final bits
			ostringstream filename;
			ostringstream filenameUnfiltered;
			filename<<"bandshape"<<k+1<<".gpt";
			threadPacket[nThreadMultiplicityTemp-1]->basicAnalysisWrite[k]->writeBandshape(filename.str().c_str());
			filename.str("");
			filename.clear();
			
			if(!info.doFilteringOnly)
			{
				filename<<"profile_filtered"<<k+1<<".gpt";
				filenameUnfiltered<<"profile_unfiltered"<<k+1<<".gpt";
				threadPacket[nThreadMultiplicityTemp-1]->advancedAnalysis[k]->writeProfile(filename.str().c_str(),filenameUnfiltered.str().c_str());
			}
	
		}
	}
	else
	{
		
		threadPacket[nThreadMultiplicityTemp-1]->basicAnalysisWrite[0]->writeBandshape("bandshape.gpt");
		if(!info.doFilteringOnly)
			threadPacket[nThreadMultiplicityTemp-1]->advancedAnalysis[0]->writeProfile("profile_filtered.gpt","profile_unfiltered.gpt");		
	}
	
	for(int j=0;j<nThreadMultiplicityTemp;j++) //free'ing last packets
		threadPacket[j]->freeMem();
	cout<<endl;
}
void Runtime::action(int threadPacketIndex,int actionIndex)
{
	switch(actionIndex)
	{
		case -1:
			plotTasks(threadPacketIndex);
			break;
		case 0:
			ioTasks(threadPacketIndex);
			break;
		case 1:
			floatConversionTasks(threadPacketIndex);
			break;
		case 2:

			channelTasks(threadPacketIndex,0);
			timeTasks(threadPacketIndex,0);
			channelTasks(threadPacketIndex,1);

			break;
		case 3:

			timeTasks(threadPacketIndex,1);

			break;
		case 4:
			if(!info.doFilteringOnly)			
				fullDMTask(threadPacketIndex);
			break;
			
	}
}

/*******************************************************************
*FUNCTION: ioTasks(int threadPacketIndex)
*This function performs the tasks on thread one, i.e Reading data,
*writing data and plotting
*For optimization considerations refer to document titled
*"Multithreading Considerations for gptool"
*******************************************************************/
void Runtime::ioTasks(int threadPacketIndex)
{
	
	double readTime,floatTime,restTime;
	char killFlag=0;
	while(!killFlag)
	{
		ofstream benchmarkfile;
		benchmarkfile.open("benchmark_readtime.gpt",ios::app);
		//cout<<"i/o thread id:"<<sched_getcpu()<<endl;
		while(!readDoneFlag);
		readDoneFlag=0;
		
		for(int i=0;i<nThreadMultiplicity;i++)
		{
			ThreadPacket* thisThreadPacket=threadPacket[threadPacketIndex+i];
			
			thisThreadPacket->aquireData=new AquireData(blockIndex+i);	

			readTime=omp_get_wtime(); //benchmark
			timeReadData-=omp_get_wtime(); //benchmark
			thisThreadPacket->aquireData->readData();
			readTime=omp_get_wtime()-readTime; //benchmark
			benchmarkfile<<readTime<<endl;
	
		
			hasReachedEof=thisThreadPacket->aquireData->hasReachedEof;	
		
			
			
			if(hasReachedEof)
			{
				nThreadMultiplicityTemp=i+1;
				killFlag=1;
				break;
			}
		}
		for(int i=0;i<nThreadMultiplicity;i++)
		{
			ThreadPacket* thisThreadPacket=threadPacket[threadPacketIndex+i];
			if(blockIndex>=nThreadMultiplicity*nActions)
			{
				displayBlockIndex(blockIndex+i);
				writeAll(thisThreadPacket);	
			
			}
		}
		readCompleteFlag=1;
		if(!keepRunning)
			killFlag=1;
	}
	
	

}

void Runtime::floatConversionTasks(int threadPacketIndex)
{
	
	double readTime,floatTime,restTime;
	//cout<<"i/o thread id:"<<sched_getcpu()<<endl;
	ofstream benchmarkfile;
	benchmarkfile.open("benchmark_threadtime_indv.gpt",ios::app);

	#pragma omp parallel for schedule(dynamic, 1)
	for(int i=0;i<nThreadMultiplicity;i++)
	{
		ThreadPacket* thisThreadPacket=threadPacket[threadPacketIndex+i];
		
 		timeConvertToFloat-=omp_get_wtime(); //benchmark
		thisThreadPacket->aquireData->splitRawData();		
		timeConvertToFloat+=omp_get_wtime(); //benchmark

		for(int k=0;k<info.noOfPol;k++)
		{
			thisThreadPacket->basicAnalysis[k]=new BasicAnalysis(thisThreadPacket->aquireData->splittedRawData[k],k,thisThreadPacket->aquireData->blockLength);
		}
		thisThreadPacket->basicAnalysis[0]->headerInfo=thisThreadPacket->aquireData->headerInfo;
		delete thisThreadPacket->aquireData;
		thisThreadPacket->aquireData=NULL;


	}
	

}

void Runtime::plotTasks(int threadPacketIndex)
{
	int i=nThreadMultiplicity-1;
	ThreadPacket* thisThreadPacket=threadPacket[threadPacketIndex+i];	
		
	if(!info.doFilteringOnly)
	{
		timePlot-=omp_get_wtime();			
		plot->plotAll(thisThreadPacket->basicAnalysisWrite,thisThreadPacket->advancedAnalysis,thisThreadPacket->rFIFilteringTime,thisThreadPacket->rFIFilteringChan,blockIndex+i-nActions*nThreadMultiplicity);
		timePlot+=omp_get_wtime();
				
	}
}
/*******************************************************************
*FUNCTION: void channelTasks(int threadPacketIndex)
*This function performs the tasks on either thread two or three
*depending on if channel filtering is performed before or after time
*filtering.
*The tasks are :Calculating bandshapes, finding outliers in bandshape
*and normalizing data if enabled by user.
*For optimization considerations refer to document titled
*"Multithreading Considerations for gptool"
*******************************************************************/
void Runtime::channelTasks(int threadPacketIndex,char dosecondpass)
{
	#pragma omp parallel for ordered schedule(dynamic, 1)
	for(int t=0;t<nThreadMultiplicity;t++)
	{
		//cout<<"channel thread id:"<<sched_getcpu()<<endl;
		BasicAnalysis **basicAnalysis=threadPacket[threadPacketIndex+t]->basicAnalysis;
		RFIFiltering **rFIFilteringChan=threadPacket[threadPacketIndex+t]->rFIFilteringChan;
		RFIFiltering **rFIFilteringTime=threadPacket[threadPacketIndex+t]->rFIFilteringTime;
		for(int i=0;i<info.noOfPol;i++)
		{
			if(dosecondpass==0)
			{
				switch((int)info.bandshapeToUse)
				{
					case 1:
						
						rFIFilteringChan[i]=new RFIFiltering(&(basicAnalysis[i]->bandshape[info.startChannel]),info.stopChannel-info.startChannel);					
						rFIFilteringChan[i]->inputMax=basicAnalysis[i]->maxBandshape;
						rFIFilteringChan[i]->inputMin=basicAnalysis[i]->minBandshape;
						
						break;
					case 2:

						rFIFilteringChan[i]=new RFIFiltering(&(basicAnalysis[i]->normalizedBandshape[info.startChannel]),info.stopChannel-info.startChannel);
						rFIFilteringChan[i]->inputMax=basicAnalysis[i]->maxNormalizedBandshape;
						rFIFilteringChan[i]->inputMin=basicAnalysis[i]->minNormalizedBandshape;

						break;
					case 3:
						rFIFilteringChan[i]=new RFIFiltering(&(basicAnalysis[i]->meanToRmsBandshape[info.startChannel]),info.stopChannel-info.startChannel);	
						rFIFilteringChan[i]->inputMax=basicAnalysis[i]->maxMeanToRmsBandshape;
						rFIFilteringChan[i]->inputMin=basicAnalysis[i]->minMeanToRmsBandshape;
						break;
					case 4:
						rFIFilteringChan[i]=new RFIFiltering(&(basicAnalysis[i]->meanToRmsBandshape[info.startChannel]),info.stopChannel-info.startChannel);	
						rFIFilteringChan[i]->inputMax=basicAnalysis[i]->maxMeanToRmsBandshape;
						rFIFilteringChan[i]->inputMin=basicAnalysis[i]->minMeanToRmsBandshape;
						break;
					default:
						rFIFilteringChan[i]=new RFIFiltering(&(basicAnalysis[i]->bandshape[info.startChannel]),info.stopChannel-info.startChannel);
						rFIFilteringChan[i]->inputMax=basicAnalysis[i]->maxBandshape;
						rFIFilteringChan[i]->inputMin=basicAnalysis[i]->minBandshape;
						break;
				}
			
			}
			rFIFilteringChan[i]->generateBlankFlags();
			if((info.doTimeFlag && info.doChanFlag && (info.flagOrder==2 || (dosecondpass==1)) ))
			{
				timeBandshape-=omp_get_wtime(); //benchmark
				basicAnalysis[i]->computeBandshape(rFIFilteringTime[i]->flags);	
				basicAnalysis[i]->calculateCumulativeBandshapes();					
				timeBandshape+=omp_get_wtime(); //benchmark
			}
			else
			{	
				timeBandshape-=omp_get_wtime(); //benchmark
				basicAnalysis[i]->computeBandshape();		
				basicAnalysis[i]->calculateMeanByRMSBandshape();
				timeBandshape+=omp_get_wtime(); //benchmark
			}
			timeBandshape-=omp_get_wtime(); //benchmark
			#pragma omp ordered
			if(dosecondpass==0 && info.doChanFlag)
			{
				timeRFIChanStats-=omp_get_wtime(); //benchmark
				rFIFilteringChan[i]->cutoffToRms=info.chanCutOffToRMS;
				rFIFilteringChan[i]->computeStatistics(info.chanFlagAlgo);
				timeRFIChanStats+=omp_get_wtime(); //benchmark
			
				timeRFIChanFlag-=omp_get_wtime(); //benchmark
				
				rFIFilteringChan[i]->flagData();
				rFIFilteringChan[i]->generateManualFlags(info.nBadChanBlocks,info.badChanBlocks,info.startChannel);
			}
			else
			{
				timeBandshape+=omp_get_wtime(); //benchmark
				if(info.doChanFlag)
				{
					timeRFIChanStats-=omp_get_wtime(); //benchmark
					rFIFilteringChan[i]->cutoffToRms=info.chanCutOffToRMS;
					rFIFilteringChan[i]->computeStatistics(info.chanFlagAlgo);
					timeRFIChanStats+=omp_get_wtime(); //benchmark
				
					timeRFIChanFlag-=omp_get_wtime(); //benchmark
					
					rFIFilteringChan[i]->flagData();
					if((int)info.bandshapeToUse==4)
					{
						rFIFilteringChan[i]->input=&(basicAnalysis[i]->normalizedBandshape[info.startChannel]);
						rFIFilteringChan[i]->computeStatistics(info.chanFlagAlgo);
						rFIFilteringChan[i]->flagData();	
					}
					rFIFilteringChan[i]->generateManualFlags(info.nBadChanBlocks,info.badChanBlocks,info.startChannel);
					timeRFIChanFlag+=omp_get_wtime(); //benchmark	
				}
				else
				{
					rFIFilteringChan[i]->generateBlankFlags();
					rFIFilteringChan[i]->generateManualFlags(info.nBadChanBlocks,info.badChanBlocks,info.startChannel);
				}
			}
		
		}
		if(info.doPolarMode && dosecondpass==1)
		{
			//Set smooth bandshapes for cross terms
			basicAnalysis[1]->setNormalizedBandpass(basicAnalysis[0]->smoothBandshape,basicAnalysis[2]->smoothBandshape);
			basicAnalysis[3]->setNormalizedBandpass(basicAnalysis[0]->smoothBandshape,basicAnalysis[2]->smoothBandshape);
			 //transfer RR OR LL flags to all
			char* ptrChanFlag1=rFIFilteringChan[0]->flags;
			char* ptrChanFlag2=rFIFilteringChan[2]->flags;
			for(int i=0;i<info.stopChannel-info.startChannel;i++,ptrChanFlag1++,ptrChanFlag2++)
			{
				*ptrChanFlag1=(*ptrChanFlag1 | *ptrChanFlag2);			
			}
			for(int i=0;i<info.noOfPol;i++)
			{
				ptrChanFlag1=rFIFilteringChan[0]->flags;
				ptrChanFlag2=rFIFilteringChan[i]->flags;
				for(int j=0;j<info.stopChannel-info.startChannel;j++,ptrChanFlag1++,ptrChanFlag2++)
				{
					*ptrChanFlag2=*ptrChanFlag1;			
				}
			}
		}
	}
	

}
/*******************************************************************
*FUNCTION: void TimeTasks(int threadPacketIndex)
*This function performs the tasks on either thread two or three
*depending on if time filtering is performed before or after time
*filtering.
*The tasks are :Calculating zeroDM & finding outliers in zeroDM.
*For optimization considerations refer to document titled
*"Multithreading Considerations for gptool"
*******************************************************************/
void Runtime::timeTasks(int threadPacketIndex,char secondpass)
{
	float* histogramIntervalTemp=new float[info.noOfPol];
	#pragma omp parallel for schedule(dynamic, 1)
	for(int t=0;t<nThreadMultiplicity;t++)
	{
		//cout<<"time thread id:"<<sched_getcpu()<<endl;
		BasicAnalysis **basicAnalysis=threadPacket[threadPacketIndex+t]->basicAnalysis;
		RFIFiltering **rFIFilteringChan=threadPacket[threadPacketIndex+t]->rFIFilteringChan;
		RFIFiltering **rFIFilteringTime=threadPacket[threadPacketIndex+t]->rFIFilteringTime;
		
		for(int i=0;i<info.noOfPol;i++)
		{
			timeNormalization-=omp_get_wtime(); //benchmark
			if(info.doUseNormalizedData && secondpass==1)
				basicAnalysis[i]->normalizeData();
			timeNormalization+=omp_get_wtime(); //benchmark
			if(secondpass==0)
				rFIFilteringTime[i]=new RFIFiltering(basicAnalysis[i]->zeroDM,basicAnalysis[i]->blockLength);				
			if(info.doChanFlag || (info.doTimeFlag && info.doChanFlag && (info.flagOrder==1)))
			{
				timeZeroDM-=omp_get_wtime(); //benchmark
				if(blockIndex>6*nThreadMultiplicity && secondpass==0)		
				{		
					basicAnalysis[i]->computeZeroDMNorm(rFIFilteringChan[i]->flags,cumulativeBandpass[i]);	
				}
				else
					basicAnalysis[i]->computeZeroDM(rFIFilteringChan[i]->flags);		
					
				timeZeroDM+=omp_get_wtime(); //benchmark
			}
			else
			{		
				timeZeroDM-=omp_get_wtime(); //benchmark
				basicAnalysis[i]->computeZeroDM(blankChanFlags);			
				timeZeroDM+=omp_get_wtime(); //benchmark
			}
		
			if(info.doTimeFlag)
			{
				timeRFITimeStats-=omp_get_wtime(); //benchmark
				rFIFilteringTime[i]->cutoffToRms=info.timeCutOffToRMS;
				rFIFilteringTime[i]->inputMax=basicAnalysis[i]->maxZeroDM;
				rFIFilteringTime[i]->inputMin=basicAnalysis[i]->minZeroDM;
	
		 		//rFIFilteringTime[i]->histogramInterval=histogramInterval[i];
				if(blockIndex<=6*nThreadMultiplicity)
				{

					rFIFilteringTime[i]->computeStatistics(2);
					//rFIFilteringTime[i]->histogramInterval=(basicAnalysis[i]->maxZeroDM-basicAnalysis[i]->minZeroDM)/(pow(basicAnalysis[i]->blockLength,1/3.0));
				}


				if(secondpass==0)
				{
					//cout<<centralpass0[i]<<","<<stdpass0[i]<<endl;
					if(blockIndex>6*nThreadMultiplicity)
					{
						rFIFilteringTime[i]->centralTendency=centralpass0[i];
						rFIFilteringTime[i]->rms=stdpass0[i];
						rFIFilteringTime[i]->generateBlankFlags();
						rFIFilteringTime[i]->computeStatistics(2);
						
					}
					//cout<<centralpass0[i]<<","<<rFIFilteringTime[i]->centralTendency<<endl;
					//cout<<stdpass0[i]<<","<<rFIFilteringTime[i]->rms<<","<<endl;	
					if(info.doMultiPointFilter)
						rFIFilteringTime[i]->multiPointFlagData(info.cutoff);
					else
						rFIFilteringTime[i]->flagData();
					int flagcnt=0;
					for(int k=0;k< rFIFilteringTime[i]->inputSize;k++)
					{
						flagcnt+=!(rFIFilteringTime[i]->flags[k]);
					}
					rFIFilteringTime[i]->computeStatistics(info.timeFlagAlgo);
					//cout<<centralpass0[i]<<","<<rFIFilteringTime[i]->centralTendency<<endl;
					//cout<<stdpass0[i]<<","<<rFIFilteringTime[i]->rms<<","<<flagcnt<<endl;				
				
					
				}
				else
				{

					if(blockIndex>3*nThreadMultiplicity)
						rFIFilteringTime[i]->rms=stdpass1[i]; //using earlier estimate of std only for determining histogram interval
					//rFIFilteringTime[i]->histogramInterval=(4.0*stdpass1[i])/(pow(basicAnalysis[i]->blockLength,1/3.0));
					rFIFilteringTime[i]->computeStatistics(info.timeFlagAlgo);
					rFIFilteringTime[i]->generateBlankFlags();
					if(info.doMultiPointFilter)
						rFIFilteringTime[i]->multiPointFlagData(info.cutoff);
					else
						rFIFilteringTime[i]->flagData();

					stdpass1[i]=rFIFilteringTime[i]->rms;


					if(info.doZeroDMSub==1)				
						basicAnalysis[i]->subtractZeroDM(rFIFilteringChan[i]->flags,rFIFilteringTime[i]->centralTendency);

					if(info.smoothFlagWindowLength>0)
						rFIFilteringTime[i]->smoothFlags((int)info.smoothFlagWindowLength,info.concentrationThreshold);	
					if(info.doFlagWidthThreshold)
						rFIFilteringTime[i]->flagWidthThreshold(info.flagWidthOn,info.flagWidthOff);
					if(info.doSubbandFiltering)
					{
						rFIFilteringTime[i]->subbandFlagging(info.nSubFilt,info.nSubDetect,info.noOfChannels, info.startChannel,info.stopChannel, basicAnalysis[i]->rawData, rFIFilteringChan[i]->flags);
					}
					if(info.doFlagWidthThreshold || info.doSubbandFiltering)
						memcpy(rFIFilteringTime[i]->flags,rFIFilteringTime[i]->flagsFilt,rFIFilteringTime[i]->inputSize);
					if(info.doFRB)
					{
						if(!info.doReplaceByMean)
							basicAnalysis[i]->getFilteredRawDataChar(rFIFilteringTime[i]->flags,rFIFilteringChan[i]->flags,0);
						else if(info.doReplaceByMean==1)
							basicAnalysis[i]->getFilteredRawDataChar(rFIFilteringTime[i]->flags,rFIFilteringChan[i]->flags,rFIFilteringTime[i]->centralTendency);
						else if(info.doReplaceByMean==2)
							basicAnalysis[i]->getFilteredRawDataSmoothBshapeChar(rFIFilteringTime[i]->flags,rFIFilteringChan[i]->flags);
					}
					
					if(!info.doReplaceByMean && (info.doWriteFiltered2D || info.isInline))
						basicAnalysis[i]->getFilteredRawData(rFIFilteringTime[i]->flags,rFIFilteringChan[i]->flags,0);
					if(info.doReplaceByMean==1)
						basicAnalysis[i]->getFilteredRawData(rFIFilteringTime[i]->flags,rFIFilteringChan[i]->flags,rFIFilteringTime[i]->centralTendency);
					else if(info.doReplaceByMean==2)
						basicAnalysis[i]->getFilteredRawDataSmoothBshape(rFIFilteringTime[i]->flags,rFIFilteringChan[i]->flags);
					timeRFITimeFlags+=omp_get_wtime(); //benchmark

					
				}			
				
				
			
			}
			else
			{
				rFIFilteringTime[i]->generateBlankFlags();
				if(info.doZeroDMSub==1)				
					basicAnalysis[i]->subtractZeroDM(rFIFilteringChan[i]->flags,1);
			}
			
		}
		
		if(info.doPolarMode)
		{ //transfer RR OR LL flags to all
			char* ptrTimeFlag1=rFIFilteringTime[0]->flags;
			char* ptrTimeFlag2=rFIFilteringTime[2]->flags;
			for(int i=0;i<basicAnalysis[0]->blockLength;i++,ptrTimeFlag1++,ptrTimeFlag2++)
			{
				*ptrTimeFlag1=(*ptrTimeFlag1 | *ptrTimeFlag2);			
			}
			for(int i=0;i<info.noOfPol;i++)
			{
				ptrTimeFlag1=rFIFilteringTime[0]->flags;
				ptrTimeFlag2=rFIFilteringTime[i]->flags;
				for(int j=0;j<basicAnalysis[0]->blockLength;j++,ptrTimeFlag1++,ptrTimeFlag2++)
				{
					*ptrTimeFlag2=*ptrTimeFlag1;			
				}
			}
		}
	}
	RFIFiltering **rFIFilteringTime=threadPacket[threadPacketIndex+nThreadMultiplicity-1]->rFIFilteringTime;
	for(int i=0;i<info.noOfPol;i++)
	{
		for(int i=0;i<info.noOfPol;i++)
		{
			if(secondpass==0)
			{
				centralpass0[i]=rFIFilteringTime[i]->centralTendency;
				stdpass0[i]=rFIFilteringTime[i]->rms;
			}
			else
			{
				centralpass1[i]=rFIFilteringTime[i]->centralTendency;
				stdpass1[i]=rFIFilteringTime[i]->rms;
			}
		}
	}
	//cout<<"Pass:"<<(int)secondpass<<endl;
	//for(int i=0;i<info.noOfPol;i++)
	//{
	//	histogramInterval[i]=histogramIntervalTemp[i];
	//	cout<<histogramInterval[i]<<endl;
	//}
	//delete[] histogramIntervalTemp;
	
}
/*******************************************************************
*FUNCTION: void thread4Tasks(int threadPacketIndex)
*This function performs the tasks on thread four.
*The tasks are :Calculating dedispersed time series and folding it
*to get the profile.
*For optimization considerations refer to document titled
*"Multithreading Considerations for gptool"
*******************************************************************/
void Runtime::fullDMTask(int threadPacketIndex)
{
	float timeCalc=0.0;
	float timeMerge=0.0;
	float timeNormalize=0.0;
	omp_set_nested(1);
	#pragma omp parallel for ordered schedule(dynamic, 1) 
	for(int t=0;t<nThreadMultiplicity;t++)
	{
		//cout<<"dedisp thread id:"<<sched_getcpu()<<endl;
		BasicAnalysis **basicAnalysis=threadPacket[threadPacketIndex+t]->basicAnalysis;
		RFIFiltering **rFIFilteringChan=threadPacket[threadPacketIndex+t]->rFIFilteringChan;
		RFIFiltering **rFIFilteringTime=threadPacket[threadPacketIndex+t]->rFIFilteringTime;
		AdvancedAnalysis **advancedAnalysis=threadPacket[threadPacketIndex+t]->advancedAnalysis;
		AdvancedAnalysis **advancedAnalysisOld;
		if(t==0)
			advancedAnalysisOld=threadPacket[threadPacketIndex]->advancedAnalysisOld;

		else
			advancedAnalysisOld=threadPacket[threadPacketIndex+t-1]->advancedAnalysis;
		for(int k=0;k<info.noOfPol;k++)
		{
			advancedAnalysis[k]=new AdvancedAnalysis(blockIndex+t-(nActions-1)*nThreadMultiplicity,k,basicAnalysis[k]->rawData,basicAnalysis[k]->blockLength);
			timeFullDMCalc-=omp_get_wtime(); //benchmark
			if(info.doReplaceByMean)
				advancedAnalysis[k]->calculateFullDM(basicAnalysis[k]->filteredRawData);
			else
				advancedAnalysis[k]->calculateFullDM(rFIFilteringTime[k]->flags,rFIFilteringChan[k]->flags);
			timeFullDMCalc+=omp_get_wtime(); //benchmark
		}
		timeFullDMCalc-=omp_get_wtime(); //benchmark
		
		#pragma omp ordered
		for(int k=0;k<info.noOfPol;k++)
			advancedAnalysis[k]->mergeExcess(advancedAnalysisOld[k]->excess,advancedAnalysisOld[k]->countExcess,advancedAnalysisOld[k]->excessUnfiltered,advancedAnalysisOld[k]->countExcessUnfiltered);
		timeFullDMCalc+=omp_get_wtime(); //benchmar
		
		for(int k=0;k<info.noOfPol;k++)
		{
			
			timeFullDMCalc-=omp_get_wtime(); //benchmark
			advancedAnalysis[k]->normalizeFullDM();
			timeFullDMCalc+=omp_get_wtime(); //benchmark
			
			timeProfileCalc-=omp_get_wtime(); //benchmark
			advancedAnalysis[k]->calculateProfile();	
			timeProfileCalc+=omp_get_wtime(); //benchmark
		}		
	}
	
}



void intHandler(int) {
    	keepRunning = false;
}


int main(int argc, char *argv[])
{
	double totalTime=omp_get_wtime(); //benchmark	
	
	Information info;		
	info.startTime=0.0;
	info.doFilteringOnly=0;	
	info.doUseTempo2=0;
	info.doZeroDMSub=0;
	info.doRunFilteredMode=0;
	info.psrcatdbPath=NULL;
	info.isInline=0;
	info.doFRB=0;
	info.shmID=1;
	int arg = 1;
	int nThreadMultiplicity=1;
	info.meanval=8*1024;
	//Hard-coding optimization for Band-4 FRB data

	info.doFlagWidthThreshold=0;
	info.flagWidthOn=3;
	info.flagWidthOff=3;

	info.doSubbandFiltering=0;
	info.nSubFilt=5;
	info.nSubDetect=3;
	//...
  	if(argc <= 1)
  	{
    		cout<<"Not enough arguments."<<endl;
		info.displayNoOptionsHelp();
    		exit(0);
  	}
  	while(arg < argc)
  	{
  		arg++;
  	}
  	arg=1;
  	while(arg < argc)
  	{
    		if(argv[arg][0]=='-')
    		{
        		   
      			switch(argv[arg][1])
     			{
				case 'd':
        			case 'D':
        			{   
					if(string(argv[arg]) == "-doFRB")
					{
						info.doFRB=1;
						arg+=1;
					}
          				
				}
        			break;
        			case 'f':
        			case 'F':
        			{          
          				info.filepath = argv[arg+1];
          				info.doReadFromFile = 1;
          				arg+=2;
        			}
        			break;
        			case 'o':
        			{          
          				info.outputfilepath = argv[arg+1];
          				arg+=2;
        			}
        			break;
        			case 'r':
        			{          
					cout<<"Reading from shm"<<endl;	
					info.doReadFromFile=0;
          				arg+=1;
        			}
        			break;
        			case 's':
        			{       
					if(string(argv[arg]) == "-shmID")
					{
						info.shmID=info.stringToDouble(argv[arg+1]);				
          					arg+=2;
					}
					else
					{   
						if(info.doReadFromFile)
						{
							info.startTime=info.stringToDouble(argv[arg+1]);
		  					if(info.startTime<0)
		  					{
		  						cout<<"Start time cannot be negetive!"<<endl;
		  						exit(0);
		  					}
		  				}
					}
          				
          				arg+=2;
        			}
				break;
				case 't':
        			{          
					if(string(argv[arg]) == "-tempo2")
					{
						info.doUseTempo2=1;
						arg+=1;
					}
					else
					{
						nThreadMultiplicity=info.stringToDouble(argv[arg+1]);				
          					arg+=2;
					}
        			}
				break;
				case 'z':
        			{          
					if(string(argv[arg]) == "-zsub")
					{
						info.doZeroDMSub=1;
						arg+=1;
					}
        			}
				break;
				case 'i':
        			{          
					if(string(argv[arg]) == "-inline")
					{
						info.doReadFromFile=0;
						//info.doRunFilteredMode=1;
						info.isInline=1;
						arg+=1;
					}
        			}
				break;
				case 'g':
        			{          
					if(string(argv[arg]) == "-gfilt")
					{
						info.doRunFilteredMode=1;
						arg+=1;
					}
        			}
        			break; 
				case 'm':
        			{          
					if(string(argv[arg]) == "-m")
					{
						info.meanval=int(info.stringToDouble(argv[arg+1]));
						arg+=2;
					}
        			}
        			break;
				default:
        			{    
						    
					if(string(argv[arg]) == "-nodedisp")
        					info.doFilteringOnly=1;	
							
          				arg+=1;
        			}
        			break;     
      
    			}
    		}
    		else
    		{
      			cout<<"Invalid option."<<endl;
			info.displayNoOptionsHelp();
      			exit(1);
    		}
  	}
	if(!info.checkGptoolInputFileVersion())
	{
		cout<<"Old version of gptool.in file found.\n Replacing by new version formatting.\n Old version copied to gptool.in.oldver"<<endl;
		cout<<"Note this conversion will fail if the oldversion is not ver 1.5"<<endl;
		info.reformatGptoolInputFile();
	}
	info.readGptoolInputFile();

	
	

	startFlags=new char[info.startChannel];
	endFlags=new char[info.noOfChannels-info.stopChannel];
	for(int i=0;i<info.startChannel;i++)
		startFlags[i]=1;
	for(int i=0;i<info.noOfChannels-info.stopChannel;i++)
		endFlags[i]=1;
	Runtime* runtime=new Runtime(info,nThreadMultiplicity);
	runtime->intializeFiles();
	
		
		
	
	//code to capture cltr+c termination
	struct sigaction act;
    	act.sa_handler = intHandler;
    	sigaction(SIGINT, &act, NULL);
	
    	
    	#pragma omp parallel sections 
	{
			
				
		#pragma omp section
		{
			
			runtime->action(0,0);
					
		}
		#pragma omp section
		{
			runtime->fillPipe();
			double loopTime=omp_get_wtime(); //benchmark
			runtime->loopThrough();
			loopTime=omp_get_wtime()-loopTime;
			if(!keepRunning)
				runtime->quickclosePipe();
			else
				runtime->closePipe();
		}
	}

	char *gptoolPath = getenv("GPTOOL_PATH");
	cout<<gptoolPath<<endl;
	if(gptoolPath==NULL)
		cout<<"Cannot plot summary pdf. Please set GPTOOL_PATH to path of plotgptoolsummary.py"<<endl;
	else
	{
		cout<<"plotting summmary pdf..."<<endl;	
		stringstream plotcmd;
		plotcmd<<"python2 "<<gptoolPath<<"/plotgptoolsummary.py";	
		system(plotcmd.str().c_str());
	}
	//writing benchmark files
	int i=runtime->blockIndex;
	ofstream benchmarkfile;
	benchmarkfile.open("benchmark.gpt",ios::app);	
	benchmarkfile<<info.samplingInterval<<",";
	benchmarkfile<<info.blockSizeSamples<<",";	
	benchmarkfile<<(timeReadData+timeWaitTime)/(float)(i)<<",";
	benchmarkfile<<(timeConvertToFloat)/(float)(i)<<",";
	benchmarkfile<<-timeWaitTime/(float)(i)<<",";
	benchmarkfile<<timeBandshape/(float)(i)<<",";
	benchmarkfile<<timeZeroDM/(float)(i)<<",";
	benchmarkfile<<timeNormalization/(float)(i)<<",";
	benchmarkfile<<timeRFITimeStats/(float)(i)<<",";
	benchmarkfile<<timeRFITimeFlags/(float)(i)<<",";
	benchmarkfile<<timeRFITimeFlagsWrite/(float)(i)<<",";
	benchmarkfile<<timeRFIChanStats/(float)(i)<<",";
	benchmarkfile<<timeRFIChanFlag/(float)(i)<<",";
	benchmarkfile<<timeRFIChanFlagsWrite/(float)(i)<<",";
	benchmarkfile<<timeFullDMCalc/(float)(i)<<",";
	benchmarkfile<<timeFullDMWrite/(float)(i)<<",";
	benchmarkfile<<timeProfileCalc/(float)(i)<<",";
	benchmarkfile<<timeFullDMUnfilteredCalc/(float)(i)<<",";
	benchmarkfile<<timeFullDMUnfilteredWrite/(float)(i)<<",";
	benchmarkfile<<timeProfileUnfilteredCalc/(float)(i)<<",";
	benchmarkfile<<timePlot/(float)(i)<<endl;
	benchmarkfile.close();
	
	totalTime=omp_get_wtime()-totalTime;
	
	benchmarkfile.open("benchmark_threadtime.gpt",ios::app);	
	benchmarkfile<<nThreadMultiplicity<<","<<runtime->info.blockSizeSamples<<","<<i<<","<<timeThread1/(float)(numberOfThreadRuns)<<","<<timeThread2/(float)(numberOfThreadRuns)<<","<<timeThread3/(float)(numberOfThreadRuns)<<","<<timeThread4/(float)(numberOfThreadRuns)<<","<<timeWaitTime<<","<<fillTime<<","<<totalTime<<endl;
	benchmarkfile.close();
	
	benchmarkfile.open("benchmark_fillTime.gpt",ios::app);
	benchmarkfile<<nThreadMultiplicity<<","<<fillTime<<endl;	
	benchmarkfile.close();
	delete runtime;
	
	exit(0);
	
}



