
#include"Information.h"
#include <algorithm>
using namespace std;
//implementation of Information class methods

/*******************************************************************
*FUNCTION: double Information::stringToDouble(const std::string& s)
*string& s		: string to convert from
*returns *double x 	: the converted double
*******************************************************************/
double Information::stringToDouble(const std::string& s)
{
	istringstream i(s);
	double x;
	if (!(i >> x))
		return 0;
	return x;
 }

/*
void Information::reformatGptoolInputFile_old()
{
	system("cp gptool.in gptool.in.oldver");
	char* fileparstr = "gptool.in";
	string line;
	string lineTimeFlags,lineChanFlags,linefullDM;
	int k=1,CompleteFlag = 0;
	ifstream filepar(fileparstr,ios::in);
	stringstream fileOutput;
	fileOutput<<"#*#*#gptool input file v1.5#*#*#"<<endl;
	fileOutput<<"-------------------------------------------------"<<endl;

	while(getline(filepar,line))
	{
*/
		/*******************************************************************
		 *tempStr stores the current parameter read as string 
		 *This is then converted to the required data type of the parameters
		 *according to the line number. 
	 	 *******************************************************************/
/*		int c=0;
		int e=0;
	  	string tempStr;
		while(line[e]!=':' && line[e]!='\0')
		  	e++;		
		while(line[c]!=' ' && line[c]!='\0' && line[c]!='\t' && c<e)
		{
		  	tempStr+=line[c];
		  	c++;
		} 
		if(k<=35 || (k>36 && k<43) || (k>43 && k<48))
			fileOutput<<line<<endl;
		
		if(k==18)
			fileOutput<<"-1\t\t: Number of bins in folded profile (-1 for native resolution)"<<endl;
		if(k==36)
			lineChanFlags=line;
		if(k==43)
			lineTimeFlags=line;
		if(k==48)
			linefullDM=line;
		k++;
	}
	fileOutput<<"0\t\t: Replace by median values (0-> Ignore flagged samples, 1-> Replace flagged samples by window median)"<<endl;
	fileOutput<<"-------------------------------------------------"<<endl;
	fileOutput<<"#****I/O options****#"<<endl;
	fileOutput<<lineChanFlags<<endl;
	fileOutput<<lineTimeFlags<<endl;
	fileOutput<<"0\t\t: write out filtered 2D raw data (0-> no, 1-> yes)"<<endl;
	fileOutput<<linefullDM<<endl;
	filepar.close();

	ofstream newInFile(fileparstr,ios::out);
	newInFile<<fileOutput.str().c_str();
	newInFile.close();
}	
*/
/*******************************************************************
*FUNCTION: void Information::reformatGptoolInputFile()
*Checks if the gptool.in file is latest version or not.
*returns 1 if latest version otherwise returns 0.
*******************************************************************/
void Information::reformatGptoolInputFile()
{
	system("cp gptool.in gptool.in.oldver");
	char* fileparstr = "gptool.in";
	string line;
	string lineTimeFlags,lineChanFlags,linefullDM;
	int k=1,CompleteFlag = 0;
	ifstream filepar(fileparstr,ios::in);
	stringstream fileOutput;
	fileOutput<<"#*#*#gptool input file v2.0#*#*#"<<endl;
	fileOutput<<"-------------------------------------------------"<<endl;
	while(getline(filepar,line))
	{
		if((k>2 && k<56))
			fileOutput<<line<<endl;
		
		k++;
	}
	filepar.close();
	fileOutput<<"-------------------------------------------------"<<endl;
	fileOutput<<"#****manual flagging options****#"<<endl;
	fileOutput<<"0		: Number of bad channel blocks"<<endl;
	fileOutput<<"List: 		#in next line, example: [200,400],[1200,1400]"<<endl;
	fileOutput<<""<<endl;	

	ofstream newInFile(fileparstr,ios::out);
	newInFile<<fileOutput.str().c_str();
	newInFile.close();
}	
/*******************************************************************
*FUNCTION: short int Information::checkGptoolInputFileVersion()
*Checks if the gptool.in file is latest version or not.
*returns 1 if latest version otherwise returns 0.
*******************************************************************/
short int Information::checkGptoolInputFileVersion()
{
	char* fileparstr = "gptool.in";
	string firstline;
	string newfirstline="#*#*#gptool input file v2.0#*#*#";
	int k=1,CompleteFlag = 0;
	ifstream filepar(fileparstr,ios::in);
	if(!filepar.is_open())
	{
		cout<<"gptool.in not found!"<<endl;
	 	cout<<"A sample gptool.in file has been written to the current directory."<<endl;
		writeWpmonIn();
	  	exit(0);
	}
	getline(filepar,firstline);
	if(firstline.compare(newfirstline)==0)
		return 1;
	else
		return 0;

}

/*******************************************************************
*FUNCTION: void Information::readGptoolInputFile()
*Reads data from gptool.in file.
*******************************************************************/
void Information::readGptoolInputFile()
{
	  char* fileparstr = "gptool.in";
	  string line;
	  int k=1,CompleteFlag = 0;
	  ifstream filepar(fileparstr,ios::in);
	  if(!filepar.is_open())
	  {
	  	cout<<"gptool.in not found!"<<endl;
	 	cout<<"A sample gptool.in file has been written to the current directory."<<endl;
		writeWpmonIn();
	  	exit(0);
	  }

	  while(getline(filepar,line))
	  {
		/*******************************************************************
		 *tempStr stores the current parameter read as string 
		 *This is then converted to the required data type of the parameters
		 *according to the line number. 
	 	 *******************************************************************/
		int c=0;
		int e=0;
	  	string tempStr;
		while(line[e]!=':' && line[e]!='\0' && line[e]!='\n')
		  	e++;		
		while(line[c]!=' ' && line[c]!='\0' && line[c]!='\t' && c<e)
		{
		  	tempStr+=line[c];
		  	c++;
		} 
		switch(k)
	    	{
	    		
	      		case 1:
				case 2:
				case 3:
	      			break;
	      		case 4:
	      		{
					modeOperation=tempStr;
					break;
	      		}
				case 5:
	      		{
					doPolarMode=char(int(stringToDouble(tempStr)));
					if(doPolarMode)
						noOfPol=4;
					else
						noOfPol=1;
					break;
	      		}			
				case 6:
	      		{
	       			sampleSizeBytes=int(stringToDouble(tempStr));
					break;
	      		}
				case 7:
				case 8:
					break;
				case 9:
				{
					lowestFrequency=stringToDouble(tempStr);	
					break;
			  	} 
				case 10:
			  	{
					bandwidth=stringToDouble(tempStr);  
					break;
			  	}
				case 11:
			  	{
			  			int temp=int(stringToDouble(tempStr));
			  			if(temp==-1)
			  				sidebandFlag=0;
			  			else if(temp==1)
			  				sidebandFlag=1;
			  			else
			  				sidebandFlag=5; //error case
					break;
			  	}
				case 12:
			  	{
					noOfChannels=int(stringToDouble(tempStr));
					break;
			  	}
				case 13:
			  	{
					samplingInterval=stringToDouble(tempStr)/1000.0; //Converting milliseconds to seconds
					break;
			  	}
				case 14:
				case 15:
					break;
				case 16:
			  	{
					pulsarName=tempStr;
					break;
				}			
	      		case 17:
	      		{
	       			periodInMs=stringToDouble(tempStr);
	       			if(periodInMs==-1)
	       				doFixedPeriodFolding=0;
	       			else
	       				doFixedPeriodFolding=1;
				break;
	      		}
	      		case 18:
	      		{
	       			dispersionMeasure=stringToDouble(tempStr);
				break;
	      		}
	      		case 19:
				case 20:
					break; 
				case 21:
	      		{				
	       			periodInSamples=(int)stringToDouble(tempStr);
					break;
	      		}
				case 22:
	      		{				
	       			profileOffset=stringToDouble(tempStr);
				break;
	      		}
				case 23:
	      		{
	      			nCoeffPolyco=int(stringToDouble(tempStr));
	      			break;	      			
	      		}
	      		case 24:
	      		{
	      			spanPolyco=int(stringToDouble(tempStr));
	      			break;
	      		}
				case 25:
	      		{
	      			maxHA=int(stringToDouble(tempStr));
	      			break;
	      		}
				case 26:
				case 27:
					break;
				case 28:
				{
					polarChanToDisplay=char(int(stringToDouble(tempStr)));
				}
				case 29:
				{
					blockSizeSec=stringToDouble(tempStr);
					break;
				}
				case 30:      		
	      		{
	      			doManualMode=char(int(stringToDouble(tempStr)));        		
	       			break;
	      		}
	      		case 31:
	      		{
	      			doWindowDelay=char(int(stringToDouble(tempStr))); 
	       			break;
	      		}
				case 32:
				case 33:
					break;
	      		case 34:
	      		{
				startChannel=int(stringToDouble(tempStr));
	      			break;
	      		}
	      		case 35:
	      		{
	       			stopChannel=noOfChannels-int(stringToDouble(tempStr));
	      			break;        
	      		}	      		
	      		case 36:
	      		{
	       			doChanFlag=char(int(stringToDouble(tempStr)));
				break;
	      		}
	      		case 37:
	      		{
	       			bandshapeToUse=char(int(stringToDouble(tempStr)))+1;			//Plus one done to skip the now obsolete option of using mean bandshape to filter.
					break;
	      		}
	      		case 38:
	      		{
	       			chanCutOffToRMS=stringToDouble(tempStr);
	       			break;
	      		}	      		
				case 39:
				case 40:
				break;
	      		case 41:
	     		{
					doTimeFlag=char(int(stringToDouble(tempStr)));
					break;
	      		}
				case 42:
	     		{
				doUseNormalizedData=char(int(stringToDouble(tempStr)));
				break;
	      		}
	      		case 43:
	      		{
				timeFlagAlgo=char(int(stringToDouble(tempStr)));
	       			break;
	      		}	      		
	      		case 44:
	      		{
	       			timeCutOffToRMS=stringToDouble(tempStr);
				break;
	      		}   
				case 45:
				case 46:
					break;   
				case 47:
				{
					smoothingWindowLength=int(stringToDouble(tempStr));
					break;
				}
				case 48:
				{
					normalizationProcedure=char(int(stringToDouble(tempStr)));
					break;
				} 	
				case 49:
				{
					doReplaceByMean=char(int(stringToDouble(tempStr)));
					break;
				} 	
				case 50:
				case 51:
					break; 
				case 52:
	      		{
	      			doWriteChanFlags=char(int(stringToDouble(tempStr)));				
	       			break;
	      		}
				case 53:
	      		{
	      			doWriteTimeFlags=char(int(stringToDouble(tempStr)));
	       			break;
	      		}
				case 54:
	      		{
	      			doWriteFiltered2D=char(int(stringToDouble(tempStr)));
	       			break;
	      		}
	      		case 55:      		
	      		{
	      			doWriteFullDM=char(int(stringToDouble(tempStr)));        		
	       			break;
	      		}
			case 56:
			case 57:
				break;
			case 58:      		
	      		{
	      			nBadChanBlocks=char(int(stringToDouble(tempStr)));  
	       			break;
	      		}
			case 59:
				break;
			case 60:      		
	      		{
	      			parseManFlagList(tempStr);        		
	       			break;
	      		}
	      			      		
	      		default:
	      		{
				CompleteFlag = 1;
				break;
			}        	
		}
	    	k++;
	    	if(CompleteFlag == 1)
	      		break;
	}
	//Manually filling obsolete options (removed from .in file)
	smoothFlagWindowLength=-1;
	refFrequency=0;
	flagOrder=1;
	chanFlagAlgo=2;
	doMultiPointFilter=0;
	if(flagOrder==2 && doUseNormalizedData) //Channel filtering cannot be done after time filtering if data is normalized
	{
		cout<<"Error in line of gptool.in"<<endl;
		cout<<"Channel filtering cannot be done after time filtering if data is normalized."<<endl;
		cout<<"Recommended option is to do channel filtering first."<<endl;
		exit(0);
	}
	
//	calculateCutoff();	
	errorChecks();
}
void Information::parseManFlagList(std::string& s)
{
	int p=0;
	int i=0;
	int p1,p2,p3;
	int slen=s.length();
	badChanBlocks=new int[nBadChanBlocks*2];
	while(p<slen)
	{
		if(i>=nBadChanBlocks)
		{
			cout<<"Error in line 60 of gptool.in"<<endl;
			cout<<"Expected exactly "<<nBadChanBlocks<<" list of bad sub-bands"<<endl;
			exit(0);
		}
		if(s[p]=='[')
			p1=p;
		else
		{
			cout<<"Error in line 60 of gptool.in"<<endl;
			cout<<"Expected [ to mark the start of a bad sub-band"<<endl;
			exit(0);
		}
		while(s[++p]!=',' && s[p]!='[' && s[p]!=']' && p<slen);
		if(s[p]==',')
			p2=p;
		else
		{
			cout<<"Error in line 60 of gptool.in"<<endl;
			cout<<"Expected ,"<<endl;
			exit(0);
		}
		while(s[++p]!=']' && p<slen);
		if(s[p]==']')
			p3=p;
		else
		{
			cout<<"Error in line 60 of gptool.in"<<endl;
			cout<<"Expected ] to mark the start of a bad sub-band"<<endl;
			exit(0);
		}
		if(++p<slen && s[p]!=',')
		{
			cout<<"Error in line 60 of gptool.in"<<endl;
			cout<<"Expected , between list of sub-bands"<<endl;
			exit(0);
		}
		p++;
		badChanBlocks[i*2]=int(stringToDouble(s.substr(p1+1,p2-p1-1)));
		badChanBlocks[i*2+1]=int(stringToDouble(s.substr(p2+1,p3-p2-1)));
		i++;
		
	}
	if(i<nBadChanBlocks)
	{
		cout<<"Error in line 60 of gptool.in"<<endl;
		cout<<"Expected exactly "<<nBadChanBlocks<<" list of bad sub-bands"<<endl;
		exit(0);
	}
}
void Information::fillParams()
{	
	if(doRunFilteredMode)	//rerun of .gpt output file. No filtering/
	{
		doChanFlag=0;
		doTimeFlag=0;
		bandshapeToUse=1;
		doUseNormalizedData=0;
		doReplaceByMean=0;
		doWriteFiltered2D=0;
		
	}
	if(!doFilteringOnly)
	{
		if(!doFixedPeriodFolding)	//Perform actions to use polyco based folding
		{
			genMJDObs();
			if(doUseTempo2)
				genPolycoTempo2();
			else
	 			genPolycoTempo1();
	 		loadPolyco();
	 		polycoRowIndex= (int)(((MJDObs-polycoTable[0])*1440)+spanPolyco/2.0)/spanPolyco;
			double freq=polycoTable[polycoRowIndex*(3+nCoeffPolyco)+2]*60.0;
			double dt=(MJDObs-polycoTable[polycoRowIndex*(3+nCoeffPolyco)])*1440;
			/***********************************************************************************************
			*The following while loop is due to an ill understood issue with the polyco.dat file (generated by
			*tempo 1) where a particular set(s) in the file has a dataspan less than that specified by spanPolyco
			*(usually half of it, for example if spanPolyco is 60mins, there maybe a 30mins set in the file. The
			*reason is not yet understood (8th May '16)
			************************************************************************************************/
			while(dt>spanPolyco/2.0)
			{
				polycoRowIndex++;
				dt=(MJDObs-polycoTable[polycoRowIndex*(3+nCoeffPolyco)])*1440;
			}
			/***********************************************************************************************
			*The next loop is for the yet unecountered case of set(s) having more span than spanPolyco
			************************************************************************************************/
			while(dt<-spanPolyco/2.0)
			{
				polycoRowIndex--;
				dt=(MJDObs-polycoTable[polycoRowIndex*(3+nCoeffPolyco)])*1440;
			}
			
			for(int i=1;i<nCoeffPolyco;i++)  	
			{
	    			freq += i*polycoTable[polycoRowIndex*(3+nCoeffPolyco)+3+i] * pow(dt,i-1);
			}
	 		periodInMs = (1000.0*60.0)/freq;
			
	 	} 	

		if(periodInSamples==-1)
			periodInSamples=(int)(periodInMs/(samplingInterval*1000.0)+0.5);	//Calculating the period in number of samples
		if(dispersionMeasure<0) //Get DM from psrcat or polyco.dat
			autoDM();
	}

	if(blockSizeSec==0)						//Setting blocksize equal to pulsar period
		blockSizeSec=periodInMs/1000.0;
	blockSizeSamples=blockSizeSec/samplingInterval;
	startBlockIndex=floor(startTime/blockSizeSec);
	if(!doFilteringOnly)
		writeInfFile();
	if(doReadFromFile)
	{
		string temp=filepath;
		int slashpos=temp.find_last_of("/");
		filename=temp.substr(slashpos+1);
	}
	if(doWriteFiltered2D)
	{
		if(doReadFromFile)
		{
			stringstream hdrCpyCommand;
			if(outputfilepath.empty())
				outputfilepath=filepath;
			else
			{
				stringstream filenameStream;
				filenameStream<<outputfilepath<<"/"<<filename;	
				outputfilepath=filenameStream.str().c_str();
			}
			hdrCpyCommand<<"cp "<<filepath<<".hdr "<<outputfilepath<<".gpt.hdr"<<endl;
			system(hdrCpyCommand.str().c_str());
		}
		else
		{
			stringstream filenameStream;
			filenameStream<<outputfilepath;	
			outputfilepath=filenameStream.str().c_str();
		}
	}
}
void Information::errorChecks()
{
	char erFlag=0;
	if(doReadFromFile==1)
	{
		ifstream testExistance;
		testExistance.open(filepath);
		if(!testExistance.is_open())
  		{
    			cout<<"No file with name: "<< filepath<<endl;
    			erFlag=1;
 		}
	}
	if(doPolarMode != 0 && doPolarMode != 1)
	{
		cout<<"Error in line 5 of gptool.in:"<<endl<<"Polarization mode must be either 0 for for Intensity or 1 for full stokes"<<endl;
		erFlag=1;
	}
	if(doPolarMode && polarChanToDisplay>3)
	{
		cout<<"Error in line 28 of gptool.in:"<<endl;
		cout<<"Polarization channel to display must be between 0 and 3 OR -1 to display all four channels"<<endl;
		erFlag=1;
	}
	if(sampleSizeBytes !=1 && sampleSizeBytes !=2 && sampleSizeBytes != 4)
	{
		cout<<"Error in line 6 of gptool.in:"<<endl<<"The tool can only process 1 or 2 byte integer or 4 byte float."<<endl;
		erFlag=1;
	}
	if(sidebandFlag !=0 && sidebandFlag != 1)
	{
		cout<<"Error in line 11 of gptool.in:"<<endl<<"-1 for decreasing channel ordering, +1 for increasing."<<endl;
		erFlag=1;
	}
	if(!doFixedPeriodFolding || dispersionMeasure<0)
	{
		getPsrcatdbPath();
		checkPulsarName();
	}
	if(periodInSamples==0)
	{
		cout<<"Error in 21 of gptool.in:"<<endl<<"Number of bins in folded profile must be positive definite."<<endl;
		erFlag=1;
	}
	if(profileOffset >1 || profileOffset<0)
	{
		cout<<"Error in 22 of gptool.in:"<<endl<<"Pulsar phase offset must be between 0 and 1"<<endl;
		erFlag=1;
	}
	if(doManualMode !=0 && doManualMode !=1)
	{
		cout<<"Error in line 30 of  gptool.in:"<<endl<<"0 for automatic update and 1 for manual update."<<endl;
		erFlag=1;
	}
	if(startChannel>noOfChannels)
	{
		cout<<"Error in line 34 of  gptool.in:"<<endl<<"Channels to skip cannot be greater than number of channels"<<endl;
		erFlag=1;
	}
	
	if(stopChannel<startChannel)
	{
		cout<<"Error in line 35 of  gptool.in:"<<endl<<"The sum of channels to skip in the beginning and end exceeds the number of channels"<<endl;
		erFlag=1;
	}
	
	
	if(doChanFlag !=0 && doChanFlag !=1)
	{
		cout<<"Error in line 36 of  gptool.in:"<<endl<<"0 for no frequency domain filtering and 1 for filtering on."<<endl;
		erFlag=1;
	}
	
	if(bandshapeToUse !=2 && bandshapeToUse !=3 && bandshapeToUse !=4)
	{
		cout<<"Error in line 37 of  gptool.in:"<<endl<<"1 for normalized bandshape; 2 for mean to rms bandshape and 3 for both."<<endl;
		erFlag=1;
	}
	
	if(chanCutOffToRMS <=0)
	{
		cout<<"Error in line 38 of  gptool.in:"<<endl<<"Cutoff must be positive."<<endl;
		erFlag=1;
	}
	
	if(doWriteChanFlags!=0 && doWriteChanFlags!=1)
	{
		cout<<"Error in line 52 of  gptool.in:"<<endl<<"1 to write channel flags, 0 otherwise."<<endl;
		erFlag=1;
	}
	
	if(doTimeFlag !=0 && doTimeFlag !=1)
	{
		cout<<"Error in line 41 of  gptool.in:"<<endl<<"0 for no time domain filtering and 1 for filtering on."<<endl;
		erFlag=1;
	}
	
	
	if(doUseNormalizedData!=0 && doUseNormalizedData!=1)
	{
		cout<<"Error in line 42 of  gptool.in:"<<endl<<"1 to normalize data, 0 otherwise."<<endl;
		erFlag=1;
	}
	
	if(timeFlagAlgo!=2 && timeFlagAlgo!=1)
	{
		cout<<"Error in line 43 of  gptool.in:"<<endl<<"1 to use histogram based, 2 to use MAD basesd."<<endl;
		erFlag=1;
	}
	
	if(timeCutOffToRMS <=0)
	{
		cout<<"Error in line 44 of  gptool.in:"<<endl<<"Cutoff must be positive."<<endl;
		erFlag=1;
	}
	
	if(doWriteTimeFlags!=0 && doWriteTimeFlags!=1)
	{
		cout<<"Error in line 53 of  gptool.in:"<<endl<<"1 to write time flags, 0 otherwise."<<endl;
		erFlag=1;
	}
	
	if((doUseNormalizedData==1 || (doChanFlag==1 && bandshapeToUse==2)) && normalizationProcedure ==1 && (smoothingWindowLength<=0 || smoothingWindowLength>=noOfChannels))
	{
		cout<<"Error in line 47 of  gptool.in:"<<endl<<"Smooth window length must be positive and less than the number of channels."<<endl;
		erFlag=1;
	}
	
	if(normalizationProcedure!=2 && normalizationProcedure!=1)
	{
		cout<<"Error in line 48 of  gptool.in:"<<endl<<"1 to use cumulative smooth bandshape, 2 to use externally supplied bandshape.dat."<<endl;
		erFlag=1;
	}
	if(doReplaceByMean!=0 && doReplaceByMean!=1 &&  doReplaceByMean!=2)
	{
		cout<<"Error in line 49 of  gptool.in:"<<endl<<"0 to replace by zeros, 1 to replace flagged points by modal (median) value, 2 to replace by smooth bandshape"<<endl;
		erFlag=1;
	}
	if(doReplaceByMean==1 && doUseNormalizedData!=1)
	{
		cout<<"Invalid combination of choices in line 42 and 49."<<endl<<"Data must be normalized to replace by modal (median) values.";
		erFlag=1;
	}
	if(doReplaceByMean==1 && doTimeFlag!=1)
	{
		cout<<"Invalid combination of choices in line 41 and 49."<<endl<<"Time filtering must be on to replace by modal (median) values.";
		erFlag=1;
	}
	if(normalizationProcedure==2)
	{
		ifstream testExistance;
		testExistance.open("bandshape.dat");
		if(!testExistance.is_open())
  		{
    			cout<<"No bandshape.dat file found."<<endl;
    			erFlag=1;
 		}
	}
	if(doWriteFullDM!=0 && doWriteFullDM!=1)
	{
		cout<<"Error in line 55 of  gptool.in:"<<endl<<"1 to write dedispersed time series, 0 otherwise."<<endl;
		erFlag=1;
	}
	if(doWriteFiltered2D!=0 && doWriteFiltered2D!=1)
	{
		cout<<"Error in line 54 of  gptool.in:"<<endl<<"1 to write 2D time-frequency data, 0 otherwise."<<endl;
		erFlag=1;
	}
	if(doWriteFiltered2D && !doTimeFlag)
	{
		cout<<"Filtered 2D data can be written out only if time filtering is turned on. If you want to do frequency only filtering, then you can turn on time filtering and set a very large n-sigma threshold"<<endl;
		erFlag=1;
	}
	if(nBadChanBlocks!=0)
	{
		for(int i=0;i<nBadChanBlocks;i++)	
		{
			if(badChanBlocks[i*2]<0 || badChanBlocks[i*2+1]<0 )
			{
				cout<<"Error in line 60 of gptool.in"<<endl;
				cout<<"In sub-band "<<i+1<<": channel number cannot be less than zero."<<endl;
				erFlag=1;
			}
			if(badChanBlocks[i*2]>noOfChannels || badChanBlocks[i*2+1]>noOfChannels )
			{
				cout<<"Error in line 60 of gptool.in"<<endl;
				cout<<"In sub-band "<<i+1<<": channel number cannot be greater than the number of channels."<<endl;
				erFlag=1;
			}
			if(badChanBlocks[i*2]>=badChanBlocks[i*2+1])
			{
				cout<<"Error in line 60 of gptool.in"<<endl;
				cout<<"In sub-band "<<i+1<<": end channel must be strictly greater than start channel."<<endl;
				erFlag=1;
			}
		}			

	}
	if(erFlag==1)
	{
		exit(0);
	}	
	
}
void Information::calculateCutoff()
{
	float table[91][5]={{1., 0.559349, 0.39004, 0.299751, 0.243506}, {1.1, 0.62054, 0.434484,
   0.334715, 0.272346}, {1.2, 0.68246, 0.479754, 0.370477, 
  0.301926}, {1.3, 0.745061, 0.525809, 0.407005, 0.332225}, {1.4, 
  0.808296, 0.57261, 0.444271, 0.363221}, {1.5, 0.872124, 0.62012, 
  0.482245, 0.39489}, {1.6, 0.936504, 0.668302, 0.520897, 
  0.427211}, {1.7, 1.0014, 0.717119, 0.560199, 0.460161}, {1.8, 
  1.06677, 0.766538, 0.600123, 0.493716}, {1.9, 1.13259, 0.816526, 
  0.64064, 0.527854}, {2., 1.19882, 0.867051, 0.681723, 
  0.562553}, {2.1, 1.26543, 0.918084, 0.723347, 0.59779}, {2.2, 
  1.33241, 0.969594, 0.765486, 0.633543}, {2.3, 1.39971, 1.02156, 
  0.808114, 0.669793}, {2.4, 1.46733, 1.07394, 0.851208, 
  0.706516}, {2.5, 1.53523, 1.12673, 0.894744, 0.743694}, {2.6, 
  1.6034, 1.17989, 0.938702, 0.781306}, {2.7, 1.67182, 1.23341, 
  0.983059, 0.819334}, {2.8, 1.74046, 1.28725, 1.02779, 
  0.857758}, {2.9, 1.80933, 1.34142, 1.07289, 0.896561}, {3., 1.87839,
   1.39587, 1.11832, 0.935725}, {3.1, 1.94763, 1.45061, 1.16408, 
  0.975234}, {3.2, 2.01705, 1.5056, 1.21015, 1.01507}, {3.3, 2.08662, 
  1.56084, 1.2565, 1.05522}, {3.4, 2.15635, 1.61631, 1.30313, 
  1.09567}, {3.5, 2.22621, 1.67199, 1.35002, 1.1364}, {3.6, 2.2962, 
  1.72787, 1.39715, 1.1774}, {3.7, 2.36631, 1.78395, 1.44451, 
  1.21866}, {3.8, 2.43653, 1.8402, 1.4921, 1.26016}, {3.9, 2.50685, 
  1.89662, 1.53989, 1.3019}, {4., 2.57727, 1.9532, 1.58788, 
  1.34386}, {4.1, 2.64777, 2.00992, 1.63605, 1.38602}, {4.2, 2.71836, 
  2.06679, 1.6844, 1.42839}, {4.3, 2.78902, 2.12378, 1.73292, 
  1.47094}, {4.4, 2.85975, 2.18089, 1.78159, 1.51368}, {4.5, 2.93055, 
  2.23812, 1.83041, 1.55659}, {4.6, 3.00141, 2.29545, 1.87936, 
  1.59965}, {4.7, 3.07233, 2.35289, 1.92845, 1.64288}, {4.8, 3.1433, 
  2.41042, 1.97767, 1.68625}, {4.9, 3.21432, 2.46804, 2.027, 
  1.72975}, {5., 3.28538, 2.52573, 2.07644, 1.77339}, {5.1, 3.35648, 
  2.58351, 2.12599, 1.81716}, {5.2, 3.42763, 2.64136, 2.17564, 
  1.86104}, {5.3, 3.4988, 2.69928, 2.22537, 1.90504}, {5.4, 3.57001, 
  2.75726, 2.2752, 1.94914}, {5.5, 3.64126, 2.8153, 2.32511, 
  1.99334}, {5.6, 3.71253, 2.8734, 2.3751, 2.03764}, {5.7, 3.78382, 
  2.93154, 2.42516, 2.08203}, {5.8, 3.85514, 2.98974, 2.47529, 
  2.12651}, {5.9, 3.92648, 3.04798, 2.52549, 2.17106}, {6., 3.99784, 
  3.10627, 2.57575, 2.2157}, {6.1, 4.06923, 3.16459, 2.62606, 
  2.26041}, {6.2, 4.14062, 3.22296, 2.67644, 2.30518}, {6.3, 4.21204, 
  3.28135, 2.72686, 2.35003}, {6.4, 4.28346, 3.33978, 2.77733, 
  2.39493}, {6.5, 4.3549, 3.39824, 2.82785, 2.4399}, {6.6, 4.42635, 
  3.45673, 2.87841, 2.48492}, {6.7, 4.49782, 3.51524, 2.92901, 
  2.52999}, {6.8, 4.56929, 3.57378, 2.97964, 2.57511}, {6.9, 4.64077, 
  3.63233, 3.03032, 2.62028}, {7., 4.71226, 3.69091, 3.08103, 
  2.6655}, {7.1, 4.78375, 3.74951, 3.13176, 2.71076}, {7.2, 4.85525, 
  3.80813, 3.18253, 2.75605}, {7.3, 4.92676, 3.86676, 3.23333, 
  2.80139}, {7.4, 4.99827, 3.92541, 3.28415, 2.84676}, {7.5, 5.06978, 
  3.98408, 3.33499, 2.89216}, {7.6, 5.1413, 4.04275, 3.38586, 
  2.9376}, {7.7, 5.21282, 4.10144, 3.43675, 2.98306}, {7.8, 5.28435, 
  4.16014, 3.48766, 3.02856}, {7.9, 5.35587, 4.21884, 3.53859, 
  3.07408}, {8., 5.4274, 4.27756, 3.58953, 3.11962}, {8.1, 5.49893, 
  4.33628, 3.64049, 3.16519}, {8.2, 5.57046, 4.39502, 3.69147, 
  3.21078}, {8.3, 5.64198, 4.45375, 3.74246, 3.25639}, {8.4, 5.71351, 
  4.5125, 3.79346, 3.30202}, {8.5, 5.78504, 4.57125, 3.84447, 
  3.34767}, {8.6, 5.85657, 4.63, 3.89549, 3.39334}, {8.7, 5.92809, 
  4.68876, 3.94653, 3.43902}, {8.8, 5.99962, 4.74752, 3.99757, 
  3.48472}, {8.9, 6.07114, 4.80628, 4.04862, 3.53043}, {9., 6.14266, 
  4.86504, 4.09968, 3.57615}, {9.1, 6.21418, 4.92381, 4.15075, 
  3.62189}, {9.2, 6.2857, 4.98258, 4.20182, 3.66764}, {9.3, 6.35721, 
  5.04135, 4.2529, 3.7134}, {9.4, 6.42872, 5.10012, 4.30398, 
  3.75916}, {9.5, 6.50023, 5.15889, 4.35507, 3.80494}, {9.6, 6.57174, 
  5.21766, 4.40616, 3.85073}, {9.7, 6.64324, 5.27643, 4.45725, 
  3.89652}, {9.8, 6.71474, 5.3352, 4.50835, 3.94232}, {9.9, 6.78624, 
  5.39397, 4.55945, 3.98813}, {10., 6.85773, 5.45273, 4.61055, 
  4.03394}};
  
  cutoff=new float[5];
  cutoff[0]=timeCutOffToRMS;
  
  int i1,i2;
  i1=floor((cutoff[0]-1)/0.1);
  i2=ceil((cutoff[0]-1)/0.1);
  
  for(int j=1;j<=4;j++)
    	cutoff[j]=table[i1][j]+((table[i2][j]-table[i1][j])/0.1)*(cutoff[0]-table[i1][0]);
  
  
}
/*******************************************************************
*FUNCTION: void Information::genMJDObs()
*Generates Modified Julian Day (MJD) at the start of observation.
*In case of offline data it reads relevent parameters from the header
*file and in case of online data it converts the system time to MJD.
*******************************************************************/
void Information::genMJDObs()
{
	int YYYY,MM,DD,HH,mm,SS,ss;
  	stringstream convertTime;
  	long int nanoseconds;
  	double sec;
  	string command,linehdr;
	stringstream  headerName;
  	if(doReadFromFile==1) //Read from file
    		headerName<<filepath<<".hdr";
  	else //Real Time
  		headerName<<"timestamp.gpt";
    	ifstream headerFile(headerName.str().c_str(),ios::in);
    	if(!headerFile.is_open())
    	{
      		cout<<headerName.str().c_str()<<":header file not found!"<<endl;
      		exit(1);
    	}

    	//Extract date and time information and check for sanity
    	getline(headerFile,linehdr); //Ignore first line
    	getline(headerFile,linehdr); //Get the second line and process it
    	sscanf(linehdr.c_str(),"%*s %*s %d:%d:%lf",&HH,&mm,&sec);
    	getline(headerFile,linehdr);
    	sscanf(linehdr.c_str(),"%*s %d:%d:%d",&DD,&MM,&YYYY);
    	headerFile.close();

    	// Checking the sensibility of the IST Date and time Acquired:
    	if(YYYY < 0 || MM < 1 || MM > 12 || DD < 1 || DD > 31 || HH < 0 || HH > 23 || mm < 0 || mm > 59 || sec < 0.0 || sec >= 60.0 )
    	{
      		cout<<"\n\nERROR ! Awkward or invalid format of IST time and Date in the raw.hdr file.\n";
      		cout<<"\t'raw.hdr' file should STRICTLY contain the IST time and IST date of Observation in this format\n";
      		cout<<"#Start time and date\n";
      		cout<<"IST Time: HH:mm:SS.ssss\n";
      		cout<<"Date: DD:MM:YYYY\n\n";
      		exit(1);
    	}

    	if( (MM==4 || MM==6 || MM==9|| MM==11) && DD>30)
    	{
      		cout<<"\n\nERROR ! Given Month can not have Days > 30.\n";
      		cout<<"\t'raw.hdr' file should STRICTLY contain the IST time and IST date of Observation in this format\n";
      		cout<<"#Start time and date\n";
     		cout<<"IST Time: HH:mm:SS.ssss\n";
      		cout<<"Date: DD:MM:YYYY\n\n";
      		exit(1);
    	}

    	// The following line is to check for whether current year is leap year 
    	// The logic is if(Div_by_4 And (Not_div_by_100 Or Div_by_400)) then Leap year else not.
    	if((YYYY % 4 == 0) && ((YYYY % 100 != 0) || (YYYY % 400 == 0)))
    	{
      		if(MM==2 && DD>29)
      		{
        		cout<<"\n\nERROR ! In the file raw.hdr : Leap-year Month of Feb can not have Days > 29.\n";
        		exit(1);
      		}
    	}
	else
    	if(MM==2 && DD >28)
    	{
     		cout<<"\n\nERROR ! In the file raw.hdr : In non-Leap-year Month of Feb can not have Days > 28.\n";
		exit(1);
    	}

    		
    
    	//Convert IST to UTC
    	convertTime.str("");
    	convertTime<<"TZ=\"UTC\" date \"+%d %m %Y %H %M %S %N\" -d \""<<YYYY<<"-"<<MM<<"-"<<DD<<" "<<HH<<":"<<mm<<":"<<setprecision(12)<<sec<<" IST\""<<"> UTC"<<endl;
    	system(convertTime.str().c_str());
  	

  	//Read UTC File and Get UTC Time
  	ifstream UTCfile("UTC",ios::in);
  	UTCfile>>DD>>MM>>YYYY>>HH>>mm>>SS>>nanoseconds;
  	sec = SS+((double)nanoseconds/1000000000.0);
  	UTCfile.close();
  	system("rm -rf UTC");
  	//Calculate the MJD
  	stringstream calenderToMJD;
  	calenderToMJD<<"cal2mjd "<<YYYY<<" "<<MM<<" "<<DD<<" "<<HH<<" "<<mm<<" "<<setprecision(12)<<sec<<" >calenderToMJDoutput";
  	system(calenderToMJD.str().c_str());
  	ifstream mjd("calenderToMJDoutput",ios::in);
  	string mjdTemp;
  	getline(mjd,mjdTemp);
  	getline(mjd,mjdTemp);
  	sscanf(mjdTemp.c_str(),"%*s %*s %lf",&MJDObs);
  	system("rm -rf calenderToMJDoutput");
  	mjd.close();
}
/*******************************************************************
*FUNCTION: void Information::getPsrcatdbPath()
*Locates the psrcat database file.
*******************************************************************/
void Information::getPsrcatdbPath()
{
	stringstream checkPsrcatCommand;
	checkPsrcatCommand<<"echo $PSRCAT_FILE | awk 'FNR ==1 {print}' > dbpath";
	system(checkPsrcatCommand.str().c_str());
	
	ifstream* path= new ifstream("dbpath",ios::in);
	path->seekg(0,ios::end);
	size_t eof=path->tellg();
	if((int)eof<=2)
	{	
		path->close();
		system("rm -rf dbpath");
		
		stringstream dbPathCommand;
	    	dbPathCommand<<"locate \"psrcat.db\" | awk 'FNR == 1 {print}' > dbpath";
	    	system(dbPathCommand.str().c_str());
		path=new ifstream("dbpath",ios::in);
		path->seekg(0,ios::end);
		eof=path->tellg();
		if((int)eof==0)
		{
			cout<<"No psrcat.db found!"<<endl;
			exit(0);
		}
	}
	psrcatdbPath=new char[(int)eof];
	path->seekg(0,ios::beg);
	path->getline (psrcatdbPath,eof);
	path->close();
	system("rm -rf dbpath");
		
}
/*******************************************************************
*FUNCTION: void Information::checkPulsarName()
*Verify pulsar name with psr cat and see if its valid. 
*In case B name is specified this routine also gets the correspoding
*J name.
*******************************************************************/
void Information::checkPulsarName()
{
	//Generate JName and check if source name is valid
    	stringstream JNameGen;
    	JNameGen<<"psrcat -db_file "<<psrcatdbPath<<" -e "<<pulsarName<<" | grep PSRJ | awk '{print $2}' > JNamePulsar";
    	system(JNameGen.str().c_str());
    	ifstream JName("JNamePulsar",ios::in);
    	JName.seekg(0,ios::end);
    	if(int(JName.tellg())==0)
    	{
        	cout<<"Error in line 14 of gptool.in:"<<endl;
        	cout<<"Incorrect pulsar name."<<endl;
        	exit(1);
    	}
    	JName.seekg(0,ios::beg);
    	JName>>pulsarName;    	
    	JName.close();
    	system("rm -rf JNamePulsar");
}
/*******************************************************************
*FUNCTION: void Information::autoDM()
*Gets DM value from psrcat in case its not specified by the user
*******************************************************************/
void Information::autoDM()
{	
  	stringstream DMGet;
  	if(!doFixedPeriodFolding)
  	{
		ifstream *polyco;
		if(!doUseTempo2)
  			polyco=new ifstream("polyco.dat",ios::in);
		else
			polyco=new ifstream("polyco_new.dat",ios::in);
    		string temp;
    		*polyco>>temp>>temp>>temp>>temp>>dispersionMeasure;
		delete polyco;
    	}
    	else
    	{
    		DMGet<<"psrcat -db_file "<<psrcatdbPath<<" -e "<<pulsarName<<" | grep DM | awk '{print $2}' > DMDMDM";
    		system(DMGet.str().c_str());  	
  		ifstream DMGet("DMDMDM",ios::in);
  		DMGet>>dispersionMeasure;
  		DMGet.close();
  		system("rm -rf DMDMDM");
  	}
}
/*******************************************************************
*FUNCTION: void Information::genPolycoTempo1()
*Fires tempo1 to generate polyco.dat file
*Refer to tempo manual for format of the input file tz.in
*******************************************************************/
void Information::genPolycoTempo1()
{
	//Generate tz.in
	ofstream tz("tz.in",ios::out);
	tz<<"r 12 60 12 325\n\n\n";
	string tempString = pulsarName.substr(1);
	float freqTz;
	if(sidebandFlag == 0)
        	freqTz = (lowestFrequency+bandwidth-(bandwidth/noOfChannels)/2.0);
    	else 
      		freqTz = (lowestFrequency+(bandwidth/noOfChannels)/2.0);
    	tz<<tempString<<setprecision(12)<<" "<<spanPolyco<<" "<<nCoeffPolyco<<" "<<maxHA<<" "<<freqTz<<endl;
   	tz.close();
    	//gets parameter file from psr catalogue
    	stringstream parCommand;
	parCommand<<"psrcat -db_file "<<psrcatdbPath<<" -e "<<pulsarName<<" > parfile_used";
    	system(parCommand.str().c_str());
    	//fire tempo to get polycos    	
    	float MJD1=MJDObs-1.0;
    	float MJD2=MJDObs+1.0;
        stringstream tempoCommand;
        //tempoCommand<<"echo "<<MJD1<<" "<<MJD2<<"| tempo -z -f parfile_used > /dev/null";
	tempoCommand<<"echo "<<setprecision(12)<<MJD1<<" "<<setprecision(12)<<MJD2<<"| tempo -z -f parfile_used> /dev/null";
	system(tempoCommand.str().c_str());   
	fstream *fin;
	if(!doUseTempo2)
		fin=new fstream("polyco.dat", fstream::in);
	else
		fin=new fstream("polyco_new.dat", fstream::in);
	
	fin->seekg(0, ios_base::end);
	if(fin->tellg()==0)
	{		
		tempoCommand.str("");
		tempoCommand.clear();
		tempoCommand<<"echo "<<setprecision(12)<<MJD1<<" "<<setprecision(12)<<MJD2<<"| tempo -z> /dev/null";
		system(tempoCommand.str().c_str());   
	}  
	delete fin;
}

/*******************************************************************
*FUNCTION: void Information::genPolycoTempo2()
*Fires tempo2 to generate polyco.dat file
*Refer to tempo manual for format of the input file tz.in
*******************************************************************/
void Information::genPolycoTempo2()
{
	
	string tempString = pulsarName.substr(1);
	float freqTz;
	if(sidebandFlag == 0)
        freqTz = (lowestFrequency+bandwidth-(bandwidth/noOfChannels)/2.0);
    else 
      	freqTz = (lowestFrequency+(bandwidth/noOfChannels)/2.0);
 
    //gets parameter file from psr catalogue
    stringstream parCommand;
	parCommand<<"psrcat -db_file "<<psrcatdbPath<<" -e "<<pulsarName<<" >"<<pulsarName<<".par";
    system(parCommand.str().c_str());
    //fire tempo to get polycos    	
    float MJD1=MJDObs-1.0;
    float MJD2=MJDObs+1.0;
    stringstream tempoCommand;
	tempoCommand<<"tempo2 -f "<<pulsarName<<".par"<<" -polyco \""<<MJD1<<" "<<MJD2<<" "<<spanPolyco<<" "<<nCoeffPolyco<<" "<<maxHA<<" gmrt "<<freqTz<<"\" -tempo1> /dev/null";
	system(tempoCommand.str().c_str());   
	/**ifstream fin("polyco.dat", fstream::in);
	fin.seekg(0, ios_base::end);
	if(fin.tellg()==0)
	{		
		tempoCommand.str("");
		tempoCommand.clear();
		tempoCommand<<"echo "<<setprecision(12)<<MJD1<<" "<<setprecision(12)<<MJD2<<"| tempo -z> /dev/null";
		system(tempoCommand.str().c_str());   
	}  **/
}


/*******************************************************************
*FUNCTION: void Information::loadPolyco()
*Loads relevent parameters from polyco.dat file to an array
*Refer to tempo manual for format of polyco.dat
*******************************************************************/
/*Polyco array structure
1 row => 1 set
polyco[row][0] => MJD (line 1)
           [1] => RPHASE (line 2)
           [2] => Ref rotation frequency (F0) (line 2)
           [>3] => polycos (3 per line from line 3)*/
void Information::loadPolyco()
{
	string line;    
	ifstream *polyco;
	if(!doUseTempo2)
  		polyco=new ifstream("polyco.dat",ios::in);
	else
		polyco=new ifstream("polyco_new.dat",ios::in);

    	int numberOfSets=ceil(72*60.0/spanPolyco); //3 MJD corresponds to 72 hours
    	polycoTable=new double[(3+nCoeffPolyco)*numberOfSets];
    	
    	double *ptrPolycoTable=polycoTable;
    	//Calculates the number of coefficient lines. Each line in polyco.dat contains upto 3 coefficients
    	int nCoeffLines=nCoeffPolyco/3;
    	if(nCoeffPolyco%3!=0)
    		nCoeffLines++;
    	
    	for(int i=0;i<numberOfSets;i++) 
    	{
        	getline(*polyco,line);        	
            	//Check the line and get information to load into polycos
            	sscanf(line.c_str()," %*s %*s %*s %lf",ptrPolycoTable++);	//MJD at the middle of current span
            	getline(*polyco,line);  
              	sscanf(line.c_str()," %lf %lf",(ptrPolycoTable),(ptrPolycoTable+1)); //RPHASE & REF ROTATION FREQ for the current span
		ptrPolycoTable+=2;
		//loading coefficients
              	int coeffLoaded=0;              	
                for(int j=1;j<=nCoeffLines;j++)
            	{
            		getline(*polyco,line);  
			string l = line.c_str();
  			std::replace(l.begin(), l.end(), 'D', 'E'); // replace all 'D' to 'E'          		
              		if(nCoeffPolyco - coeffLoaded == 1)
              		{
                  		sscanf(l.c_str()," %le",ptrPolycoTable);
                  		coeffLoaded++;
                  		ptrPolycoTable++;
              		}
              		else if(nCoeffPolyco - coeffLoaded == 2)
              		{
				
                  		sscanf(l.c_str()," %le %le",ptrPolycoTable,ptrPolycoTable+1);
                  		ptrPolycoTable+=2;
                  		coeffLoaded+=2;
              		}
              		else
              		{
                 		sscanf(l.c_str()," %le %le %le ", ptrPolycoTable,ptrPolycoTable+1,ptrPolycoTable+2);
                  		ptrPolycoTable+=3;
                  		coeffLoaded += 3;
              		}

      		}
            
    	}    
    	polyco->close(); 
	delete polyco;
}

/*******************************************************************
*FUNCTION: void Information::display()
*Displays all run parameters to the terminal
*******************************************************************/
void Information::display()
{
	
	stringstream displays;
	displays<<endl<<"gptool ver 4.4.5"<<endl;
	if(doFilteringOnly)
		displays<<"FILTERING ONLY MODE"<<endl<<endl;
	if(isInline)
		displays<<"INLINE MODE - read from and write to a shared memory"<<endl;
	if(doFRB)
		displays<<"INLINE MODE for FRB pipeline - read from and write to a shared memory"<<endl;
	displays<<"Mode of operation: "<<modeOperation<<endl;
	if(doPolarMode)
	{
		displays<<"Polarization mode ON"<<endl;
		if(polarChanToDisplay==-1)
			displays<<"All polarizations will be displayed"<<endl;
		else
			displays<<"Only polarization chan "<<int(polarChanToDisplay)<<" will be displayed"<<endl;
	}
	switch(sampleSizeBytes)
	{
		case 1:
			displays<<"Input data is sampled as 1 byte integers"<<endl;
			break;
		case 2:
			displays<<"Input data is sampled as 2 byte integers"<<endl;
			break;
		case 4:
			displays<<"Input data is sampled as 4 byte floating point numbers."<<endl;
			break;
	}
	
	displays<<endl<<"Lowest frequency :"<<lowestFrequency<<" MHz"<<endl;
	displays<<"Bandwidth: "<<bandwidth<<" MHz"<<endl;
	
	if(sidebandFlag)
		displays<<"Sideband flag: 1. Frequency of first channel is "<<lowestFrequency<<" MHz"<<endl;
	else
		displays<<"Sideband flag: -1. Frequency of first channel is "<<lowestFrequency+bandwidth<<" MHz"<<endl;
	displays<<"Number of channels: "<<noOfChannels<<endl;	
	displays<<"Sampling interval :"<<samplingInterval*1000.0<<" ms"<<endl;

	if(!doFilteringOnly)
	{
	
		displays<<endl<<"Pulsar: "<<pulsarName<<endl;
		
		displays<<"Pulsar period: "<<setprecision(12)<<periodInMs<<" ms"<<endl;
		displays<<"Dispersion measure: "<<setprecision(10)<<dispersionMeasure<<" pc/cc"<<endl;	
	
	
	
		/*if(refFrequency)
			cout<<endl<<"Dedispersion will be done with respect to the highest frequency."<<endl;
		else
			cout<<"Dedispersion will be done with respect to the lowest frequency."<<endl; */
		displays<<"Number of bins in folded profile: "<<periodInSamples<<endl;
		displays<<"Phase of pulsar profile will be offset by "<<profileOffset<<endl;
		if(!doFixedPeriodFolding)
		{
			if(doUseTempo2)
				displays<<"Polyco based folding to be performed using tempo2."<<endl;	
			else
				displays<<"Polyco based folding to be performed using tempo1."<<endl;	
			displays<<"MJD of observation is "<<setprecision(20)<<MJDObs<<endl;
			displays<<"Number of coefficients generated for each span: "<<nCoeffPolyco<<endl;
			displays<<"Validity of each span: "<<spanPolyco<<" mins"<<endl;
			displays<<"Maximum Hour Angle: "<<maxHA<<endl;
		}
		else
			displays<<"Fixed period folding to be performed."<<endl;
	}
	displays<<endl;
	if(isInline or doFRB)
		displays<<"Display window size has been adjusted to accomodate integer multiples of shared memory buffer"<<endl;
	
	displays<<"Display window size :"<<blockSizeSec<<" sec"<<endl;
	if(doManualMode && doReadFromFile)
		displays<<"Window update will happen on key press."<<endl;
	if(doWindowDelay && doReadFromFile)
		displays<<"gptool will spend atleast "<<blockSizeSec<<" seconds on each window."<<endl;

	displays<<endl<<"Number of channels excluded at start: "<<startChannel<<endl;
	displays<<"Number of channels excluded at end: "<<noOfChannels-stopChannel<<endl;
	if(nBadChanBlocks!=0)
	{
		displays<<endl<<"Number of bad subbands to exclude: "<<nBadChanBlocks<<endl;
		for(int i=0;i<nBadChanBlocks;i++)				
			displays<<"Sub-band "<<i+1<<" : chan # "<<badChanBlocks[i*2]<<" to "<<badChanBlocks[i*2+1]<<endl;		
		
	}
	
	
	if(doChanFlag)
	{
		displays<<endl<<"Channel flagging turned on. Details of procedure: "<<endl;
		switch(bandshapeToUse)
		{
			case 1:
				displays<<"\tMean bandshape will be used to detect RFI."<<endl;
				break;
			case 2:
				displays<<"\tBandhshape will be smoothened and then normalized.\n\tNormalized bandshape will be used to detect RFI."<<endl;
				if(normalizationProcedure==1)
					displays<<"\t"<<smoothingWindowLength<<" channels will be used as window length for smoothing"<<endl;
				else
					displays<<"\t"<<"Data from bandshape.dat will be used to normalize"<<endl;
				break;
			case 3:
				displays<<"\tMean to rms bandshape will be used to detect RFI."<<endl;
				break;
			case 4:
				displays<<"\tMean to rms bandshape as well as normalized bandshape will be used to detect RFI."<<endl;
				break;
		}
		/*
		if(chanFlagAlgo==1)
			cout<<"\tHistogram based algorithm selected."<<endl;		
		else if(chanFlagAlgo==2)
			cout<<"\tMAD based algorithm selected."<<endl;	
		*/	
		displays<<"\tCutOff to RMS ratio: "<<chanCutOffToRMS<<endl;				
	}
	else
	{
		displays<<endl<<"No channel flagging will be performed."<<endl;
	}
	if(doTimeFlag)
	{
		displays<<endl<<"Time flagging turned on. Details of procedure: "<<endl;	
		if(doUseNormalizedData)
		{
			displays<<"\t2-D time-frequency data will be normalized using smoothened bandshape before filtering"<<endl;	
			if(normalizationProcedure==1)
				displays<<"\t"<<smoothingWindowLength<<" channels will be used as window length for smoothing"<<endl;
			else
				displays<<"\t"<<"Data from bandshape.dat will be used to normalize"<<endl;
		}
		if(timeFlagAlgo==1)
			displays<<"\tHistogram based algorithm selected."<<endl;
		else if(timeFlagAlgo==2)
			displays<<"\tMAD based algorithm selected."<<endl;
		
		if(doMultiPointFilter)
		{
			displays<<"\tMultipoint flagging ON. CutOff to rms : "<<endl;
			for(int j=0;j<3;j++)
				cout<<"\t"<<cutoff[j]<<" ";
			displays<<endl;
		}
		else
			displays<<"\tCutOff to RMS ratio: "<<timeCutOffToRMS<<endl; 
		displays<<endl<<"Filtered data will have a rescaled mean of "<<meanval<<endl;
		if(smoothFlagWindowLength>0)
		{
			displays<<"\tAny time block of length "<<smoothFlagWindowLength<<" seconds will be rejected if it has more than "<<concentrationThreshold<<" % of corrupted data"<<endl;
		}				
	}
	else
	{
		displays<<endl<<"No time flagging will be performed."<<endl;
	}

	if(doTimeFlag && doChanFlag)
	{	
		if(doReplaceByMean)
		{
			if(timeFlagAlgo==1)
				displays<<endl<<"Flagged samples will be replaced by modal value of zero DM time series.";
			else if(timeFlagAlgo==2)
				displays<<endl<<"Flagged samples will be replaced by median value of zero DM time series.";

		}
		else
			displays<<endl<<"Flagged samples will be ignored"<<endl;
	}
	if(doChanFlag && doWriteChanFlags)
		displays<<"Flag output to chanflag.gpt"<<endl;	
	if(doTimeFlag && doWriteTimeFlags)
		displays<<"Flag output to timeflag.gpt"<<endl;	
	if(doZeroDMSub==1)
		displays<<"Appropiately scaled version of the zero DM time series will be subtracted to minimize the rms of each spectral channel."<<endl;	
	if(doWriteFiltered2D)
	{
			displays<<endl<<"Filtered 2D raw data output to "<<outputfilepath<<".gpt"<<endl;

	}
	/*if(doTimeFlag && doChanFlag)
	{
		if(flagOrder==1)
			cout<<endl<<"Channel filtering will be done first."<<endl;
		else if(flagOrder==2)
			cout<<"Time filtering will be done first."<<endl;
		else if(flagOrder==3)
			cout<<"Time & channel filtering will be done independent of each other."<<endl;
		cout<<endl;
	}*/
	
	
	if(doWriteFullDM)
		displays<<"Dedispersed time series will be written out"<<endl<<endl;

	if(doRunFilteredMode)
		displays<<endl<<"WARNING : gptool run in readback mode. Inputs in lines 36,41,42,48 & 54 were overridden."<<endl<<endl;
	if(!doReadFromFile)
		displays<<"Taking data from shared memory"<<endl;
	else
		displays<<"Raw file path: "<<filepath<<endl;
	cout<<displays.str().c_str();
	time_t now = time(0);   
  	 //convert now to string form
   	char* dt = ctime(&now);
	ofstream f("log.gpt",ios::app);
	f<<"Time of run: "<<dt<<endl;
	f<<displays.str().c_str()<<endl<<endl;
	f<<"--------------------------------------------"<<endl;
	
}
/*******************************************************************
*FUNCTION: void Information::e()
*Displays error message when no option is given while running wpmon.
*******************************************************************/
void Information::displayNoOptionsHelp()
{
	cout<<"gptool -f [filename] -r -shmID [shm_ID] -s [start_time_in_sec] -o [output_2d_filtered_file] -m [mean_value_of_2d_op] -tempo2 -nodedisp  -zsub -inline -gfilt"<<endl<<endl;
	cout<<"-f [filename] \t\t\t :Read from GMRT format file [filename]"<<endl;
	cout<<"-r  \t\t\t\t :Attach to shared memory"<<endl;
	cout<<"-shmID [shm_ID] \t\t :shm_ID = \t1 -> Standard correlator shm \n\t\t\t\t\t\t2-> File simulator shm \n\t\t\t\t\t\t3-> Inline gptool shm"<<endl; 
	cout<<"-s [start_time_in_sec] \t\t :start processing the file after skipping some time"<<endl;
	cout<<"-o [output_2d_filtered_file] \t :path to GMRT format filtered output file"<<endl;
	cout<<"-m [mean_value_of_2d_op] \t :mean value of output filtered file \n \t\t\t\t this is the value by which the gptool normalized data \n\t\t\t\t is scaled by before writing out filtered file"<<endl;
	cout<<"-tempo2 \t\t\t :use tempo2 instead of tempo1 to get polycos"<<endl;
	cout<<"-nodedisp \t\t\t :just perform RFI filtering without further processing"<<endl;
	cout<<"-zsub \t\t\t\t :optimized zero DM subtraction. \n\t\t\t\t Experimental feature, use with caution"<<endl;
	cout<<"-inline \t\t\t :inline mode of gptool, read from a shared memory \n\t\t\t\t write filtered output to another shared memory"<<endl;
	cout<<"-gfilt \t\t\t\t :turns off all filtering options, \n\t\t\t\t will over-ride gptool.in inputs"<<endl;
	
}

/*******************************************************************
*FUNCTION: void Information::generateInfFile()
*Writes a fullDM_filtered.inf to be used by presto to process the
*dedispersed series generated
*******************************************************************/
void Information::writeInfFile()
{
	//Getting RA and DEC from atnf psrcat
	string RA;
	string DEC;
	if(psrcatdbPath!=NULL)
	{
		stringstream RACommand;
	    	RACommand<<"psrcat -db_file "<<psrcatdbPath<<" -e "<<pulsarName<<" | grep RAJ | awk '{print $2}' > RAJ";
	    	system(RACommand.str().c_str());
	    	ifstream RAJ("RAJ",ios::in);    	
	    	RAJ>>RA;    	
	    	RAJ.close();
	    	system("rm -rf RAJ");
	
		stringstream DECCommand;
	    	DECCommand<<"psrcat -db_file "<<psrcatdbPath<<" -e "<<pulsarName<<" | grep DECJ | awk '{print $2}' > DECJ";
	    	system(DECCommand.str().c_str());
	    	ifstream DECJ("DECJ",ios::in);    	
	    	DECJ>>DEC;    	
	    	DECJ.close();
	    	system("rm -rf DECJ");
	}
	else
	{
		RA="";
		DEC="";
	}
	stringstream fileOutput;
	fileOutput<<"Telescope used\t\t\t\t= "<<"GMRT"<<endl;
 	fileOutput<<"Instrument used\t\t\t\t= "<<"GWB"<<endl;
	fileOutput<<"Object being observed\t\t\t= "<<&pulsarName[1]<<endl;
	fileOutput<<"J2000 Right Ascension (hh:mm:ss.ssss)\t= "<<RA<<endl;
	fileOutput<<"J2000 Declination     (dd:mm:ss.ssss)\t= "<<DEC<<endl;
	fileOutput<<"Data observed by\t\t\t= "<<"GMRT"<<endl;
 	fileOutput<<"Epoch of observation (MJD)\t\t= "<<setprecision(12)<<MJDObs<<endl;
	fileOutput<<"Barycentered?           (1=yes, 0=no)\t= "<<"0"<<endl;
	fileOutput<<"Number of bins in the time series\t= "<<endl;
	fileOutput<<"Width of each time series bin (sec)\t= "<<setprecision(12)<<samplingInterval<<endl;  
 	fileOutput<<"Any breaks in the data? (1=yes, 0=no)\t= "<<"0"<<endl;
	fileOutput<<"Type of observation (EM band)\t\t= "<<"Radio"<<endl;
	fileOutput<<"Beam diameter (arcsec)\t\t\t= "<<endl;
 	fileOutput<<"Dispersion measure (cm-3 pc)\t\t= "<<dispersionMeasure<<endl;
	fileOutput<<"Central freq of low channel (Mhz)\t= "<<lowestFrequency<<endl; 
 	fileOutput<<"Total bandwidth (Mhz)\t\t\t= "<<bandwidth<<endl;
 	fileOutput<<"Number of channels\t\t\t= "<<1<<endl;
	fileOutput<<"Channel bandwidth (Mhz)\t\t\t= "<<bandwidth/noOfChannels<<endl;
	fileOutput<<"Data analyzed by\t\t\t= "<<"gptool"<<endl;

	ofstream inFilteredFile("fullDM_filtered.inf",ios::out);
	inFilteredFile<<"Data file name without suffix\t\t= "<<"fullDM_filtered"<<endl;
	inFilteredFile<<fileOutput.str().c_str();
	inFilteredFile.close();

	ofstream inUnFilteredFile("fullDM_unfiltered.inf",ios::out);
	inUnFilteredFile<<"Data file name without suffix\t\t= "<<"fullDM_unfiltered"<<endl;
	inUnFilteredFile<<fileOutput.str().c_str();
	inUnFilteredFile.close();
}
/*******************************************************************
*FUNCTION: void Information::writeWpmonIn()
*Writes a sample gptool.in when it is not found.
*******************************************************************/
void Information::writeWpmonIn()
{
	ofstream inFile("gptool.in",ios::out);
	inFile<<"#*#*#gptool input file v2.0#*#*#"<<endl;
	inFile<<"-------------------------------------------------"<<endl;
	inFile<<"#****Mode of observation****#"<<endl;
	inFile<<"PA\t\t: Beam mode"<<endl;
	inFile<<"0\t\t: Polarization mode (0-> intesity, 1-> stokes data)"<<endl;
	inFile<<"2\t\t: Sample size of data (in bytes, usually 2)"<<endl;
	inFile<<"-------------------------------------------------"<<endl;
	inFile<<"#****Observation Paramaters****#"<<endl;
	inFile<<"1030\t\t: Frequency band (lowest value in Mhz)"<<endl;
	inFile<<"200\t\t: Bandwidth(in Mhz)"<<endl;
	inFile<<"-1\t\t: Sideband flag (-1-> decreasing +1-> increasing)"<<endl;
	inFile<<"2048\t\t: Number of channels"<<endl;
	inFile<<"1.31072\t\t: Sampling Interval (in ms)"<<endl;
	inFile<<"-------------------------------------------------"<<endl;
	inFile<<"#****Pulsar Parameters****#"<<endl;
	inFile<<"J1807-0847\t: Pulsar name"<<endl;
	inFile<<"-1\t\t: Pulsar period (in milliseconds)"<<endl;
	inFile<<"-1\t\t: DM (in pc/cc)"<<endl;
	inFile<<"-------------------------------------------------"<<endl;
	inFile<<"#****Dedispersion & Folding parameters****#"<<endl;
	inFile<<"-1\t\t: Number of bins in folded profile (-1 for native resolution)"<<endl;
	inFile<<"0\t\t: Phase offset for folding"<<endl;
	inFile<<"12\t\t: Number of coefficients for each polyco span (nCoeff)"<<endl;
	inFile<<"60\t\t: Validity of each span (nSpan in mins)"<<endl;
	inFile<<"12\t\t: Maximum hour angle"<<endl;
	inFile<<"-------------------------------------------------"<<endl;
	inFile<<"#****Display Parameters****#"<<endl;
	inFile<<"0\t\t: Polarization channel to display (0-3 or -1 for all four)"<<endl;
	inFile<<"1\t\t: Display window size (seconds, 0-> pulsar period)"<<endl;
	inFile<<"0\t\t: Update mode	    	(0-> automatic, 1-> manual)"<<endl;
	inFile<<"0\t\t: Time delay between window updates (0-> no delay, 1-> emulate real time)"<<endl;
	inFile<<"-------------------------------------------------"<<endl;
	inFile<<"#****Spectral line RFI mitigation options****#"<<endl;
	inFile<<"50\t\t: Number of channels to flag at band beginning"<<endl;
	inFile<<"50\t\t: Number of channels to flag at band end"<<endl;
	inFile<<"1\t\t: Frequency flagging options (0-> no flagging, 1-> real time calculation)"<<endl;
	inFile<<"1\t\t: Bandshape to use for frequency flagging (1-> normalized bandshape, 2-> mean-to-rms bandshape, 3-> Both)"<<endl;
	inFile<<"2.5\t\t: Threshold for frequency flagging (in units of RMS deviation)"<<endl;				
	inFile<<"-------------------------------------------------"<<endl;
	inFile<<"#****Time domain impulsive RFI mitigation options****#"<<endl;
	inFile<<"1\t\t: Time flagging options 	(0-> no flagging, 1-> real time calculation)"<<endl;
	inFile<<"1\t\t: Data normalization before filtering (0-> no, 1-> yes)"<<endl;
	inFile<<"1\t\t: Time flagging algorithm	(1-> histogram based, 2-> MAD based)"<<endl;
	inFile<<"3\t\t: Threshold for time flagging (in units of RMS deviation)"<<endl;
	inFile<<"-------------------------------------------------"<<endl;
	inFile<<"#****Other options****#"<<endl;
	inFile<<"20\t\t: Smoothing window size for bandshape normalization (in number of channels)"<<endl;
	inFile<<"1\t\t: Normalization procedure (1-> cumulative smooth bandshape, 2-> externally supplied bandshape.dat)"<<endl;
	inFile<<"0\t\t: Replace by median values (0-> Ignore flagged samples, 1-> Replace flagged samples by window median, 2-> Replace by smooth bandshape)"<<endl;
	inFile<<"-------------------------------------------------"<<endl;
	inFile<<"#****I/O options****#"<<endl;
	inFile<<"0\t\t: Write channel flag file (0-> no,1-> yes)"<<endl;	
	inFile<<"0\t\t: Write time flag file (0-> no, 1-> yes)"<<endl;
	inFile<<"0\t\t: write out filtered 2D raw data (0-> no, 1-> yes)"<<endl;
	inFile<<"0\t\t: Write out fullDM.raw	(0-> no, 1-> yes)"<<endl;
	inFile<<"-------------------------------------------------"<<endl;
	inFile<<"#****manual flagging options****#"<<endl;
	inFile<<"0\t\t: Number of bad channel blocks"<<endl;
	inFile<<"List\t\t: #in next line, example: [200,400],[1200,1400]"<<endl;
	inFile<<""<<endl;
}
//end of information class methods implementations
