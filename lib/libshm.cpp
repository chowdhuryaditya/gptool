/*************************************************************
shm library
This library is aimed at interfacing between various
kinds of shared memory structures used by different
pulsar codes at the GMRT.
				-Aditya Chowdhury, 14th March 2016
************************************************************/
#include "SHM.h"
using namespace std;



DasHdrType*	Correlator::dataHdrWrite;
DataBufType*	Correlator::dataBufferWrite;
DataTabType*	Correlator::dataTabWrite;

DataHeader*	Correlator::dataHdrWriteFRB;
DataBuffer*	Correlator::dataBufferWriteFRB;

DasHdrType*	Correlator::dataHdrRead;
DataBufType*	Correlator::dataBufferRead;
DataTabType*	Correlator::dataTabRead;

DataHeader*	Correlator::dataHdrReadFRB;
DataBuffer*	Correlator::dataBufferReadFRB;

int		Correlator::recNumRead=0;
int		Correlator::recNumWrite=0;
long int	Correlator::currentReadBlock=0;


Correlator::Correlator(DasHdrType *_dataHdrRead ,DataBufType *_dataBufferRead)
{
	DataOff=4096;
	debug=1;
	dataHdrRead=_dataHdrRead;
	dataBufferRead=_dataBufferRead;
}
Correlator::Correlator(int _nchan,float _sampling)
{
	DataOff=4096;
	debug=1;
	nchan=_nchan;
	sampling=_sampling;
	dataHdrRead=NULL;
	dataBufferRead=NULL;
}
/*******************************************************************
*FUNCTION: void AquireData::initializeSHM()
*Initializes collect_psr shared memory of GSB/GWB
*******************************************************************/
void Correlator::initializeReadSHM()
{
  	int iddataHdr,idDataBuffer;
	iddataHdr = shmget(DAS_H_KEY,sizeof(DasHdrType),644);
	idDataBuffer = shmget(DAS_D_KEY,sizeof(DataBufType),0);
	cout<<"iddataHdr="<<iddataHdr<<endl<<"idDataBuffer="<<idDataBuffer<<endl;
	
	if(iddataHdr < 0 || iddataHdr < 0)
	{
		exit(1);
	}
	dataHdrRead = (DasHdrType*)shmat(iddataHdr,0,SHM_RDONLY);
  	dataBufferRead = (DataBufType*)shmat(idDataBuffer,0,SHM_RDONLY);
	if((dataBufferRead)==(DataBufType*)-1)
	{
		cout<<"Cannot attach to shared memory!"<<endl;
		exit(1);
	}
	cout<<"Attached to shared memory:"<<dataHdrRead<<","<<dataBufferRead<<endl;
	dataTabRead = dataBufferRead-> dtab;
	cout<<"Max no of blocks="<<dataBufferRead->maxblocks<<endl;

	 /*   find a block, two blocks before the current block of the shm for reading data */
	//if(dataBuffer->cur_rec > (dataBuffer->maxblocks)/2)
	recNumRead = (dataBufferRead->cur_rec-2+MaxDataBuf)%MaxDataBuf;
	currentReadBlock = dataTabRead[recNumRead].seqnum;  	
}
/*******************************************************************
*FUNCTION: void AquireData::initializeSHM()
*Initializes collect_psr shared memory of GSB/GWB
*******************************************************************/
void Correlator::initializeReadSHM_FRB(char fileSHM)
{
  	int iddataHdr,idDataBuffer;
	if(!fileSHM)
	{
		iddataHdr = shmget(DasHeaderKey,sizeof(DataHeader),644 );
		idDataBuffer = shmget(DasBufferKey,sizeof(DataBuffer),644);
	}
	else
	{
		iddataHdr = shmget(DasHeaderKey_SIM,sizeof(DataHeader),644);
		idDataBuffer = shmget(DasBufferKey_SIM,sizeof(DataBuffer),644);
	}
	cout<<"iddataHdr="<<iddataHdr<<endl<<"idDataBuffer="<<idDataBuffer<<endl;
	
	if(iddataHdr < 0 || iddataHdr < 0)
	{
		exit(1);
	}
	dataHdrReadFRB = (DataHeader*)shmat(iddataHdr,0,SHM_RDONLY);
  	dataBufferReadFRB = (DataBuffer*)shmat(idDataBuffer,0,SHM_RDONLY);
	cout<<"Attached to shared memory:"<<dataHdrReadFRB<<","<<dataBufferReadFRB<<endl;
	
	cout<<"Max no of blocks="<<MaxDataBlocks<<endl;
	if((dataBufferReadFRB)==(DataBuffer*)-1)
	{
		cout<<"Cannot attach to shared memory!"<<endl;
		exit(1);
	}
	 /*   find a block, two blocks before the current block of the shm for reading data */
	//if(dataBuffer->cur_rec > (dataBuffer->maxblocks)/2)
	recNumRead = (dataBufferReadFRB->curRecord-2+MaxDataBlocks)%MaxDataBlocks;
	currentReadBlock = dataBufferReadFRB->curBlock-2;  	
}
void Correlator::copyHeaderInfo()
{
	dataHdrWrite->active=dataHdrRead->active;
	dataHdrWrite->status=dataHdrRead->status;
	dataHdrWrite->scan=dataHdrRead->scan;
	dataHdrWrite->scan_off=dataHdrRead->scan_off;
	dataHdrWrite->corr=dataHdrRead->corr;
	dataHdrWrite->model=dataHdrRead->model;
	dataHdrWrite->BeamHeader=dataHdrRead->BeamHeader;
 	dataBufferWrite->blocksize=dataBufferRead->blocksize;
	dataBufferWrite->maxblocks=dataBufferRead->maxblocks;
}
/*******************************************************************
*FUNCTION: void AquireData::initializeSHM()
*Initializes collect_psr shared memory of GSB/GWB
*******************************************************************/
int Correlator::initializeWriteSHM()
{
  	int iddataHdr,idDataBuffer;
	if(dataHdrRead==NULL)
	{
		iddataHdr = shmget(DAS_H_KEY_GPTOOL,sizeof(DasHdrType),IPC_CREAT | 0666 );
		idDataBuffer = shmget(DAS_D_KEY_GPTOOL,sizeof(DataBufType),IPC_CREAT | 0666 );
	}
	else
	{
		iddataHdr = shmget(DAS_H_KEY_GPTOOL_INLINE,sizeof(DasHdrType),IPC_CREAT | 0666 );
		idDataBuffer = shmget(DAS_D_KEY_GPTOOL_INLINE,sizeof(DataBufType),IPC_CREAT | 0666 );
	}
	cout<<"iddataHdr="<<iddataHdr<<endl<<"idDataBuffer="<<idDataBuffer<<endl;
	
	if(iddataHdr < 0 || iddataHdr < 0)
	{
		cout<<"Error creating shared memory"<<endl;
		exit(1);
	}
	dataHdrWrite = (DasHdrType*)shmat(iddataHdr,0,0);
  	dataBufferWrite = (DataBufType*)shmat(idDataBuffer,0,0);
	if((dataBufferWrite)==(DataBufType*)-1)
	{
		cout<<"Cannot attach to shared memory!"<<endl;
		exit(1);
	}
	cout<<"Attached to write shared memory:"<<dataHdrWrite<<","<<dataBufferWrite<<endl;

	if(dataHdrRead==NULL)
	{	
		dataBufferWrite->blocksize=2*nchan*int(BLOCKTIME/sampling)+DataOff;
		dataBufferWrite->maxblocks=int(DAS_BUFSIZE/(dataBufferWrite->blocksize));
	}
	else
	{
		copyHeaderInfo();
		cout<<"header info copied"<<endl;	
	}
	dataTabWrite = dataBufferWrite-> dtab;
	dataBufferWrite->cur_rec=0;
	dataBufferWrite->cur_block=0;
	recNumWrite = (dataBufferWrite->cur_rec)%MaxDataBuf;
	dataTabWrite[recNumWrite].seqnum=0;
	dataTabWrite[recNumWrite].rec=0;
	for(int i=0;i<MaxDataBuf;i++)
		dataTabWrite[i].flag=0;
	cout<<"Max no of blocks="<<dataBufferWrite->maxblocks<<endl;
	cout<<"blocksize: "<<dataBufferWrite->blocksize<<endl;
	dataHdrWrite->status = DAS_START;
	return dataBufferWrite->blocksize-DataOff;
}
/*******************************************************************
*FUNCTION: void AquireData::initializeSHM()
*Initializes collect_psr shared memory of GSB/GWB
*******************************************************************/
int Correlator::initializeWriteSHM_FRB(char fileSHM)
{
  	int iddataHdr,idDataBuffer;
	if(!fileSHM)
	{
		idDataBuffer = shmget(DasBufferKey,sizeof(DataBuffer),IPC_CREAT | 0666 );
		iddataHdr = shmget(DasHeaderKey,sizeof(DataHeader),IPC_CREAT | 0666 );
	}
	else
	{
		idDataBuffer = shmget(DasBufferKey_SIM,sizeof(DataBuffer),IPC_CREAT | 0666 );
		iddataHdr = shmget(DasHeaderKey_SIM,sizeof(DataHeader),IPC_CREAT | 0666 );
	}

	
	cout<<"iddataHdr="<<iddataHdr<<endl<<"idDataBuffer="<<idDataBuffer<<endl;
	
	if(iddataHdr < 0 || iddataHdr < 0)
	{
		cout<<"Error creating shared memory"<<endl;
		exit(1);
	}
	dataHdrWriteFRB = (DataHeader*)shmat(iddataHdr,0,0);
  	dataBufferWriteFRB = (DataBuffer*)shmat(idDataBuffer,0,0);
	if(dataBufferWriteFRB==(DataBuffer*)-1)
	{
		cout<<"Cannot attach to shared memory!"<<endl;
		exit(1);
	}
	cout<<"Attached to write shared memory:"<<dataHdrWriteFRB<<","<<dataBufferWriteFRB<<endl;

	
	
	dataBufferWriteFRB->curRecord=0;
	dataBufferWriteFRB->curBlock=0;
	recNumWrite = (dataBufferWriteFRB->curRecord)%MaxDataBlocks;
	
	cout<<"Max no of blocks="<<MaxDataBlocks<<endl;
	cout<<"blocksize: "<<DataSize<<endl;
	dataHdrWriteFRB->active = 1;
	return DataSize;
}
void Correlator::writeToSHM_FRB(unsigned char* rawData)
{  	
 	//long int fetched=0;
	//ofstream warnFile;
	//dataTabWrite[recNumWrite].seqnum=dataBufferWrite->cur_block;
	//dataTabWrite[recNumWrite].rec=(dataBufferWrite->cur_block)%(dataBufferWrite->maxblocks);
	
	/**
	if(debug)
	{
		cout<<endl<<"recNum "<<recNumWrite<<endl;
		cout<<"dataBuffer->cur_rec "<<dataBufferWrite->cur_rec<<endl;
		cout<<"MaxDataBuf "<<MaxDataBuf<<endl;
		cout<<"dataBuffer->cur_block "<<dataBufferWrite->cur_block<<endl;
		cout<<"dataBuffer->maxblocks "<<dataBufferWrite->maxblocks<<endl;
		cout<<"dataTabWrite[recNumWrite].rec "<<dataTabWrite[recNumWrite].rec<<endl;
		cout<<"dataTabWrite[recNumWrite].flag "<<dataTabWrite[recNumWrite].flag<<endl;
		cout<<"dataTabWrite[recNumWrite].seqnum"<<dataTabWrite[recNumWrite].seqnum<<endl;
		cout<<"dataHdrWrite->status "<<dataHdrWrite->status<<endl;
		cout<<dataBufferWrite->blocksize<<endl;
	}	
	**/	
	cout<<"Memcpy to SHM"<<endl;
	cout<<"Writing Record:"<<dataBufferWriteFRB->curRecord<<endl;
	cout<<"Writing Block:"<<dataBufferWriteFRB->curBlock<<endl;
	memcpy( dataBufferWriteFRB->data+(long)DataSize*(long)recNumWrite,rawData, DataSize);
	cout<<"Done memcpy to SHM"<<endl;
  		
	dataBufferWriteFRB->curRecord=(recNumWrite+1)%MaxDataBlocks;
	dataBufferWriteFRB->curBlock+=1;
	//warnFile.close();
	//dataTabWrite[(recNumWrite+1)%MaxDataBuf].flag=0;
	//dataTabWrite[(recNumWrite)].flag=BufReady;
	recNumWrite=(recNumWrite+1)%MaxDataBlocks;  

}
void Correlator::writeToSHM(unsigned short int* rawData)
{  	
 	long int fetched=0;
	ofstream warnFile;
	dataTabWrite[recNumWrite].seqnum=dataBufferWrite->cur_block;
	dataTabWrite[recNumWrite].rec=(dataBufferWrite->cur_block)%(dataBufferWrite->maxblocks);
	
	
	if(debug)
	{
		cout<<endl<<"recNum "<<recNumWrite<<endl;
		cout<<"dataBuffer->cur_rec "<<dataBufferWrite->cur_rec<<endl;
		cout<<"MaxDataBuf "<<MaxDataBuf<<endl;
		cout<<"dataBuffer->cur_block "<<dataBufferWrite->cur_block<<endl;
		cout<<"dataBuffer->maxblocks "<<dataBufferWrite->maxblocks<<endl;
		cout<<"dataTabWrite[recNumWrite].rec "<<dataTabWrite[recNumWrite].rec<<endl;
		cout<<"dataTabWrite[recNumWrite].flag "<<dataTabWrite[recNumWrite].flag<<endl;
		cout<<"dataTabWrite[recNumWrite].seqnum"<<dataTabWrite[recNumWrite].seqnum<<endl;
		cout<<"dataHdrWrite->status "<<dataHdrWrite->status<<endl;
		cout<<dataBufferWrite->blocksize<<endl;
	}	
		
	memcpy(dataBufferWrite->buf+dataTabWrite[recNumWrite].rec*(dataBufferWrite->blocksize)+DataOff,rawData, dataBufferWrite->blocksize-DataOff);
	
  		
	dataBufferWrite->cur_rec=(recNumWrite+1)%MaxDataBuf;
	dataBufferWrite->cur_block+=1;
	warnFile.close();
	dataTabWrite[(recNumWrite+1)%MaxDataBuf].flag=0;
	dataTabWrite[(recNumWrite)].flag=BufReady;
	recNumWrite=(recNumWrite+1)%MaxDataBuf;  

}
void Correlator::writeToSHM(short int* rawData,char* header)
{  	
	dataTabWrite[recNumWrite].seqnum=dataBufferWrite->cur_block;
	dataTabWrite[recNumWrite].rec=(dataBufferWrite->cur_block)%(dataBufferWrite->maxblocks);
	if(debug)
	{
		cout<<endl<<"recNum "<<recNumWrite<<endl;
		cout<<"dataBuffer->cur_rec "<<dataBufferWrite->cur_rec<<endl;
		cout<<"MaxDataBuf "<<MaxDataBuf<<endl;
		cout<<"dataBuffer->cur_block "<<dataBufferWrite->cur_block<<endl;
		cout<<"dataBuffer->maxblocks "<<dataBufferWrite->maxblocks<<endl;
		cout<<"dataTabWrite[recNumWrite].rec "<<dataTabWrite[recNumWrite].rec<<endl;
		cout<<"dataTabWrite[recNumWrite].flag "<<dataTabWrite[recNumWrite].flag<<endl;
		cout<<"dataTabWrite[recNumWrite].seqnum"<<dataTabWrite[recNumWrite].seqnum<<endl;
		cout<<"dataHdrWrite->status "<<dataHdrWrite->status<<endl;
		cout<<dataBufferWrite->blocksize<<endl;
	}	
	cout<<"start memcopy rawdata"<<endl;	
	memcpy(dataBufferWrite->buf+dataTabWrite[recNumWrite].rec*(dataBufferWrite->blocksize)+DataOff,rawData, dataBufferWrite->blocksize-DataOff);
	cout<<"memcpy done"<<endl;
	//cout<<"start memcopy header"<<endl;
	//memcpy(dataBufferWrite->buf+dataTabWrite[recNumWrite].rec*(dataBufferWrite->blocksize),header,DataOff);	
 	//cout<<"memcpy done"<<endl; 		
	dataBufferWrite->cur_rec=(recNumWrite+1)%MaxDataBuf;
	dataBufferWrite->cur_block+=1;
	dataTabWrite[(recNumWrite+1)%MaxDataBuf].flag=0;
	dataTabWrite[(recNumWrite)].flag=BufReady;
	recNumWrite=(recNumWrite+1)%MaxDataBuf;  

}
/*******************************************************************
*FUNCTION: AquireData::readFromSHM()
*Reads from collect_psr shared memory of GSB/GWB
*******************************************************************/
void Correlator::readFromSHM(unsigned short int* rawData)
{  	

	ofstream warnFile;
	warnFile.open("realTimeWarning.gpt",ios::app);
	int flag=0;
	while((dataHdrRead->status == DAS_START) && (dataTabRead[recNumRead].flag &BufReady) == 0)
	{
		usleep(2000);
		if(flag==0)
		{
			cout<<"Waiting"<<endl;
			flag=1;
		}
	}
	if(flag==1)
		cout<<"Ready"<<endl;
		
	if(dataHdrRead->status != DAS_START)
	{
		if ((dataTabRead[recNumRead].flag & BufReady) == 0)
		{
			cout<<"DAS not in START mode!!"<<endl;
			exit(0);
		}
	}
	currentReadBlock = dataTabRead[recNumRead].seqnum;
	if(debug)
	{
		cout<<endl<<"recNum "<<recNumRead<<endl;
		cout<<"dataBuffer->cur_rec "<<dataBufferRead->cur_rec<<endl;
		cout<<"MaxDataBuf "<<MaxDataBuf<<endl;
		cout<<"dataBuffer->cur_block "<<dataBufferRead->cur_block<<endl;
		cout<<"currentReadBlock "<<currentReadBlock<<endl;
		cout<<"dataBuffer->maxblocks "<<dataBufferRead->maxblocks<<endl<<endl;
	}
		
	if(dataBufferRead->cur_block - currentReadBlock >=dataBufferRead->maxblocks-1)
	{
		warnFile<<"recNum = "<<recNumRead<<", Reading Sequence: "<<currentReadBlock<<", Collect's Sequence: "<<(dataBufferRead->cur_block-1)<<endl;
		warnFile<<"Processing lagged behind..."<<endl;
			
		cout<<"recNum = "<<recNumRead<<", Reading Sequence: "<<currentReadBlock<<", Collect's Sequence: "<<(dataBufferRead->cur_block-1)<<"\nRealiging...\n";
		recNumRead = (dataBufferRead->cur_rec-1-2+MaxDataBuf)%MaxDataBuf;
		currentReadBlock = dataTabRead[(recNumRead)].seqnum;
	}
		
	memcpy(rawData, dataBufferRead->buf+dataTabRead[recNumRead].rec*(dataBufferRead->blocksize)+DataOff, dataBufferRead->blocksize-DataOff);
  	recNumRead=(recNumRead+1)%MaxDataBuf;
  	
	warnFile.close();
}

/*******************************************************************
*FUNCTION: AquireData::readFromSHM()
*Reads from collect_psr shared memory of GSB/GWB
*******************************************************************/
void Correlator::readFromSHM_FRB(unsigned char* rawData)
{  	

	ofstream warnFile;
	warnFile.open("realTimeWarning.gpt",ios::app);
	int flag=0;
	while(currentReadBlock==dataBufferReadFRB->curRecord)
	{
		usleep(2000);
		if(flag==0)
		{
			cout<<"Waiting"<<endl;
			flag=1;
		}
	}
	if(flag==1)
		cout<<"Ready"<<endl;
		
	
	currentReadBlock++;
	if(debug)
	{
		cout<<"dataBuffer->curRecord "<<dataBufferReadFRB->curRecord<<endl;
		cout<<"MaxDataBlocks "<<MaxDataBlocks<<endl;
		cout<<"dataBuffer->curBlock "<<dataBufferReadFRB->curBlock<<endl;
		cout<<"currentReadBlock "<<currentReadBlock<<endl;
		cout<<"recNumRead"<<recNumRead<<endl;
	}
		
	if(dataBufferReadFRB->curBlock - currentReadBlock >=MaxDataBlocks-1)
	{
		
		warnFile<<"Processing lagged behind..."<<endl;
			
		cout<<"\nRealiging...\n";
		recNumRead = (dataBufferReadFRB->curRecord-2+MaxDataBlocks)%MaxDataBlocks;
		currentReadBlock = dataBufferReadFRB->curBlock;
	}
	memcpy(rawData, dataBufferReadFRB->data+((long)DataSize*(long)recNumRead),DataSize);
  	recNumRead=(recNumRead+1)%MaxDataBlocks;
  	
	warnFile.close();
}

/*******************************************************************
*FUNCTION: AquireData::readFromSHM()
*Reads from collect_psr shared memory of GSB/GWB
*******************************************************************/
/*
void Correlator::readFromSHM(char* rawData)
{  	

	ofstream warnFile;
	warnFile.open("realTimeWarning.gpt",ios::app);
	int flag=0;
	while((dataHdrRead->status == DAS_START) && (dataTabRead[recNumRead].flag &BufReady) == 0)
	{
		usleep(2000);
		if(flag==0)
		{
			cout<<"Waiting"<<endl;
			flag=1;
		}
	}
	if(flag==1)
		cout<<"Ready"<<endl;
		
	if(dataHdrRead->status != DAS_START)
	{
		if ((dataTabRead[recNumRead].flag & BufReady) == 0)
		{
			cout<<"DAS not in START mode!!"<<endl;
			exit(0);
		}
	}
	currentReadBlock = dataTabRead[recNumRead].seqnum;
	if(debug)
	{
		cout<<endl<<"recNum "<<recNumRead<<endl;
		cout<<"dataBuffer->cur_rec "<<dataBufferRead->cur_rec<<endl;
		cout<<"MaxDataBuf "<<MaxDataBuf<<endl;
		cout<<"dataBuffer->cur_block "<<dataBufferRead->cur_block<<endl;
		cout<<"currentReadBlock "<<currentReadBlock<<endl;
		cout<<"dataBuffer->maxblocks "<<dataBufferRead->maxblocks<<endl<<endl;
	}
		
	if(dataBufferRead->cur_block - currentReadBlock >=dataBufferRead->maxblocks-1)
	{
		warnFile<<"recNum = "<<recNumRead<<", Reading Sequence: "<<currentReadBlock<<", Collect's Sequence: "<<(dataBufferRead->cur_block-1)<<endl;
		warnFile<<"Processing lagged behind..."<<endl;
			
		cout<<"recNum = "<<recNumRead<<", Reading Sequence: "<<currentReadBlock<<", Collect's Sequence: "<<(dataBufferRead->cur_block-1)<<"\nRealiging...\n";
		recNumRead = (dataBufferRead->cur_rec-1-2+MaxDataBuf)%MaxDataBuf;
		currentReadBlock = dataTabRead[(recNumRead)].seqnum;
	}
		
	memcpy(rawData, dataBufferRead->buf+dataTabRead[recNumRead].rec*(dataBufferRead->blocksize)+DataOff, dataBufferRead->blocksize-DataOff);
  	recNumRead=(recNumRead+1)%MaxDataBuf;
  	
	warnFile.close();
}*/
