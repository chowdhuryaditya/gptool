#define _FILE_OFFSET_BITS 64
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <sys/time.h>
#include <sys/timeb.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <string.h>
#include <fcntl.h>
#include <math.h>

#include "subBand_gwb.h"

int shmID;
CodisBuffer *codisBfr;

void connectSHM()
{
        shmID=shmget(CodisBufferKey,sizeof(CodisBuffer),SHM_RDONLY);
        if(shmID<0)
        {
                fprintf(stderr,"\nError in attaching to Codis SHM\n");
                exit(-1);
        }
        codisBfr=(CodisBuffer*)shmat(shmID,NULL,0);
}

int main(int argc, char *argv[])
{
	if(argc!=5)
	{
		printf("\nInvalid number of parameters. Use ./<executable> <numSubBands> <intg> <timeObs (s)> <OutputFile>\n\n");
		exit(-1);
	}
	
	int numSubBands=atoi(argv[1]), intg=atoi(argv[2]), timeObs=atoi(argv[3]),numBlocks,curPos,curRec=0,curBlock,timeStampFlag=1,doneBlocks=0,i,j,numSamp=0,numAvgSamp=0,writeBlockNum=0;
	double blockInc;
	FILE *outFile, *ftstamp;
	char fileName[200],timestamp[200];
	
	short holdBuffer[CodisBufferNum];
	double avgBuffer[numSubBands];
	short outputBuffer[outputBufferSize];
        
	sprintf(fileName,"%s.dat",argv[4]);
	sprintf(timestamp,"%s.timestamp",argv[4]);
	ftstamp=fopen(timestamp,"w");
	if (ftstamp==NULL)
	{	
		printf("Cannot open timestamp.\n");
		exit(-1);
	}
	
	outFile=fopen(fileName,"wb");
	if(outFile==NULL)
	{
		printf("\nUnable to open file\n");
		exit(-1);
	}
	
	connectSHM();
	
	blockInc=codisBfr->blockInc;
	numBlocks=(int)ceil(timeObs/blockInc);
	printf("\nNumber of subbands is %d. Integration is %d. Time of observation is %d s. Number of Codis Blocks is %d. Block Increment is %lf s. \n NUMBER OF CODIS BLOCKS PER WRITE BLOCK IS %d. \nTIME PER WRITE BLOCK IS %lf s.\n",numSubBands,intg,timeObs,numBlocks,blockInc,outputBufferSize/CodisBufferNum*intg,outputBufferSize/CodisBufferNum*intg*blockInc);
	
	while(codisBfr->codisBlock!=1) usleep(10000);
	curRec=codisBfr->codisRec;
	
	while(doneBlocks<numBlocks)
	{
		while(curRec >= codisBfr->codisRec) usleep (10000);
		curBlock = curRec%CodisDataBlocks;
		while(curRec+CodisDataBlocks <= codisBfr->codisRec ) {
                fprintf(stderr, "\n\n\n%d -- (SHM) buffers lost for CURBLOCK = %d SHM_CODIS_BLOCK = %d\n\n\n",CodisDataBlocks,curBlock,codisBfr->codisBlock);
                curRec +=CodisDataBlocks;         //show buffer loss
                }
                
                printf("\nRead CURREC = %d SHM_CODIS_REC = %d CURBLOCK = %d SHM_CODIS_BLOCK = %d",curRec,codisBfr->codisRec,curBlock,codisBfr->codisBlock);
                memcpy(holdBuffer,codisBfr->codisIntData+curBlock*CodisBufferNum,CodisBufferNum*sizeof(short int));
                
                if(doneBlocks==0)
    		{
       			fprintf(ftstamp,"#Start time and date\n");
                	fprintf(ftstamp,"IST Time: %02d:%02d:%2.9lf\n",(codisBfr->codisTimeStamp[curBlock]).tm_hour,(codisBfr->codisTimeStamp[curBlock]).tm_min,(double)((codisBfr->codisTimeStamp[curBlock]).tm_sec)+codisBfr->fracTimeStamp[curBlock]);
	               	fprintf(ftstamp,"Date: %02d:%02d:%d\n",(codisBfr->codisTimeStamp[curBlock]).tm_mday,(codisBfr->codisTimeStamp[curBlock]).tm_mon+1,(codisBfr->codisTimeStamp[curBlock]).tm_year+1900);
        	       	fprintf(ftstamp,"#Start ACQ SEQ NO = %d\n",curRec);
			fclose(ftstamp);
       		}	//for first blocks create the timestamp file and close it
                
                for(i=0;i<CodisBufferNum;i+=numSubBands)
                {		
			for(j=i;j<i+numSubBands;j++)
			{
				avgBuffer[j-i]+=(double)holdBuffer[j];
			}
			numSamp++;
			if(numSamp%intg==0)
			{
				for(j=0;j<numSubBands;j++)
				{
					outputBuffer[numAvgSamp*numSubBands+j]=(short)(avgBuffer[j]/intg);
					avgBuffer[j]=0;
				}
				numAvgSamp++;
				if(numAvgSamp*numSubBands==outputBufferSize)
				{
					if(fwrite(outputBuffer,sizeof(short int),outputBufferSize,outFile)!=outputBufferSize)
						perror("WRITE");
					
					writeBlockNum++;
					printf("\nWRITTEN BLOCK NUMBER %d TO FILE\n",writeBlockNum);
					
					numAvgSamp=0;
				}
			}
                }
                
                doneBlocks++;
		curRec++;
	}
	if(numAvgSamp!=0)
	{
		if(fwrite(outputBuffer,sizeof(short int),numAvgSamp*numSubBands,outFile)!=numAvgSamp*numSubBands)
			perror("WRITE");
		printf("\nWRITTEN LAST FILE BLOCK\n");
	}
	fclose(outFile);	
}

