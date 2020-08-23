#include "SHM.h"




# include <stdlib.h>
# include <iostream>
# include <fstream>
# include <string>
# include <string.h>
# include <unistd.h>
# include <stdio.h>
#include <sstream>
# include <sys/shm.h>
#include <time.h>
#include <signal.h>
# define BUFSIZE 2147483648
using namespace std;

int main(int argc, char *argv[])
{
	int arg = 1;
	long int shmsize;
	char* filepath;
	int nchan=2048*2;
	float sampling=2*0.08192e-3;
	int nSHMBuf;
	int doFRB=0;
	unsigned short int* rawdata;
	unsigned char* rawdataChar;
	int counter=0;
	if(argc <= 1)
  	{
    		cout<<"Not enough arguments."<<endl;
    		exit(0);
  	}
  	while(arg < argc)
  	{
  		arg++;
  	}
  	arg=1;
  	while(arg < argc)
  	{
		cout<<argv[arg]<<endl;
    		if(argv[arg][0]=='-')
    		{
        		   
      			switch(argv[arg][1])
     			{
				cout<<argv[arg]<<endl;
        			case 'd':        			
        			{          
          				if(string(argv[arg]) == "-doFRB")
					{
						doFRB=1;
						arg++;
					}
        			}
        			break;
        			case 'f':
        			case 'F':
        			{          
          				filepath = argv[arg+1];          				
          				arg+=2;
        			}
        			break;
				
			}
		}
	}
	ifstream datafile;
	cout<<"Filepath:"<<filepath<<endl;
	datafile.open(filepath,ios::binary);	
	datafile.seekg(0,ios::end);
	long filesize=datafile.tellg();	
	cout<<"Filesize:"<<filesize<<endl;
	datafile.seekg(0,ios::beg);
	Correlator corrSHM(nchan,sampling);
	int nfilebuf=1;
	if(!doFRB)
	{
		nfilebuf=5;
		shmsize = corrSHM.initializeWriteSHM();
		cout<<shmsize<<endl;
		nfilebuf=int(filesize/BUFSIZE);
		cout<<"nfilebuf"<<nfilebuf<<endl;
		if(nfilebuf>20)
			nfilebuf=20;
		if(nfilebuf<1)
			nfilebuf=1;
		cout<<"nfilebuf"<<nfilebuf<<endl;
		nSHMBuf=int(BUFSIZE/(shmsize));
		cout<<"Number of shared memory blocks in buffer: "<<nSHMBuf<<endl;
		cout<<"Buffering "<<(long)nfilebuf*nSHMBuf*shmsize*sampling/float(2*nchan)<<" seconds of data from file"<<endl;
		cout<<"Time per buffer: "<<(long)shmsize*sampling/float(2*nchan)<<" seconds"<<endl;
		rawdataChar=new unsigned char[(long)nfilebuf*nSHMBuf*shmsize];
		rawdata=new unsigned short int[(long)nfilebuf*nSHMBuf*shmsize];
		datafile.read((char *)rawdataChar,(long)nfilebuf*nSHMBuf*shmsize/2);
		datafile.close();
		unsigned short int *ptrRawData;
		unsigned char *ptrRawDataChar;
		ptrRawData=rawdata;
		ptrRawDataChar=rawdataChar;
		for(long int i=0;i<(long)nfilebuf*nSHMBuf*shmsize;i++,ptrRawData++,ptrRawDataChar++)
		{

			*ptrRawData = *ptrRawDataChar;
			//cout<<(*ptrRawData)<<","<<(*ptrRawDataChar)<<endl;
			//cout<<i<<",";
		}
		delete[] rawdataChar;
		cout<<"Buffering done.."<<endl;
		cout<<"Number of shared memory blocks in buffer: "<<nSHMBuf<<endl;
	}
	else
	{
		corrSHM.initializeWriteSHM_FRB(1);
		shmsize=DataSize;
		nfilebuf=int(filesize/(MaxDataBlocks*shmsize));
		cout<<"nfilebuf"<<nfilebuf<<endl;
		if(nfilebuf>5)
			nfilebuf=5;
		if(nfilebuf<1)
			nfilebuf=1;
		nSHMBuf=MaxDataBlocks;
		cout<<"Number of shared memory blocks in buffer: "<<nSHMBuf<<endl;
		cout<<"Size of SHM: "<<shmsize<<endl;
		cout<<"Buffering "<<(long)nfilebuf*nSHMBuf*(long)shmsize*sampling/float(nchan)<<" seconds of data from file"<<endl;
		rawdataChar=new unsigned char[(long)nfilebuf*nSHMBuf*shmsize];
		datafile.read((char *)rawdataChar,(long)nfilebuf*nSHMBuf*shmsize);
		cout<<"Buffering done.."<<endl;
		cout<<"Number of shared memory blocks in buffer: "<<nSHMBuf<<endl;
	}
	
	clock_t init,start, end;
     	double cpu_time_used;
	double shmtime=shmsize*sampling/float(nchan);
	if(!doFRB)
		shmtime/=2;
	init=clock();
	while(1)
	{
		
     
		start = clock();
 
    

		if(!doFRB)
		{
			cout<<"Copying file buffer to SHM : "<<counter<<endl;
			cout<<"Time: "<<((double)(clock()-init))/CLOCKS_PER_SEC<<" s"<<endl;
			corrSHM.writeToSHM(rawdata+counter*shmsize/2);
			cout<<"Counter: "<<counter<<endl;

		}
		else
		{
			cout<<"Copying file buffer to FRB SHM : "<<counter<<endl;
			cout<<"Time: "<<((double)(clock()-init))/CLOCKS_PER_SEC<<" s"<<endl;
			corrSHM.writeToSHM_FRB(rawdataChar+counter*shmsize);
		}
		counter=(counter+1)%(nSHMBuf*nfilebuf);
		do
		{
			end = clock();
			cpu_time_used = ((double) (end - start)) / CLOCKS_PER_SEC;
		}while(cpu_time_used<shmtime);
	}

}
